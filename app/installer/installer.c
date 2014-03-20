/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 * Maintainer:  Long Wei <yilang2007lw@gamil.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include "dwebview.h"
#include "i18n.h"
#include "fs_util.h"
#include "part_util.h"
#include "misc.h"

#define INSTALLER_HTML_PATH     "file://"RESOURCE_DIR"/installer/index.html"
#define INSTALLER_WIN_WIDTH     786
#define INSTALLER_WIN_HEIGHT    576

static GtkWidget *installer_container = NULL;
char **global_argv = NULL;
static int server_sockfd;
gchar *extract_mode = NULL;

static GOptionEntry entries[] = 
{
    { "mode", 'm', 0, G_OPTION_ARG_STRING, &extract_mode, "fast or safe"},
    { NULL }
};

gboolean installer_is_running ()
{
    //int server_sockfd;
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0'; //make it be an name unix socket
    int path_size = g_sprintf (server_addr.sun_path+1, "%s", "installer.app.deepin");
    server_addr.sun_family = AF_UNIX;
    server_len = 1 + path_size + offsetof(struct sockaddr_un, sun_path);

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (0 == bind(server_sockfd, (struct sockaddr *)&server_addr, server_len)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static gboolean
move_window (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    g_debug ("installer:move window");

    if (event->y > 50 || (event->x > 740) && (event->y < 50)) {
        return TRUE;
    }

    if (event->button == 1) {
        g_debug ("move window:in drag x_root->%g, y_root->%g", event->x_root, event->y_root);

        gtk_widget_set_can_focus (widget, TRUE);
        gtk_widget_grab_focus (widget);

        gtk_window_begin_move_drag (GTK_WINDOW (widget), 
                                    event->button, 
                                    event->x_root,
                                    event->y_root,
                                    event->time);

    }

    return FALSE;
}

static gboolean 
expose_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    return FALSE;
}

static void
move_window_center ()
{
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (installer_container));
    gint s_width = gdk_screen_get_width (screen);
    gint s_height = gdk_screen_get_height (screen);
    gint x = s_width > 1250 ? ((s_width - (750)) / 2) : 0;
    gint y = s_height > 540 ? ((s_height - 540) / 2) : 0;
    
    gtk_window_move (GTK_WINDOW (installer_container), x, y);
}

JS_EXPORT_API
void installer_finish_install ()
{
    finish_install_cleanup ();
    gtk_main_quit ();
}

JS_EXPORT_API
void installer_finish_reboot ()
{
    finish_install_cleanup ();
    //installer_reboot ();
    g_spawn_command_line_async ("reboot", NULL);
    gtk_main_quit ();
}

JS_EXPORT_API
void installer_emit_webview_ok ()
{
    static gboolean inited = FALSE;
    if (!inited) {
        inited = TRUE;
        init_parted ();
        //inhibit_disk ();
    }
}

static void
sigterm_cb (int sig)
{
    installer_finish_install ();
}

JS_EXPORT_API
void installer_restart_installer ()
{
    close (server_sockfd);
    execv (global_argv[0], global_argv);
}

int main(int argc, char **argv)
{
    GOptionContext *context = g_option_context_new ("- Deepin Installer");
    g_option_context_add_main_entries (context, entries, "INSTALLER");
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (context, &argc, &argv, NULL)) {
        g_warning ("context parse failed\n");
    }
    g_option_context_free (context);

    global_argv = argv;
    gtk_init (&argc, &argv);

    setlocale(LC_MESSAGES, "");
    textdomain("INSTALLER");

    if (geteuid () != 0) {
        g_warning ("must run installer as root\n");
        exit (0);
    }

    if (installer_is_running ()) {
        g_warning ("another instance of installer is running\n");
        exit (0);
    }

    signal (SIGTERM, sigterm_cb);
    signal (SIGINT, sigterm_cb);
    signal (SIGQUIT, sigterm_cb);
    signal (SIGKILL, sigterm_cb);
    signal (SIGTSTP, sigterm_cb);

    installer_container = create_web_container (FALSE, TRUE);
    g_signal_connect (installer_container, "button-press-event", G_CALLBACK (move_window), NULL);

    gtk_window_set_decorated (GTK_WINDOW (installer_container), FALSE);
    //gtk_window_set_skip_taskbar_hint (GTK_WINDOW (installer_container), TRUE);
    //gtk_window_set_skip_pager_hint (GTK_WINDOW (installer_container), TRUE);
    GtkWidget *webview = d_webview_new_with_uri (INSTALLER_HTML_PATH);
    g_signal_connect (webview, "draw", G_CALLBACK (expose_cb), NULL);

    gtk_container_add (GTK_CONTAINER (installer_container), GTK_WIDGET (webview));
    gtk_window_set_default_size (GTK_WINDOW (installer_container), INSTALLER_WIN_WIDTH, INSTALLER_WIN_HEIGHT);
    gtk_window_set_resizable (GTK_WINDOW (installer_container), FALSE);
    gtk_window_set_position (GTK_WINDOW (installer_container), GTK_WIN_POS_CENTER);

    GdkGeometry geometry;
    geometry.min_width = INSTALLER_WIN_WIDTH;
    geometry.max_width = INSTALLER_WIN_WIDTH;
    geometry.base_width = INSTALLER_WIN_WIDTH;
    geometry.min_height = INSTALLER_WIN_HEIGHT;
    geometry.max_height = INSTALLER_WIN_HEIGHT;
    geometry.base_height = INSTALLER_WIN_HEIGHT;
    gtk_window_set_geometry_hints (GTK_WINDOW (installer_container), webview, &geometry, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE);
    //move_window_center ();

    gtk_widget_realize (installer_container);
    gtk_widget_show_all (installer_container);

    monitor_resource_file ("installer", webview);
    gtk_main ();

    return 0;
}

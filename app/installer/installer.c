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
#include <signal.h>
#include "dwebview.h"
#include "dcore.h"
#include "i18n.h"
#include "fs_util.h"
#include "part_util.h"

#define INSTALLER_HTML_PATH     "file://"RESOURCE_DIR"/installer/index.html"
#define INSTALLER_WIN_WIDTH     786
#define INSTALLER_WIN_HEIGHT    576

static GtkWidget *installer_container = NULL;
char **global_argv = NULL;

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

JS_EXPORT_API
void installer_finish_install ()
{
    gtk_main_quit ();
}

JS_EXPORT_API
void installer_finish_reboot ()
{
    g_spawn_command_line_async ("reboot", NULL);
    gtk_main_quit ();
}

JS_EXPORT_API
void installer_restart_installer ()
{
    extern int server_sockfd;
    close (server_sockfd);
    execv (global_argv[0], global_argv);
}

JS_EXPORT_API
void installer_emit_webview_ok ()
{
    static gboolean inited = FALSE;
    if (!inited) {
        inited = TRUE;
        init_parted ();
    }
}

static void
sigterm_cb (int sig)
{
    installer_finish_install ();
}

int main(int argc, char **argv)
{
    gtk_init (&argc, &argv);
    g_log_set_default_handler((GLogFunc)log_to_file, "deepin-installer");

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
    GtkWidget *webview = d_webview_new_with_uri (INSTALLER_HTML_PATH);
    g_signal_connect (webview, "draw", G_CALLBACK (erase_background), NULL);

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

    gtk_widget_realize (installer_container);
    gtk_widget_show_all (installer_container);
/*#ifndef NDEBUG*/
    /*monitor_resource_file("installer", webview);*/
/*#endif*/
    gtk_main ();
    
    return 0;
}

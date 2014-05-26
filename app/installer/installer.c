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
#include "i18n.h"
#include "fs_util.h"
#include "part_util.h"
#include "misc.h"
#include "wubi.h"

#define INSTALLER_HTML_PATH     "file://"RESOURCE_DIR"/installer/index.html"
#define INSTALLER_WIN_WIDTH     786
#define INSTALLER_WIN_HEIGHT    576

static GtkWidget *installer_container = NULL;
char **global_argv = NULL;
gchar *xrandr_size = NULL;

gchar *opt_extract_mode;
gint opt_use_processors;
gchar *opt_iso_path;
gboolean opt_automatic;
gchar *opt_target;
gchar *opt_home;
gchar *opt_username;
gchar *opt_hostname;
gchar *opt_password;
gchar *opt_layout;
gchar *opt_variant;
gchar *opt_timezone;
gchar *opt_locale;
gchar *opt_grub;
gboolean opt_debug;

static GOptionEntry entries[] = 
{
    { "schema", 's', 0, G_OPTION_ARG_STRING, &opt_extract_mode, "fast use unsquashfs, safe copy file one by one", "fast or safe"},
    { "cpu", 'c', 0, G_OPTION_ARG_INT, &opt_use_processors, "num of processors used in unsquashfs mode", "count"},
    { "iso", 'i', 0, G_OPTION_ARG_STRING, &opt_iso_path, "set iso file path when install without live os", "path"},
    { "automatic", 'a', 0, G_OPTION_ARG_NONE, &opt_automatic, "gather info from command line then install automatic", NULL},
    { "target", 't', 0, G_OPTION_ARG_STRING, &opt_target, "device to install system, required when automatic", "/dev/sdaX"},
    { "home", 'm', 0, G_OPTION_ARG_STRING, &opt_home, "partition mount as home in target system, only work when automatic", "/dev/sdaX"},
    { "username", 'u', 0, G_OPTION_ARG_STRING, &opt_username, "username of target system, required when automatic", "deepin"}, 
    { "hostname", 'n', 0, G_OPTION_ARG_STRING, &opt_hostname, "hostname of target system, only work when automatic", "hostname"},
    { "password", 'p', 0, G_OPTION_ARG_STRING, &opt_password, "password of target system, required when automatic", "password"},
    { "layout", 'l', 0, G_OPTION_ARG_STRING, &opt_layout, "keyboard layout of target system, only work when automatic", "layout code"},
    { "variant", 'v', 0, G_OPTION_ARG_STRING, &opt_variant, "keyboard variant of target system, only work when automatic", "variant code"},
    { "zone", 'z', 0, G_OPTION_ARG_STRING, &opt_timezone, "timezone of target system, only work when automatic", "Asia/Shanghai"},
    { "locale", 'e', 0, G_OPTION_ARG_STRING, &opt_locale, "locale of target system, only work when automatic", "zh_CN.UTF-8"},
    { "grub", 'g', 0, G_OPTION_ARG_STRING, &opt_grub, "device path or uefi to install bootloader, only work when automatic", "/dev/sdaX or uefi"},
    { "debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug, "set log level to debug", NULL},
    { NULL }
};

JS_EXPORT_API
gboolean installer_is_installation_auto ()
{
    if (!opt_automatic) {
        return FALSE;
    }
    if (opt_target == NULL) {
        g_warning ("is installation auto:must specified target\n");
        return FALSE;
    }
    if (opt_username == NULL) {
        g_warning ("is installation auto:must specified username\n");
        return FALSE;
    }
    if (opt_password == NULL) {
        g_warning ("is installation auto:must specified  password\n");
        return FALSE;
    }
    return TRUE;
}

JS_EXPORT_API
JSObjectRef installer_get_installation_info ()
{
    GRAB_CTX ();
    JSObjectRef json = json_create ();
    json_append_string (json, "target", opt_target);
    json_append_string (json, "home", opt_home);
    json_append_string (json, "username", opt_username);
    json_append_string (json, "hostname", opt_hostname);
    json_append_string (json, "password", opt_password);
    json_append_string (json, "layout", opt_layout);
    json_append_string (json, "variant", opt_variant);
    json_append_string (json, "timezone", opt_timezone);
    json_append_string (json, "locale", opt_locale);
    json_append_string (json, "grub", opt_grub);
    UNGRAB_CTX ();
    return json;
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
        xrandr_size = get_xrandr_size ();
        init_parted ();
        if (is_use_wubi ()) {
            g_debug ("emit webview ok:use wubi\n");
            opt_automatic = TRUE;
            sync_wubi_config ();
        }
    }
}

static void
sigterm_cb (int sig)
{
    installer_finish_install ();
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

    gtk_widget_realize (installer_container);
    gtk_widget_show_all (installer_container);
    //monitor_resource_file ("installer", webview);
    gtk_main ();
    
    return 0;
}

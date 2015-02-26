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
#include <glib.h>
#include <fcntl.h>
#include "dwebview.h"
#include "dcore.h"
#include "i18n.h"
#include "fs_util.h"
#include "part_util.h"
#include "background.h"

#define INSTALLER_HTML_PATH     "file://"RESOURCE_DIR"/installer/index.html"

guint INSTALLER_WIN_WIDTH = 786;
guint INSTALLER_WIN_HEIGHT = 576;

static GtkWidget *installer_container = NULL;
char **global_argv = NULL;

char* auto_conf_path = NULL;
char* log_path = NULL;
gboolean nowm = FALSE;
static GOptionEntry entries[] =
{
    { "conf", 'c', 0, G_OPTION_ARG_STRING, &auto_conf_path, "set configure file path when installing with automate mode ", "path"},
    { "log", 'l', 0, G_OPTION_ARG_STRING, &log_path, "write log message to ", "log path"},
    { "without-wm", 'w', 0, G_OPTION_ARG_NONE, &nowm, "launch deepin-installer without window manager", NULL},
    { NULL }
};

/*
 * Move specific widget as window.
 */
static gboolean move_window(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    g_debug("installer:move window");
    if(event->y > 50 ||(event->x > 740) &&(event->y < 50)) {
        return TRUE;
    }
    if(event->button == 1) {
        g_debug("move window:in drag x_root->%g, y_root->%g", event->x_root, event->y_root);
        gtk_widget_set_can_focus(widget, TRUE);
        gtk_widget_grab_focus(widget);

        gtk_window_begin_move_drag(GTK_WINDOW(widget),
                                    event->button,
                                    event->x_root,
                                    event->y_root,
                                    event->time);
    }
    return FALSE;
}

/*
 * Whether installer is in debug mode.
 */
JS_EXPORT_API gboolean installer_is_debug()
{
#ifdef NDEBUG
    return FALSE;
#endif
    return TRUE;
}

/*
 * Js export api of utils.c: spawn_command_sync().
 */
JS_EXPORT_API void installer_spawn_command_sync(const char* cmd,gboolean sync )
{
    spawn_command_sync(cmd,sync);
}

/*
 * Simply quit application.
 */
JS_EXPORT_API void installer_finish_install()
{
    g_warning("installer_finish_install()");
    gtk_main_quit();
}

/*
 * Use `magic-key' to shutdown system.
 */
JS_EXPORT_API void installer_shutdown()
{
    g_warning("installer_shutdown()");
    GError* error=NULL;
    g_spawn_command_line_async("sh -c \"echo o > /proc/sysrq-trigger\"", &error);
    if(error != NULL){
        g_warning("use echo to write b into /proc/sysrq-trigger failed:%s",error->message);
        g_error_free(error);
        error = NULL;
    }
}

/*
 * Use `magic-key' to reboot system.
 */
JS_EXPORT_API void installer_finish_reboot()
{
    g_warning("installer_finish_reboot()");
    gtk_main_quit();
    GError* error=NULL;
    g_spawn_command_line_async("sh -c \"echo b > /proc/sysrq-trigger\"", &error);
    if(error != NULL){
        g_warning("use echo to write b into /proc/sysrq-trigger failed:%s",error->message);
        g_error_free(error);
        error = NULL;
    }
}

/*
 * Restart installer with same arguments.
 */
JS_EXPORT_API void installer_restart_installer()
{
    // NOTE: Don't know what is this.
    extern int server_sockfd;
    close(server_sockfd);

    execv(global_argv[0], global_argv);
}

/*
 * Seems like to emit some signals when ready.
 * NOTE: Not sure about this.
 */
JS_EXPORT_API void installer_emit_webview_ok()
{
    static gboolean inited = FALSE;
    if(!inited)
    {
        inited = TRUE;

        if(nowm)
        {
            js_post_signal("without_wm");
        }

        if(is_virtual_pc())
        {
            js_post_signal("is_virtual_machine");
        }

        if(auto_conf_path == NULL)
        {
            init_parted();
        }
        else
        {
            js_post_signal("auto_mode");
        }
    }
}

/*
 * Callback of some `kill' signals.
 */
static void sigterm_cb(int sig)
{
    installer_finish_install();
}

/*
 * Redirect all log to specific `path'.
 */
void redirect_log(const char* path)
{
    if(path == NULL) {
        path="/var/log/deepin-installer.log";
    }
    g_message("[%s]:deepin-installer.log path is:====%s=========",__func__ , path);
    int log_file = open(path, O_RDWR| O_CREAT| O_TRUNC, 0644);
    if(log_file == -1) {
        perror("redirect_log failed!");
        return ;
    }
    dup2(log_file, 1);
    dup2(log_file, 2);
}

/*
 * Seems like to fix some errors while launched without wm.
 * NOTE: Not sure about this.
 */
void fix_without_wm(GtkWidget* child)
{
    {
        // Fix wrong cursor.
        GdkCursor* cursor = gdk_cursor_new(GDK_LEFT_PTR);
        gdk_window_set_cursor(gdk_get_default_root_window(), cursor);
        g_object_unref(cursor);
    }

    {
        // Set installer to screen size.
        INSTALLER_WIN_WIDTH = gdk_screen_width();
        INSTALLER_WIN_HEIGHT = gdk_screen_height();
        gtk_window_move(GTK_WINDOW(installer_container), 0, 0);
        BackgroundInfo* bg_info = create_background_info(installer_container, child);
        background_info_set_background_by_file(bg_info, "/usr/share/backgrounds/default_background.jpg");
    }
}

/*
 * Entry function.
 */
int main(int argc, char **argv)
{
    if(argc == 2 && 0 == g_strcmp0(argv[1], "-d"))
    {
        g_setenv("G_MESSAGES_DEBUG", "all", FALSE);
    }

    {
        // Option context parser.
        GOptionContext *context = g_option_context_new("- Deepin Installer");
        g_option_context_add_main_entries(context, entries, "INSTALLER");
        g_option_context_add_group(context, gtk_get_option_group(TRUE));
        if(!g_option_context_parse(context, &argc, &argv, NULL))
        {
            g_warning("context parse failed\n");
        }
        if(auto_conf_path != NULL && !g_file_test(auto_conf_path, G_FILE_TEST_IS_REGULAR))
        {
            g_warning("the configure is valid: %s", auto_conf_path);
            exit(1);
        }
        g_option_context_free(context);
    }

    redirect_log(log_path);

    gtk_init(&argc, &argv);

    // TODO: Set to correct locale.
    setlocale(LC_MESSAGES, "");
    textdomain("INSTALLER");

    // TODO: Remove root priviledge requirement.
    if(geteuid() != 0)
    {
        g_warning("must run installer as root\n");
        exit(0);
    }

    if(installer_is_running())
    {
        g_warning("another instance of installer is running\n");
        exit(0);
    }

    // Catch all signals to make sure we quit properly.
    signal(SIGTERM, sigterm_cb);
    signal(SIGINT, sigterm_cb);
    signal(SIGQUIT, sigterm_cb);
    signal(SIGKILL, sigterm_cb);
    signal(SIGTSTP, sigterm_cb);

    // Create container widget for the webview.
    installer_container = create_web_container(FALSE, TRUE);
    gtk_window_set_decorated(GTK_WINDOW(installer_container), FALSE);
    GtkWidget *webview = d_webview_new_with_uri(INSTALLER_HTML_PATH);
    g_signal_connect(webview, "draw", G_CALLBACK(erase_background), NULL);
    gtk_container_add(GTK_CONTAINER(installer_container), GTK_WIDGET(webview));

    // Disable context-menu of the webview.
    WebKitWebSettings *setting = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
    g_object_set(G_OBJECT(setting), "enable-default-context-menu", FALSE, NULL);

    if(nowm)
    {
        fix_without_wm(webview);
    }
    else
    {
        g_signal_connect(installer_container, "button-press-event", G_CALLBACK(move_window), NULL);
        gtk_window_set_position(GTK_WINDOW(installer_container), GTK_WIN_POS_CENTER);
    }

    {
        // Set window size & position.
        gtk_window_set_default_size(GTK_WINDOW(installer_container), INSTALLER_WIN_WIDTH, INSTALLER_WIN_HEIGHT);
        gtk_window_set_resizable(GTK_WINDOW(installer_container), FALSE);
        GdkGeometry geometry;
        geometry.min_width = INSTALLER_WIN_WIDTH;
        geometry.max_width = INSTALLER_WIN_WIDTH;
        geometry.base_width = INSTALLER_WIN_WIDTH;
        geometry.min_height = INSTALLER_WIN_HEIGHT;
        geometry.max_height = INSTALLER_WIN_HEIGHT;
        geometry.base_height = INSTALLER_WIN_HEIGHT;
        gtk_window_set_geometry_hints(GTK_WINDOW(installer_container), webview, &geometry, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE);
    }

    gtk_widget_show_all(installer_container);

    gtk_main();

    return 0;
}

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
#include "jsextension.h"
#include "dwebview.h"
#include "i18n.h"
#include "utils.h"
#include "part_util.h"
#include "fs_util.h"
#include "misc.h"
#include "account.h"
#include "extract.h"
#include "keyboard.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>

#define INSTALLER_HTML_PATH "file://"RESOURCE_DIR"/installer/index.html"
#define PACKAGES_LIST_PATH RESOURCE_DIR"/installer/blacklist.ini"

extern int chroot(const char *path);
extern int fchdir(int fd);
extern int chdir(const char *path);

static GtkWidget *installer_container = NULL;

gboolean installer_is_running ()
{
    int server_sockfd;
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

    if (event->y > 90 || ((event->x > 600) && (event->y > 60) && (90 > event->y)) || (event->x > 725) && (event->y < 30)) {
        g_debug ("move window:html click area");
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

static void
adapt_location_for_help ()
{
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (installer_container));
    gint s_width = gdk_screen_get_width (screen);
    gint s_height = gdk_screen_get_height (screen);
    gint x = s_width > 1255 ? ((s_width - (755  + 500)) / 2) : 0;
    gint y = s_height > 540 ? ((s_height - 540) / 2) : 0;

    gtk_window_move (GTK_WINDOW (installer_container), x, y);
}

static void
move_window_center ()
{
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (installer_container));
    gint s_width = gdk_screen_get_width (screen);
    gint s_height = gdk_screen_get_height (screen);
    gint x = s_width > 1255 ? ((s_width - (755)) / 2) : 0;
    gint y = s_height > 540 ? ((s_height - 540) / 2) : 0;
    
    gtk_window_move (GTK_WINDOW (installer_container), x, y);
}

JS_EXPORT_API 
void installer_show_help ()
{
    GError *error = NULL;

    if (g_find_program_in_path ("installerhelp") == NULL) {
        g_warning ("installerhelp not found\n");
        return ;
    }

    g_spawn_command_line_async ("installerhelp", &error);
    if (error != NULL) {
        g_warning ("run installerhelp:%s\n", error->message);
        g_error_free (error);
    }
    adapt_location_for_help ();
}

JS_EXPORT_API 
void installer_hide_help ()
{
    GError *error = NULL;

    gchar *cmd = g_strdup ("pkill -9 installerhelp");
    g_spawn_command_line_async (cmd, &error);
    if (error != NULL) {
        g_warning ("hide installer help:%s\n", error->message);
        g_error_free (error);
    }
    g_free (cmd);
}

JS_EXPORT_API 
gboolean installer_is_help_running ()
{
    gboolean running = FALSE;
    GError *error = NULL;
    gchar *output = NULL;

    gchar *cmd = g_strdup ("sh -c \"ps aux |grep installerhelp\"");
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error); 
    g_free (cmd);
    if (error != NULL) {
        g_warning ("is help running:%s\n", error->message);
        g_error_free (error);
    }
    gchar **items = g_strsplit (output, "\n", -1);
    if (g_strv_length (items) > 3) {
        running = TRUE;
    }
    g_strfreev (items);
    g_free (output);

    return running;
}

//unmount after break chroot
static void 
unmount_target (const gchar *target)
{
    guint target_before = get_mount_target_count (target);
    if (target_before < 1) {
        return;
    }

    gchar *umount_sys_cmd = g_strdup_printf ("umount -l %s/sys", target);
    gchar *umount_proc_cmd = g_strdup_printf ("umount -l %s/proc", target);
    gchar *umount_devpts_cmd = g_strdup_printf ("umount -l %s/dev/pts", target);
    gchar *umount_dev_cmd = g_strdup_printf ("umount -l %s/dev", target);
    gchar *umount_target_cmd = g_strdup_printf ("umount -l %s", target);

    g_spawn_command_line_async (umount_sys_cmd, NULL);
    g_spawn_command_line_async (umount_proc_cmd, NULL);
    g_spawn_command_line_async (umount_devpts_cmd, NULL);
    g_spawn_command_line_async (umount_dev_cmd, NULL);
    g_spawn_command_line_async (umount_target_cmd, NULL);

    g_free (umount_sys_cmd);
    g_free (umount_proc_cmd);
    g_free (umount_devpts_cmd);
    g_free (umount_dev_cmd);
    g_free (umount_target_cmd);
}

static void
remove_packages ()
{
    extern gboolean in_chroot;
    if (!in_chroot) {
        g_warning ("remove packages:not in chroot\n");
        return ;
    }

    GError *error = NULL;
    gchar *cmd = NULL;
    gchar *contents = NULL;
    gchar **strarray = NULL;
    gchar *packages = NULL;

    if (!g_file_test (PACKAGES_LIST_PATH, G_FILE_TEST_EXISTS)) {
        g_warning ("remove packages:%s not exists\n", PACKAGES_LIST_PATH);
        goto out;
    }
    g_file_get_contents (PACKAGES_LIST_PATH, &contents, NULL, &error);
    if (error != NULL) {
        g_warning ("remove packages:get packages list %s\n", error->message);
        goto out;
    }
    if (contents == NULL) {
        g_warning ("remove packages:contents NULL\n");
        goto out;
    }
    strarray = g_strsplit (contents, "\n", -1);
    if (strarray == NULL) {
       g_warning ("remove packages:strarray NULL\n"); 
       goto out;
    }
    packages = g_strjoinv (" ", strarray);
    if (packages == NULL) {
        g_warning ("remove packages:packages NULL\n");
        goto out;
    }

    if (g_file_test ("/var/lib/apt/lock", G_FILE_TEST_EXISTS)) {
       g_unlink ("/var/lib/apt/lock"); 
    }
    
    cmd = g_strdup_printf ("apt-get remove -y %s", packages);
    g_spawn_command_line_async (cmd, &error);
    if (error != NULL) {
        g_warning ("remove packages:%s\n", error->message);
    }
    goto out;

out:
    g_free (cmd);
    g_free (contents);
    g_strfreev (strarray);
    g_free (packages);
    if (error != NULL) {
        g_error_free (error);
    }
}

static void
finish_install_cleanup () 
{
    remove_packages ();
    installer_hide_help ();
    ped_device_free_all ();

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("finish install:target is NULL\n");

    } else {
        extern gboolean in_chroot;
        if (in_chroot) {
            extern int chroot_fd;
            if (fchdir (chroot_fd) < 0) {
                g_warning ("finish install:reset to chroot fd dir failed\n");
            } else {
                int i = 0;
                for (i = 0; i < 1024; i++) {
                    chdir ("..");
                }
                chroot (".");
                unmount_target (target);
            }
        }
    }
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
        inhibit_disk ();
    }
}

int main(int argc, char **argv)
{
    init_i18n ();
    gtk_init (&argc, &argv);

    if (geteuid () != 0) {
        g_warning ("must run installer as root\n");
        exit (0);
    }

    if (installer_is_running ()) {
        g_warning ("another instance of installer is running\n");
        exit (0);
    }

    installer_container = create_web_container (FALSE, TRUE);

    gtk_window_set_decorated (GTK_WINDOW (installer_container), FALSE);
    //gtk_window_set_skip_taskbar_hint (GTK_WINDOW (installer_container), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (installer_container), TRUE);

    GtkWidget *webview = d_webview_new_with_uri (INSTALLER_HTML_PATH);

    g_signal_connect (installer_container, "button-press-event", G_CALLBACK (move_window), NULL);
    gtk_container_add (GTK_CONTAINER (installer_container), GTK_WIDGET (webview));
    gtk_window_set_default_size (GTK_WINDOW (installer_container), 755, 540);
    gtk_window_set_resizable (GTK_WINDOW (installer_container), FALSE);
    //gtk_window_set_position (GTK_WINDOW (installer_container), GTK_WIN_POS_CENTER);
    GdkGeometry geometry;
    geometry.min_width = 755;
    geometry.max_width = 755;
    geometry.base_width = 755;
    geometry.min_height = 540;
    geometry.max_height = 540;
    geometry.base_height = 540;

    gtk_window_set_geometry_hints (GTK_WINDOW (installer_container), webview, &geometry, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE);
    move_window_center ();
    gtk_widget_realize (installer_container);
    gtk_widget_show_all (installer_container);

    monitor_resource_file ("installer", webview);
    gtk_main ();

    return 0;
}

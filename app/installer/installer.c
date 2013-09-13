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
#include <sys/socket.h>
#include <sys/un.h>

#define INSTALLER_HTML_PATH "file://"RESOURCE_DIR"/installer/index.html"

static GtkWidget *installer_container = NULL;

gboolean installer_is_running()
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

JS_EXPORT_API
void installer_finish_install ()
{
    extern const gchar *target;
    if (target == NULL) {
        g_warning ("finish install:target is NULL\n");
        gtk_main_quit ();
        return ;
    }

    //fix me, exit chroot environment first
    //gchar *umount_sys = g_strdup_printf ("umount %s/sys", target);
    //gchar *umount_proc = g_strdup_printf ("umount %s/proc", target);
    //gchar *umount_devpts = g_strdup_printf ("umount %s/dev/pts", target);
    //gchar *umount_dev = g_strdup_printf ("umount %s/dev", target);

    //g_spawn_command_line_async (umount_sys, NULL);
    //g_spawn_command_line_async (umount_proc, NULL);
    //g_spawn_command_line_async (umount_devpts, NULL);
    //g_spawn_command_line_async (umount_dev, NULL);

    //g_free (umount_dev);
    //g_free (umount_devpts);
    //g_free (umount_proc);
    //g_free (umount_sys);

    ped_device_free_all ();
    gtk_main_quit ();
}

int main(int argc, char **argv)
{
    init_i18n ();
    gtk_init (&argc, &argv);

    if (installer_is_running ()) {
        g_warning ("another instance of installer is running\n");
        exit (0);
    }

    init_parted ();
    installer_container = create_web_container (FALSE, TRUE);

    gtk_window_set_decorated (GTK_WINDOW (installer_container), FALSE);
    //gtk_window_set_skip_taskbar_hint (GTK_WINDOW (installer_container), TRUE);
    //gtk_window_set_skip_pager_hint (GTK_WINDOW (installer_container), TRUE);

    GtkWidget *webview = d_webview_new_with_uri (INSTALLER_HTML_PATH);
    gtk_container_add (GTK_CONTAINER (installer_container), GTK_WIDGET (webview));
    gtk_window_set_position (GTK_WINDOW (installer_container), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (installer_container), 755, 540);

    gtk_widget_realize (installer_container);
    gtk_widget_show_all (installer_container);

    monitor_resource_file ("installer", webview);
    gtk_main ();

    return 0;
}

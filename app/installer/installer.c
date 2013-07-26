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
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (installer_container), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (installer_container), TRUE);
    gtk_window_fullscreen (GTK_WINDOW (installer_container));

    GtkWidget *webview = d_webview_new_with_uri (INSTALLER_HTML_PATH);
    gtk_container_add (GTK_CONTAINER (installer_container), GTK_WIDGET (webview));
    gtk_widget_realize (installer_container);

    GdkWindow *gdkwindow = gtk_widget_get_window (installer_container);
    GdkRGBA rgba = { 0, 0, 0, 0.0 };
    gdk_window_set_background_rgba (gdkwindow, &rgba);
    gdk_window_set_cursor (gdkwindow, gdk_cursor_new (GDK_LEFT_PTR));

    gtk_widget_show_all (installer_container);

    monitor_resource_file ("installer", webview);
    gtk_main ();
    return 0;
}

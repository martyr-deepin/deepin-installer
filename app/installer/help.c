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
#include <glib.h>
#include <glib/gprintf.h>
#include "dwebview.h"
#include "i18n.h"
#include "utils.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>

#define HELP_HTML_PATH "file://"RESOURCE_DIR"/installer/help.html"

static GtkWidget *help_container = NULL;

gboolean help_is_running ()
{
    int server_sockfd;
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0'; //make it be an name unix socket
    int path_size = g_sprintf (server_addr.sun_path+1, "%s", "help.installer.app.deepin");
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

    if (event->y > 90 || ((event->x > 360) && (event->y > 50) && (90 > event->y))
            || ((event->x > 480) && (event->y <30))) {
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

void
installerhelp_exit ()
{
    gtk_main_quit ();
}

static void
adapt_location_with_installer ()
{
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (help_container));
    gint s_width = gdk_screen_get_width (screen);
    gint s_height = gdk_screen_get_height (screen);
    gint x = (s_width - (755  + 500)) / 2 + 755;
    gint y = (s_height - 540) / 2;

    gtk_window_move (GTK_WINDOW (help_container), x, y);
}

int main(int argc, char **argv)
{
    init_i18n ();
    gtk_init (&argc, &argv);

    if (help_is_running ()) {
        g_warning ("another instance of help is running\n");
        exit (0);
    }

    help_container = create_web_container (FALSE, TRUE);
    gtk_window_set_decorated (GTK_WINDOW (help_container), FALSE);

    GtkWidget *webview = d_webview_new_with_uri (HELP_HTML_PATH);

    g_signal_connect (help_container, "button-press-event", G_CALLBACK (move_window), NULL);

    gtk_container_add (GTK_CONTAINER (help_container), GTK_WIDGET (webview));
    //gtk_window_set_position (GTK_WINDOW (help_container), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (help_container), 500, 540);

    gtk_widget_realize (help_container);
    gtk_widget_show_all (help_container);

    adapt_location_with_installer ();

    gtk_main ();

    return 0;
}

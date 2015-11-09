/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *               2013 ~ 2013 Liqiang Lee
 *
 * Author:      snyh <snyh@snyh.org>
 * Maintainer:  snyh <snyh@snyh.org>
 *              Liqiang Lee <liliqiang@linuxdeepin.com>
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
#include "background.h"
#include "theme.h"

#define SCHEMA_ID "com.deepin.dde.personalization"
#define GREETER_THEME_KEY "greeter-theme"
#define GREETER_THEME_PATH ""RESOURCE_DIR"/language-selector/greeter-theme"

GSettings* s;

const char* get_theme_path()
{
    return GREETER_THEME_PATH;
}

char* get_theme_config()
{
    return g_settings_get_string(s, GREETER_THEME_KEY);
}

char* get_current_bg_path()
{
    return g_settings_get_string(s, "current-picture");
}

void set_theme_background(GtkWidget* container,GtkWidget* child)
{
    char* theme = get_theme_config();
    g_message("[%s] theme: %s\n", __func__, theme);
    const char* bg_path = g_strdup_printf("%s/%s/bg.jpg", GREETER_THEME_PATH,
                                          theme);
    g_free(theme);
    g_message("[%s] theme_bg_path: %s\n", __func__, bg_path);
    GFile* gf = g_file_new_for_path(bg_path);
    if(!g_file_query_exists(gf,NULL)){
        const char* bg_url = get_current_bg_path();
        gf = g_file_new_for_uri(bg_url);
        bg_path = g_file_get_path(gf);
        g_message("[%s] bg does not exist, current bg: %s\n", __func__,
                  bg_path);
    }
    g_object_unref(gf);
    BackgroundInfo* bg_info = create_background_info(container, child);
    background_info_set_background_by_file(bg_info, bg_path);
}

void draw_background_by_theme(GtkWidget* widget, GtkWidget* child,
                              struct DisplayInfo info)
{
    g_message("[%s] width: %d, height: %d, x: %d y: %d\n", __func__,
              info.width, info.height, info.x, info.y);

    gtk_widget_set_size_request(widget, info.width, info.height);
    gtk_window_move(GTK_WINDOW(widget), info.x, info.y);

    set_theme_background(widget,child);
    gtk_widget_realize (widget);
    GdkWindow* gdkwindow = gtk_widget_get_window (widget);
    gdk_window_set_accept_focus(gdkwindow,FALSE);
    gdk_window_set_override_redirect (gdkwindow, TRUE);
    gtk_widget_show (widget);
}

void init_theme()
{
    s = g_settings_new(SCHEMA_ID);
}

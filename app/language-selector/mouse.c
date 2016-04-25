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
#include "display_info.h"
#include "jsextension.h"
#include "mouse.h"

void get_mouse_position(int* x, int* y)
{
    GdkDeviceManager *device_manager;
    GdkDevice *pointer;

    device_manager = gdk_display_get_device_manager(gdk_display_get_default());
    pointer = gdk_device_manager_get_client_pointer(device_manager);
    gdk_device_get_position(pointer, NULL, x, y);
}

gboolean is_in_range(int x,int x0,int x1)
{
    if ((x >= x0) && (x <= x1)) return TRUE;
    else return FALSE;
}

gboolean is_in_rect(int x, int y, struct DisplayInfo info)
{
    int x0 = info.x;
    int y0 = info.y;
    int x1 = info.x + info.width;
    int y1 = info.y + info.height;
    if(is_in_range(x,x0,x1) && is_in_range(y,y0,y1)){
        g_message("[%s]:: (%d,%d) is in (%d,%d,%d,%d)",__func__, x, y, x0, y0, x1, y1);
        return TRUE;
    }else{
        g_message("[%s]:: (%d,%d) is not in (%d,%d,%d,%d)",__func__, x, y, x0, y0, x1, y1);
        return FALSE;
    }
}

gboolean update_workarea_rect_by_mouse(struct DisplayInfo* area)
{
    struct DisplayInfo info;
    int x,y;
    get_mouse_position(&x,&y);
    for(int i = 0; i < update_monitors_num(); i++)
    {
        update_n_monitor_info(i,&info);
        if (is_in_rect(x, y, info)) break;
    }
    area->x = info.x;
    area->y = info.y;
    area->width = info.width;
    area->height = info.height;
    area->index = info.index;
    g_message("[%s]:: index:[%d]:: %d*%d (%d,%d)",__func__, area->index,area->width,area->height,area->x,area->y);
    return TRUE;
}

gboolean leave_notify(GtkWidget* widget,
                      GdkEventCrossing* e G_GNUC_UNUSED,
                      gpointer data G_GNUC_UNUSED)
{
    g_message("======[%s]=======",__func__);
    struct DisplayInfo info;
    update_workarea_rect_by_mouse(&info);
    widget_move_by_rect(widget,info);
    JSObjectRef size_info = json_create();
    json_append_number(size_info, "x", info.x);
    json_append_number(size_info, "y", info.y);
    json_append_number(size_info, "width", info.width);
    json_append_number(size_info, "height", info.height);
    js_post_message("leave-notify", size_info);
    return FALSE;
}

void listen_leave_notify_signal(GtkWidget* widget, gpointer data)
{
    g_signal_connect(widget, "leave-notify-event", G_CALLBACK(leave_notify), data);
}

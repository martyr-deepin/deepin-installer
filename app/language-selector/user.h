/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 *              bluth <yuanchenglu001@gmail.com>
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

#ifndef _USER_H
#define _USER_H

#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <cairo-xlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <stdio.h>
#include "dwebview.h"
#include "jsextension.h"

GDBusProxy *get_user_proxy (const gchar *username);

gchar * get_user_icon (const gchar *username);

gchar * get_user_background (const gchar *username);

gchar * get_user_realname (const gchar *username);

gboolean is_need_pwd (const gchar *username);

void draw_user_background (JSValueRef canvas, const gchar *username);

void keep_user_background (const gchar *username);

void kill_user_lock (const gchar *username, const gchar *password);

#endif

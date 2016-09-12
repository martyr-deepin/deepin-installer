/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

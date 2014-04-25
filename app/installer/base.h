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

#ifndef __BASE_H
#define __BASE_H

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "jsextension.h"
#include "utils.h"

gboolean installer_is_running ();

void emit_progress (const gchar *step, const gchar *progress);

gchar *get_matched_string (const gchar *target, const gchar *regex_string);

gchar *get_xrandr_size ();

JS_EXPORT_API double installer_get_memory_size ();

JS_EXPORT_API double installer_get_keycode_from_keysym (double keysym);

JS_EXPORT_API gboolean installer_detect_capslock ();

double get_free_memory_size ();

guint get_cpu_num ();

gchar *get_partition_mount_point (const gchar *path);

void unmount_partition_by_device (const gchar *path);

guint get_mount_target_count (const gchar *target);

gchar *get_partition_uuid (const gchar *path);

gchar *get_partition_label (const gchar *path);

JS_EXPORT_API void  installer_draw_background (JSValueRef canvas, const gchar *path);

#endif

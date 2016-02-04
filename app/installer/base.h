/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __BASE_H
#define __BASE_H

#define _GNU_SOURCE
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

gchar *get_matched_string_old (const gchar *target, const gchar *regex_string);
gchar* get_matched_string (const gchar *target, const gchar *regex_string);

JS_EXPORT_API double installer_get_memory_size ();

JS_EXPORT_API double installer_get_keycode_from_keysym (double keysym);

JS_EXPORT_API gboolean installer_detect_capslock ();

double get_free_memory_size ();

guint get_cpu_num ();

void unmount_partition_by_device (const gchar *path);

guint get_mount_target_count (const gchar *target);

gchar *get_partition_uuid (const gchar *path);

gchar *get_partition_label (const gchar *path);

#endif

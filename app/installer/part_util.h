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
#ifndef __PARTED_UTIL_H
#define __PARTED_UTIL_H

#include <parted/parted.h>
#include <stdio.h>
#include <errno.h>
#include <mntent.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "utils.h"
#include "jsextension.h"

JS_EXPORT_API gchar* installer_rand_uuid ();

void init_parted (void);

JS_EXPORT_API JSObjectRef installer_list_disks();

JS_EXPORT_API gchar * installer_get_disk_path (const gchar *disk);

JS_EXPORT_API gchar * installer_get_disk_model (const gchar *disk);

JS_EXPORT_API double installer_get_disk_max_primary_count (const char *disk);

JS_EXPORT_API double installer_get_disk_length (const gchar *disk);

JS_EXPORT_API double installer_get_disk_sector_size (const gchar *disk);

JS_EXPORT_API JSObjectRef installer_get_disk_partitions (const gchar *disk);

JS_EXPORT_API void installer_is_device_slow (const gchar *uuid);

JS_EXPORT_API gchar* installer_get_partition_type (const gchar *part);

JS_EXPORT_API gchar* installer_get_partition_name (const gchar *part);

JS_EXPORT_API gchar* installer_get_partition_path (const gchar *part);

JS_EXPORT_API gchar* installer_get_partition_mp (const gchar *part);

JS_EXPORT_API double installer_get_partition_start (const gchar *part);

JS_EXPORT_API double installer_get_partition_length (const gchar *part);

JS_EXPORT_API double installer_get_partition_end (const gchar *part);

JS_EXPORT_API const gchar* installer_get_partition_fs (const gchar *part);

JS_EXPORT_API gchar* installer_get_partition_label (const gchar *part);

JS_EXPORT_API gboolean installer_get_partition_busy (const gchar *part);

JS_EXPORT_API gboolean installer_get_partition_flag (const gchar *part, const gchar *flag_name);

JS_EXPORT_API void installer_get_partition_free (const gchar *part);

JS_EXPORT_API gchar* installer_get_partition_os (const gchar *part);

JS_EXPORT_API gboolean installer_new_disk_partition (const gchar *part_uuid, const gchar *disk, const gchar *type, const gchar *fs, double start, double end);

JS_EXPORT_API gboolean installer_delete_disk_partition (const gchar *disk, const gchar *part);

JS_EXPORT_API gboolean installer_update_partition_fs (const gchar *part, const gchar *fs);

JS_EXPORT_API gboolean installer_write_partition_mp (const gchar *part, const gchar *mp);

JS_EXPORT_API gboolean installer_set_partition_flag (const gchar *part, const gchar *flag_name, gboolean status);

JS_EXPORT_API gboolean installer_write_disk (const gchar *disk);

JS_EXPORT_API gboolean installer_mount_target (const gchar *part);

JS_EXPORT_API gboolean installer_update_grub (const gchar *uuid);

void emit_progress (const gchar *step, const gchar *progress);

#endif

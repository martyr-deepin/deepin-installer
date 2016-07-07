/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __PARTED_UTIL_H
#define __PARTED_UTIL_H

#include "base.h"

//read operation
JS_EXPORT_API
gchar* installer_rand_uuid (const char* prefix);

void init_parted (void);

JS_EXPORT_API
JSObjectRef installer_list_disks();

JS_EXPORT_API
gchar * installer_get_disk_path (const gchar *disk);

JS_EXPORT_API
gchar *installer_get_disk_type (const gchar *disk);

JS_EXPORT_API
gchar * installer_get_disk_model (const gchar *disk);

JS_EXPORT_API
double installer_get_disk_max_primary_count (const char *disk);

JS_EXPORT_API
double installer_get_disk_size (const gchar *disk);

JS_EXPORT_API
JSObjectRef installer_get_disk_partitions (const gchar *disk);

JS_EXPORT_API
gboolean installer_disk_support_efi (const gchar *disk);

JS_EXPORT_API
void installer_is_device_slow (const gchar *uuid);

JS_EXPORT_API
gchar* installer_get_partition_type (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_name (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_path (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_mp (const gchar *part);

JS_EXPORT_API
double installer_get_partition_start (const gchar *part);

JS_EXPORT_API
double installer_get_partition_length (const gchar *part);

JS_EXPORT_API
double installer_get_partition_end (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_fs (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_label (const gchar *part);

JS_EXPORT_API
gboolean installer_get_partition_busy (const gchar *part);

JS_EXPORT_API
gboolean installer_get_partition_flag (const gchar *part, const gchar *flag_name);

JS_EXPORT_API
void installer_get_partition_free (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_os (const gchar *part);

JS_EXPORT_API
gchar* installer_get_partition_os_desc (const gchar *part);

//write operation
JS_EXPORT_API
gboolean installer_new_disk_partition (const gchar *part_uuid, const gchar *disk, const gchar *type, const gchar *fs, double byte_start, double byte_end);

JS_EXPORT_API
gboolean installer_delete_disk_partition (const gchar *part);

JS_EXPORT_API
gboolean installer_update_partition_geometry (const gchar *uuid, double byte_start, double byte_size);

JS_EXPORT_API
gboolean installer_update_partition_fs (const gchar *part, const gchar *fs);

JS_EXPORT_API
gboolean installer_write_disk (const gchar *disk);

JS_EXPORT_API
gboolean installer_set_partition_flag (const gchar *part, const gchar *flag_name, gboolean status);

JS_EXPORT_API
void installer_unmount_partition (const gchar *part);

JS_EXPORT_API gboolean installer_system_support_efi ();

char* find_partition_path_by_sector_and_disk_path(const char* path, int byte_start);
void ped_print();
#endif

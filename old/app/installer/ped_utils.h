/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __PED_UTILS_H__
#define __PED_UTILS_H__

#include <parted/parted.h>
#include <glib.h>

typedef gboolean (*PartitionFilter)(PedPartition*, gpointer);

PedPartition* find_partition(PedDisk* disk,  PartitionFilter filter, gpointer user_data, GDestroyNotify notify);
PedPartition* create_and_add_partition(PedDisk* disk, PedPartitionType type, const PedFileSystemType* fs, PedSector start, PedSector end);

gboolean partition_filter_by_path(PedPartition* part, const char* path);

char* query_esp_path_by_disk_path(const char* path);

gchar * get_partition_mount_point (const gchar *path);

#endif

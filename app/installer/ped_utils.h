#ifndef __PED_UTILS_H__
#define __PED_UTILS_H__

#include <parted/parted.h>
#include <glib.h>

typedef gboolean (*PartitionFilter)(PedPartition*, gpointer);

PedPartition* find_partition(PedDisk* disk,  PartitionFilter filter, gpointer user_data, GDestroyNotify notify);

gboolean partition_filter_by_path(PedPartition* part, const char* path);

char* query_esp_path_by_disk_path(const char* path);

gchar * get_partition_mount_point (const gchar *path);

#endif

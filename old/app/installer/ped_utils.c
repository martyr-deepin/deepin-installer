/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "ped_utils.h"
#include <glib/gstdio.h>

gboolean partition_filter_by_path(PedPartition* part, const char* path)
{
    char* part_path = ped_partition_get_path(part);
    if (part_path == NULL) {
        return FALSE;
    }
    if (g_strcmp0(part_path , path) == 0) {
        g_free(part_path);
        return TRUE;
    } else {
        g_free(part_path);
        return FALSE;
    }
}

PedPartition* create_and_add_partition(PedDisk* disk, PedPartitionType type,
                                       const PedFileSystemType* fs,
                                       PedSector start, PedSector end)
{
    PedPartition* part = ped_partition_new (disk, type, fs, start, end);
    g_return_val_if_fail(part != NULL, FALSE);
    PedGeometry* geom = ped_geometry_new (disk->dev, start, end - start + 1);
    g_assert(geom != NULL);
    PedConstraint* constraint = ped_constraint_new_from_max (geom);
    g_assert(constraint != NULL);
    ped_disk_add_partition(disk, part, constraint);
    ped_constraint_destroy (constraint);
    ped_geometry_destroy (geom);
    return part;
}

PedPartition* find_partition(PedDisk* disk,  PartitionFilter filter,
                             gpointer user_data, GDestroyNotify notify)
{
    PedPartition* part = disk->part_list;
    gboolean found = false;
    while (part) {
        if (filter(part, user_data)) {
            found = true;
            break;
        }
        part = ped_disk_next_partition(disk, part);
    }
    if (notify) {
        notify(user_data);
    }
    return part;
}

int has_efi_directory(PedPartition* part)
{
    int is_busy = ped_partition_is_busy(part);

    GError* error = NULL;

    char* mount_point = NULL;
    char *path = ped_partition_get_path(part);

    if (!is_busy) {
        mount_point = g_dir_make_tmp("efi_detectorXXXXXX", &error);
        if (error != NULL) {
            g_warning("[%s] create efi_detector failed :%s\n", __func__,
                      error->message);
            g_error_free(error);
            error = NULL;
        }
        char* cmd = g_strdup_printf ("mount -t vfat %s %s", path, mount_point);
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
        g_free(cmd);
        if (error != NULL) {
            g_warning("[%s] Can't detect whether is $ESP, cmd: %s, error: %s",
                      __func__, cmd, error->message);
            g_error_free(error);
            error = NULL;
            return FALSE;
        }
    }

    if (mount_point == NULL) {
        mount_point = get_partition_mount_point(path);
    }
    g_free(path);

    char* esp_path = g_build_filename(mount_point, "EFI", NULL);
    int is_esp = g_file_test (esp_path, G_FILE_TEST_IS_DIR);
    g_free(esp_path);

    if (!is_busy) {
        char* cmd = g_strdup_printf ("umount -l %s", mount_point);
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
        g_free(cmd);
        if (error != NULL) {
            g_warning("[%s] Can't detect whether is $ESP, cmd: %s, error: %s",
                      __func__, cmd, error->message);
            g_error_free(error);
            g_free(mount_point);
            return is_esp;
        }

        //don't rm the dir if umount failed.
        g_rmdir(mount_point);
        g_free(mount_point);
    }

    return is_esp;
}

gboolean filter_partition_by_esp(PedPartition* part)
{
    if (part->num <  0) {
        return FALSE;
    }

    if (part->fs_type == 0 || strncmp(part->fs_type->name, "fat32", 5) != 0) {
        return FALSE;
    }

    return has_efi_directory(part);
}

char* query_esp_path_by_disk_path(const char* path)
{
    PedDevice* device = ped_device_get(path);
    PedDiskType *type = ped_disk_probe(device);
    if (type == 0) {
        return NULL;
    }
    if (strncmp(type->name, "loop", 5) == 0) {
        return NULL;
    }
    PedDisk* disk = ped_disk_new(device);
    if (disk == 0) {
        return NULL;
    }

    PedPartition* esp = find_partition(disk,
        (PartitionFilter)filter_partition_by_esp, NULL, NULL);
    if (esp != NULL) {
        return ped_partition_get_path(esp);
    }

    return NULL;
}


gchar * get_partition_mount_point (const gchar *path)
{
    g_message("[%s], path: %s\n", __func__, path);
    gchar *mp = NULL;
    gchar *swap_cmd = NULL;
    gchar *swap_output = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning("[%s]: invalid path %s\n", __func__, path);
        return mp;
    }

    swap_cmd = g_strdup_printf(
        "sh -c \"cat /proc/swaps |grep %s |awk '{print $1}'\"", path);
    g_spawn_command_line_sync (swap_cmd, &swap_output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning("[%s]: run swap cmd error->%s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (swap_cmd);

    if (swap_output != NULL && g_strcmp0(g_strstrip(swap_output), path) == 0) {
        return g_strdup ("swap");
    }

    cmd = g_strdup_printf ("findmnt -k -f -n -o TARGET -S %s", path);
    g_spawn_command_line_sync (cmd, &mp, NULL, NULL, &error);
    if (error != NULL) {
        g_warning("[%s]: run cmd failed: %s,  error: %s\n", __func__, cmd,
                  error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (cmd);
    if (mp != NULL) {
        mp = g_strstrip (mp);
        if (g_strcmp0 (mp, "") == 0) {
            g_free (mp);
            return NULL;
        }
    }

    return mp;
}

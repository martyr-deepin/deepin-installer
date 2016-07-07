/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <parted/parted.h>
#include <mntent.h>
#include <sys/mount.h>
#include "part_util.h"
#include "fs_util.h"
#include "ped_utils.h"
#include "info.h"
#include <math.h>
#include <unistd.h>

static GHashTable *disks;
static GHashTable *partitions;
static GHashTable *disk_partitions;
static GHashTable *partition_os = NULL;
static GHashTable *partition_os_desc = NULL;
void mkfs_latter(const char* path, const char* fs);

JS_EXPORT_API
gchar* installer_rand_uuid(const char* prefix)
{
    gchar *result = NULL;

    static gint32 number = 0;
    result = g_strdup_printf("%s%i", prefix, number++);

    return result;
}

static gpointer
thread_os_prober(gpointer data)
{
    partition_os = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                         g_free);

    partition_os_desc = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              g_free, g_free);

    g_spawn_command_line_sync("pkill -9 os-prober", NULL, NULL, NULL, NULL);

    gchar *output = NULL;
    GError *error = NULL;
    g_spawn_command_line_sync("env WINOSDATA=1 os-prober", &output, NULL,
                              NULL, &error);
    if (error != NULL) {
        g_warning("[%s] get os-prober error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }

    gchar **items = g_strsplit (output, "\n", -1);
    int i, j;
    for (i = 0; i < g_strv_length (items); i++) {
        gchar *item = g_strdup (items[i]);
        gchar **os = g_strsplit (item, ":", -1);

        if (g_strv_length (os) == 4) {
            g_hash_table_insert(partition_os, g_strdup(os[0]),
                                g_strdup(os[2]));
            g_hash_table_insert(partition_os_desc, g_strdup(os[0]),
                                g_strdup(os[1]));
        }

        g_strfreev (os);
        g_free (item);
    }
    g_strfreev (items);
    g_free (output);

    GRAB_CTX ();
    js_post_message ("os_prober", NULL);
    UNGRAB_CTX ();
    return NULL;
}



static
gboolean is_freespace_and_smaller_than_10MB(PedPartition* part)
{
    if (part->type & PED_PARTITION_FREESPACE == 0)
    return FALSE;
    int sector_size = part->disk->dev->sector_size;
    return part->geom.length * sector_size < 10 * 1024 * 1024;
}

GList* build_part_list(PedDisk* disk)
{
    g_message("[%s], disk: %s\n", __func__, disk->dev->path);
    GList *part_list = NULL;

    PedPartition *part = NULL;
    for (part = ped_disk_next_partition(disk, NULL); part;
         part = ped_disk_next_partition(disk, part)) {
        if ((part->type & PED_PARTITION_METADATA) == PED_PARTITION_METADATA) {
            continue;
        }
        if (is_freespace_and_smaller_than_10MB(part)) {
            continue;
        }

        gchar *part_uuid = installer_rand_uuid ("part");
        part_list = g_list_append (part_list, g_strdup (part_uuid));
        g_hash_table_insert (partitions, g_strdup (part_uuid), part);
        g_free (part_uuid);
    }
    return part_list;
}

const PedDiskType* best_disk_type()
{
    if (installer_system_support_efi()) {
        return ped_disk_type_get("gpt");
    } else {
        return ped_disk_type_get("msdos");
    }
}

PedDisk* try_build_disk(PedDevice* device)
{
    g_message("[%s], device: %s\n", __func__, device->path);
    if (device->read_only) {
        return NULL;
    }
    PedDiskType* type = ped_disk_probe(device);
    if (type == NULL) {
        return ped_disk_new_fresh(device, best_disk_type());
    } else if (strncmp(type->name, "gpt", 3) != 0 &&
               strncmp(type->name, "msdos", 5) != 0) {
        //filter other type of disks, like raid's partition type = "loop"
        return NULL;
    } else {
        return ped_disk_new (device);
    }
}


static gpointer thread_init_parted (gpointer data)
{
    g_message("[%s]\n", __func__);
    ped_device_probe_all ();

    PedDevice *device = NULL;
    while ((device = ped_device_get_next (device))) {
        PedDisk* disk = try_build_disk(device);
        if (disk == 0) {
            continue;
        }

        gchar *uuid = installer_rand_uuid("disk");
        g_hash_table_insert(disks, g_strdup (uuid), disk);

        g_hash_table_insert(disk_partitions, g_strdup(uuid),
                            build_part_list(disk));
        g_free (uuid);
    }

    GRAB_CTX ();
    js_post_message ("init_parted", NULL);
    UNGRAB_CTX ();

    return NULL;
}

void init_parted ()
{
    g_message("[%s]\n", __func__);
    disks = g_hash_table_new_full((GHashFunc) g_str_hash,
                                  (GEqualFunc) g_str_equal,
                                  (GDestroyNotify) g_free,
                                  NULL);

    disk_partitions = g_hash_table_new_full((GHashFunc) g_str_hash,
                                            (GEqualFunc) g_str_equal,
                                            (GDestroyNotify) g_free,
                                            (GDestroyNotify) g_list_free);

    partitions = g_hash_table_new_full((GHashFunc) g_str_hash,
                                       (GEqualFunc) g_str_equal,
                                       (GDestroyNotify) g_free,
                                       NULL);


    GThread *thread = g_thread_new("init-parted",
                                   (GThreadFunc)thread_init_parted, NULL);
    g_thread_unref (thread);

    GThread *prober_thread = g_thread_new("os-prober",
                                          (GThreadFunc)thread_os_prober, NULL);
    g_thread_unref (prober_thread);
}

JS_EXPORT_API
JSObjectRef installer_list_disks()
{
    g_return_val_if_fail(disks != NULL, json_array_create());

    JSObjectRef array = json_array_create ();
    GList *disk_keys = g_hash_table_get_keys (disks);
    for (int i = 0; i < g_list_length (disk_keys); i++) {
        json_array_insert(array, i,
                          jsvalue_from_cstr(get_global_context(),
                                            g_list_nth_data(disk_keys, i)));
    }
    g_list_free(disk_keys);

    return array;
}


JS_EXPORT_API
gchar *installer_get_disk_path (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    g_return_val_if_fail(uuid != NULL, g_strdup("Unknow"));

    PedDisk* disk = (PedDisk *) g_hash_table_lookup (disks, uuid);
    g_return_val_if_fail(disk != NULL, g_strdup("Unknow"));

    return g_strdup (disk->dev->path);
}

JS_EXPORT_API
gchar *installer_get_disk_type (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    g_return_val_if_fail(uuid != NULL, g_strdup("Unknow"));

    PedDisk* peddisk = (PedDisk *) g_hash_table_lookup (disks, uuid);
    g_return_val_if_fail(peddisk != NULL, g_strdup("Unknow"));

    PedDiskType* type = ped_disk_probe (peddisk->dev);
    g_return_val_if_fail(type != NULL, g_strdup("Unknow"));
    return g_strdup (type->name);
}

JS_EXPORT_API
gchar *installer_get_disk_model (const gchar *uuid)
{
    g_return_val_if_fail(uuid != NULL, g_strdup("Unknow"));
    PedDisk* disk = (PedDisk *) g_hash_table_lookup(disks, uuid);
    g_return_val_if_fail(disk != NULL, g_strdup("Unknow"));

    return g_strdup (disk->dev->model);
}

JS_EXPORT_API
double installer_get_disk_max_primary_count (const gchar *uuid)
{
    g_return_val_if_fail(uuid != NULL, 0);
    PedDisk* disk = (PedDisk *) g_hash_table_lookup (disks, uuid);
    g_return_val_if_fail(disk != NULL, 0);

    return ped_disk_get_max_primary_partition_count (disk);
}

JS_EXPORT_API
double installer_get_disk_size (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    g_return_val_if_fail(uuid != NULL, 0);
    PedDisk *disk = (PedDisk *) g_hash_table_lookup(disks, uuid);
    g_return_val_if_fail(disk != NULL, 0);

    return disk->dev->length * disk->dev->sector_size;
}

JS_EXPORT_API
JSObjectRef installer_get_disk_partitions (const gchar *disk)
{
    g_message("[%s], disk: %s\n", __func__, disk);
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();
    int i;
    GList *parts = NULL;
    if (disk == NULL) {
        g_warning("[%s]: disk NULL\n", __func__);
    } else {
        parts = (GList *) g_hash_table_lookup (disk_partitions, disk);
    }

    if (parts != NULL) {
        for (i = 0; i < g_list_length (parts); i++) {
            json_array_insert(array, i,
                              jsvalue_from_cstr(get_global_context(),
                                                g_list_nth_data (parts, i)));
        }
    }

    UNGRAB_CTX ();
    return array;
}

JS_EXPORT_API
gboolean installer_system_support_efi ()
{
    return g_file_test("/sys/firmware/efi", G_FILE_TEST_IS_DIR);
}

JS_EXPORT_API
gboolean installer_disk_is_gpt(const char* disk)
{
    g_message("[%s], disk: %s\n", __func__, disk);
    PedDisk* peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        if ((peddisk->type != NULL) &&
            (g_strcmp0 ("gpt", peddisk->type->name) == 0)) {
            return TRUE;
        }
    } else {
        g_warning("[%s]: find peddisk by %s failed\n", __func__, disk);
    }
    return FALSE;
}

JS_EXPORT_API
gchar* installer_get_partition_type (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar *type = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return type;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);

    if (pedpartition != NULL) {
        PedPartitionType part_type = pedpartition->type;
        switch (part_type) {
            case PED_PARTITION_NORMAL:
                type = g_strdup ("normal");
                break;
            case PED_PARTITION_LOGICAL:
                type = g_strdup ("logical");
                break;
            case PED_PARTITION_EXTENDED:
                type = g_strdup ("extended");
                break;
            case PED_PARTITION_FREESPACE:
                type = g_strdup ("freespace");
                break;
            case PED_PARTITION_METADATA:
                type = g_strdup ("metadata");
                break;
            case PED_PARTITION_PROTECTED:
                type = g_strdup ("protected");
                break;
            default:
                if (part_type > PED_PARTITION_PROTECTED) {
                    type = g_strdup ("protected");
                } else if (part_type > PED_PARTITION_METADATA) {
                    type = g_strdup ("metadata");
                } else if (part_type > PED_PARTITION_FREESPACE) {
                    type = g_strdup ("freespace");
                } else {
                    g_warning ("invalid type:%d\n", part_type);
                    type = g_strdup ("protected");
                }
                break;
        }
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }

    return type;
}

JS_EXPORT_API
gchar *installer_get_partition_name (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar *name = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return name;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        if (ped_disk_type_check_feature(pedpartition->disk->type,
                                        PED_DISK_TYPE_PARTITION_NAME)) {
            name = g_strdup (ped_partition_get_name (pedpartition));
        }
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }

    return name;
}

JS_EXPORT_API
gchar* installer_get_partition_path (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    PedPartition *part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_assert(part != NULL);

    if (part->num == -1) {
        char* ret = g_strdup_printf("free:%s:%d", part->disk->dev->path,
                                    (int)part->geom.start);
        g_warning("[%s] ret: %s\n", __func__, ret);
    return ret;
    }

    gchar *path = ped_partition_get_path (part);
    g_assert(path != NULL);
    return path;
}

JS_EXPORT_API
gchar* installer_get_partition_mp (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    g_return_val_if_fail(uuid != NULL, NULL);
    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_return_val_if_fail(part != NULL, NULL);
    g_return_val_if_fail(part->num != -1, NULL);

    gchar* path = ped_partition_get_path (part);
    gchar* mp = get_partition_mount_point (path);
    g_free (path);

    return mp;
}

JS_EXPORT_API
double installer_get_partition_start (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    if (uuid == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return 0;
    }

    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    if (part != NULL) {
        return part->geom.start * part->disk->dev->sector_size;
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, uuid);
    }

    return 0;
}

JS_EXPORT_API
double installer_get_partition_size (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    if (uuid == NULL) {
        g_warning("[%s]: part NULL\n", __func__);
        return 0;
    }

    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    if (part != NULL) {
        return part->geom.length * part->disk->dev->sector_size;
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, uuid);
    }

    return 0;
}

JS_EXPORT_API
double installer_get_partition_end (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    if (uuid == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return 0;
    }

    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    if (part != NULL) {
        return part->geom.end * part->disk->dev->sector_size;
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, uuid);
    }

    return 0;
}

JS_EXPORT_API
gchar *installer_get_partition_fs (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar *fs = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return fs;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        PedGeometry *geom = ped_geometry_duplicate (&pedpartition->geom);
        PedFileSystemType *fs_type = ped_file_system_probe (geom);
        if (fs_type != NULL) {
            fs = g_strdup (fs_type->name);
        }
        ped_geometry_destroy (geom);

    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }
    if (fs != NULL) {
        g_debug("[%s]: fs is %s\n", __func__, fs);
        if (g_strrstr (fs, "swap") != NULL) {
            return g_strdup ("swap");
        }
    }

    return fs;
}

JS_EXPORT_API
gchar* installer_get_partition_label (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar *label = NULL;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    if (part == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return NULL;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
        return NULL;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning("[%s]: get part %s path failed\n", __func__, part);
        return NULL;
    }
    label = get_partition_label (path);
    if (label != NULL) {
        label = g_strstrip (label);
    }
    g_free (path);

    return label;
}

JS_EXPORT_API
gboolean installer_is_partition_busy (const gchar *uuid)
{
    g_message("[%s], uuid: %s\n", __func__, uuid);
    g_return_val_if_fail(uuid != NULL, FALSE);
    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_return_val_if_fail(part != NULL, FALSE);

    return ped_partition_is_busy(part);
}

JS_EXPORT_API
gboolean installer_get_partition_flag (const gchar *part,
                                       const gchar *flag_name)
{
    g_message("[%s], part: %s, flag_name: %s\n", __func__, part, flag_name);
    gboolean result = FALSE;
    PedPartition *pedpartition = NULL;
    PedPartitionFlag flag;
    if (part == NULL || flag_name == NULL) {
        g_warning("[%s]: part or flag name is NULL\n", __func__);
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning("[%s] :find pedpartition %s failed\n", __func__, part);
        goto out;
    }
    if (!ped_partition_is_active (pedpartition)) {
        g_warning("[%s]: ped partition flag not active\n", __func__);
        goto out;
    }
    flag = ped_partition_flag_get_by_name (flag_name);
    if (flag == 0) {
        g_warning("[%s]: ped partition flag get by name failed\n", __func__);
        goto out;
    }
    if (ped_partition_is_flag_available (pedpartition, flag)) {
        result = (gboolean) ped_partition_get_flag (pedpartition, flag);
    }
    goto out;

out:
    return result;
}

JS_EXPORT_API
void installer_get_partition_free (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part NULL\n", __func__);
        return;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        PedPartitionType part_type = pedpartition->type;

        if (part_type != PED_PARTITION_NORMAL &&
            part_type != PED_PARTITION_LOGICAL &&
            part_type != PED_PARTITION_EXTENDED) {
            g_warning("[%s]: no meaning for none used\n", __func__);
            return;
        }

        const gchar *fs = NULL;
        PedGeometry *geom = ped_geometry_duplicate (&pedpartition->geom);
        PedFileSystemType *fs_type = ped_file_system_probe (geom);
        if (fs_type != NULL) {
            fs = fs_type->name;
        }
        ped_geometry_destroy (geom);
        if (g_strstr_len(fs, -1, "swap")) {
            JSObjectRef message = json_create ();
            json_append_string(message, "part", part);
            json_append_number(message, "free",
                               installer_get_partition_size (part));
            js_post_message ("used", message);
            return;
        }

        if (fs == NULL) {
            g_warning("[%s]: get partition file system failed\n", __func__);
            return;
        }

        gchar *path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            struct FsHandler *handler = g_new0 (struct FsHandler, 1);
            handler->path = g_strdup (path);
            handler->part = g_strdup (part);
            handler->fs = g_strdup (fs);
            GThread *thread = g_thread_new("get_partition_free",
                                           (GThreadFunc) get_partition_free,
                                           (gpointer) handler);
            g_thread_unref (thread);
        } else {
            g_warning("[%s]: get %s path failed\n", __func__, part);
        }
        g_free (path);
    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }
}

JS_EXPORT_API
gchar* installer_get_partition_os (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar* result = NULL;
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part is NULL\n", __func__);
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            result = g_strdup (g_hash_table_lookup (partition_os, path));

        } else {
            g_warning("[%s]: get %s path failed\n", part, __func__);
        }
        g_free (path);

    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }

    return result;
}

JS_EXPORT_API
gchar* installer_get_partition_os_desc (const gchar *part)
{
    g_message("[%s], part: %s\n", __func__, part);
    gchar* result = NULL;
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning("[%s]: part NULL\n", __func__);
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            result = g_strdup (g_hash_table_lookup (partition_os_desc, path));

        } else {
            g_warning("[%s]: get %s path failed\n", __func__, part);
        }
        g_free (path);

    } else {
        g_warning("[%s]: find pedpartition %s failed\n", __func__, part);
    }

    return result;
}

JS_EXPORT_API
gboolean installer_new_disk_partition(const gchar *part_uuid,
                                      const gchar *disk_uuid,
                                      const gchar *type,
                                      const gchar *fs,
                                      double byte_start,
                                      double byte_end)
{
    g_message("[%s], part_uuid: %s, disk_uuid: %s, type: %s, fs: %s, "
              "byte_start: %f, byte_end: %f\n",
              __func__, part_uuid, disk_uuid, type, fs, byte_start, byte_end);
    g_assert(part_uuid != NULL);
    g_assert(disk_uuid != NULL);
    g_assert(type != NULL);
    g_assert(byte_start >= 0);
    g_assert(byte_end > 0);

    PedPartitionType part_type;
    if (g_strcmp0 (type, "normal") == 0) {
        part_type = PED_PARTITION_NORMAL;
    } else if (g_strcmp0 (type, "logical") == 0) {
        part_type = PED_PARTITION_LOGICAL;
    } else if (g_strcmp0 (type, "extended") == 0) {
        part_type = PED_PARTITION_EXTENDED;
    } else {
        g_assert_not_reached();
    }

    const PedFileSystemType *part_fs = NULL;
    if (part_type != PED_PARTITION_EXTENDED) {
        part_fs = ped_file_system_type_get (fs);
        if (part_fs == NULL) {
            g_warning("[%s]: ped file system type get for %s is NULL\n",
                      __func__, fs);
        }
    }

    PedDisk* disk = (PedDisk *) g_hash_table_lookup (disks, disk_uuid);
    g_assert(disk != NULL);
    PedSector start = (PedSector) ceil(byte_start / disk->dev->sector_size);
    PedSector end = (PedSector) floor(byte_end / disk->dev->sector_size);
    PedPartition* part = create_and_add_partition(disk, part_type, part_fs,
                                                  start, end);
    g_return_val_if_fail(part != NULL, FALSE);

    g_hash_table_insert (partitions,  g_strdup (part_uuid), part);
    return TRUE;
}

JS_EXPORT_API
gboolean installer_delete_disk_partition (const gchar *part_uuid)
{
    g_message("[%s], part_uuid: %s\n", __func__, part_uuid);
    g_assert(part_uuid != NULL);

    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions,
                                                             part_uuid);
    g_assert(part != NULL);
    g_message("[%s] delete path: %s, with uuid: %s\n", __func__,
              part->disk->dev->path, part_uuid);

    if ((ped_disk_delete_partition (part->disk, part) != 0)) {
        return TRUE;
    } else {
        g_warning("[%s]: delete partition failed\n", __func__);
    return FALSE;
    }
}

JS_EXPORT_API
gboolean installer_update_partition_geometry(const gchar *uuid,
                                             double byte_start,
                                             double byte_size)
{
    g_message("[%s], uuid: %s, byte_start: %f, byte_size: %f\n",
              __func__, uuid, byte_start, byte_size);
    g_return_val_if_fail(uuid != NULL, FALSE);
    g_return_val_if_fail(byte_start >= 0, FALSE);
    g_return_val_if_fail(byte_size > 0, FALSE);

    PedGeometry *geom = NULL;

    PedPartition *part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_assert(part != NULL);

    geom = &part->geom;
    g_assert(geom != NULL);

    PedSector start = (PedSector)ceil(byte_start /
                                      part->disk->dev->sector_size);
    PedSector length = (PedSector)floor(byte_size /
                                        part->disk->dev->sector_size);
    g_message("[%s]: update part->%s geometry start->%d length->%d\n",
              __func__, uuid, (int)start, (int)length);
    ped_geometry_set (geom,  start, length);
    return TRUE;
}

JS_EXPORT_API
gboolean installer_update_partition_fs (const gchar *uuid, const gchar *fs)
{
    g_message("[%s] uuid: %s, fs: %s\n", __func__, uuid, fs);
    g_return_val_if_fail(uuid != NULL, FALSE);
    g_return_val_if_fail(fs != NULL, FALSE);

    PedPartition *part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_return_val_if_fail(part != NULL, FALSE);

    const PedFileSystemType *part_fs_type = NULL;
    if (g_strcmp0 (fs, "efi") == 0) {
        part_fs_type = ped_file_system_type_get ("fat32");
    } else if (g_strcmp0 (fs, "swap") == 0) {
        part_fs_type = ped_file_system_type_get ("linux-swap");
    } else {
        part_fs_type = ped_file_system_type_get (fs);
    }

    if (part_fs_type == NULL) {
        g_warning("[%s]: get part fs type %s failed\n", __func__, fs);
        return FALSE;
    }

    if ((ped_partition_set_system (part, part_fs_type)) == 0) {
        g_warning("[%s]: ped partition set system %s failed\n", __func__, fs);
        return FALSE;
    }

    const gchar *part_path = ped_partition_get_path (part);
    g_assert(part_path != NULL);

    //TODO: create EFI directory
    if (g_strcmp0 (fs, "efi") == 0) {
        mkfs_latter(part_path, "fat32");
        if (! installer_set_partition_flag (uuid, "boot", 1)) {
            g_warning("[%s]: set flag for uefi failed\n", __func__);
        }
    } else {
        mkfs_latter(part_path, fs);
    }
    return TRUE;
}

JS_EXPORT_API
gboolean installer_write_disk (const gchar *uuid)
{
    g_message("[%s] uuid: %s", __func__, uuid); 
    g_assert(uuid != NULL);

    PedDisk *disk = (PedDisk *) g_hash_table_lookup (disks, uuid);
    if (disk != NULL) {
        g_message("[%s] write disk->%s\n", __func__, disk->dev->path);
        g_message("[%s] will call ped_disk_commit_to_dev(dev)\n", __func__);
        if ((ped_disk_commit_to_dev (disk)) == 0) {
            g_warning("[%s] write disk(%s): commit to dev failed\n",
                      __func__, disk->dev->path);
            return FALSE;
        }
        g_message("[%s] will call ped_disk_commit_to_os(disk)\n", __func__);
        if ((ped_disk_commit_to_os (disk)) == 0) {
            // Retry if failed
            sleep(1);
            if ((ped_disk_commit_to_os (disk)) == 0) {
              g_warning("[%s] write disk(%s): commit to os failed\n",
                        __func__, disk->dev->path);
              return FALSE;
            }
        }
        // Watches the udev event queue.
        // Wait for kernel to handle pending events
        g_message("[%s] will call udevadm settle --timeout=5\n", __func__);
        g_spawn_command_line_sync ("udevadm settle --timeout=5",
                                   NULL, NULL, NULL, NULL);
        return TRUE;
    } else {
        g_warning("[%s]: find peddisk %s failed\n", __func__, uuid);
        return FALSE;
    }
}

JS_EXPORT_API
gboolean installer_set_partition_flag (const gchar *uuid,
                                       const gchar *flag_name,
                                       gboolean status)
{
    g_message("[%s], uuid: %s, flag_name: %s, status: %d\n",
              __func__, uuid, flag_name, (int)status);
    g_return_val_if_fail(uuid!= NULL, FALSE);
    g_return_val_if_fail(flag_name != NULL, FALSE);

    PedPartition* part = (PedPartition *)g_hash_table_lookup(partitions, uuid);
    g_return_val_if_fail(part != NULL, FALSE);

    PedPartitionFlag flag = ped_partition_flag_get_by_name (flag_name);
    if (ped_partition_is_flag_available (part, flag)) {
        ped_partition_set_flag (part, flag, status);
        return TRUE;
    } else {
        g_warning("[%s]: flag %s is not available\n", __func__, flag_name);
        return FALSE;
    }
}

char* find_partition_path_by_sector_and_disk_path(const char* path, int start)
{
    g_message("[%s], path: %s, start: %d\n", __func__, path, start);
    PedDevice* dev = ped_device_get(path);
    g_return_val_if_fail(dev != NULL, NULL);
    PedDisk* disk = ped_disk_new(dev);
    g_return_val_if_fail(disk != NULL, NULL);

    PedPartition* part = ped_disk_get_partition_by_sector(disk,
                                                          (PedSector)start);
    g_return_val_if_fail(part != NULL, NULL);
    g_return_val_if_fail(part->num != -1, NULL);

    return g_strdup(ped_partition_get_path(part));
}

//when dectect mount partition, tell user to unmount them
JS_EXPORT_API
void installer_unmount_partition (const gchar *part)
{
    g_message("[%s] part: %s\n", __func__, part);
    if (part == NULL) {
        g_warning("[%s]: part NULL\n", __func__);
        return;
    }
    gchar *path = installer_get_partition_path (part);
    unmount_partition_by_device (path);
    g_free (path);
}

void partition_print(char* uuid, PedPartition* part)
{
    printf("[%s] Partition uuid: %s, path: %s, num: %d\n", __func__, uuid,
           ped_partition_get_path(part), part->num);
}

void disk_print(char* uuid, PedDisk* disk)
{
    printf("[%s] Disk uuid: %s, path: %s\n", __func__, uuid, disk->dev->path);
}

void ped_print()
{
    g_hash_table_foreach(disks, (GHFunc)disk_print, NULL);
    g_hash_table_foreach(partitions, (GHFunc)partition_print, NULL);
}

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

#include <parted/parted.h>
#include <mntent.h>
#include <sys/mount.h>
#include "part_util.h"
#include "fs_util.h"
#include "info.h"

#define PART_INFO_LENGTH 4096

static GHashTable *disks;
static GHashTable *partitions;
static GHashTable *disk_partitions;
static GHashTable *partition_os = NULL;
static GHashTable *partition_os_desc = NULL;
int chroot_fd;
gboolean in_chroot = FALSE;
GList *mounted_list = NULL;
static GAsyncQueue *op_queue = NULL;
static GMutex op_mutex;
static gint op_count = 0;

JS_EXPORT_API 
gchar* installer_rand_uuid ()
{
    gchar *result = NULL;

    gint32 number;
    GRand *rand = g_rand_new ();
    number = g_rand_int_range (rand, 10000000, 99999999);
    g_rand_free (rand);

    result = g_strdup_printf("%i", number);

    return result;
}

static gpointer 
thread_os_prober (gpointer data)
{
    partition_os = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                          (GEqualFunc) g_str_equal, 
                                          (GDestroyNotify) g_free, 
                                          (GDestroyNotify) g_free);

    partition_os_desc = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                          (GEqualFunc) g_str_equal, 
                                          (GDestroyNotify) g_free, 
                                          (GDestroyNotify) g_free);

    gchar *cmd = g_find_program_in_path ("os-prober");
    if (cmd == NULL) {
        g_warning ("os:os-prober not installed\n");
    }
    g_spawn_command_line_sync ("pkill -9 os-prober", NULL, NULL, NULL, NULL);

    gchar *output = NULL;
    GError *error = NULL;
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition os:os-prober %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }

    gchar **items = g_strsplit (output, "\n", -1);
    int i, j;
    for (i = 0; i < g_strv_length (items); i++) {
        gchar *item = g_strdup (items[i]);
        gchar **os = g_strsplit (item, ":", -1);

        if (g_strv_length (os) == 4) {
            //g_warning ("get partition os:insert key %s value %s\n", os[0], os[2]);
            g_hash_table_insert (partition_os, g_strdup (os[0]), g_strdup (os[2]));
            g_hash_table_insert (partition_os_desc, g_strdup (os[0]), g_strdup (os[1]));
        }

        g_strfreev (os);
        g_free (item);
    }
    g_strfreev (items);
    g_free (output);
    g_free (cmd);

    GRAB_CTX ();
    js_post_message ("os_prober", NULL);
    UNGRAB_CTX ();
    return NULL;
}

static gpointer
thread_init_parted (gpointer data)
{
    disks = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                   (GEqualFunc) g_str_equal, 
                                   (GDestroyNotify) g_free, 
                                   NULL);

    disk_partitions = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                             (GEqualFunc) g_str_equal, 
                                             (GDestroyNotify) g_free, 
                                             (GDestroyNotify) g_list_free);

    partitions = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                        (GEqualFunc) g_str_equal, 
                                        (GDestroyNotify) g_free, 
                                        NULL);

    PedDevice *device = NULL;
    PedDisk *disk = NULL;

    ped_device_probe_all ();

    while ((device = ped_device_get_next (device))) {
        if (device->read_only) {
            g_warning ("init parted:device read only\n");
            continue;
        } 

        if (ped_disk_probe (device) != NULL) {
            disk = ped_disk_new (device);

        } else {
            g_printf ("init parted:new disk partition table for %s\n", device->path);
            const PedDiskType *type;
            long long size = device->sector_size;
            PedSector length = device->length;
            //if (size * length > (long long) 2*1024*1024*1024*1024) {
            if (size * length > (long long) 2 << 40) {
                type = ped_disk_type_get ("gpt");
            } else {
                type = ped_disk_type_get ("msdos");
            }

            if (type != NULL) {
                disk = ped_disk_new_fresh (device, type);
            } else {
                g_warning ("init parted:ped disk type get failed:%s\n", device->path);
                continue;
            }
        }

        gchar *uuid_num = installer_rand_uuid ();
        gchar *uuid = g_strdup_printf ("disk%s", uuid_num);
        g_free (uuid_num);

        GList *part_list = NULL;
        if (uuid != NULL && disk != NULL ) {
            g_hash_table_insert (disks, g_strdup (uuid), disk);

            PedPartition *partition = NULL;
            for (partition = ped_disk_next_partition (disk, NULL); partition; 
                    partition = ped_disk_next_partition (disk, partition)) {

                gchar *uuid_num = installer_rand_uuid ();
                gchar *part_uuid = g_strdup_printf("part%s", uuid_num);
                g_free (uuid_num);

                part_list = g_list_append (part_list, g_strdup (part_uuid));
                g_hash_table_insert (partitions, g_strdup (part_uuid), partition);
                g_free (part_uuid);
            }
        }
        g_hash_table_insert (disk_partitions, g_strdup (uuid), part_list);
        g_free (uuid);
    }
    GRAB_CTX ();
    js_post_message ("init_parted", NULL);
    UNGRAB_CTX ();
    return NULL;
}

void init_parted ()
{
    GThread *thread = g_thread_new ("init-parted", (GThreadFunc) thread_init_parted, NULL);
    g_thread_unref (thread);

    GThread *prober_thread = g_thread_new ("os-prober", (GThreadFunc) thread_os_prober, NULL);
    g_thread_unref (prober_thread);
}

JS_EXPORT_API 
JSObjectRef installer_list_disks()
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();
    int i;
    if (disks == NULL) {
        g_warning ("installer list disks:disks NULL\n");
        UNGRAB_CTX ();
        return array;
    }

    GList *disk_keys = g_hash_table_get_keys (disks);
    if (disk_keys == NULL) {
        g_warning ("installer list disks:disk keys NULL\n");
        UNGRAB_CTX ();
        return array;
    }

    for (i = 0; i < g_list_length (disk_keys); i++) {
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), g_list_nth_data (disk_keys, i)));
    }

    UNGRAB_CTX ();
    return array;
}

JS_EXPORT_API
gchar *installer_get_disk_path (const gchar *disk)
{
    gchar *result = NULL;
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("get disk path:disk NULL\n");
        return result;
    }

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        g_assert(device != NULL);

        result = g_strdup (device->path);

    } else {
        g_warning ("get disk path:find peddisk by %s failed\n", disk);
    }

    return result;
}

JS_EXPORT_API
gchar *installer_get_disk_type (const gchar *disk)
{
    gchar *result = NULL;
    PedDisk *peddisk = NULL;
    PedDevice *device = NULL;
    PedDiskType *type = NULL;
    if (disk == NULL) {
        g_warning ("get disk type:disk NULL\n");
        return result;
    }

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    if (peddisk == NULL) {
        g_warning ("get disk type:find peddisk by %s failed\n", disk);
        return result;
    } 
    device = peddisk->dev;
    if (device == NULL) {
        g_warning ("get disk type:get device for %s failed\n", disk);
        return result;
    }
    type = ped_disk_probe (device);
    if (type == NULL) {
        g_warning ("get disk type:get PedDiskType for %s failed\n", disk);
        return result;
    }
    result = g_strdup (type->name);

    return result;
}

JS_EXPORT_API
gchar *installer_get_disk_model (const gchar *disk)
{
    gchar *result = NULL;
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("get disk model:disk NULL\n");
        return result;
    }

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        if (device != NULL) {
            result = g_strdup (device->model);
        }

    } else {
        g_warning ("get disk model:find peddisk by %s failed\n", disk);
    }

    return result;
}

JS_EXPORT_API
double installer_get_disk_max_primary_count (const gchar *disk)
{
    double count = 0;
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("get disk max primary count:disk NULL\n");
        return count;
    }

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    if (peddisk != NULL) {
        count = ped_disk_get_max_primary_partition_count (peddisk);

    } else {
        g_warning ("get disk max primary count:find peddisk by %s failed\n", disk);
    }

    return count;
}

JS_EXPORT_API
double installer_get_disk_length (const gchar *disk)
{
    double length = 0;
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("get disk length:disk NULL\n");
        return length;
    }

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        if (device != NULL) {
            length = device->length;
        }

    } else {
        g_warning ("get disk length:find peddisk by %s failed\n", disk);
    }
    
    return length;
}

JS_EXPORT_API 
double installer_get_disk_sector_size (const gchar *disk)
{
    double sector_size = 0;
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("get disk sector size:disk NULL\n");
        return sector_size;
    }

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        if (device != NULL) {
            sector_size = device->sector_size;
        }

    } else {
        g_warning ("get disk sector size:find peddisk by %s failed\n", disk);
    }
    
    return sector_size;
}

JS_EXPORT_API
JSObjectRef installer_get_disk_partitions (const gchar *disk)
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();
    int i;
    GList *parts = NULL;
    if (disk == NULL) {
        g_warning ("get disk partitions:disk NULL\n");
    } else {
        parts = (GList *) g_hash_table_lookup (disk_partitions, disk);
    }
   
    if (parts != NULL) {
        for (i = 0; i < g_list_length (parts); i++) {
            json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), g_list_nth_data (parts, i)));
        }
    }

    UNGRAB_CTX ();
    return array;
}

JS_EXPORT_API
gboolean installer_disk_support_efi (const gchar *disk)
{
    PedDisk *peddisk = NULL;
    if (disk == NULL) {
        g_warning ("disk support efi:disk NULL\n");
        return FALSE;
    }

    if (!g_file_test ("/sys/firmware/efi", G_FILE_TEST_IS_DIR)) {
        return FALSE;
    }

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        if ((peddisk->type != NULL) && (g_strcmp0 ("gpt", peddisk->type->name) == 0)) {
            return TRUE;
        }
    } else {
        g_warning ("get disk sector size:find peddisk by %s failed\n", disk);
    }

    return FALSE;
}

//generally, you should get device speed by disk, not partition
JS_EXPORT_API 
void installer_is_device_slow (const gchar *uuid)
{
    gchar *path = NULL;

    if (g_str_has_prefix (uuid, "disk")) {
        path = installer_get_disk_path (uuid);
    } else if (g_str_has_prefix (uuid, "part")) {
        path = installer_get_partition_path (uuid);
    } else {
        g_warning ("is device slow:invalid uuid %s\n", uuid);
    }

    if (path != NULL) {
        struct SpeedHandler *handler = g_new (struct SpeedHandler, 1);
        handler->path = g_strdup (path);
        handler->uuid = g_strdup (uuid);

        GThread *speed_thread = g_thread_new ("speed", (GThreadFunc) is_slowly_device, handler);
        g_thread_unref (speed_thread);

    } else {
        g_warning ("is device slow:get device path for %s failed\n", uuid);
    }

    g_free (path);
}

JS_EXPORT_API
gchar* installer_get_partition_type (const gchar *part)
{
    gchar *type = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition type:part NULL\n");
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
        g_warning ("get partition type:find pedpartition %s failed\n", part);
    }

    return type;
}

JS_EXPORT_API
gchar *installer_get_partition_name (const gchar *part)
{
    gchar *name = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition name:part NULL\n");
        return name;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        if (ped_disk_type_check_feature (pedpartition->disk->type, PED_DISK_TYPE_PARTITION_NAME)) {
            name = g_strdup (ped_partition_get_name (pedpartition));
        }
    } else {
        g_warning ("get partition name:find pedpartition %s failed\n", part);
    }

    return name;
}

JS_EXPORT_API
gchar* installer_get_partition_path (const gchar *part)
{
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition path:part NULL\n");
        return path;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        path = ped_partition_get_path (pedpartition);
    } else {
        g_warning ("get partition path:find pedpartition %s failed\n", part);
    }

    return path;
}

JS_EXPORT_API 
gchar* installer_get_partition_mp (const gchar *part)
{
    gchar *mp = NULL;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    if (part == NULL) {
        g_warning ("get partition mp:part NULL\n");
        return mp;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("get partition mp:find pedpartition %s failed\n", part);
        return mp;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("get partition mp:get partition path failed\n");
        return mp;
    }
    mp = get_partition_mount_point (path);

    g_free (path);
    return mp;
}

JS_EXPORT_API
double installer_get_partition_start (const gchar *part)
{
    double start = 0;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition start:part NULL\n");
        return start;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        start = pedpartition->geom.start;

    } else {
        g_warning ("get partition start:find pedpartition %s failed\n", part);
    }

    return start;
}

JS_EXPORT_API
double installer_get_partition_length (const gchar *part)
{
    double length = 0;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition length:part NULL\n");
        return length;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        length = pedpartition->geom.length;

    } else {
        g_warning ("get partition length:find pedpartition %s failed\n", part);
    }

    return length;
}

JS_EXPORT_API
double installer_get_partition_end (const gchar *part)
{
    double end = 0;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition end:part NULL\n");
        return end;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        end = pedpartition->geom.end;
    } else {
        g_warning ("get partition end:find pedpartition %s failed\n", part);
    }

    return end;
}

JS_EXPORT_API
gchar *installer_get_partition_fs (const gchar *part)
{
    gchar *fs = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition fs:part NULL\n");
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
        g_warning ("get partition fs:find pedpartition %s failed\n", part);
    }
    if (fs != NULL) {
        g_debug ("get partition fs:fs is %s\n", fs);
        if (g_strrstr (fs, "swap") != NULL) {
            return g_strdup ("swap");
        }
    } 

    return fs;
}

JS_EXPORT_API 
gchar* installer_get_partition_label (const gchar *part)
{
    gchar *label = NULL;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    if (part == NULL) {
        g_warning ("get partition label:part NULL\n");
        return NULL;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("get partition label:find pedpartition %s failed\n", part);
        return NULL;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("get partition label:get part %s path failed\n", part);
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
gboolean installer_get_partition_busy (const gchar *part)
{
    gboolean busy = FALSE;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition busy:part NULL\n");
        return busy;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        if ((ped_partition_is_busy (pedpartition)) == 1) { 
            busy = TRUE;
        }
    } else {
        g_warning ("get partition busy:find pedpartition %s failed\n", part);
    }

    return busy;
}

JS_EXPORT_API 
gboolean installer_get_partition_flag (const gchar *part, const gchar *flag_name)
{
    gboolean result = FALSE;
    PedPartition *pedpartition = NULL;
    PedPartitionFlag flag;
    if (part == NULL || flag_name == NULL) {
        g_warning ("get partition flag:part or flag name NULL\n");
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("get partition flag:find pedpartition %s failed\n", part);
        goto out;
    }
    if (!ped_partition_is_active (pedpartition)) {
        g_warning ("get partition flag: ped partition flag not active\n");
        goto out;
    }
    flag = ped_partition_flag_get_by_name (flag_name);
    if (flag == 0) {
        g_warning ("get partition flag: ped partition flag get by name failed\n");
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
    GRAB_CTX ();
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition free:part NULL\n");
        return;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        PedPartitionType part_type = pedpartition->type;

        if (part_type != PED_PARTITION_NORMAL && part_type != PED_PARTITION_LOGICAL && part_type != PED_PARTITION_EXTENDED) {
            g_printf ("get partition free:no meaning for none used\n");
            return;
        }

        const gchar *fs = NULL;
        PedGeometry *geom = ped_geometry_duplicate (&pedpartition->geom);
        PedFileSystemType *fs_type = ped_file_system_probe (geom);
        if (fs_type != NULL) {
            fs = fs_type->name;
        }
        ped_geometry_destroy (geom);
        if (fs == NULL) {
            g_warning ("get partition free:get partition file system failed\n");
            return;
        }

        gchar *path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            struct FsHandler *handler = g_new0 (struct FsHandler, 1);
            handler->path = g_strdup (path);
            handler->part = g_strdup (part);
            handler->fs = g_strdup (fs);
            GThread *thread = g_thread_new ("get_partition_free", 
                                            (GThreadFunc) get_partition_free, 
                                            (gpointer) handler);
            g_thread_unref (thread);
        } else {
            g_warning ("get pedpartition free: get %s path failed\n", part);
        }
        g_free (path);

    } else {
        g_warning ("get partition free:find pedpartition %s failed\n", part);
    }
    UNGRAB_CTX ();
}

JS_EXPORT_API 
gchar* installer_get_partition_os (const gchar *part)
{
    gchar* result = NULL;
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition os:part NULL\n");
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            result = g_strdup (g_hash_table_lookup (partition_os, path));

        } else {
            g_warning ("get pedpartition os: get %s path failed\n", part);
        }
        g_free (path);

    } else {
        g_warning ("get partition os:find pedpartition %s failed\n", part);
    }

    return result;
}

JS_EXPORT_API 
gchar* installer_get_partition_os_desc (const gchar *part)
{
    gchar* result = NULL;
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;
    if (part == NULL) {
        g_warning ("get partition os desc:part NULL\n");
        return result;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        path = ped_partition_get_path (pedpartition);
        if (path != NULL) {
            result = g_strdup (g_hash_table_lookup (partition_os_desc, path));

        } else {
            g_warning ("get pedpartition os desc: get %s path failed\n", part);
        }
        g_free (path);

    } else {
        g_warning ("get partition os desc:find pedpartition %s failed\n", part);
    }

    return result;
}

gboolean 
handle_new_disk_partition (const gchar *part_uuid, const gchar *disk, const gchar *type, const gchar *fs, double start, double end)
{
    gboolean ret = FALSE;

    PedDisk *peddisk = NULL;
    PedPartition *pedpartition = NULL;
    PedPartitionType part_type;
    const PedFileSystemType *part_fs = NULL;
    PedSector part_start;
    PedSector part_end;
    PedGeometry *pedgeometry = NULL;
    PedConstraint *pedconstraint = NULL;

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    if (peddisk == NULL) {
        g_warning ("new disk partition:find peddisk %s failed\n", disk);
        goto out;
    }
    if (g_strcmp0 (type, "normal") == 0) {
        part_type = PED_PARTITION_NORMAL;
    } else if (g_strcmp0 (type, "logical") == 0) {
        part_type = PED_PARTITION_LOGICAL;
    } else if (g_strcmp0 (type, "extended") == 0) {
        part_type = PED_PARTITION_EXTENDED;
    } else {
        g_warning("new disk partition:invalid partition type %s\n", type);
        goto out;
    }

    if (part_type != PED_PARTITION_EXTENDED) {
        part_fs = ped_file_system_type_get (fs);
        if (part_fs == NULL) {
            g_warning("new disk partition:ped file system type get for %s is NULL\n", fs);
        }
    }
    part_start = (PedSector) start;
    part_end = (PedSector) end;

    pedpartition = ped_partition_new (peddisk, part_type, part_fs, part_start, part_end);
    if (pedpartition == NULL) {
        g_warning ("new disk partition:new partition failed\n");
        goto out;
    }
    pedgeometry = ped_geometry_new (peddisk->dev, part_start, part_end - part_start + 1);
    if (pedgeometry == NULL) {
        g_warning ("new disk partition:new geometry failed\n");
        goto out;
    }
    pedconstraint = ped_constraint_new_from_max (pedgeometry);
    if (pedconstraint == NULL) {
        g_warning ("new disk partitoin:new constraint failed\n");
        goto out;
    }
    if (ped_disk_add_partition (peddisk, pedpartition, pedconstraint) == 0 ) {
        g_warning ("new disk partition:add disk partition failed\n");
        goto out;
    }
    g_hash_table_insert (partitions,  g_strdup (part_uuid), pedpartition);
    ret = TRUE;
    goto out;

out:
    if (pedconstraint != NULL) {
        ped_constraint_destroy (pedconstraint);
    }
    if (pedgeometry != NULL) {
        ped_geometry_destroy (pedgeometry);
    }
    return ret;
}

JS_EXPORT_API 
gboolean installer_new_disk_partition (const gchar *part_uuid, const gchar *disk, const gchar *type, const gchar *fs, double start, double end)
{
    if (part_uuid == NULL || disk == NULL || type == NULL || fs == NULL) {
        g_warning ("new disk partition:some param NULL\n");
        return FALSE;
    }

    if (op_queue == NULL) {
        op_queue = g_async_queue_new ();
    }
    g_mutex_lock (&op_mutex);
    op_count += 1;
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (table, "op", g_strdup ("new_part"));
    g_hash_table_insert (table, "part", g_strdup (part_uuid));
    g_hash_table_insert (table, "disk", g_strdup (disk));
    g_hash_table_insert (table, "type", g_strdup (type));
    g_hash_table_insert (table, "fs", g_strdup (fs));
    g_hash_table_insert (table, "start", GINT_TO_POINTER (start));
    g_hash_table_insert (table, "end", GINT_TO_POINTER (end));
    g_async_queue_push (op_queue, (gpointer) table);
    g_mutex_unlock (&op_mutex);
    return TRUE;
}

gboolean handle_delete_disk_partition (const gchar *disk, const gchar *part)
{
    gboolean ret = FALSE;
    PedDisk *peddisk = NULL;
    PedPartition *pedpartition = NULL;

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    
    if (peddisk != NULL && pedpartition != NULL && pedpartition->disk == peddisk) {

        if ((ped_disk_delete_partition (peddisk, pedpartition) != 0)) {
            g_printf("delete disk partition:ok\n");
            ret = TRUE;
        } else {
            g_warning ("delete disk partition:failed\n");
        }
    } else {
        g_warning ("delete disk partition:find peddisk %s failed\n", disk);
    }

    return ret;
}

JS_EXPORT_API 
gboolean installer_delete_disk_partition (const gchar *disk, const gchar *part)
{
    if (disk == NULL || part == NULL) {
        g_warning ("delete disk partition:some param NULL\n");
        return FALSE;
    }
    if (op_queue == NULL) {
        op_queue = g_async_queue_new ();
    }
    g_mutex_lock (&op_mutex);
    op_count += 1;
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (table, "op", g_strdup ("delete_part"));
    g_hash_table_insert (table, "disk", g_strdup (disk));
    g_hash_table_insert (table, "part", g_strdup (part));
    g_async_queue_push (op_queue, (gpointer) table);
    g_mutex_unlock (&op_mutex);
    return TRUE;
}

gboolean 
handle_update_partition_geometry (const gchar *part, double start, double length) 
{
    gboolean ret = FALSE;
    PedPartition *pedpartition = NULL;
    PedGeometry *geom = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        geom = &pedpartition->geom;
        if (geom != NULL) {
            ped_geometry_set (geom, (PedSector) start, (PedSector) length);
            ret = TRUE;
        }
    } else {
        g_warning ("update partition geometry:find pedpartition %s failed\n", part);
    }

    return ret;
}

JS_EXPORT_API 
gboolean installer_update_partition_geometry (const gchar *part, double start, double length) 
{
    if (part == NULL) {
        g_warning ("update partition geometry:part NULL\n");
        return FALSE;
    }
    if (op_queue == NULL) {
        op_queue = g_async_queue_new ();
    }
    g_mutex_lock (&op_mutex);
    op_count += 1;
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (table, "op", g_strdup ("update_geometry"));
    g_hash_table_insert (table, "part", g_strdup (part));
    g_hash_table_insert (table, "start", GINT_TO_POINTER (start));
    g_hash_table_insert (table, "length", GINT_TO_POINTER (length));
    g_async_queue_push (op_queue, (gpointer) table);
    g_mutex_unlock (&op_mutex);
    return TRUE;
}

gboolean handle_update_partition_fs (const gchar *part, const gchar *fs)
{
    gboolean ret = FALSE;

    PedPartition *pedpartition = NULL;
    const PedFileSystemType *part_fs_type = NULL;
    const gchar *part_path = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        if (g_strcmp0 (fs, "efi") == 0) {
            part_fs_type = ped_file_system_type_get ("fat32");
        } else if (g_strcmp0 (fs, "swap") == 0) {
            part_fs_type = ped_file_system_type_get ("linux-swap");
        } else {
            part_fs_type = ped_file_system_type_get (fs);
        }
        if (part_fs_type != NULL) {
            if ((ped_partition_set_system (pedpartition, part_fs_type)) == 0) {
                g_warning ("update partition fs: ped partition set system %s failed\n", fs);
                return ret;
            }
        } else {
            g_warning ("update partition fs:get part fs type %s failed\n", fs);
            return ret;
        }

        part_path = ped_partition_get_path (pedpartition);
        if (part_path != NULL) {
            if (g_strcmp0 (fs, "efi") == 0) {
                set_partition_filesystem (part_path, "fat32");
                if (! installer_set_partition_flag (part, "boot", 1)) {
                    g_warning ("set flag for uefi failed\n");
                }
            } else {
                set_partition_filesystem (part_path, fs);
            }
            ret = TRUE;
        } else {
            g_warning ("update partition fs:don't know the partition path\n");
        }
    } else {
        g_warning ("update partition fs:find pedpartition %s failed\n", part);
    }

    return ret;
}

JS_EXPORT_API
gboolean installer_update_partition_fs (const gchar *part, const gchar *fs)
{
    if (part == NULL || fs == NULL) {
        g_warning ("update partition fs:part or fs NULL\n");
        return FALSE;
    }
    if (op_queue == NULL) {
        op_queue = g_async_queue_new ();
    }
    g_mutex_lock (&op_mutex);
    op_count += 1;
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (table, "op", g_strdup ("update_fs"));
    g_hash_table_insert (table, "part", g_strdup (part));
    g_hash_table_insert (table, "fs", g_strdup (fs));
    g_async_queue_push (op_queue, (gpointer) table);
    g_mutex_unlock (&op_mutex);
    return TRUE;
}

gboolean handle_write_disk (const gchar *disk)
{
    gboolean ret = FALSE;
    PedDisk *peddisk = NULL;

    peddisk = (PedDisk *) g_hash_table_lookup (disks, disk);
    if (peddisk != NULL) {

        if ((ped_disk_commit_to_dev (peddisk)) == 0) {
            g_warning ("write disk:commit to dev failed\n");
            return ret;
        }
        if ((ped_disk_commit_to_os (peddisk)) == 0) {
            g_warning ("write disk:commit to dev failed\n");
            return ret;
        }

        g_spawn_command_line_async ("sync", NULL);
        ret = TRUE;
        g_debug ("write disk:%s succeed\n", disk);

    } else {
        g_warning ("write disk:find peddisk %s failed\n", disk);
    }

    return ret;
}

JS_EXPORT_API 
gboolean installer_write_disk (const gchar *disk)
{
    if (disk == NULL) {
        g_warning ("write disk:disk NULL\n");
        return FALSE;
    }
    if (op_queue == NULL) {
        op_queue = g_async_queue_new ();
    }
    g_mutex_lock (&op_mutex);
    op_count += 1;
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (table, "op", g_strdup ("write_disk"));
    g_hash_table_insert (table, "disk", g_strdup (disk));
    g_async_queue_push (op_queue, (gpointer) table);
    g_mutex_unlock (&op_mutex);
    return TRUE;
}

static gpointer 
handle_part_operation_thread (gpointer data)
{
    guint i;
    for (i = 0; i < op_count; i++) {
        g_mutex_lock (&op_mutex);
        GHashTable *table  = (GHashTable *) g_async_queue_pop (op_queue);
        if (table == NULL) {
            g_warning ("pop table NULL\n");
        }
        gchar *op = (gchar *) g_hash_table_lookup (table, "op");

        if (g_strcmp0 ("delete_part", op) == 0) {
            gchar *disk = (gchar *) g_hash_table_lookup (table, "disk");
            gchar *part = (gchar *) g_hash_table_lookup (table, "part");
            g_warning ("-----------delete disk->%s partition->%s-----------\n", disk, part);
            handle_delete_disk_partition (disk, part);
            g_free (disk);
            g_free (part);

        } else if (g_strcmp0 ("update_fs", op) == 0) {
            gchar *part = (gchar *) g_hash_table_lookup (table, "part");
            gchar *fs = (gchar *) g_hash_table_lookup (table, "fs");
            g_warning ("-----------update part->%s fs->%s-----------\n", part, fs);
            handle_update_partition_fs (part, fs);
            g_free (part);
            g_free (fs);

        } else if (g_strcmp0 ("update_geometry", op) == 0) { 
            gchar *part = (gchar *) g_hash_table_lookup (table, "part");
            double start = (double) GPOINTER_TO_INT (g_hash_table_lookup (table, "start"));
            double length = (double) GPOINTER_TO_INT (g_hash_table_lookup (table, "length"));
            g_warning ("-----------update part->%s geometry start->%f length->%f\n------------", part, start, length);
            handle_update_partition_geometry (part, start, length);
            g_free (part);

        } else if (g_strcmp0 ("new_part", op) == 0) {
            gchar *disk = (gchar *) g_hash_table_lookup (table, "disk");
            gchar *part = (gchar *) g_hash_table_lookup (table, "part");
            gchar *type = (gchar *) g_hash_table_lookup (table, "type");
            gchar *fs = (gchar *) g_hash_table_lookup (table, "fs");
            double start = (double) GPOINTER_TO_INT (g_hash_table_lookup (table, "start"));
            double end = (double) GPOINTER_TO_INT (g_hash_table_lookup (table, "end"));
            g_warning ("------------new part:disk->%s part->%s type->%s fs->%s start->%f end->%f------------\n", disk, part, type, fs, start, end);
            handle_new_disk_partition (part, disk, type, fs, start, end);
            g_free (disk);
            g_free (part);
            g_free (type);
            g_free (fs);
            
        } else if (g_strcmp0 ("write_disk", op) == 0) {
            gchar *disk = (gchar *) g_hash_table_lookup (table, "disk");
            g_warning ("write disk->%s\n", disk);
            handle_write_disk (disk);
            g_free (disk);

        } else {
            g_warning ("unkonw op:%s\n", op);
        }
        g_free (op);
        g_hash_table_destroy (table);
        g_mutex_unlock (&op_mutex);
        if (i == op_count - 1) {
            GRAB_CTX ();
            js_post_message ("part_operation", NULL);
            UNGRAB_CTX ();
        }
    }
}

JS_EXPORT_API 
void installer_start_part_operation ()
{
    GThread *handle_thread = g_thread_new ("handle_operation", (GThreadFunc) handle_part_operation_thread, NULL);
    g_thread_unref (handle_thread);
}

//call after chroot
JS_EXPORT_API 
gboolean installer_write_partition_mp (const gchar *part, const gchar *mp)
{
    gboolean ret = FALSE;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    gchar *fs = NULL;
    gchar *uuid = NULL;
    gchar *fsname = NULL;
    gchar *mount_cmd = NULL;
    gchar *comment_line = NULL;
    PedGeometry *geom = NULL;
    PedFileSystemType *fs_type = NULL;
    struct mntent mnt;
    FILE *mount_file = NULL;
    static gboolean header_inited = FALSE;

    if (part == NULL || mp == NULL) {
        g_warning ("write fs tab:part or mount point is NULL\n");
        goto out;
    }

    if (!header_inited) {
        const gchar *contents = "# /etc/fstab: static file system information.\n"                               \
                                "#\n"                                                                           \
                                "# Use 'blkid' to print the universally unique identifier for a\n"              \
                                "# device; this may be used with UUID= as a more robust way to name devices\n"  \
                                "# that works even if disks are added and removed. See fstab(5).\n"             \
                                "#\n"                                                                           \
                                "# <file system> <mount point>   <type>  <options>       <dump>  <pass>\n"; 
        g_file_set_contents ("/etc/fstab", contents, -1, NULL);
        header_inited = TRUE;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("write fs tab:find pedpartition %s failed\n", part);
        goto out;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("write fs tab:get partition %s path failed\n", part);
        goto out;
    }

    geom = ped_geometry_duplicate (&pedpartition->geom);
    fs_type = ped_file_system_probe (geom);
    if (fs_type == NULL) {
        g_warning ("write fs tab:probe filesystem failed\n");
        goto out;
    }
    fs = g_strdup (fs_type->name);
    if (fs == NULL) {
        g_warning ("write fs tab:get partition %s fs failed\n", part);
        goto out;
    }
    uuid = get_partition_uuid (path);
    if (uuid == NULL) {
        g_warning ("write fs tab:uuid NULL\n");
        goto out;
    }
    fsname = g_strdup_printf ("UUID=%s", g_strstrip(uuid));

    mount_file = setmntent ("/etc/fstab", "a");
    if (mount_file == NULL) {
        g_warning ("write fs tab: setmntent failed\n");
        goto out;
    }
    mnt.mnt_fsname = fsname;
    mnt.mnt_dir = g_strdup (mp);
    mnt.mnt_type = fs;
    mnt.mnt_opts = "defaults";
    mnt.mnt_freq = 0;
    mnt.mnt_passno = 2;
    if (g_strcmp0 ("/", mp) == 0) {
        if (g_strcmp0 (fs, "btrfs") != 0) {
            mnt.mnt_opts = "errors=remount-ro";
        }
        mnt.mnt_passno = 1;
    } else if (g_strcmp0 ("swap", mp) == 0 || g_strcmp0 ("linux-swap", fs) == 0) {
        mnt.mnt_dir = "none";
        mnt.mnt_type = "swap";
        mnt.mnt_opts = "sw";
        mnt.mnt_passno = 0;
    } else if (!g_str_has_prefix (mp, "/")) {
        g_warning ("write fs tab:invalid mp->%s\n", mp);
        goto out;
    }

    if (!g_file_test (mp, G_FILE_TEST_EXISTS)) {
        g_mkdir_with_parents (mp, 0755);
    }

    if (g_strcmp0 ("fat16", fs) == 0 || g_strcmp0 ("fat32", fs) == 0) {
        mnt.mnt_type = "vfat";
    } else if (g_strcmp0 ("ntfs", fs) == 0) {
        mnt.mnt_type = "ntfs-3g";
    }

    comment_line = g_strdup_printf ("# %s was on %s during installation\n", mp, path);
    gchar *p = comment_line;
    size_t s = 0;
    while(*p != '\0') {
        p++;
        s++;
    }
    if (fwrite (comment_line, 1, s, mount_file) != s) {
        g_warning ("write fs tab: fwrite %s failed\n", comment_line);
    }

    if ((addmntent(mount_file, &mnt)) != 0) {
        g_warning ("write fs tab: addmntent failed %s\n", strerror (errno));
        goto out;
    }
    fflush (mount_file);
    ret = TRUE;
    goto out;

out:
    g_free (path);
    g_free (fs);
    g_free (uuid);
    g_free (fsname);
    g_free (mount_cmd);
    g_free (comment_line);
    if (geom != NULL) {
        ped_geometry_destroy (geom);
    }
    if (mount_file != NULL) {
        endmntent (mount_file);
    }
    if (!ret) {
        //write fs tab stage goes together with timezone
        emit_progress ("timezone", "terminate");
    }
    return ret;
}

JS_EXPORT_API 
gboolean installer_set_partition_flag (const gchar *part, const gchar *flag_name, gboolean status)
{
    gboolean ret = FALSE;
    PedPartition *pedpartition = NULL;
    PedPartitionFlag flag;
    if (part == NULL || flag_name == NULL) {
        g_warning ("set partition flag:part or flag NULL\n");
        return ret;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        flag = ped_partition_flag_get_by_name (flag_name);
        if (ped_partition_is_flag_available (pedpartition, flag)) {
            ped_partition_set_flag (pedpartition, flag, status);
            ret = TRUE;
        } else {
            g_warning ("set partition flag:flag %s is not available\n", flag_name);
        }
    } else {
        g_warning ("set partition flag:find pedpartition %s failed\n", part);
    }

    return ret;
}

JS_EXPORT_API
gboolean installer_mount_partition (const gchar *part, const gchar *mp)
{
    gboolean result = FALSE;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    gchar *mount_target = NULL;
    gchar *cmd = NULL;
    gchar *fs = NULL;
    GError *error = NULL;

    if (part == NULL || mp == NULL) {
        g_warning ("installer mount partition:part or mp NULL\n");
        goto out;
    }

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("mount partition:pedpartition for %s NULL", part);
        goto out;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("mount partition:%s path is NULL\n", part);
        goto out;
    }

    fs = installer_get_partition_fs (part);
    if (fs == NULL) {
        g_warning ("mount partition:partition fs NULL\n");
        goto out;
    } else if (g_strcmp0 ("fat16", fs) == 0 || g_strcmp0 ("fat32", fs) == 0) {
        g_free (fs);
        fs = g_strdup ("vfat");
    } else if (g_strcmp0 ("ntfs", fs) == 0) {
        g_free (fs);
        fs = g_strdup ("ntfs-3g");
    }

    mount_target = g_strdup_printf ("%s%s", TARGET, mp);
    if (g_mkdir_with_parents (mount_target, 0755) == -1) {
        g_warning ("init parted:create target directory failed\n");
        goto out;
    }

    guint before = get_mount_target_count (mount_target);
    cmd = g_strdup_printf ("mount -t %s %s %s", fs, path, mount_target);
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);


    {
	g_mkdir_with_parents ("/target/host/cdrom", 0755);
	g_spawn_command_line_sync ("mount --bind / /target/host", NULL, NULL, NULL, &error);
	if (error != NULL) {
	    g_warning("Can't bind host: %s\n", error->message);
	    g_error_free(error);
	    error = NULL;
	}

	g_mkdir_with_parents ("/target/media/cdrom", 0755);
	g_spawn_command_line_sync("mount --bind /cdrom  /target/media/cdrom", NULL, NULL, NULL, &error);
	if (error != NULL) {
	    g_warning("Can't bind cdrom: %s\n", error->message);
	    g_error_free(error);
	    error = NULL;
	}
    }


    if (error != NULL) {
        g_warning ("mount partition:mount path %s with fs %s error:%s\n", path, fs, error->message);
        goto out;
    }

    guint after = get_mount_target_count (mount_target);
    if (after != before + 1) {
        g_warning ("mount partition:mount path %s wifth fs %s count from %d to %d\n", path, fs, before, after);
    } else {
        mounted_list = g_list_append (mounted_list, g_strdup (mount_target));
        result = TRUE;
    }
    goto out;
out:
    g_free (path);
    g_free (mount_target);
    g_free (cmd);
    g_free (fs);
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
    if ((!result) && (g_strcmp0 ("/", mp) == 0)) {
        emit_progress ("extract", "terminate");
    }
    return result;
}

//when dectect mount partition, tell user to unmount them
JS_EXPORT_API
void installer_unmount_partition (const gchar *part)
{
    if (part == NULL) {
        g_warning ("unmount partition:part NULL\n");
        return;
    }
    gchar *path = installer_get_partition_path (part);
    unmount_partition_by_device (path);
    g_free (path);
}

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

#include "part_util.h"
#include "fs_util.h"

#define PART_INFO_LENGTH 4096

static GHashTable *disks;
static GHashTable *partitions;
static GHashTable *disk_partitions;
static GHashTable *partition_os = NULL;
const gchar *target;

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

void init_parted ()
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

    partition_os = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                          (GEqualFunc) g_str_equal, 
                                          (GDestroyNotify) g_free, 
                                          (GDestroyNotify) g_free);

    GError *error = NULL;
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

    if (g_find_program_in_path ("os-prober") == NULL) {
        g_warning ("os:os-prober not installed\n");
    }

    gchar *output = NULL;
    g_spawn_command_line_sync ("os-prober", &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition os:os-prober %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    gchar **items = g_strsplit (output, "\n", -1);
    int i, j;
    for (i = 0; i < g_strv_length (items); i++) {
        gchar *item = g_strdup (items[i]);
        gchar **os = g_strsplit (item, ":", -1);

        if (g_strv_length (os) == 4) {
            //g_printf ("get partition os:insert key %s value %s\n", os[0], os[2]);
            g_hash_table_insert (partition_os, g_strdup (os[0]), g_strdup (os[2]));
        }

        g_strfreev (os);
        g_free (item);
    }

    g_strfreev (items);
    g_free (output);
}

JS_EXPORT_API 
JSObjectRef installer_list_disks()
{
    JSObjectRef array = json_array_create ();
    int i;

    g_assert (disks != NULL);
    GList *disk_keys = g_hash_table_get_keys (disks);
    g_assert (disk_keys != NULL);

    for (i = 0; i < g_list_length (disk_keys); i++) {
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), g_list_nth_data (disk_keys, i)));
    }

    return array;
}

JS_EXPORT_API
gchar *installer_get_disk_path (const gchar *disk)
{
    gchar *result = NULL;

    PedDisk *peddisk = NULL;

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
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
gchar *installer_get_disk_model (const gchar *disk)
{
    gchar *result = NULL;

    PedDisk *peddisk = NULL;

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        g_assert(device != NULL);

        result = g_strdup (device->model);

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

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        g_assert(device != NULL);

        length = device->length;
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

    peddisk = (PedDisk *) g_hash_table_lookup(disks, disk);
    if (peddisk != NULL) {
        PedDevice *device = peddisk->dev;
        g_assert(device != NULL);

        sector_size = device->sector_size;
    } else {
        g_warning ("get disk sector size:find peddisk by %s failed\n", disk);
    }
    
    return sector_size;
}

JS_EXPORT_API
JSObjectRef installer_get_disk_partitions (const gchar *disk)
{
    JSObjectRef array = json_array_create ();
    int i;

    GList *parts =  NULL;
    parts = (GList *) g_hash_table_lookup (disk_partitions, disk);
   
    if (parts != NULL) {
        for (i = 0; i < g_list_length (parts); i++) {
            json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), g_list_nth_data (parts, i)));
        }
    }

    return array;
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
            case PED_PARTITION_METADATA:
                type = g_strdup ("metadata");
                break;
            case PED_PARTITION_FREESPACE:
                type = g_strdup ("freespace");
                break;
            case PED_PARTITION_PROTECTED:
                type = g_strdup ("protected");
                break;
            default:
                //g_warning("get partition type:invalid type %d\n", part_type);
                type = g_strdup ("protected");
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

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("get partition mp:find pedpartition %s failed\n", part);
        goto out;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("get partition mp:get partition path failed\n");
        goto out;
    }
    mp = g_strdup (get_partition_mount_point (path));
    goto out;

out:
    g_free (path);
    return mp;
}

JS_EXPORT_API
double installer_get_partition_start (const gchar *part)
{
    double start = 0;
    PedPartition *pedpartition = NULL;

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

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        end = pedpartition->geom.end;

    } else {
        g_warning ("get partition end:find pedpartition %s failed\n", part);
    }

    return end;
}

JS_EXPORT_API
const gchar *installer_get_partition_fs (const gchar *part)
{
    const gchar *fs;
    PedPartition *pedpartition = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {
        PedGeometry *geom = ped_geometry_duplicate (&pedpartition->geom);
        PedFileSystemType *fs_type = ped_file_system_probe (geom);
        if (fs_type != NULL) {
            fs = fs_type->name;
        }
        ped_geometry_destroy (geom);

    } else {
        g_warning ("get partition fs:find pedpartition %s failed\n", part);
    }

    return fs;
}

JS_EXPORT_API 
gchar* installer_get_partition_label (const gchar *part)
{
    gchar *label = NULL;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    GError *error = NULL;
    gchar **tmp = NULL;
    gchar *device = NULL;
    const gchar *object_path = NULL;
    GDBusProxy *proxy = NULL;
    GVariant *label_var = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("get partition label:find pedpartition %s failed\n", part);
        goto out;
    }
    path = ped_partition_get_path (pedpartition);
    if (path == NULL) {
        g_warning ("get partition label:get part %s path failed\n", part);
        goto out;
    }
    tmp = g_strsplit (path, "/", 3);
    device = g_strdup (tmp[2]);
    object_path = g_strdup_printf ("/org/freedesktop/UDisks2/block_devices/%s", device);
    if (!g_variant_is_object_path (object_path)) {
        g_warning ("get partition label:object path invalid %s\n", object_path);
        goto out;
    }
    proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           NULL,
                                           "org.freedesktop.UDisks2",
                                           object_path,
                                           "org.freedesktop.UDisks2.Block",
                                           NULL,
                                           &error);
    if (error != NULL) {
        g_warning ("get partition label: dbus proxy %s\n", error->message);
        goto out;
    }
    label_var = g_dbus_proxy_get_cached_property (proxy, "IdLabel");
    if (error != NULL) {
        g_warning ("get partition label:get property IdLabel %s\n", error->message);
        goto out;
    }
    label = g_variant_dup_string (label_var, NULL); 
    goto out;

out:
    g_free (path);
    g_strfreev (tmp);
    g_free (device);
    g_free ((gchar *) object_path);
    if (label_var != NULL) {
        g_variant_unref (label_var);
    }
    if (proxy != NULL) {
        g_object_unref (proxy);
    }
    if (error != NULL) {
        g_error_free (error);
    }
    return label;
}

JS_EXPORT_API 
gboolean installer_get_partition_busy (const gchar *part)
{
    gboolean busy = FALSE;
    PedPartition *pedpartition = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        if ((ped_partition_is_busy (pedpartition)) == 1) { 
            busy = TRUE;
        }
    } else {
        g_warning ("get partition busy:find pedpartition %s failed\n", part);
        busy = TRUE;
    }

    return busy;
}

JS_EXPORT_API 
gboolean installer_get_partition_flag (const gchar *part, const gchar *flag_name)
{
    gboolean result = FALSE;
    PedPartition *pedpartition = NULL;
    PedPartitionFlag flag;

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
    PedPartition *pedpartition = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);

    if (pedpartition != NULL) {
        PedPartitionType part_type = pedpartition->type;

        if (part_type != PED_PARTITION_NORMAL && part_type != PED_PARTITION_LOGICAL && part_type != PED_PARTITION_EXTENDED) {
            g_printf ("get partition free:no meaning for none used\n");
            return ;
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
            return ;
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

        } else {
            g_warning ("get pedpartition free: get %s path failed\n", part);
        }
        g_free (path);

    } else {
        g_warning ("get partition free:find pedpartition %s failed\n", part);
    }
}

JS_EXPORT_API 
gchar* installer_get_partition_os (const gchar *part)
{
    gchar* result = NULL;
    gchar *path = NULL;
    PedPartition *pedpartition = NULL;

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
gboolean installer_new_disk_partition (const gchar *part_uuid, const gchar *disk, const gchar *type, const gchar *fs, double start, double end)
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
    if (ped_disk_add_partition (peddisk, pedpartition, pedconstraint) != 0 ) {
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
gboolean installer_delete_disk_partition (const gchar *disk, const gchar *part)
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
gboolean installer_update_partition_fs (const gchar *part, const gchar *fs)
{
    gboolean ret = FALSE;

    PedPartition *pedpartition = NULL;
    const PedFileSystemType *part_fs_type = NULL;
    const gchar *part_path = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition != NULL) {

        part_fs_type = ped_file_system_type_get (fs);
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
            set_partition_filesystem (part_path, fs);
            g_printf ("update partition fs:ok\n");
            ret = TRUE;
        } else {
            g_warning ("update partition fs:don't know the partition path\n");
        }
    } else {
        g_warning ("update partition fs:find pedpartition %s failed\n", part);
    }

    return ret;
}

//call after chroot
JS_EXPORT_API 
gboolean installer_write_partition_mp (const gchar *part, const gchar *mp)
{
    gboolean ret = FALSE;
    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    gchar *fs = NULL;
    PedGeometry *geom = NULL;
    PedFileSystemType *fs_type = NULL;
    struct mntent mnt;
    FILE *mount_file = NULL;

    if (mp == NULL) {
        g_warning ("write fs tab:mount point is NULL\n");
        goto out;
    }
    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("write fs tab:find pedpartition %s failed\n", part);
        goto out;
    }
    path = g_strdup (ped_partition_get_path (pedpartition));
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
    mount_file = setmntent ("/etc/fstab", "a");
    
    if (mount_file == NULL) {
        g_warning ("write fs tab: setmntent failed\n");
        goto out;
    }
    mnt.mnt_fsname = path;
    mnt.mnt_dir = g_strdup (mp);
    mnt.mnt_type = fs;
    mnt.mnt_opts = "defaults";
    mnt.mnt_freq = 0;
    mnt.mnt_passno = 2;
    if (g_strcmp0 ("/", mp) == 0) {
        mnt.mnt_opts = "errors=remount-ro";
        mnt.mnt_passno = 1;
    } else if (g_strcmp0 ("swap", mp) == 0 || g_strcmp0 ("linux-swap", fs) == 0) {
        mnt.mnt_type = "swap";
        mnt.mnt_opts = "sw,pri=1";
        mnt.mnt_passno = 0;
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
    if (geom != NULL) {
        ped_geometry_destroy (geom);
    }
    if (mount_file != NULL) {
        endmntent (mount_file);
    }
    if (ret) {
        emit_progress ("chroot", "finish");
    } else {
        emit_progress ("chroot", "terminate");
    }
    return ret;
}

//used for bios_grub when install grub to gpt disk with bios
JS_EXPORT_API 
gboolean installer_set_partition_flag (const gchar *part, const gchar *flag_name, gboolean status)
{
    gboolean ret = FALSE;

    PedPartition *pedpartition = NULL;
    PedPartitionFlag flag;

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
gboolean installer_write_disk (const gchar *disk)
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
gboolean installer_mount_target (const gchar *part)
{
    gboolean result = FALSE;

    PedPartition *pedpartition = NULL;
    gchar *path = NULL;
    gchar *target_uuid = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    pedpartition = (PedPartition *) g_hash_table_lookup (partitions, part);
    if (pedpartition == NULL) {
        g_warning ("installer mount target:pedpartition for %s NULL", part);
        goto out;
    }
    path = g_strdup (ped_partition_get_path (pedpartition));
    if (path == NULL) {
        g_warning ("mount target:%s path is NULL\n", part);
        goto out;
    }

    target_uuid = installer_rand_uuid ();
    target = g_strdup_printf ("/mnt/target%s", target_uuid);
    if (g_file_test (target, G_FILE_TEST_EXISTS)) {
        g_warning ("mount target:re rand uuid as target exists\n");
        //g_free (target_uuid);
        //target_uuid = NULL;
        //target = NULL;
        target_uuid = installer_rand_uuid ();
        target = g_strdup_printf ("/mnt/target%s", target_uuid);
    }

    if (g_mkdir_with_parents (target, 0777) != -1) {
        cmd = g_strdup_printf ("mount %s %s", path, target);
        g_spawn_command_line_async (cmd, &error);
        if (error != NULL) {
            g_warning ("mount target:mount failed %s\n", error->message);
            goto out;
        } 
        result = TRUE;
    }
    goto out;

out:
    g_free (path);
    g_free (target_uuid);
    g_free (cmd);
    if (error != NULL) {
        g_error_free (error);
    }
    if (!result) {
        emit_progress ("extract", "terminate");
    }
    return result;
}

JS_EXPORT_API 
gboolean installer_update_grub (const gchar *uuid)
{
    gboolean ret = FALSE;
    gchar *path = NULL;
    gchar *grub_install = NULL;
    GError *error = NULL;

    if (g_str_has_prefix (uuid, "disk")) {
        path = installer_get_disk_path (uuid);
    } else if (g_str_has_prefix (uuid, "part")) {
        path = installer_get_partition_path (uuid);
    } else {
        g_warning ("update grub:invalid uuid %s\n", uuid);
        goto out;
    }

    grub_install = g_strdup_printf ("grub-install --no-floppy --force %s", path);
    g_spawn_command_line_sync (grub_install, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("update grub:grub-install %s\n", error->message);
        goto out;
    }

    g_spawn_command_line_sync ("update-grub", NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("update grub:update grub %s\n", error->message);
        goto out;
    }
    ret = TRUE;
    goto out;

out:
    g_free (path);
    g_free (grub_install);
    if (error != NULL) {
        g_error_free (error);
    }
    if (ret) {
        emit_progress ("grub", "finish");
    } else {
        emit_progress ("grub", "terminate");
    }
    return ret;
}

void emit_progress (const gchar *step, const gchar *progress)
{
    js_post_message_simply ("progress", "{\"stage\":\"%s\",\"progress\":\"%s\"}", step, progress);
}

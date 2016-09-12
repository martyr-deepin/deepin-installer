/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "esp.h"
#include "info.h"
#include "ped_utils.h"
#include "fs_util.h"

#include <glib.h>


gboolean  create_esp_by_split(PedPartition* old, char** esp, char** new_part);

PedPartition* must_get_partition(const char* disk_path, const char* part_path)
{
    PedDevice* dev = ped_device_get(disk_path);
    g_assert(dev != NULL);
    PedDisk* disk = ped_disk_new(dev);
    g_assert(disk != NULL);
    PedPartition* part = find_partition(disk, (PartitionFilter)partition_filter_by_path, (gpointer)part_path, NULL);
    g_assert(part != NULL);
    //TODO: duplicate the partition and free disk/dev
    //ped_disk_destroy(disk);
    //ped_device_destroy(dev);
    return part;
}

void ugly_handle_esp()
{
    char* esp = query_esp_path_by_disk_path(InstallerConf.root_disk);
    if (esp == NULL) {
        PedPartition* part = must_get_partition(InstallerConf.root_disk,
                                                InstallerConf.root_partition);
        char* root;
        create_esp_by_split(part, &esp, &root);
        g_assert(esp != NULL);
        installer_record_mountpoint_info(root, "/");
    }
    installer_record_bootloader_info(esp, true);
}

void auto_handle_esp()
{
    g_assert(InstallerConf.simple_mode);

    ugly_handle_esp();


    /*if (!installer_system_support_efi())*/
	/*return;*/

    /*if (!installer_disk_is_gpt(get_target_disk())) {*/
	/*if (target_can_use_all_space()) {*/
	    /*PedPartition* all_space = disk_clobber("gpt");*/
	    /*PedPartition* root = create_esp_by_split(all_space);*/
	    /*PedPartition* esp = detect_esp(get_targeet_disk());*/
	    /*installer_record_bootloader_info(esp, true);*/
	/*}*/
    /*}*/
}

void init_esp_partition(PedPartition* part)
{
    int is_busy = ped_partition_is_busy(part);

    char *path = ped_partition_get_path(part);

    char* mount_point = NULL; 
    GError* error = NULL;

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
            g_warning("[%s] can't detect whether is $ESP : %s", __func__,
                      error->message);
            g_error_free(error);
            error = NULL;
            return;
        }
    }

    if (mount_point == NULL) {
        mount_point = get_partition_mount_point(path);
    }

    char* efi_path = g_build_filename(mount_point, "EFI", NULL);
    g_mkdir(efi_path, 0755);
    g_free(efi_path);

    if (!is_busy) {
        char* cmd = g_strdup_printf ("umount -l %s", mount_point);
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
        g_free(cmd);
        if (error != NULL) {
            g_warning("[%s] can't detect whether is $ESP : %s", __func__,
                      error->message);
            g_error_free(error);
            g_free(mount_point);
            return;
        }

        g_rmdir(mount_point);
        g_free(mount_point);
    }
}

gboolean create_esp_by_split(PedPartition* old, char** esp_path,
                             char** new_path)
{
    PedSector length512 = 512 * 1024 * 1024 / old->disk->dev->sector_size;

    PedSector old_length = old->geom.length;
    g_assert(g_strcmp0("gpt", old->disk->type->name) == 0);
    g_assert(old->geom.length >= length512 * 8  + length512); //at least 4.5GB space
    g_assert(old->type == PED_PARTITION_NORMAL ||
             old->type == PED_PARTITION_FREESPACE);

    if (old->type == PED_PARTITION_NORMAL) {
        ped_disk_remove_partition(old->disk, old);
    }

    PedGeometry geom;
    PedConstraint* cst = NULL;

    ped_geometry_init(&geom, old->disk->dev, old->geom.start, length512);
    cst = ped_constraint_new_from_max(&geom);
    PedPartition* esp_part = ped_partition_new(old->disk,
        PED_PARTITION_NORMAL, ped_file_system_type_get("fat32"),
        geom.start, geom.end);
    ped_partition_set_flag(esp_part, PED_PARTITION_BOOT, 1);
    ped_partition_set_flag(esp_part, PED_PARTITION_HIDDEN, 1);
    ped_partition_set_name(esp_part, "ESP");
    ped_disk_add_partition (old->disk, esp_part, cst);
    ped_disk_commit (old->disk);
    ped_constraint_destroy(cst);

    ped_geometry_init(&geom, old->disk->dev,  esp_part->geom.end + 1,
                      old_length - length512);

    cst = ped_constraint_new_from_max(&geom);
    PedPartition* new_part = ped_partition_new (old->disk,
        PED_PARTITION_NORMAL, ped_file_system_type_get("ext4"), geom.start,
        geom.end);
    ped_partition_set_flag(new_part, PED_PARTITION_BOOT, 1);

    ped_disk_add_partition(old->disk, new_part, cst);
    ped_disk_commit(old->disk);

    // Watches the udev event queue.
    // Wait for kernel to handle pending events
    g_message("[%s] will call udevadm settle --timeout=5\n", __func__);
    g_spawn_command_line_sync ("udevadm settle --timeout=5",
                               NULL, NULL, NULL, NULL);

    ped_constraint_destroy(cst);

    *esp_path = ped_partition_get_path(esp_part);
    *new_path= ped_partition_get_path(new_part);

    mkfs(*esp_path, "fat32");
    mkfs(*new_path, "ext4");
    init_esp_partition(esp_part);

    ped_partition_destroy(esp_part);
    ped_partition_destroy(new_part);
    return TRUE;
}

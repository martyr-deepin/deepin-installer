#include "esp.h"

#include <glib.h>

PedPartition* create_esp_by_split(PedDisk* disk, PedPartition* old)
{
    PedSector length512 = 512 * 1024 * 1024 / disk->dev->sector_size;

    PedSector old_length = old->geom.length;
    g_assert(ped_disk_type_get("gpt") == disk->type);
    g_assert(old->geom.length >= length512 * 8  + length512); //at least 4.5GB space
    g_assert(old->type == PED_PARTITION_NORMAL || old->type == PED_PARTITION_FREESPACE);

    if (old->type == PED_PARTITION_NORMAL) {
	ped_disk_remove_partition(disk, old);
    }

    PedGeometry geom;
    PedConstraint* cst = NULL;

    ped_geometry_init(&geom, disk->dev, old->geom.start, length512);
    cst = ped_constraint_new_from_max(&geom);
    PedPartition* esp_part = ped_partition_new(disk, PED_PARTITION_NORMAL, ped_file_system_type_get("fat32"), geom.start, geom.end);
    ped_partition_set_flag(esp_part, PED_PARTITION_BOOT, 1);
    ped_partition_set_flag(esp_part, PED_PARTITION_HIDDEN, 1);
    ped_partition_set_name(esp_part, "ESP");
    ped_disk_add_partition (disk, esp_part, cst);
    ped_disk_commit (disk);
    ped_constraint_destroy(cst);

    ped_geometry_init(&geom, disk->dev,  esp_part->geom.end + 1, old_length - length512);

    cst = ped_constraint_new_from_max(&geom);
    PedPartition* new_part = ped_partition_new (disk, PED_PARTITION_NORMAL, ped_file_system_type_get("ext4"), geom.start, geom.end);
    ped_partition_set_flag(new_part, PED_PARTITION_BOOT, 1);

    ped_disk_add_partition(disk, new_part, cst);
    ped_disk_commit(disk);
    ped_constraint_destroy(cst);

    ped_partition_destroy(esp_part);
    return new_part;
}

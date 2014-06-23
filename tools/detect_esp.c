#include <parted/parted.h>
#include <glib.h>
#include <glib/gstdio.h>

gchar *get_partition_mount_point (const gchar *path);

int has_efi_directory(PedPartition* part)
{
    int is_busy = ped_partition_is_busy(part);
    char *path = ped_partition_get_path(part);
    //TODO: free path
    char* mount_point = NULL; 
    GError* error = NULL;

    if (!is_busy) {
	mount_point = g_dir_make_tmp("efi_detectorXXXXXX", &error);
	if (error != NULL) {
	    g_warning("create efi_detector failed :%s\n", error->message);
	    g_error_free(error);
	    error = NULL;
	}
	char* cmd = g_strdup_printf ("mount -t vfat %s %s", path, mount_point);
	g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
	g_free(cmd);
	if (error != NULL) {
	    g_warning("Can't detect whether is $ESP : %s", error->message);
	    g_error_free(error);
	    error = NULL;
	    return FALSE;
	}
    }

    mount_point = get_partition_mount_point(path);
    char* esp_path = g_build_filename(mount_point, "EFI", NULL);

    int is_esp = g_file_test (esp_path, G_FILE_TEST_IS_DIR);
    g_free(esp_path);

    if (!is_busy) {
	char* cmd = g_strdup_printf ("umount -l %s", mount_point);
	g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
	g_free(cmd);
	if (error != NULL) {
	    g_warning("Can't detect whether is $ESP : %s", error->message);
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

gchar *get_partition_mount_point (const gchar *path)
{
    gchar *mp = NULL;
    gchar *swap_cmd = NULL;
    gchar *swap_output = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("get partition mount point:invalid path %s\n", path);
        return mp;
    }

    swap_cmd = g_strdup_printf ("sh -c \"cat /proc/swaps |grep %s |awk '{print $1}'\"", path);
    g_spawn_command_line_sync (swap_cmd, &swap_output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition mount point:run swap cmd error->%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (swap_cmd);

    if (swap_output != NULL && g_strcmp0 (g_strstrip(swap_output), path) == 0) {
        return g_strdup ("swap");
    }

    cmd = g_strdup_printf ("findmnt -k -f -n -o TARGET -S %s", path);
    g_spawn_command_line_sync (cmd, &mp, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition mount point:run cmd error->%s\n", error->message);
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

char* query_esp_path_by_disk(const char* path)
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

    PedPartition* part = 0;
    for (part = ped_disk_next_partition(disk, NULL); part;
	    part = ped_disk_next_partition(disk, part) )
    {
	if (part->num <  0) {
	    continue;
	}

	if (part->fs_type == 0 || strncmp(part->fs_type->name, "fat32", 5) != 0) {
	    continue;
	}


	int is_esp = has_efi_directory(part);
	g_debug("%s is ESP ? :%d\n", ped_partition_get_path(part), is_esp);
	if (is_esp) {
	    return ped_partition_get_path(part);
	}
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    PedDevice *device = NULL;
    PedDisk *disk = NULL;

    ped_device_probe_all ();

    if (argc != 2) {
	while ((device = ped_device_get_next (device))) {
	    char* esp = query_esp_path_by_disk(device->path);
	    if (esp != NULL) {
		printf("ESP=%s #at %s\n", esp, device->path);
	    }
	}
    } else {
	const char* disk_path = (argv[1]);
	char* esp = query_esp_path_by_disk(disk_path);
	if (esp != NULL) {
	    printf("ESP=%s #at %s\n", esp, argv[1]);
	}
    }
    return 0;
}

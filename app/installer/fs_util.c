/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "fs_util.h"

//when command output supply free blocks number and block size
gdouble
_get_partition_free_size (const gchar *cmd, const gchar *free_regex,
                          const gchar *free_num_regex, const gchar *unit_regex,
                          const gchar *unit_num_regex)
{
    gdouble result = 0;

    gchar *output = NULL;
    gint exit_status;
    gchar *free_blocks_line = NULL;
    gchar *free_blocks_num = NULL;
    gchar *block_size_line = NULL;
    gchar *block_size_num = NULL;

    gdouble free_double = 0;
    gdouble size_double = 0;
    if (cmd == NULL || free_regex == NULL || free_num_regex == NULL ||
        unit_regex == NULL || unit_num_regex == NULL) {
        g_warning ("[%s] error: some param NULL\n", __func__);
        return result;
    }

    g_spawn_command_line_sync (cmd, &output, NULL, &exit_status, NULL);
    if (exit_status == -1) {
        g_warning ("[%s] error: run command %s failed\n", __func__, cmd);
    }

    free_blocks_line = get_matched_string_old (output, free_regex);
    if (free_blocks_line != NULL) {
        free_blocks_num = get_matched_string_old (free_blocks_line,
                                                  free_num_regex);
    }
    g_free (free_blocks_line);
    if (free_blocks_num != NULL) {
        free_double = g_ascii_strtod (free_blocks_num, NULL);
    }
    g_free (free_blocks_num);

    if (output != NULL) {
        block_size_line = get_matched_string_old (output, unit_regex);
    }
    if (block_size_line != NULL) {
        block_size_num = get_matched_string_old (block_size_line,
                                                 unit_num_regex);
    }
    g_free (block_size_line);

    if (block_size_num != NULL) {
        size_double = g_ascii_strtod(block_size_num, NULL);
    }
    g_free (block_size_num);

    g_free (output);

    result = free_double * size_double;

    return result;
}

gdouble
get_mounted_partition_free (const gchar *path)
{
    g_message("[%s]: path, %s\n", __func__, path);
    gchar *result = NULL;
    gchar *output = NULL;
    gint exit_status;

    if (path == NULL) {
        g_warning ("[%s] error: path is NULL\n", __func__);
        return -1;
    }

    gchar *cmd = g_strdup_printf (
        "sh -c \"df %s | tail -1 | awk '{ print $4 }'\" ", path);
    g_message("get_mounted_partition_free cmd:==%s==",cmd);
    g_spawn_command_line_sync (cmd, &output, NULL, &exit_status, NULL);
    if (exit_status == -1 ) {
        g_warning ("[%s] error: run command %s failed\n", __func__, cmd);
        g_free (cmd);
        g_free (output);
        g_free (result);
        return -1;
    }
    g_free (cmd);
    result = g_strdup (output);
    g_free (output);

    gdouble free = g_ascii_strtod (result, NULL) * 1024;
    g_free(result);
    g_message("[%s]: %s===%f===", __func__, path, free);
    return free;
}

double
_get_ext4_free (const gchar *path)
{
    double free = 0;

    gchar *ext4_cmd = g_find_program_in_path ("dumpe2fs");
    if (ext4_cmd == NULL) {
        g_warning ("[%s]: dumpe2fs not installed\n", __func__);
        return free;
    }

    gchar *cmd = g_strdup_printf ("%s -h %s", ext4_cmd, path);
    gchar *free_regex = g_strdup ("Free blocks:\\s+\\d+");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("Block size:\\s+\\d+");
    gchar *unit_num_regex = g_strdup("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex,
                                     unit_regex, unit_num_regex);

    g_free (ext4_cmd);
    g_free (cmd);
    g_free (free_regex);
    g_free (free_num_regex);
    g_free (unit_regex);
    g_free (unit_num_regex);

    return free;
}

double
_get_ext3_free (const gchar *path)
{
    return _get_ext4_free (path);
}

double
_get_ext2_free (const gchar *path)
{
    return _get_ext4_free (path);
}

double
_get_reiserfs_free (const gchar *path)
{
    double free = 0;

    gchar *reiserfs_cmd = g_find_program_in_path ("debugreiserfs");
    if (reiserfs_cmd  == NULL) {
        g_warning ("[%s]: debugreiserfs not installed\n", __func__);
        return free;
    }

    gchar *cmd = g_strdup_printf ("%s %s", reiserfs_cmd, path);
    gchar *free_regex = g_strdup ("Free blocks.*");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("Blocksize.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex,
                                     unit_regex, unit_num_regex);

    g_free (reiserfs_cmd);
    g_free (cmd);
    g_free (free_regex);
    g_free (free_num_regex);
    g_free (unit_regex);
    g_free (unit_num_regex);

    return free;
}

double
_get_swap_free (const gchar *path)
{
    double free = 0;
    g_warning ("[%s]: not implemented\n", __func__);

    return free;
}

double
_get_xfs_free (const gchar *path)
{
    double free = 0;

    gchar *xfs_cmd = g_find_program_in_path ("xfs_db");
    if (xfs_cmd == NULL) {
        g_warning ("[%s]: xfs_db not installed\n", __func__);
        return free;
    }

    gchar *cmd = g_strdup_printf (
        "%s -c 'sb 0' -c 'print blocksize' -c 'print fdblocks' -r %s",
        xfs_cmd, path);
    gchar *free_regex = g_strdup ("fdblocks.*");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("blocksize.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex,
                                     unit_regex, unit_num_regex);

    g_free (xfs_cmd);
    g_free (cmd);
    g_free (free_regex);
    g_free (free_num_regex);
    g_free (unit_regex);
    g_free (unit_num_regex);

    return free;
}

double
_get_jfs_free (const gchar *path)
{
    double free = 0;

    gchar *jfs_cmd = g_find_program_in_path ("jfs_debugfs");
    if (jfs_cmd == NULL) {
        g_warning ("[%s]: jfs_debugfs not installed\n", __func__);
        return free;
    }

    gchar *cmd = g_strdup_printf ("/bin/sh -c 'echo dm | %s %s'", jfs_cmd,
                                  path);
    gchar *free_regex = g_strdup ("dn_nfree.*0[xX][0-9a-fA-F]+");
    gchar *free_num_regex = g_strdup ("0[xX][0-9a-fA-F]+");
    gchar *unit_regex = g_strdup ("Block Size.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex,
                                     unit_regex, unit_num_regex);

    g_free (jfs_cmd);
    g_free (cmd);
    g_free (free_regex);
    g_free (free_num_regex);
    g_free (unit_regex);
    g_free (unit_num_regex);

    return free;
}

double
_get_fat16_free (const gchar *path)
{
    double free = 0;
    gchar *output = NULL;
    GError *error = NULL;
    double unit_size = 0;
    double used_size = 0;
    double total_size = 0;
    gchar *cmd = NULL;
    gchar *unit_cluster = NULL;
    gchar *unit_num = NULL;
    gchar *space_cluster = NULL;
    gchar *used_cluster = NULL;
    gchar *total_cluster = NULL;

    gchar *fat16_cmd = g_find_program_in_path ("dosfsck");
    if (fat16_cmd  == NULL) {
         g_warning ("[%s]: dosfsck not installed\n", __func__);
         goto out;
     }

    cmd = g_strdup_printf ("%s -n -v %s", fat16_cmd, path);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s]: run cmd %s failed\n", __func__, cmd);
        goto out;
    }

    unit_cluster = get_matched_string_old (output, "\\d+ bytes per cluster");
    if (unit_cluster == NULL) {
        g_warning ("[%s]: get unit cluster failed\n", __func__);
        goto out;
    }

    unit_num = get_matched_string_old (unit_cluster, "\\d+");
    if (unit_num == NULL) {
        g_warning ("[%s]: get unit cluster num failed\n", __func__);
        goto out;
    }
    unit_size = g_ascii_strtod (unit_num, NULL);

    space_cluster = get_matched_string_old (output, "\\d+/\\d+.*clusters");
    if (space_cluster == NULL) {
        g_warning ("[%s]: get space cluster failed\n", __func__);
        goto out;
    }

    used_cluster = get_matched_string_old (space_cluster, "\\d+/");
    if (used_cluster == NULL) {
        g_warning ("[%s]: get used cluster failed\n", __func__);
        goto out;
    }
    used_size = g_ascii_strtod (used_cluster, NULL);

    total_cluster = get_matched_string_old (output, "\\d+.*clusters");
    if (total_cluster == NULL) {
        g_warning ("[%s]: get total cluster failed\n", __func__);
        goto out;
    }
    total_size = g_ascii_strtod (total_cluster, NULL);

    free = unit_size * (total_size - used_size);
    goto out;

out:
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
    g_free (fat16_cmd);
    g_free (cmd);
    g_free (total_cluster);
    g_free (used_cluster);
    g_free (space_cluster);
    g_free (unit_num);
    g_free (unit_cluster);
    g_free (output);
    return free;
}

double
_get_fat32_free (const gchar *path)
{
    return _get_fat16_free (path);
}

double
_get_btrfs_free (const gchar *path)
{
    double free = 0;
    gchar *output = NULL;
    GError *error = NULL;
    double used_size = 0;
    double total_size = 0;
    gchar *cmd = NULL;
    gchar *total = NULL;
    gchar *total_num = NULL;
    gchar *space_inuse = NULL;
    gchar *used_num = NULL;

    gchar *btrfs_cmd = g_find_program_in_path ("btrfs");
    if (btrfs_cmd == NULL) {
        g_warning ("[%s]: btrfs not installed\n", __func__);
        goto out;
    }

    cmd = g_strdup_printf ("%s filesystem show %s", btrfs_cmd, path);
    g_message("[%s] cmd: %s\n", __func__, cmd);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s]: run cmd %s failed\n", __func__, cmd);
        goto out;
    }
    g_message("[%s]: output: %s\n", __func__, output);
    total = get_matched_string_old (output, "size.*used");
    if (total == NULL) {
        g_warning ("[%s]: get total failed\n", __func__);
        goto out;
    }

    total_num = get_matched_string_old (total, "\\d+");
    if (total_num == NULL) {
        g_warning ("[%s]: get total num failed\n", __func__);
        goto out;
    }

    total_size = g_ascii_strtod (total_num, NULL);
    if ((g_strrstr (total, "GB") != NULL) || (g_strrstr (total, "GiB") != NULL)){
        total_size = total_size * 1024 * 1024;
    }

    space_inuse = get_matched_string_old (output, "used.*path");
    if (space_inuse == NULL) {
        g_warning ("[%s]: get used failed\n", __func__);
        goto out;
    }

    used_num = get_matched_string_old (space_inuse, "\\d+");
    if (used_num == NULL) {
        g_warning ("[%s]: parse used num failed\n", __func__);
        goto out;
    }
    used_size = g_ascii_strtod (used_num, NULL);
    if ((g_strrstr (space_inuse, "GB") != NULL) ||
        (g_strrstr (space_inuse, "GiB") != NULL)){
        used_size = used_size * 1024 * 1024;
    }

    /*g_message("KiB::::total:%s,total_num:%s,total_size:%f",total,total_num,total_size);*/
    /*g_message("KiB::::space_inuse:%s,used_num:%s,used_size:%f",space_inuse,used_num,used_size);*/
    free = (total_size - used_size) * 1024;
    g_message("[%s]: free: %f, total_size: %f, used_size: %f", __func__,
              free, total_size, used_size);
    goto out;

out:
    if (error != NULL) {
        g_warning("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (btrfs_cmd);
    g_free (cmd);
    g_free (total_num);
    g_free (total);
    g_free (used_num);
    g_free (space_inuse);
    g_free (output);
    g_message("[%s] free: %f", __func__, free);
    return free;
}

double _get_ntfs_free (const gchar *path)
{
    gchar *ntfs_cmd = g_find_program_in_path ("ntfsinfo");
    if (ntfs_cmd == NULL) {
        g_warning ("[%s]: ntfsresize not installed\n", __func__);
        return -1;
    }

    GError* error = NULL;
    char* cmd = g_strdup_printf ("%s -m %s", ntfs_cmd, path);
    char* output = NULL;
    g_message("[%s] cmd: %s", __func__, cmd);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] run cmd `%s` failed: %s\n", __func__, cmd,
                   error->message);
        g_error_free(error);
        g_free(output);
        return -1;
    }

    char* cluster_size_str = get_matched_string(output,
                                                "Cluster Size:\\s+(\\d+)");

    if (cluster_size_str == NULL) {
        g_warning ("[%s]: get cluster_size failed: %s\n", __func__, output);
        g_free(output);
        return get_mounted_partition_free(path);
    }
    long cluster_size = (int)g_ascii_strtod (cluster_size_str, NULL);
    g_free(cluster_size_str);


    char* free_cluster_str = get_matched_string(output,
                                                "Free Clusters:\\s+(\\d+)");
    if (free_cluster_str == NULL) {
        g_warning ("[%s]: get used failed\n", __func__);
        g_free(output);
        return -1;
    }
    double free_cluster = (int)g_ascii_strtod(free_cluster_str, NULL);
    g_free(free_cluster_str);

    g_free(output);
    return free_cluster * cluster_size;
}

gpointer
get_partition_free (gpointer data)
{
    g_message("[%s]\n", __func__);
    double free = 0;

    struct FsHandler *handler = (struct FsHandler *) data;

    if (handler == NULL) {
        g_warning ("[%s]: handler NULL\n", __func__);
        return NULL;
    }

    gchar *path = handler->path;
    gchar *fs = handler->fs;
    gchar *part = handler->part;


    if (g_strcmp0 (fs, "ext4") == 0) {
        free = _get_ext4_free (path);

    } else if (g_strcmp0 (fs, "ext3") == 0) {
        free = _get_ext3_free (path);

    } else if (g_strcmp0 (fs, "ext2") == 0) {
        free = _get_ext2_free (path);

    } else if (g_strcmp0 (fs, "reiserfs") == 0) {
        free = _get_reiserfs_free (path);

    } else if (g_strcmp0 (fs, "swap") == 0) {
        free = _get_swap_free (path);

    } else if (g_strcmp0 (fs, "xfs") == 0) {
        free = _get_xfs_free (path);

    } else if (g_strcmp0 (fs, "jfs") == 0) {
        free = _get_jfs_free (path);

    } else if (g_strcmp0 (fs, "fat16") == 0) {
        free = _get_fat16_free (path);

    } else if (g_strcmp0 (fs, "fat32") == 0) {
        free = _get_fat32_free (path);

    } else if (g_strcmp0 (fs, "btrfs") == 0) {
        free = _get_btrfs_free (path);

    } else if (g_strcmp0 (fs, "ntfs") == 0) {
        free = _get_ntfs_free (path);

    } else if (g_strrstr (fs, "swap") != NULL) {
        free = _get_swap_free (path);

    } else {
        g_warning ("[%s]: not support for fs %s\n", __func__, fs);
    }

    g_message ("[%s] part: %s, free: %f, fs: %s, path: %s", __func__, part,
               free, fs, path);
    GRAB_CTX ();
    JSObjectRef message = json_create ();
    json_append_string (message, "part", part);
    json_append_number (message, "free", free);
    js_post_message ("used", message);
    UNGRAB_CTX ();

    g_free (handler->path);
    g_free (handler->part);
    g_free (handler->fs);
    g_free (handler);

    return NULL;
}

void mkfs(const gchar *path, const gchar *fs)
{
    g_message("[%s], path: %s, fs: %s\n", __func__, path, fs);
    if (path == NULL || fs == NULL) {
        g_warning ("[%s]: invalid path: %s or fs: %s\n", __func__, path, fs);
        return;
    }
    unmount_partition_by_device (path);

    gchar *cmd = NULL;
    gchar *fs_cmd = NULL;
    GError *error = NULL;

    if (g_strcmp0 (fs, "ext4") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext4");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.ext4 not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.ext4 -F -F %s", path);

    } else if (g_strcmp0 (fs, "ext3") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext3");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.ext3 not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.ext3 -F -F %s", path);

    } else if (g_strcmp0 (fs, "ext2") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext2");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.ext2 not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.ext2 -F -F %s", path);

    } else if (g_strcmp0 (fs, "fat16") == 0) {
        fs_cmd = g_find_program_in_path ("mkdosfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkdosfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkdosfs -F16 -I %s", path);

    } else if (g_strcmp0 (fs, "fat32") == 0) {
        fs_cmd = g_find_program_in_path ("mkdosfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkdosfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkdosfs -F32 -I %s", path);

    } else if  (g_strcmp0 (fs, "jfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.jfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.jfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.jfs -q %s", path);

    } else if (g_strstr_len(fs, -1, "swap")) {
        fs_cmd = g_find_program_in_path ("mkswap");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkswap not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkswap -f %s", path);

    } else if (g_strcmp0 (fs, "ntfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkntfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkntfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkntfs -Q -v %s", path);

    } else if (g_strcmp0 (fs, "reiserfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkreiserfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkreiserfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkreiserfs -f -f %s", path);

    } else if (g_strcmp0 (fs, "btrfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.btrfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.btrfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.btrfs -f %s", path);

    } else if (g_strcmp0 (fs, "xfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.xfs");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkfs.xfs not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkfs.xfs -f %s", path);

    } else if (g_strrstr (fs, "swap") != NULL) {
        fs_cmd = g_find_program_in_path ("mkswap");
        if (fs_cmd == NULL) {
            g_warning ("[%s]: mkswap not installed\n", __func__);
            return;
        }
        cmd = g_strdup_printf ("mkswap -f %s", path);

    } else {
        g_warning ("[%s]: fs `%s` currently not supported\n", __func__, fs);
        return;
    }

    g_message("[%s] cmd: %s\n", __func__, cmd);
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] cmd: %s, error: %s\n", __func__, cmd, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (fs_cmd);
    g_free (cmd);
}

gboolean
inhibit_disk ()
{
    g_message("[%s]\n", __func__);
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar *installer = g_find_program_in_path ("installer");
    if (installer == NULL) {
        g_warning ("[%s]: find installer failed\n", __func__);
        return ret;
    }

    gchar* inhibit_cmd = g_strdup_printf (
        "/usr/lib/udisks2/udisks2-inhibit  %s", installer);

    ret = g_spawn_command_line_async (inhibit_cmd, &error);
    if (error != NULL) {
        g_warning ("[%s] cmd: %s, error: %s\n", __func__, inhibit_cmd,
                   error->message);
        g_error_free (error);
        error = NULL;
    }

    g_free (inhibit_cmd);
    g_free (installer);

    return ret;
}

//attention this test is block, put it in a thread
gpointer
is_slowly_device (gpointer data)
{
    struct SpeedHandler *handler = (struct SpeedHandler *)data;
    if (handler == NULL || handler->path == NULL || handler->uuid == NULL) {
        g_warning ("[%s]: parse SpeedHandler failed\n", __func__);
        return NULL;
    }
    gchar *hdparm_cmd = NULL;
    gchar *output = NULL;
    gchar *matched = NULL;
    gchar *speed_cmd = NULL;
    gchar *speed = NULL;
    GError *error = NULL;

    hdparm_cmd = g_find_program_in_path ("hdparm");
    if (hdparm_cmd == NULL) {
        g_warning ("[%s]: hdparm not installed\n", __func__);
        goto out;
    }

    speed_cmd = g_strdup_printf ("hdparm -t %s", handler->path);
    g_spawn_command_line_sync (speed_cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] cmd: %s, error: %s\n", __func__, speed_cmd,
                   error->message);
        goto out;
    }

    matched = get_matched_string_old (output, "\\d+(\\.\\d+)?\\sMB/sec");
    if (matched == NULL) {
        g_warning ("[%s]: get MB/sec failed\n", __func__);
        goto out;
    }
    //g_printf ("is slowly device:matched string-> %s\n", matched);
    speed = get_matched_string_old (matched, "\\d+(\\.\\d+)?");
    if (speed == NULL) {
        g_warning ("[%s]: parse speed from %s failed\n", __func__, matched);
        goto out;
    }

    gdouble num = g_ascii_strtod (speed, NULL);
    //g_printf ("is slowly device:speed for %s is %g MB/sec\n", handler->path, num);
    if (num < 10) {
        GRAB_CTX ();
        JSObjectRef message = json_create ();
        json_append_string (message, "uuid", handler->uuid);
        js_post_message ("slow", message);
        UNGRAB_CTX ();
    }
    goto out;

out:
    g_free (speed);
    g_free (hdparm_cmd);
    g_free (speed_cmd);
    g_free (output);
    g_free (matched);
    if (error != NULL) {
        g_warning("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free ((gchar *)handler->path);
    g_free ((gchar *)handler->uuid);
    g_free (handler);
    return NULL;
}

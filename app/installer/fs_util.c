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

#include "fs_util.h"

//when command output supply free blocks number and block size
gdouble 
_get_partition_free_size (const gchar *cmd, const gchar *free_regex, const gchar *free_num_regex, 
                          const gchar *unit_regex, const gchar *unit_num_regex) 
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

    g_spawn_command_line_sync (cmd, &output, NULL, &exit_status, NULL);
    if (exit_status == -1) {
        g_warning ("_get_partition_free_size:run command %s failed\n", cmd);
    }

    free_blocks_line = get_matched_string (output, free_regex);
    if (free_blocks_line != NULL) {
        free_blocks_num = get_matched_string (free_blocks_line, free_num_regex);
    }
    g_free (free_blocks_line);
    if (free_blocks_num != NULL) {
        free_double = g_ascii_strtod (free_blocks_num, NULL);
    }
    g_free (free_blocks_num);

    if (output != NULL) {
        block_size_line = get_matched_string (output, unit_regex);
    }
    if (block_size_line != NULL) {
        block_size_num = get_matched_string (block_size_line, unit_num_regex);
    }
    g_free (block_size_line);

    if (block_size_num != NULL) {
        size_double = g_ascii_strtod(block_size_num, NULL);
    }
    g_free (block_size_num);

    g_free (output);

    result = (free_double * size_double) / (1024 * 1024);
    //result = (free_double * size_double) >> 20;

    return result;
}

gchar*
get_mounted_partition_used (const gchar *path) 
{
    gchar *result = NULL;

    gchar *output = NULL;
    gint exit_status;

    gchar *cmd = g_strdup_printf ("sh -c \"df -h %s | awk '{ print $3 }'\" ", path);

    g_spawn_command_line_sync (cmd, &output, NULL, &exit_status, NULL);
    if (exit_status == -1 ) {
        g_warning ("run command %s failed\n", cmd);
    }
    g_free (cmd);
    result = g_strdup (output);
    g_free (output);

    return result;
}

double
_get_ext4_free (const gchar *path)
{
    double free = 0;

    gchar *ext4_cmd = g_find_program_in_path ("dumpe2fs");
    if (ext4_cmd == NULL) {
        g_warning ("_get_ext4_free:dumpe2fs not installed\n");
        return free;
    }
    
    gchar *cmd = g_strdup_printf ("%s -h %s", ext4_cmd, path);
    gchar *free_regex = g_strdup ("Free blocks:\\s+\\d+");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("Block size:\\s+\\d+");
    gchar *unit_num_regex = g_strdup("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex, unit_regex, unit_num_regex);

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
        g_warning ("_get_reiserfs_free:debugreiserfs not installed\n");
        return free;
    }
    
    gchar *cmd = g_strdup_printf ("%s %s", reiserfs_cmd, path);
    gchar *free_regex = g_strdup ("Free blocks.*");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("Blocksize.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex, unit_regex, unit_num_regex);

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
    g_warning ("_get_swap_free:not implemented\n");

    return free;
}

double
_get_xfs_free (const gchar *path)
{
    double free = 0;

    gchar *xfs_cmd = g_find_program_in_path ("xfs_db");
    if (xfs_cmd == NULL) {
        g_warning ("_get_xfs_free:xfs_db not installed\n");
        return free;
    }
        
    gchar *cmd = g_strdup_printf ("%s -c 'sb 0' -c 'print blocksize' -c 'print fdblocks' -r %s", xfs_cmd, path);
    gchar *free_regex = g_strdup ("fdblocks.*");
    gchar *free_num_regex = g_strdup ("\\d+");
    gchar *unit_regex = g_strdup ("blocksize.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex, unit_regex, unit_num_regex);

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
        g_warning ("_get_jfs_free:jfs_debugfs not installed\n");
        return free;
    }
        
    gchar *cmd = g_strdup_printf ("/bin/sh -c 'echo dm | %s %s'", jfs_cmd, path);
    gchar *free_regex = g_strdup ("dn_nfree.*0[xX][0-9a-fA-F]+");
    gchar *free_num_regex = g_strdup ("0[xX][0-9a-fA-F]+");
    gchar *unit_regex = g_strdup ("Block Size.*");
    gchar *unit_num_regex = g_strdup ("\\d+");

    free = _get_partition_free_size (cmd, free_regex, free_num_regex, unit_regex, unit_num_regex);

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
         g_warning ("_get_fat16_free:dosfsck not installed\n");
         goto out;
     }
         
    cmd = g_strdup_printf ("%s -n -v %s", fat16_cmd, path);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("_get_btrfs_free:run cmd %s failed\n", cmd);
        goto out;
    }

    unit_cluster = get_matched_string (output, "\\d+ bytes per cluster");
    if (unit_cluster == NULL) {
        g_warning ("_get_fat16_free:get unit cluster failed\n");
        goto out;
    }

    unit_num = get_matched_string (unit_cluster, "\\d+");
    if (unit_num == NULL) {
        g_warning ("_get_fat16_free:get unit cluster num failed\n");
        goto out;
    }
    unit_size = g_ascii_strtod (unit_num, NULL);

    space_cluster = get_matched_string (output, "\\d+/\\d+.*clusters");
    if (space_cluster == NULL) {
        g_warning ("_get_fat16_free:get space cluster failed\n");
        goto out;
    }
    
    used_cluster = get_matched_string (space_cluster, "\\d+/");
    if (used_cluster == NULL) {
        g_warning ("_get_fat16_free:get used cluster failed\n");
        goto out;
    }
    used_size = g_ascii_strtod (used_cluster, NULL);

    total_cluster = get_matched_string (output, "\\d+.*clusters");
    if (total_cluster == NULL) {
        g_warning ("_get_fat16_free:get total cluster failed\n");
        goto out;
    }
    total_size = g_ascii_strtod (total_cluster, NULL);

    free = unit_size * (total_size - used_size) / (1000 * 1000) ;
    goto out;

out:
    if (error != NULL) {
        g_error_free (error);
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
        g_warning ("btrfs not installed\n");
        goto out;
    }

    cmd = g_strdup_printf ("%s filesystem show %s", btrfs_cmd, path);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("_get_btrfs_free:run cmd %s failed\n", cmd);
        goto out;
    }

    total = get_matched_string (output, "size.*used");
    if (total == NULL) {
        g_warning ("_get_btrfs_free:get total failed\n");
        goto out;
    }

    total_num = get_matched_string (total, "\\d+");
    if (total_num == NULL) {
        g_warning ("_get_btrfs_free:get total num failed\n");
        goto out;
    }

    total_size = g_ascii_strtod (total_num, NULL);

    space_inuse = get_matched_string (output, "used.*path");
    if (space_inuse == NULL) {
        g_warning ("_get_btrfs_free:get used failed\n");
        goto out;
    }

    used_num = get_matched_string (space_inuse, "\\d+");
    if (used_num == NULL) {
        g_warning ("_get_btrfs_free:parse used num failed\n");
        goto out;
    }
    used_size = g_ascii_strtod (used_num, NULL);

    free = total_size - used_size;

    if (g_strrstr (total, "GB") != NULL) {
        free = free * 1024;
    }
    goto out;

out:
    if (error != NULL) {
        g_error_free (error);
    }
    g_free (btrfs_cmd);
    g_free (cmd);
    g_free (total_num);
    g_free (total);
    g_free (used_num);
    g_free (space_inuse);
    g_free (output);
    return free;
}

double
_get_ntfs_free (const gchar *path)
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

    gchar *ntfs_cmd = g_find_program_in_path ("ntfsresize");
    if (ntfs_cmd == NULL) {
        g_warning ("_get_ntfs_free:ntfsresize not installed\n");
        goto out;
    }

    cmd = g_strdup_printf ("%s -i -f -P %s", ntfs_cmd, path);
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("_get_ntfs_free:run cmd %s failed\n", cmd);
        goto out;
    }

    total = get_matched_string (output, "bytes.*\\d+.*MB");
    if (total == NULL) {
        g_warning ("_get_ntfs_free:get total failed\n");
        goto out;
    }

    total_num = get_matched_string (total, "\\d+");
    if (total_num == NULL) {
        g_warning ("_get_ntfs_free:get total num failed\n");
        goto out;
    }

    total_size = g_ascii_strtod (total_num, NULL);

    space_inuse = get_matched_string (output, "Space in use.*MB");
    if (space_inuse == NULL) {
        g_warning ("_get_ntfs_free:get used failed\n");
        goto out;
    }

    used_num = get_matched_string (space_inuse, "\\d+");
    if (used_num == NULL) {
        g_warning ("_get_ntfs_free:parse used num failed\n");
        goto out;
    }
    used_size = g_ascii_strtod (used_num, NULL);

    free = total_size - used_size;
    goto out;

out:
    if (error != NULL) {
        g_error_free (error);
    }
    g_free (ntfs_cmd);
    g_free (cmd);
    g_free (total_num);
    g_free (total);
    g_free (used_num);
    g_free (space_inuse);
    g_free (output);
    return free;
}

gpointer 
get_partition_free (gpointer data)
{
    double free = 0;

    struct FsHandler *handler = (struct FsHandler *) data;

    if (handler == NULL) {
        g_warning ("get partition free:handler NULL\n");
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
        g_warning ("get partition free:not support for fs %s\n", fs);
    }

    //g_warning ("js_post message for used:part->%s, free->%f, fs->%s, path->%s", part, free, fs, path);
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

void 
set_partition_filesystem (const gchar *path, const gchar *fs)
{
    gchar *cmd = NULL;
    gchar *fs_cmd = NULL;
    GError *error = NULL;

    if (g_strcmp0 (fs, "ext4") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext4");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.ext4 not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.ext4 %s", path);

    } else if (g_strcmp0 (fs, "ext3") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext3");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.ext3 not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.ext3 %s", path);

    } else if (g_strcmp0 (fs, "ext2") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.ext2");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.ext2 not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.ext2 %s", path);

    } else if (g_strcmp0 (fs, "fat16") == 0) {
        fs_cmd = g_find_program_in_path ("mkdosfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkdosfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkdosfs -F16 %s", path);

    } else if (g_strcmp0 (fs, "fat32") == 0) {
        fs_cmd = g_find_program_in_path ("mkdosfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkdosfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkdosfs -F32 %s", path);

    } else if  (g_strcmp0 (fs, "jfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.jfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.jfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.jfs -q %s", path);

    } else if  (g_strcmp0 (fs, "linux-swap") == 0) {
        fs_cmd = g_find_program_in_path ("mkswap");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkswap not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkswap %s", path);

    } else if (g_strcmp0 (fs, "ntfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkntfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkntfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkntfs -Q -v %s", path);
    
    } else if (g_strcmp0 (fs, "reiserfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkreiserfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkreiserfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkreiserfs -f %s", path);

    } else if (g_strcmp0 (fs, "btrfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.btrfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.btrfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.btrfs %s", path);

    } else if (g_strcmp0 (fs, "xfs") == 0) {
        fs_cmd = g_find_program_in_path ("mkfs.xfs");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkfs.xfs not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkfs.xfs -f %s", path);

    } else if (g_strrstr (fs, "swap") != NULL) {
        fs_cmd = g_find_program_in_path ("mkswap");
        if (fs_cmd == NULL) {
            g_warning ("set partition filesystem:mkswap not installed\n");
            return ;
        }
        cmd = g_strdup_printf ("mkswap %s", path);

    } else {
        g_warning ("set partition filesystem:%s currently not supported\n", fs);
        return ;
    }

    g_spawn_command_line_async (cmd, &error);
    if (error != NULL) {
        g_warning ("set partition filesystem %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    g_free (fs_cmd);
    g_free (cmd);
}

gboolean 
inhibit_disk ()
{
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar *installer = g_find_program_in_path ("installer");
    if (installer == NULL) {
        g_warning ("inhibit disk:find installer failed\n");
        return ret;
    }

    gchar* inhibit_cmd = g_strdup_printf ("/usr/lib/udisks2/udisks2-inhibit  %s", installer);

    ret = g_spawn_command_line_async (inhibit_cmd, &error);
    if (error != NULL) {
        g_warning ("inhibit disk:%s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    
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
        g_warning ("is slowly device:parse SpeedHandler failed\n");
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
        g_warning ("is slowly device:hdparm not installed\n");
        goto out;
    }

    speed_cmd = g_strdup_printf ("hdparm -t %s", handler->path);
    g_spawn_command_line_sync (speed_cmd, &output, NULL, NULL, &error); 
    if (error != NULL) {
        g_warning ("is slowly device:run hdparm %s\n", error->message);
        goto out;
    }

    matched = get_matched_string (output, "\\d+(\\.\\d+)?\\sMB/sec");
    if (matched == NULL) {
        g_warning ("is slowly device:get MB/sec failed\n");
        goto out;
    }
    //g_printf ("is slowly device:matched string-> %s\n", matched);
    speed = get_matched_string (matched, "\\d+(\\.\\d+)?");
    if (speed == NULL) {
        g_warning ("is slowly device:parse speed from %s failed\n", matched);
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
        g_error_free (error);
    }
    g_free ((gchar *)handler->path);
    g_free ((gchar *)handler->uuid);
    g_free (handler);
    return NULL;
}

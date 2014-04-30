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

#include "wubi.h"
#include "fs_util.h"
#include <stdio.h>
#include <mntent.h>

#define WUBI_CONFIG_FILE     "/target/preseed.cfg"

extern gchar *opt_target;
extern gchar *opt_home;
extern gchar *opt_username;
extern gchar *opt_hostname;
extern gchar *opt_password;
extern gchar *opt_layout;
extern gchar *opt_variant;
extern gchar *opt_timezone;
extern gchar *opt_locale;
extern gchar *opt_grub;

JS_EXPORT_API
gboolean installer_is_use_wubi ()
{
    gint status = -1;
    g_spawn_command_line_sync ("grep install-automatic /proc/cmdline", NULL, NULL, &status, NULL); 
    if (status == 0) {
        return TRUE;
    }
    g_spawn_command_line_sync ("grep install-nodesktop /proc/cmdline", NULL, NULL, &status, NULL); 
    if (status == 0) {
        return TRUE;
    }
    return FALSE;
}

void sync_wubi_config ()
{
    gsize index = 0;
    GError *error = NULL;
    GFile *file = NULL;
    GFileInputStream *input = NULL;
    GDataInputStream *data_input = NULL;

    file = g_file_new_for_path (WUBI_CONFIG_FILE);
    if (!g_file_query_exists (file, NULL)) {
        g_warning ("get wubi config:%s not exists\n", WUBI_CONFIG_FILE);
        goto out;
    }

    input = g_file_read (file, NULL, &error);
    if (error != NULL){
        g_warning ("get wubi config:read config file error->%s", error->message);
        goto out;
    }

    data_input = g_data_input_stream_new ((GInputStream *) input);
    if (data_input == NULL) {
        g_warning ("get wubi config:get data input stream failed\n");
        goto out;
    }
    
    char *data = (char *) 1;
    while (data) {
        gsize length = 0;
        data = g_data_input_stream_read_line (data_input, &length, NULL, &error);
        if (error != NULL) {
            g_warning ("get wubi config:read line error->%s", error->message);
            g_error_free (error);
            error = NULL;
            continue;
        }
        if (data != NULL) {
            if (g_str_has_prefix (data, "d-i")){
                gchar **line = g_strsplit (data, "\t", -1);
                if (line == NULL) {
                    g_warning ("get wubi config:split %s failed\n", data);
                } else {
                    if (g_strcmp0 (line[1], "netcfg/get_hostname") == 0) {
                        opt_hostname = g_strdup(line[3]);
                    } else if (g_strcmp0 (line[1], "passwd/username") == 0) {
                        opt_username = g_strdup(line[3]);
                    } else if (g_strcmp0 (line[1], "passwd/user-passwd-crypted") == 0) {
                        opt_password = g_strdup(line[3]);
                    } else if (g_strcmp0 (line[1], "time/zone") == 0) {
                        opt_timezone = g_strdup(line[3]);
                    }
                    g_strfreev (line);
                }
            }
        } 
    }
    goto out;
out:
    if (file != NULL) {
        g_object_unref (file);
    }
    if (data_input != NULL) {
        g_object_unref (data_input);
    }
    if (input != NULL) {
        g_object_unref (input);
    }
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
}

JS_EXPORT_API 
void installer_update_fs_wubi (const gchar *path, const gchar *fs)
{
    set_partition_filesystem (path, fs);
}

JS_EXPORT_API 
void installer_mount_path_wubi (const gchar *path, const gchar *mp)
{
    gchar *cmd = g_strdup_printf ("mount %s %s", path, mp);
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, NULL);
    g_free (cmd);
}

JS_EXPORT_API 
gboolean installer_write_mp_wubi (const gchar *path, const gchar *fs, const gchar *mp)
{
    gboolean ret = FALSE;
    gchar *comment_line = NULL;
    struct mntent mnt;
    FILE *mount_file = NULL;
    static gboolean header_inited = FALSE;

    if (path == NULL || fs == NULL || mp == NULL) {
        g_warning ("write mp wubi:path or fs or mp is NULL\n");
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

    mount_file = setmntent ("/etc/fstab", "a");
    if (mount_file == NULL) {
        g_warning ("write wubi mp: setmntent failed\n");
        goto out;
    }
    mnt.mnt_fsname = g_strdup (path);
    mnt.mnt_dir = g_strdup (mp);
    mnt.mnt_type = g_strdup (fs);
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
        g_warning ("write wubi mp:invalid mp->%s\n", mp);
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
    g_free (comment_line);
    if (mount_file != NULL) {
        endmntent (mount_file);
    }
    if (!ret) {
        //write fs tab stage goes together with timezone
        emit_progress ("timezone", "terminate");
    }
}

JS_EXPORT_API 
void installer_update_bootloader_wubi (const gchar *path)
{
    gchar *install_cmd = g_strdup_printf ("grub-install %s", path);
    g_spawn_command_line_sync (install_cmd, NULL, NULL, NULL, NULL);
    g_free (install_cmd);
    g_spawn_command_line_sync ("update-grub", NULL, NULL, NULL, NULL);
}


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

#include <sys/sysinfo.h>
#include <ftw.h>
#include "extract.h"
#include "keyboard.h"

static int 
copy_file_cb (const char *path, const struct stat *sb, int typeflag)
{
    extern const gchar *target;
    if (path == NULL) {
        return 1;
    }
    static GFile *target_f; 
    if (target_f == NULL) {
        target_f = g_file_new_for_path (target);
    }
    static GFile *target_squash_f;
    if (target_squash_f == NULL) {
        gchar *squash = g_strdup_printf ("%s/squashfs", target);
        target_squash_f = g_file_new_for_path (squash);
        g_free (squash);
    }

    GFile *src = g_file_new_for_path (path);
    gchar *origin = g_file_get_relative_path (target_squash_f, src);
    if (origin == NULL) {
        g_warning ("copy file cb:origin NULL for %s\n", path);
    }
    GFile *dest = g_file_resolve_relative_path (target_f, origin);
    g_free (origin);

    g_warning ("copy file from %s to %s\n", g_file_get_path (src), g_file_get_path (dest));

    g_file_copy (src, dest, G_FILE_COPY_OVERWRITE | G_FILE_COPY_NOFOLLOW_SYMLINKS,  NULL, NULL, NULL, NULL);

    g_object_unref (src);
    g_object_unref (dest);

    return 0;
}

JS_EXPORT_API
void installer_extract_iso ()
{
    gboolean succeed = FALSE;
    extern const gchar *target;
    GError *error = NULL;
    gchar *target_iso;
    gchar *mount_cmd;
    gchar *umount_cmd;

    if (target == NULL) {
        g_warning ("extract iso:target NULL\n");
        goto out;
    }

    target_iso = g_strdup_printf ("%s/squashfs", target);
    if (g_mkdir_with_parents (target_iso, 0755) == -1) {
        g_warning ("extract iso:mkdir failed\n");
        goto out;
    }

    mount_cmd = g_strdup_printf ("mount /cdrom/casper/filesystem.squashfs %s", target_iso);
    g_spawn_command_line_sync (mount_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("extract iso:mount squashfs->%s\n", error->message);
        goto out;
    }

    ftw (target_iso, copy_file_cb, 65536);

    umount_cmd  = g_strdup_printf ("umount %s", target_iso);
    g_spawn_command_line_sync (umount_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("extract iso:umount squashfs->%s\n", error->message);
        goto out;
    }
    succeed = TRUE;
    goto out;
out:
    g_free (mount_cmd);
    g_free (umount_cmd);
    g_free (target_iso);
    if (error != NULL) {
        g_error_free (error);
    }
    if (succeed) {
        emit_progress ("extract", "finish");
    } else {
        emit_progress ("extract", "terminate");
    }
}

static void
watch_extract_child (GPid pid, gint status, gpointer data)
{
    GError *error = NULL;
    if (data == NULL) {
        g_warning ("watch extract child:arg data is NULL\n");
    }
    g_spawn_check_exit_status (status, &error);
    if (error != NULL) {
        g_warning ("watch extract child:error->%s\n", error->message);
        g_error_free (error);
        emit_progress ("extract", "terminate");
    } else {
        g_printf ("watch extract child:extract finish\n");
        emit_progress ("extract", "finish");
    }

    guint cb_id = *(guint *) data;
    if (cb_id > 0) {
        g_source_remove (cb_id);
        cb_id = 0;
    }

    g_spawn_close_pid (pid);
}

static gboolean
timeout_emit_cb (gpointer data)
{
    //GIOChannel *channel = (GIOChannel *) data;
    //gchar *string;

    //g_io_channel_read_line (channel, &string, NULL, NULL, NULL);
    //gchar *match = get_matched_string (string, "\\d{1,3}%");

    //if (match != NULL) {
    //    g_warning ("cb timeout: emit extract progress:%s\n", match);
    //    emit_progress ("extract", match);
    //}
    //g_free (match);
    //g_free (string);
    emit_progress ("extract", "50%");

    return TRUE;
}

JS_EXPORT_API
void installer_extract_squashfs ()
{
    gchar *squashfs_cmd = g_find_program_in_path ("unsquashfs");
    if (squashfs_cmd == NULL) {
        g_warning ("extract squashfs: unsquashfs not installed\n");
        emit_progress ("extract", "terminate");
        return;
    }
    g_free (squashfs_cmd);

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("extract squash fs:target is NULL\n");
        emit_progress ("extract", "terminate");
        return;
    }
    const gchar *iso = "/cdrom/casper/filesystem.squashfs";
    if (!g_file_test (iso, G_FILE_TEST_EXISTS)) {
        g_warning ("extract squashfs:iso not exists\n");
        emit_progress ("extract", "terminate");
        return;
    }

    guint processors = get_cpu_num ();
    guint puse = 1;
    if (processors > 2) {
        puse = processors - 1;
    }

    gchar **argv = g_new0 (gchar *, 8);
    argv[0] = g_strdup ("unsquashfs");
    argv[1] = g_strdup ("-f");
    argv[2] = g_strdup ("-p");
    argv[3] = g_strdup_printf ("%d", puse);
    argv[4] = g_strdup ("-d");
    argv[5] = g_strdup (target);
    argv[6] = g_strdup ("/cdrom/casper/filesystem.squashfs");

    gint std_output;
    GError *error = NULL;
    GPid pid;
    GIOChannel *out_channel = NULL;
    guint cb_id = 0;

    g_spawn_async_with_pipes (NULL,
                              argv,
                              NULL,
                              G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
                              NULL,
                              NULL,
                              &pid,
                              NULL,
                              &std_output,
                              NULL,
                              &error);
    if (error != NULL) {
        g_warning ("extract squashfs:spawn async pipes %s\n", error->message);
        g_error_free (error);
    }
    out_channel = g_io_channel_unix_new (std_output);
    cb_id = g_timeout_add (1000, (GSourceFunc) timeout_emit_cb, out_channel);
    g_child_watch_add (pid, (GChildWatchFunc) watch_extract_child, &cb_id);

    g_strfreev (argv);
}

static gboolean
is_outdated_machine ()
{
    const gchar *cmd = "udevadm info --query=property --name=/dev/input/mouse0";
    gchar *output = NULL;
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, NULL);
    if (output != NULL) {
        if (g_strrstr (output, "vmware") != NULL || g_strrstr (output, "virtualbox") != NULL) {
            g_free (output);
            return FALSE;
        }
    } else {
        g_warning ("is outdated machine:udevadm\n");
    }
    g_free (output);

    const gchar *kvm_cmd  = "lscpu";
    gchar *kvm_output = NULL;
    g_spawn_command_line_sync (kvm_cmd, &kvm_output, NULL, NULL, NULL);
    if (kvm_output != NULL) {
        if (g_strrstr (kvm_output, "KVM") != NULL) {
            g_free (kvm_output);
            return FALSE;
        }
    } else {
        g_warning ("is outdated machine:lscpu not kvm\n");
    }
    g_free (kvm_output);
    
    //double freeram = get_free_memory_size ();
    double freeram = installer_get_memory_size ();
    if (freeram > 0 && freeram < 1024 * 1024 * 1024) {
        g_warning ("is outdated machine:free mem %f less than 1G\n", freeram);
        return TRUE;
    }

    guint processors = get_cpu_num ();
    if (processors < 2) {
        return TRUE;
    }

    return FALSE;
}

JS_EXPORT_API
void installer_extract_intelligent ()
{
    if (is_outdated_machine ()) {
        g_printf ("extract intelligent:use extract iso\n");
        installer_extract_iso ();
    } else {
        g_printf ("extract intelligent:use extract squashfs\n");
        installer_extract_squashfs ();
    }
}

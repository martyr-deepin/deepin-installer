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

    guint* cb_ids = (guint *) data;
    if (cb_ids[0] > 0) {
        g_source_remove (cb_ids[0]);
        cb_ids[0] = 0;
    }
    if (cb_ids[1] > 0) {
        g_source_remove (cb_ids[1]);
        cb_ids[1] = 0;
    }
    if (cb_ids[2] > 0) {
        g_source_remove (cb_ids[2]);
        cb_ids[2] = 0;
    }
    g_free (cb_ids);

    g_spawn_close_pid (pid);
}

static gboolean 
cb_out_watch (GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar **progress = (gchar **) data;

    gchar *string;
    gsize  size;

    if (cond == G_IO_HUP) {
        //g_printf ("cb out watch: io hup\n");
        g_io_channel_unref (channel);
        return FALSE;
    }

    g_io_channel_read_line (channel, &string, &size, NULL, NULL);
    //g_printf ("cb out watch:%s***cb out watch finish\n", string);
    gchar *match = get_matched_string (string, "\\d{1,3}%");
    //g_printf ("cb out watch:match->              %s\n", match);
    if (match == NULL) {
        g_debug ("cb out watch:line without extract progress\n");
    } else {
        if (*progress != NULL) {
            g_free (*progress);
            *progress = NULL;
        }
        *progress = g_strdup (match);
    }

    g_free (match);
    g_free (string);

    return TRUE;
}

static gboolean
cb_err_watch (GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if (cond == G_IO_HUP) {
        g_io_channel_unref (channel);
        return FALSE;
    }

    g_io_channel_read_line (channel, &string, &size, NULL, NULL);
    //fix me, parse error here
    g_printf ("cb err watch:%s\n", string); 

    g_free (string);

    return TRUE;
}

//will stop automaticly when extract finish
static gboolean
cb_timeout (gpointer data)
{
    gchar **progress = (gchar **)data;

    if (progress != NULL) {
        if (*progress != NULL) {
            g_warning ("cb timeout: emit extract progress:%s\n", *progress);
            emit_progress ("extract", *progress);
        } else {
            g_warning ("cb timeout:*progress null\n");
        }
    } else {
        g_warning ("cb timeout:progress null\n");
    }

    return TRUE;
}

JS_EXPORT_API
void installer_extract_squashfs ()
{
    if (g_find_program_in_path ("unsquashfs") == NULL) {
        g_warning ("extract squashfs: unsquashfs not installed\n");
        emit_progress ("extract", "terminate");
        return;
    }

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
    gint std_error;
    GError *error = NULL;
    GPid pid;
    GIOChannel *out_channel = NULL;
    GIOChannel *err_channel = NULL;

    guint* cb_ids = g_new0 (guint, 3);
    gchar** progress = g_new0 (gchar*, 1);

    g_spawn_async_with_pipes (NULL,
                              argv,
                              NULL,
                              G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
                              NULL,
                              NULL,
                              &pid,
                              NULL,
                              &std_output,
                              &std_error,
                              &error);
    if (error != NULL) {
        g_warning ("extract squashfs:spawn async pipes %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    out_channel = g_io_channel_unix_new (std_output);
    err_channel = g_io_channel_unix_new (std_error);

    cb_ids[0] = g_io_add_watch_full (out_channel,
                                     G_PRIORITY_LOW, 
                                     G_IO_IN | G_IO_HUP, 
                                     (GIOFunc) cb_out_watch, 
                                     progress, 
                                     (GDestroyNotify) g_io_channel_unref);

    cb_ids[1] = g_io_add_watch_full (err_channel, 
                                     G_PRIORITY_LOW, 
                                     G_IO_IN | G_IO_HUP, 
                                     (GIOFunc) cb_err_watch, 
                                     progress, 
                                     (GDestroyNotify) g_io_channel_unref);
    //g_io_add_watch (out_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_out_watch, progress);
    //g_io_add_watch (err_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_err_watch, progress);

    cb_ids[2] = g_timeout_add (2000, (GSourceFunc) cb_timeout, progress);
    g_child_watch_add (pid, (GChildWatchFunc) watch_extract_child, cb_ids);
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
    //free memory used by keyboard layout variant to help extract
    extern GHashTable *layout_variants_hash;
    if (layout_variants_hash != NULL) {
        g_hash_table_destroy (layout_variants_hash);
    }

    extern GHashTable *layout_desc_hash;
    if (layout_desc_hash != NULL) {
        g_hash_table_destroy (layout_desc_hash);
    }

    if (is_outdated_machine ()) {
        g_printf ("extract intelligent:use extract iso\n");
        installer_extract_iso ();
    } else {
        g_printf ("extract intelligent:use extract squashfs\n");
        installer_extract_squashfs ();
    }
}

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

#include "base.h"
#include <sys/sysinfo.h>

void emit_progress (const gchar *step, const gchar *progress)
{
    GRAB_CTX ();
    JSObjectRef message = json_create ();
    json_append_string (message, "stage", step);
    json_append_string (message, "progress", progress);
    js_post_message ("progress", message);
    UNGRAB_CTX ();
}

gchar *
get_matched_string (const gchar *target, const gchar *regex_string) 
{
    gchar *result = NULL;
    GError *error = NULL;
    GRegex *regex;
    GMatchInfo *match_info;

    if (target == NULL || regex_string == NULL) {
        g_warning ("get matched string:paramemter NULL\n");
        return NULL;
    }

    regex = g_regex_new (regex_string, 0, 0, &error);
    if (error != NULL) {
        g_warning ("get matched string:%s\n", error->message);
        g_error_free (error);
        return NULL;
    }
    error = NULL;

    g_regex_match (regex, target, 0, &match_info);
    if (g_match_info_matches (match_info)) {
        result = g_match_info_fetch (match_info, 0);

    } else {
        g_warning ("get matched string failed!\n");
    }

    g_match_info_free (match_info);
    g_regex_unref (regex);

    return result;
}

JS_EXPORT_API 
double installer_get_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("get memory size:%s\n", strerror (errno));
        return 0;
    }

    return info.totalram;
}

double get_free_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("get free memory size:%s\n", strerror (errno));
        return 0;
    }

    return info.freeram;
}

guint
get_cpu_num ()
{
    guint num = 0;
    gchar *output = NULL;
    const gchar *cmd = "sh -c \"cat /proc/cpuinfo |grep processor |wc -l\"";
    GError *error = NULL;

    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
       g_warning ("get cpu num:%s\n", error->message);
        g_error_free (error);
    }
    if (output == NULL) {
        return num;
    }
    num = g_strtod (g_strstrip (output), NULL);

    g_free (output);
    return num;
}

gchar *
get_partition_mount_point (const gchar *path)
{
    gchar *mp = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("get partition mount point:invalid path %s\n", path);
        return mp;
    }

    cmd = g_strdup_printf ("findmnt -k -f -n -o TARGET -S %s", path);
    g_spawn_command_line_sync (cmd, &mp, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition mount point:run cmd error->%s\n", error->message);
        g_error_free (error);
    }
    g_free (cmd);
    if (mp != NULL) {
        mp = g_strstrip (mp);
    }

    return mp;
}

guint 
get_mount_target_count (const gchar *target)
{
    guint count = 0;
    gchar *findcmd = NULL;
    gchar *output = NULL;
    gchar **array = NULL;
    GError *error = NULL;

    if (target == NULL) {
        g_warning ("get target mount count:target NULL\n");
        goto out;
    }

    findcmd = g_strdup_printf ("findmnt --target %s", target);
    g_spawn_command_line_sync (findcmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get target mount count:run %s error->%s\n", findcmd, error->message);
        goto out;
    }
    if (output == NULL || output == "") {
        g_warning ("get target mount count:output invalid\n");
        goto out;
    }
    array = g_strsplit (output, "\n", -1);
    if (array == NULL) {
        g_warning ("get target mount count:array NULL\n");
        goto out;
    }
    count = g_strv_length (array);
    if (count > 2) {
        count = count - 2;
    } else {
        count = 0;
    }
    goto out;
out:
    g_free (findcmd);
    g_free (output);
    if (array != NULL) {
        g_strfreev (array);
    }
    if (error != NULL) {
        g_error_free (error);
    }
    return count;
}

gchar *
get_partition_uuid (const gchar *path)
{
    gchar *uuid = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("get partition uuid:path %s not exists", path);
        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s UUID -o value %s", path);
    g_spawn_command_line_sync (cmd, &uuid, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition uuid:%s\n", error->message);
        g_error_free (error);
    }
    g_free (cmd);

    return uuid;
}

gchar *
get_partition_label (const gchar *path)
{
    gchar *label = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("get partition label:path %s not exists", path);

        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s LABEL -o value %s", path);
    g_spawn_command_line_sync (cmd, &label, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition label:%s\n", error->message);
        g_error_free (error);
    }
    g_free (cmd);

    return label;
}

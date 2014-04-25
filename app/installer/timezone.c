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

#include "timezone.h"

static GList *timezone_list = NULL;

JS_EXPORT_API
JSObjectRef installer_get_timezone_list ()
{
    GRAB_CTX ();
    gsize index = 0;
    GError *error = NULL;
    GFile *file = NULL;
    GFileInputStream *input = NULL;
    GDataInputStream *data_input = NULL;

    JSObjectRef timezones = json_array_create ();

    file = g_file_new_for_path ("/usr/share/zoneinfo/zone.tab");
    if (!g_file_query_exists (file, NULL)) {
        g_warning ("get timezone list:zone.tab not exists\n");
        goto out;
    }

    input = g_file_read (file, NULL, &error);
    if (error != NULL){
        g_warning ("get timezone list:read zone.tab error->%s", error->message);
        goto out;
    }

    data_input = g_data_input_stream_new ((GInputStream *) input);
    if (data_input == NULL) {
        g_warning ("get timezone list:get data input stream failed\n");
        goto out;
    }
    
    char *data = (char *) 1;
    while (data) {
        data = g_data_input_stream_read_line (data_input, NULL, NULL, NULL);
        if (data == NULL) {
            break;
        }
        if (g_str_has_prefix (data, "#")){
            g_debug ("get timezone list:comment line, just pass");
            continue;
        } else {
            gchar **line = g_strsplit (data, "\t", -1);
            if (line == NULL) {
                g_warning ("get timezone list:split %s failed\n", data);
            } else {
                json_array_insert (timezones, index, jsvalue_from_cstr (get_global_context (), line[2]));
                index++;
                g_strfreev (line);
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
    UNGRAB_CTX ();

    return timezones;
}

JS_EXPORT_API 
void installer_set_timezone (const gchar *timezone)
{
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar *timezone_file = NULL;
    gchar *zoneinfo_path = NULL;
    gchar *localtime_path = NULL;
    GFile *zoneinfo_file = NULL;
    GFile *localtime_file = NULL;
    gchar *timezone_content = NULL;

    if (timezone == NULL) {
        g_warning ("set timezone:timezone NULL\n");
        goto out;
    }
    timezone_file = g_strdup ("/etc/timezone");
    timezone_content = g_strdup_printf ("%s\n", timezone);
    g_file_set_contents (timezone_file, timezone_content, -1, &error);
    if (error != NULL) {
        g_warning ("set timezone:write timezone %s\n", error->message);
        goto out;
    }
    zoneinfo_path = g_strdup_printf ("/usr/share/zoneinfo/%s", timezone);
    localtime_path = g_strdup ("/etc/localtime");
    zoneinfo_file = g_file_new_for_path (zoneinfo_path);
    localtime_file = g_file_new_for_path (localtime_path);
    g_file_copy (zoneinfo_file, localtime_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("set timezone:cp %s to /etc/localtime %s\n", zoneinfo_path, error->message);
        goto out;
    }
    ret = TRUE;
    goto out;

out:
    g_free (timezone_content);
    g_free (timezone_file);
    g_free (zoneinfo_path);
    g_free (localtime_path);
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
    if (zoneinfo_file != NULL) {
        g_object_unref (zoneinfo_file);
    }
    if (localtime_file != NULL) {
        g_object_unref (localtime_file);
    }
    if (ret) {
        emit_progress ("timezone", "finish");
    } else {
        g_warning ("set timezone failed, just skip this step");
        emit_progress ("timezone", "finish");
    }
}

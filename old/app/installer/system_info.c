/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#define _GNU_SOURCE
#include <glib.h>
#include <gio/gio.h>
#include "jsextension.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

JS_EXPORT_API 
gchar* installer_get_current_locale ()
{
    gchar *locale = NULL;
    gchar *cmd = g_strdup(
        "sh -c \"locale | head -n1 | awk -F= '{print $2}' | tr -d '\n' \"");
    GError* error = NULL;
    g_spawn_command_line_sync (cmd, &locale, NULL, NULL, &error);
    if (error != NULL) {
        g_warning("[%s] failed: %s", __func__, error->message);
        g_error_free(error);
    }
    g_free (cmd);
    return locale;
}

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
        g_warning("[%s]: zone.tab not exists\n", __func__);
        goto out;
    }

    input = g_file_read (file, NULL, &error);
    if (error != NULL){
        g_warning("[%s] read zone.tab error->%s", __func__, error->message);
        goto out;
    }

    data_input = g_data_input_stream_new ((GInputStream *) input);
    if (data_input == NULL) {
        g_warning("[%s] get data input stream failed\n", __func__);
        goto out;
    }
    
    char *data = (char *) 1;
    while (data) {
        data = g_data_input_stream_read_line (data_input, NULL, NULL, NULL);
        if (data == NULL) {
            break;
        }
        if (g_str_has_prefix (data, "#")){
            g_debug("[%s] comment line, just pass", __func__);
            continue;
        } else {
            gchar **line = g_strsplit (data, "\t", -1);
            if (line == NULL) {
                g_warning("[%s] split %s failed\n", __func__, data);
            } else {
                json_array_insert(timezones, index,
                    jsvalue_from_cstr(get_global_context(), line[2]));
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

#define LOG_FILE_PATH           "/tmp/deepin-installer.log"

JS_EXPORT_API 
void  installer_show_log ()
{
    gchar *cmd = g_strdup_printf ("xdg-open %s\n", LOG_FILE_PATH);
    g_spawn_command_line_async (cmd, NULL);
    g_free (cmd);
}

JS_EXPORT_API 
JSObjectRef installer_get_system_users()
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();

    struct passwd *user;
    gchar *username = NULL;
    int i = 0;

    while ((user = getpwent ()) != NULL){
        if (user->pw_uid >= 1000 || g_strcmp0 ("deepin", user->pw_name) == 0) {
            continue;
        }
        username = g_strdup (user->pw_name);
        json_array_insert(array, i, jsvalue_from_cstr (get_global_context(),
                          username));
        i++;
        g_free (username);
    }
    endpwent ();
    UNGRAB_CTX ();

    return array;
}

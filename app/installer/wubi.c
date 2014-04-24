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

gboolean is_use_wubi ()
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
    }
}

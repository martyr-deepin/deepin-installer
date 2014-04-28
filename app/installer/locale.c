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

#include "locale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

JS_EXPORT_API 
gchar* installer_get_current_locale ()
{
    gchar *locale = NULL;
    gchar *cmd = g_strdup ("sh -c \"locale | head -n1 | awk -F= '{print $2}'\"");
    g_spawn_command_line_sync (cmd, &locale, NULL, NULL, NULL);
    g_free (cmd);
    return locale;
}

static gboolean
is_locale_valid (const gchar *local_part)
{
    if (local_part == NULL) {
        return FALSE;
    }
    return TRUE;
}

static gboolean
is_charset_valid (const gchar *charset_part)
{
    if (charset_part == NULL) {
        return FALSE;
    }
    return TRUE;
}

JS_EXPORT_API 
void  installer_set_target_locale (const gchar *locale)
{
    gchar **split = NULL;
    gchar *locale_part = NULL;
    gchar *charset_part = NULL;
    gchar *localedef_cmd = NULL;
    gchar *contents = NULL;
    GError *error = NULL;

    if (locale == NULL) {
        g_warning ("set target locale:locale NULL\n");
        locale_part = g_strdup ("en_US");
        charset_part = g_strdup ("UTF-8");
    } else {
        split = g_strsplit (locale, ",", -1);
        if (g_strv_length (split) == 2) {
            locale_part = g_strdup (split[0]);
            charset_part = g_strdup (split[1]);
        } else {
            locale_part = g_strdup (split[0]);
            charset_part = g_strdup ("UTF-8");
        }
        g_strfreev (split);
    }

    if (!g_file_test ("/etc/default/locale", G_FILE_TEST_EXISTS)) {
        g_creat ("/etc/default/locale", 0644);
    }
    if (!is_locale_valid (locale_part)) {
        if (locale_part != NULL) {
            g_free (locale_part);
        }
        locale_part = g_strdup ("en_US");
    }
    if (!is_charset_valid (charset_part)) {
        if (charset_part != NULL) {
            g_free (charset_part);
        }
        charset_part = g_strdup ("UTF-8");
    }

    contents = g_strdup_printf ("%s.%s", locale_part, charset_part);
    g_file_set_contents ("/etc/default/locale", contents, -1, &error);
    if (error != NULL) {
        g_warning ("set target locale:write %s to /etc/default/locale failed\n", contents);
        g_error_free (error);
        error = NULL;
    }

    localedef_cmd = g_strdup_printf ("localedef -f %s -i %s %s", charset_part, locale_part, contents);
    g_spawn_command_line_async (localedef_cmd, &error);
    if (error != NULL) {
        g_warning ("set target locale:run localdef->%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }

    g_free (locale_part);
    g_free (charset_part);
    g_free (contents);
    g_free (localedef_cmd);
}

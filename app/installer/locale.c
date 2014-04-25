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
    return FALSE;
}

static gboolean
is_charset_valid (const gchar *charset_part)
{
    return FALSE;
}

JS_EXPORT_API 
void  installer_set_target_locale (const gchar *locale)
{
    gchar **split = NULL;
    gchar *locale_part = NULL;
    gchar *charset_part = NULL;

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
        g_create ("/etc/default/locale", 0644);
    }
}

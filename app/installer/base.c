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
    js_post_message_simply ("progress", "{\"stage\":\"%s\",\"progress\":\"%s\"}", step, progress);
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

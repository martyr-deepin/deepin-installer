/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 Liqiang Lee
 *
 * Author:      Liqiang Lee <liliqiang@linuxdeepin.com>
 * Maintainer:  Liqiang Lee <liliqiang@linuxdeepin.com>
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
#include "desktop_action.h"

#define DESKTOP_ACTION_PATTERN ".* Shortcut Group|Desktop Action .*"

struct Action* action_new(char const* name, char const* exec)
{
    struct Action* action = g_new0(struct Action, 1);
    action->name = g_strdup(name);
    action->exec = g_strdup(exec);
    return action;
}


void action_free(struct Action* action)
{
    g_free(action->name);
    g_free(action->exec);
    g_free(action);
}


static
GKeyFile* _read_as_key_file(GDesktopAppInfo* app)
{
    char const* filename = g_desktop_app_info_get_filename(app);
    if (filename == NULL)
        return NULL;

    GError* error = NULL;
    GKeyFile* file = g_key_file_new();
    g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, &error);

    if (error != NULL) {
        g_warning("[%s] %s", __func__, error->message);
        g_error_free(error);
        g_key_file_unref(file);
        return NULL;
    }

    return file;
}


static
GRegex* _desktop_action_pattern()
{
    GError* error = NULL;
    GRegex* desktop_action_pattern = g_regex_new(DESKTOP_ACTION_PATTERN,
                                                 G_REGEX_DUPNAMES
                                                 | G_REGEX_OPTIMIZE,
                                                 0,
                                                 &error
                                                );

    if (error != NULL) {
        g_warning("[%s] %s", __func__, error->message);
        g_error_free(error);
    }

    return desktop_action_pattern;
}


static
struct Action* _get_action(GKeyFile* file, const char* group_name)
{
    GError* error = NULL;
    gchar* name = g_key_file_get_locale_string(file,
                                               group_name,
                                               G_KEY_FILE_DESKTOP_KEY_NAME,
                                               NULL,
                                               &error);
    if (error != NULL) {
        g_warning("[%s] %s", __func__, error->message);
        g_error_free(error);
        return NULL;
    }

    gchar* exec = g_key_file_get_string(file,
                                        group_name,
                                        G_KEY_FILE_DESKTOP_KEY_EXEC,
                                        &error);
    if (error != NULL) {
        g_warning("[%s] %s", __func__, error->message);
        g_error_free(error);
        g_free(name);
        return NULL;
    }
    /* g_debug("[%s] name: %s, exec: %s", __func__, name, exec); */
    struct Action* action = action_new(name, exec);

    g_free(name);
    g_free(exec);

    return action;
}


GPtrArray* get_app_actions(GDesktopAppInfo* app)
{
    gchar** groups = NULL;
    GPtrArray* actions = NULL;

    GKeyFile* file = _read_as_key_file(app);

    if (file == NULL)
        return NULL;

    GRegex* desktop_action_pattern = _desktop_action_pattern();
    if (desktop_action_pattern == NULL)
        goto out;

    gsize len = 0;
    groups = g_key_file_get_groups(file, &len);

    if (len == 0)
        goto out;

    actions = g_ptr_array_new_with_free_func((GDestroyNotify)action_free);
    for (int i = 0; groups[i] != NULL; ++i) {
        if (g_regex_match(desktop_action_pattern, groups[i], 0, NULL)) {
            struct Action* action = NULL;
            if ((action = _get_action(file, groups[i])) != NULL) {
                g_ptr_array_add(actions, action);
            }
        }
    }

out:
    g_strfreev(groups);

    if (file != NULL)
        g_key_file_unref(file);

    if (desktop_action_pattern != NULL)
        g_regex_unref(desktop_action_pattern);

    return actions;
}


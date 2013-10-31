/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *
 * Author:      snyh <snyh@snyh.org>
 * Maintainer:  snyh <snyh@snyh.org>
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
#include "xid2aid.h"
#include "utils.h"
#include <string.h>
#include <glib.h>
#include <unistd.h>

/*MEMORY_TESTED*/

#define FILTER_ARGS_PATH DATA_DIR"/filter_arg.ini"
#define FILTER_WMNAME_PATH DATA_DIR"/filter_wmname.ini"
#define FILTER_WMCLASS_PATH DATA_DIR"/filter_wmclass.ini"
#define FILTER_WMINSTANCE_PATH DATA_DIR"/filter_wminstance.ini"
#define PROCESS_REGEX_PATH DATA_DIR"/process_regex.ini"
#define DEEPIN_ICONS_PATH DATA_DIR"/deepin_icons.ini"

static GKeyFile* filter_args = NULL;
static GKeyFile* filter_wmname = NULL;
static GKeyFile* filter_wmclass = NULL;
static GKeyFile* filter_wminstance = NULL;
static GKeyFile* deepin_icons = NULL;

static GRegex* prefix_regex = NULL;
static GRegex* suffix_regex = NULL;
static GHashTable* white_apps = NULL;
static gboolean _is_init = FALSE;

static
void _build_filter_info(GKeyFile* filter, const char* path)
{
    if (g_key_file_load_from_file(filter, path, G_KEY_FILE_NONE, NULL)) {
        gsize size;
        char** groups = g_key_file_get_groups(filter, &size);
        for (gsize i=0; i<size; i++) {
            gsize key_len;
            char** keys = g_key_file_get_keys(filter, groups[i], &key_len, NULL);
            for (gsize j=0; j<key_len; j++) {
                g_hash_table_insert(white_apps, g_key_file_get_string(filter, groups[i], keys[j], NULL), NULL);
            }
        }
    }
}


static
void _init()
{
    white_apps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    // load and build process regex config information
    GKeyFile* process_regex = g_key_file_new();
    if (g_key_file_load_from_file(process_regex, PROCESS_REGEX_PATH, G_KEY_FILE_NONE, NULL)) {
        char* str = g_key_file_get_string(process_regex, "DEEPIN_PREFIX", "skip_prefix", NULL);
        prefix_regex = g_regex_new(str, G_REGEX_OPTIMIZE, 0, NULL);
        g_free(str);

        str = g_key_file_get_string(process_regex, "DEEPIN_PREFIX", "skip_suffix", NULL);
        suffix_regex = g_regex_new(str, G_REGEX_OPTIMIZE, 0, NULL);
        g_free(str);
    }
    if (prefix_regex == NULL) {
        g_warning("Can't build prefix_regex, use fallback config!");
        prefix_regex = g_regex_new(
                "(^gksu(do)?$)|(^sudo$)|(^mono$)|(^ruby$)|(^padsp$)|(^aoss$)|(^python(\\d.\\d)?$)|(^(ba)?sh$)",
                G_REGEX_OPTIMIZE, 0, NULL
                );

    }
    if (suffix_regex == NULL) {
        g_warning("Can't build suffix_regex, use fallback config!");
        suffix_regex = g_regex_new( "((-|.)bin$)|(.py$)", G_REGEX_OPTIMIZE, 0, NULL);
    }
    g_key_file_free(process_regex);

    // load filters and build white_list
    _build_filter_info(filter_args = g_key_file_new(), FILTER_ARGS_PATH);
    _build_filter_info(filter_wmclass = g_key_file_new(), FILTER_WMCLASS_PATH);
    _build_filter_info(filter_wminstance = g_key_file_new(), FILTER_WMINSTANCE_PATH);
    _build_filter_info(filter_wmname = g_key_file_new(), FILTER_WMNAME_PATH);

    // set init flag
    _is_init = TRUE;
    g_assert(suffix_regex != NULL);
    g_assert(prefix_regex != NULL);
}


static
void _get_exec_name_args(char** cmdline, gsize length, char** name, char** args)
{
    g_assert(length != 0);
    *args = NULL;

    gsize name_pos = 0;

    if (cmdline[0] != NULL) {
        char* space_pos = NULL;
        if ((space_pos = strchr(cmdline[0], ' ')) != NULL && g_strrstr(cmdline[0], "chrom") != NULL) {
            *space_pos = '\0';
            for (gsize i = length - 1; i > 0; --i) {
                cmdline[i + 1] = cmdline[i];
            }
            length += 1;
            cmdline[1] = space_pos + 1;
        }
        char* basename = g_path_get_basename(cmdline[0]);
        if (g_regex_match(prefix_regex, basename, 0, NULL))
            while (cmdline[++name_pos] && cmdline[name_pos][0] == '-')
                ; // empty body
        g_free(basename);
    }

    cmdline[length] = NULL;

    int diff = length - name_pos;
    if (diff == 0) {
        *name = g_path_get_basename(cmdline[0]);
        if (length > 1) {
            *args = g_strjoinv(" ", cmdline+1);
        }
    } else if (diff >= 1){
        *name = g_path_get_basename(cmdline[name_pos]);
        if (diff >= 2)
            *args = g_strjoinv(" ", cmdline + name_pos + 1);
    }

    char* tmp = *name;
    g_assert(tmp != NULL);
    g_assert(suffix_regex != NULL);
    *name = g_regex_replace_literal (suffix_regex, tmp, -1, 0, "", 0, NULL);
    g_free(tmp);

    for (int i=0; i<strlen(*name); i++) {
        if ((*name)[i] == ' ') {
            (*name)[i] = '\0';
            break;
        }
    }
}

static
char* _find_app_id_by_filter(const char* name, const char* keys_str, GKeyFile* filter)
{
    if (filter == NULL) return NULL;
    g_assert(name != NULL && keys_str != NULL);
    if (g_key_file_has_group(filter, name)) {
        gsize size = 0;
        char** keys = g_key_file_get_keys(filter, name, &size, NULL);
        for (gsize i=0; i<size; i++) {
            if (g_strstr_len(keys_str , -1, keys[i])) {
                char* value = g_key_file_get_string(filter, name, keys[i], NULL);
                g_strfreev(keys);
                return value;
            }
        }
        g_strfreev(keys);
        /*g_debug("find \"%s\" in filter.ini but can't find the really desktop file\n", name);*/
    }
    return NULL;
}

char* find_app_id(const char* exec_name, const char* key, int filter)
{
    if (_is_init == FALSE) {
        _init();
    }
    g_assert(exec_name != NULL && key != NULL);
    switch (filter) {
        case APPID_FILTER_WMCLASS:
            return _find_app_id_by_filter(exec_name, key, filter_wmclass);
        case APPID_FILTER_WMNAME:
            return _find_app_id_by_filter(exec_name, key, filter_wmname);
        case APPID_FILTER_ARGS:
            return _find_app_id_by_filter(exec_name, key, filter_args);
        case APPID_FILTER_WMINSTANCE:
            return _find_app_id_by_filter(exec_name, key, filter_wminstance);
        default:
            g_error("filter %d is not support !", filter);
    }
    return NULL;
}

void get_pid_info(int pid, char** exec_name, char** exec_args)
{
    if (_is_init == FALSE) {
        _init();
    }
    char* cmd_line = NULL;
    char* path = g_strdup_printf("/proc/%d/cmdline", pid);

    gsize size=0;
    if (g_file_get_contents(path, &cmd_line, &size, NULL) && size > 0) {
        char** name_args = g_new(char*, 1024);
        gsize j = 0;
        name_args[j] = cmd_line;
        for (gsize i=1; i<size && j<1024; i++) {
            if (cmd_line[i] == 0) {
                name_args[++j] = cmd_line + i + 1;
            }
        }
        name_args[j ? : j+1] = NULL;

        _get_exec_name_args(name_args, j+1, exec_name, exec_args);

        g_free(name_args);

    } else {
        *exec_name = get_exe(NULL, pid);
        *exec_args = NULL;
    }
    g_free(path);
    g_free(cmd_line);
}

gboolean is_app_in_white_list(const char* name)
{
    if (!_is_init) {
        _init();
    }
    return is_chrome_app(name) || g_hash_table_contains(white_apps, name);
}


gboolean is_deepin_app_id(const char* app_id)
{
    if (deepin_icons == NULL) {
        deepin_icons = g_key_file_new();
        if (!g_key_file_load_from_file(deepin_icons, DEEPIN_ICONS_PATH, G_KEY_FILE_NONE, NULL)) {
            g_key_file_free(deepin_icons);
            deepin_icons = NULL;
            return FALSE;
        }
    }
    return g_key_file_has_group(deepin_icons, app_id);

}

int get_deepin_app_id_operator(const char* app_id)
{
    g_assert(deepin_icons != NULL);
    return g_key_file_get_integer(deepin_icons, app_id, "operator", NULL);
}

char* get_deepin_app_id_value(const char* app_id)
{
    g_assert(deepin_icons != NULL);
    return g_key_file_get_string(deepin_icons, app_id, "value", NULL);
}


char* get_exe(const char* app_id, int pid)
{
    char buf[8095] = {0};
    char* path = g_strdup_printf("/proc/%d/exe", pid);
    // header doesn't work, add this to avoid warning
    extern ssize_t readlink(const char*, char*, size_t);
    gsize len = readlink(path, buf, 8095);
    if (len > 8095) {
        g_debug("PID:%d's exe is to long!", pid);
        buf[8095] = 0;
    }
    g_free(path);
    return g_strdup(buf);
}


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
#include "i18n.h"
#include "category.h"
#include <stdlib.h>
#include <glib.h>
#include "sqlite3.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "jsextension.h"

#define DEEPIN_SOFTWARE_CENTER_DATA_DIR    "/usr/share/deepin-software-center/data"
#define DATA_NEWEST_ID    DEEPIN_SOFTWARE_CENTER_DATA_DIR"/data_newest_id.ini"
#define CATEGORY_NAME_DB_PATH   DEEPIN_SOFTWARE_CENTER_DATA_DIR"/update/%s/desktop/new_desktop.db"
#define CATEGORY_INDEX_DB_PATH   DEEPIN_SOFTWARE_CENTER_DATA_DIR"/update/%s/category/category.db"


static
int for_translate(int argc, char *argv[])
{
    const char* const categories[] = {
        _(ALL),
        _(INTERNET),
        _(MULTIMEDIA),
        _(GAMES),
        _(GRAPHICS),
        _(PRODUCTIVITY),
        _(INDUSTRY),
        _(EDUCATION),
        _(DEVELOPMENT),
        _(SYSTEM),
        _(UTILITIES),
        _(OTHER)
    };

    return 0;
}


PRIVATE
gboolean _need_to_update(const char* db_path)
{
    if (db_path[0] == '\0')
        return TRUE;

    static time_t _last_modify_time[2] = {0};
    struct stat newest;
    if (!stat(DATA_NEWEST_ID, &newest)) {
        return FALSE;
    }

    int index = (int)g_str_has_suffix(db_path, "category.db");

    if (newest.st_mtime != _last_modify_time[index]) {
        _last_modify_time[index] = newest.st_mtime;
        return TRUE;
    }

    return FALSE;
}


PRIVATE
void _get_db_path(char* db_path, size_t path_len, char const* db_path_template)
{
    if (_need_to_update(db_path)) {
        GKeyFile* id_file = g_key_file_new();
        if (g_key_file_load_from_file(id_file, DATA_NEWEST_ID, G_KEY_FILE_NONE, NULL)) {
            gchar* newest_id = g_key_file_get_value(id_file, "newest", "data_id", NULL);
            g_key_file_free(id_file);
            g_snprintf(db_path, path_len, db_path_template, newest_id);
            g_free(newest_id);
        }
    }
}

const char* get_category_name_db_path()
{
    // the path to db has fixed 69 bytes, and uuid is 36 bytes.
#define PATH_LEN 106
    static gchar db_path[PATH_LEN] = {0};
    _get_db_path(db_path, PATH_LEN, CATEGORY_NAME_DB_PATH);
#undef PATH_LEN
    return db_path;
}


const char* get_category_index_db_path()
{
    // the path to db has fixed 67 bytes, and uuid is 36 bytes.
#define PATH_LEN 104
    static char db_path[PATH_LEN] = {0};
    _get_db_path(db_path, PATH_LEN, CATEGORY_INDEX_DB_PATH);
#undef PATH_LEN
    return db_path;
}


gboolean search_database(const char* db_path, const char* sql, SQLEXEC_CB fn, void* res)
{
    sqlite3* db = NULL;
    gboolean is_good = SQLITE_OK == sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (is_good) {
        char* error = NULL;
        sqlite3_exec(db, sql, fn, res, &error);
        sqlite3_close(db);
        if (error != NULL) {
            g_warning("%s\n", error);
            sqlite3_free(error);
            is_good = FALSE;
        }
    }

    return is_good;
}


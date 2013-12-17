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
#ifndef _CATEGORY_H_
#define _CATEGORY_H_

#include <glib.h>
#include <gio/gdesktopappinfo.h>

#define CATEGORY_END_TAG -100
#define ALL_CATEGORY_ID (-1)
#define OTHER_CATEGORY_ID (-2)

#define ALL "all"
#define INTERNET "internet"
#define MULTIMEDIA "multimedia"
#define GAMES "games"
#define GRAPHICS "graphics"
#define PRODUCTIVITY "productivity"
#define INDUSTRY "industry"
#define EDUCATION "education"
#define DEVELOPMENT "development"
#define SYSTEM "system"
#define UTILITIES "utilities"
#define OTHER "other"

typedef int (*SQLEXEC_CB) (void*, int, char**, char**);

const char* get_category_db_path();
const char** get_category_list();
GList* get_deepin_categories(GDesktopAppInfo*);
const GPtrArray* get_all_categories_array();
const char* get_category_index_db_path();
const char* get_category_name_db_path();
gboolean search_database(const char* db_path, const char* sql, SQLEXEC_CB fn, void* res);

#endif

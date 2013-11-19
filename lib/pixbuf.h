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
#ifndef _PIXBUF_H__
#define _PIXBUF_H__
char* generate_directory_icon(const char* p1, const char* p2, const char* p3, const char* p4);
char* get_data_uri_by_path(const char* path);

#include <gdk-pixbuf/gdk-pixbuf.h>
char* get_data_uri_by_pixbuf(GdkPixbuf* pixbuf);
char* pixbuf_to_canvas_data(GdkPixbuf* pixbuf);
#endif

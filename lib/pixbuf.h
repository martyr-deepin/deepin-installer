/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef _PIXBUF_H__
#define _PIXBUF_H__
char* generate_directory_icon(const char* p1, const char* p2, const char* p3, const char* p4);
char* get_data_uri_by_path(const char* path);

#include <gdk-pixbuf/gdk-pixbuf.h>
char* get_data_uri_by_pixbuf(GdkPixbuf* pixbuf);
char* pixbuf_to_canvas_data(GdkPixbuf* pixbuf);
#endif

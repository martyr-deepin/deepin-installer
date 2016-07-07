/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __BACKGROUND__
#define __BACKGROUND__

#include <gtk/gtk.h>

typedef struct _BackgroundInfo {
    GtkWidget* container;
    cairo_surface_t* bg;
    double alpha;
    GMutex m;
} BackgroundInfo;

gboolean background_info_draw_callback(GtkWidget* w, cairo_t* cr, BackgroundInfo* info);
void background_info_set_background_by_drawable(BackgroundInfo* info, guint32 drawable);
void background_info_set_background_by_file(BackgroundInfo* info, const char* file);
void background_info_change_alpha(BackgroundInfo* info, double alpha);
BackgroundInfo* create_background_info(GtkWidget* container, GtkWidget* child);
void background_info_clear(BackgroundInfo* info);

void setup_background(GtkWidget* container, GtkWidget* webview,const char* xatom_name);
#endif

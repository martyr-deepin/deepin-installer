/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef THEME_H
#define THEME_H
#include "display_info.h"

const char* get_theme_path();
char* get_theme_config();
char* get_current_bg_path();
void set_theme_background(GtkWidget* container,GtkWidget* child);
void draw_background_by_theme(GtkWidget* widget, GtkWidget* child, struct DisplayInfo info);
void init_theme();

#endif /* end of THEME_H */


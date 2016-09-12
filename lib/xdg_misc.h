/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __XDG_MISC_H__
#define __XDG_MISC_H__
#include <glib.h>
#include <gio/gio.h>
#include <jsextension.h>

// the really icon path is determined by theme
void set_default_theme(const char* theme);
// deal with the NotShowIn/OnlyShowIn filed. 
void set_desktop_env_name(const char* name);

// change the desktop file 's current locale name to "name"
gboolean change_desktop_entry_name(const char* path, const char* name);

// get the $XDG_DESKTOP_DIR value
#define DESKTOP_DIR() (g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP))
#define TEMPLATES_DIR() (g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES))
#define HOME_DIR() (g_get_home_dir())

// convert the icon name to the really icon path, seea also "set_default_theme"
char* icon_name_to_path(const char* name, int size);
char* icon_name_to_path_with_check_xpm(const char* name, int size);
char* lookup_icon_by_gicon(GIcon* icon);

#endif

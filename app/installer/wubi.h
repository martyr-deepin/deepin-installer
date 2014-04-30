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

#ifndef __WUBI_H
#define __WUBI_H

#include "base.h"

JS_EXPORT_API gboolean installer_is_use_wubi ();

void sync_wubi_config ();

JS_EXPORT_API void installer_update_fs_wubi (const gchar *path, const gchar *fs);

JS_EXPORT_API void installer_mount_path_wubi (const gchar *path, const gchar *mp);

JS_EXPORT_API gboolean installer_write_mp_wubi (const gchar *path, const gchar *fs, const gchar *mp);

JS_EXPORT_API void installer_update_bootloader_wubi (const gchar *path);

#endif

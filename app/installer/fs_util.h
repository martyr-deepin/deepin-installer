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

#ifndef __FS_UTIL_H
#define __FS_UTIL_H

#include "base.h"

struct SpeedHandler {
    const gchar *path;
    const gchar *uuid;
};

struct FsHandler {
    gchar *path;
    gchar *fs;
    gchar *part;
};

double get_mounted_partition_free (const gchar *path);

gpointer get_partition_free (gpointer data);

void mkfs(const gchar *path, const gchar *fs);

gboolean inhibit_disk ();

gpointer is_slowly_device (gpointer data);

#endif

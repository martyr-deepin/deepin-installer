/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

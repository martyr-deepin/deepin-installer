/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __INFO_H__
#define __INFO_H__

#include <glib.h>
#define TARGET "/target/"
void write_installer_conf(const char* path);

struct _InstallerConf {
    char* bootloader;
    gboolean uefi;

    char* user_name;
    char* password;
    char* host_name;

    char* locale;

    char* layout;
    char* layout_variant;

    char* timezone;

    gboolean simple_mode;

    GHashTable* mount_points;
    char* root_disk;
    char* root_partition;
};
extern struct _InstallerConf InstallerConf;

void installer_record_bootloader_info(const char* uuid, gboolean uefi);
void installer_record_mountpoint_info(const char* part, const char* mountpoint);

#endif

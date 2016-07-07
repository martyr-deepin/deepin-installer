/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "info.h"
#include <glib.h>
#include <stdio.h>
#include "jsextension.h"
#include "part_util.h"

struct _InstallerConf InstallerConf;

static
char* find_path_by_uuid(const char* uuid)
{
    g_assert(uuid != NULL);

    if (g_str_has_prefix (uuid, "disk")) {
        return installer_get_disk_path (uuid);
    } else if (g_str_has_prefix (uuid, "part")) {
        return installer_get_partition_path (uuid);
    } else if (g_str_has_prefix (uuid, "free")) {
        char** sets = g_strsplit(uuid, ":", 3);
        g_assert(g_strv_length(sets) == 3);
        char* path = find_partition_path_by_sector_and_disk_path(sets[1],
            g_ascii_strtod(sets[2], NULL));
        g_debug("[%s] ry find free partition's path: %s: %d = %s\n", __func__,
                sets[1], (int)g_ascii_strtod(sets[2], NULL), path);
        g_strfreev(sets);
        return path;
    } else {
        return g_strdup(uuid);
    }
}

char* installer_conf_to_string()
{
    g_assert(InstallerConf.root_partition != NULL);
    g_assert(InstallerConf.root_disk != NULL);

    GString* mp = g_string_new(NULL);
    GHashTableIter iter;
    gpointer key, value;

    if (InstallerConf.mount_points != NULL) {
        g_hash_table_iter_init(&iter, InstallerConf.mount_points);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            g_string_append_printf(mp, "%s=%s;", (gchar*)key, (gchar*)value);
        }
    }

    return g_strdup_printf("\n"
        "DI_BOOTLOADER=\"%s\"\n"
        "DI_SIMPLE_MODE=\"%s\"\n"
        "DI_UEFI=\"%s\"\n"
        "DI_USERNAME=\"%s\"\n"
        "DI_PASSWORD=\"%s\"\n"
        "DI_HOSTNAME=\"%s\"\n"
        "DI_TIMEZONE=\"%s\"\n"
        "DI_LOCALE=\"%s\"\n"
        "DI_LAYOUT=\"%s\"\n"
        "DI_LAYOUT_VARIANT=\"%s\"\n"
        "DI_MOUNTPOINTS=\"%s\"\n"
        "DI_ROOT_PARTITION=\"%s\"\n"
        "DI_ROOT_DISK=\"%s\"\n",
        InstallerConf.bootloader,
        InstallerConf.simple_mode ? "true" : "false",
        InstallerConf.uefi ? "true" : "false",
        InstallerConf.user_name,
        InstallerConf.password,
        InstallerConf.host_name,
        InstallerConf.timezone ? : "",
        InstallerConf.locale ? : "",
        InstallerConf.layout ? : "",
        InstallerConf.layout_variant ? : "",
        g_string_free(mp, FALSE),
        InstallerConf.root_partition,
        InstallerConf.root_disk
    );
}
void normalization()
{
    g_assert(InstallerConf.root_disk != NULL);
    g_assert(g_file_test(InstallerConf.root_disk, G_FILE_TEST_EXISTS));
    g_assert(InstallerConf.root_partition != NULL);
    //g_assert(g_file_test(InstallerConf.root_partition, G_FILE_TEST_EXISTS));
    if (InstallerConf.simple_mode && InstallerConf.bootloader == NULL) {
        InstallerConf.bootloader = g_strdup(InstallerConf.root_disk);
    }
    g_assert(InstallerConf.bootloader != NULL);
}

void write_installer_conf(const char* path)
{
    g_assert(path != NULL);

    normalization();
    char* content = installer_conf_to_string();
    GError* error = NULL;
    g_file_set_contents(path, content, -1, &error);
    g_free(content);

    if (error != NULL) {
        g_warning("[%s] conf(to %s) failed:%s\n", __func__, path,
                  error->message);
        g_error_free(error);
    }
}

JS_EXPORT_API
void installer_record_mountpoint_info(const char* uuid, const char* mountpoint)
{
    g_message("[%s] uuid: %s, mountpoint: %s\n", __func__, uuid, mountpoint);
    g_return_if_fail(uuid != NULL);
    g_return_if_fail(mountpoint != NULL);

    if (InstallerConf.mount_points == NULL) {
        InstallerConf.mount_points = g_hash_table_new_full(g_str_hash,
            g_str_equal, g_free, g_free);
    }
    char* path = find_path_by_uuid(uuid);
    g_return_if_fail(path != NULL);

    g_message("[%s]: uuid: %s, path: %s, mountpoint: %s \n", __func__, uuid,
              path, mountpoint);
    if (g_strcmp0(mountpoint, "/") == 0) {
        if (InstallerConf.root_partition) {
            g_free(InstallerConf.root_partition);
        }
        ped_print();
        InstallerConf.root_partition = path;
    } else {
        g_hash_table_insert(InstallerConf.mount_points, path,
                            g_strdup(mountpoint));
    }
}

JS_EXPORT_API
void installer_record_accounts_info(const char* name, const char* hostname,
                                    const char* password)
{
    g_return_if_fail(name != NULL);
    g_return_if_fail(hostname != NULL);
    g_return_if_fail(password != NULL);

    if(InstallerConf.user_name)
    g_free(InstallerConf.user_name);
    if(InstallerConf.password)
    g_free(InstallerConf.password);
    if(InstallerConf.host_name)
    g_free(InstallerConf.host_name);

    InstallerConf.user_name = g_strdup(name);
    InstallerConf.password = g_base64_encode(password, strlen(password));
    InstallerConf.host_name = g_strdup(hostname);
}

JS_EXPORT_API
void installer_record_bootloader_info(const char* uuid, gboolean uefi)
{
    if (uefi == FALSE) {
        g_assert(uuid != NULL);
    }

    InstallerConf.uefi = uefi;

    if (uuid != NULL) {
    if (InstallerConf.bootloader)
        g_free(InstallerConf.bootloader);
        InstallerConf.bootloader = find_path_by_uuid(uuid);
    }

    g_debug("[%s] uuid: %s, uefi: %d\n", __func__, uuid, uefi);

}

JS_EXPORT_API
void installer_record_locale_info(const char* locale)
{
    g_return_if_fail(locale != NULL);
    if (InstallerConf.locale) {
        g_free(InstallerConf.locale);
    }

    InstallerConf.locale = g_strdup(locale);
}

JS_EXPORT_API
void installer_record_timezone_info(const char* timezone)
{
    g_return_if_fail(timezone != NULL);
    if (InstallerConf.timezone) {
        g_free(InstallerConf.timezone);
    }

    InstallerConf.timezone = g_strdup(timezone);
}

JS_EXPORT_API
void installer_record_keyboard_layout_info(const char* layout, const char* variant)
{
    g_return_if_fail(layout != NULL);

    if (InstallerConf.layout) {
        g_free(InstallerConf.layout);
    }
    if (InstallerConf.layout_variant) {
        g_free(InstallerConf.layout_variant);
    }

    InstallerConf.layout = g_strdup(layout);
    InstallerConf.layout_variant = g_strdup(variant);
}


JS_EXPORT_API
void installer_record_simple_mode_info(gboolean simple)
{
    InstallerConf.simple_mode = simple;
}

JS_EXPORT_API
void installer_record_root_disk_info(const char* disk)
{
    g_return_if_fail(disk != NULL);

    if (InstallerConf.root_disk) {
        g_free(InstallerConf.root_disk);
    }
    InstallerConf.root_disk = find_path_by_uuid(disk);
}

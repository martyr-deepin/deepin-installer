/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "scheduler.h"
#include "jsextension.h"
#include "info.h"
#include "fs_util.h"
#include "esp.h"

#include <glib.h>
#include <gio/gio.h>

#define CONF_PATH "/etc/deepin-installer.conf"

void run_hooks_before_chroot();
void run_hooks_in_chroot();
void run_hooks_after_chroot();

enum {
    STAGE_START_INSTALL,
    STAGE_HOOKS_BEFORE_CHROOT,
    STAGE_HOOKS_IN_CHROOT,
    STAGE_HOOKS_AFTER_CHROOT,
    STAGE_INSTALL_FINISH,
};

void update_install_progress(int v)
{
    static int current_per = 0;
    if (v < current_per) {
        g_debug("[%s] INSTALL progress small previous PROGRESS: %d <= %d\n",
                __func__, v, current_per);
        return;
    }
    current_per=  v;
    js_post_message("install_progress",
                    jsvalue_from_number(get_global_context(), v));
}

void installer_terminate()
{
    g_message("[%s]\n", __func__);
    js_post_message("install_terminate", NULL);
}

void enter_next_stage()
{
    static int current_stage = STAGE_START_INSTALL;
    g_message("[%s], current_stage: %d\n", __func__, current_stage);

    switch (current_stage) {
    case STAGE_START_INSTALL:
        current_stage = STAGE_HOOKS_BEFORE_CHROOT;
        run_hooks_before_chroot();
        break;

    case STAGE_HOOKS_BEFORE_CHROOT:
        current_stage = STAGE_HOOKS_IN_CHROOT;
        run_hooks_in_chroot();
        break;

    case STAGE_HOOKS_IN_CHROOT:
        current_stage = STAGE_HOOKS_AFTER_CHROOT;
        run_hooks_after_chroot();
        break;

    case STAGE_HOOKS_AFTER_CHROOT:
        current_stage = STAGE_INSTALL_FINISH;
        update_install_progress(100);
        break;

    default:
        g_assert_not_reached();
    }
}

static GHashTable* mkfs_pending_list = NULL;

void mkfs_latter(const char* path, const char* fs)
{
    g_message("[%s], path: %s, fs: %s\n", __func__, path, fs);
    g_return_if_fail(path != NULL);
    g_return_if_fail(fs != NULL);
    if (mkfs_pending_list == NULL) {
        mkfs_pending_list = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                  g_free, g_free);
    }
    g_hash_table_insert(mkfs_pending_list, g_strdup(path), g_strdup(fs));
}

static void do_mkfs()
{
    g_message("[%s]\n", __func__);
    g_assert(mkfs_pending_list != NULL);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, mkfs_pending_list);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        mkfs((char*)key, (char*)value);
    }
    g_hash_table_destroy(mkfs_pending_list);
}

static void start_run_installer()
{
    g_message("[%s]\n", __func__);
    ped_device_free_all();
    enter_next_stage();
}

static void start_prepare_conf()
{
    g_message("[%s]\n", __func__);
    extern char* auto_conf_path;
    if (auto_conf_path != NULL) {
        GError* error = NULL;
        char* cmd = g_strdup_printf("sh -c '[ %s -ef %s ] || cp -f %s %s'",
                                    auto_conf_path, CONF_PATH,
                                    auto_conf_path, CONF_PATH);
        g_warning("%s\n", cmd);
        int exit_code = 0;
        g_spawn_command_line_sync(cmd, NULL, NULL, &exit_code, &error);
        g_free(cmd);
        if (error != NULL) {
            //TODO: report error
            g_warning("[%s] auto install mode failed: %s, cmd: %s", __func__,
                      error->message, cmd);
            g_clear_error(&error);
            installer_terminate();
            return;
        }
        if (exit_code != 0) {
            installer_terminate();
            return;
        }
    } else {
        if (InstallerConf.simple_mode && InstallerConf.uefi) {
            auto_handle_esp();
        }
        write_installer_conf(CONF_PATH);
    }

    start_run_installer();
}

JS_EXPORT_API
void installer_start_install()
{
    g_message("[%s]\n", __func__);
    if (mkfs_pending_list != NULL) {
        GTask* task = g_task_new(NULL, NULL, start_prepare_conf, NULL);
        g_task_run_in_thread(task, do_mkfs);
    } else {
        start_prepare_conf();
    }
}

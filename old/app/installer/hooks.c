/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#define _GNU_SOURCE
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "base.h"
#include "scheduler.h"

typedef struct _HookInfo {
    const char* jobs_path;
    int progress_begin;
    int progress_end;
    GList* jobs;
    int current_job_num;
} HookInfo;

// chroot_fd is used to break out of chroot jail.
static int chroot_fd;
static gboolean in_chroot = FALSE;

HookInfo before_chroot_info = { HOOKS_DIR"/before_chroot", 5, 80, NULL, 0};

#define TMP_HOOKS_DIR "/tmp/hooks"
HookInfo in_chroot_info = { TMP_HOOKS_DIR"/in_chroot", 80, 90, NULL, 0};

HookInfo after_chroot_info = { HOOKS_DIR"/after_chroot", 90, 100, NULL, 0};


static void run_hooks(HookInfo* info);
static gboolean monitor_extract_progress();
static void setup_monitor_extract_progress();


void ensure_we_can_find_in_chroot_hooks()
{
    g_assert(!in_chroot);
    g_mkdir_with_parents("/target/"TMP_HOOKS_DIR"/in_chroot", 0755);
    GError* error = NULL;
    g_spawn_command_line_sync("sh -c 'cp -rf "HOOKS_DIR"/in_chroot/* /target/"TMP_HOOKS_DIR"/in_chroot/'", NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning("[%s] can't setup in_chroot_hooks:%s", __func__,
                  error->message);
        g_error_free(error);
        g_assert_not_reached();
    }
}

// Chroot to `/target`
gboolean enter_chroot()
{
    //Never use "." instead of "/" otherwise if "." equal TARGET then we can't break chroot"
    if ((chroot_fd = open ("/", O_RDONLY)) < 0) {
        g_warning ("[%s]: set chroot fd failed\n", __func__);
        return FALSE;
    }

    chdir("/target"); //change to a valid directory
    if (chroot ("/target") == 0) {
        chdir("/"); //change to a valid directory
        in_chroot = TRUE;
        return TRUE;
    } else {
        g_warning ("[%s]: chroot to /target falied:%s\n", __func__,
                   g_strerror (errno));
        return FALSE;
    }
}

// Escape from a chroot jail
gboolean break_chroot()
{
    if (in_chroot) {
        if (fchdir (chroot_fd) != 0) {
            g_warning ("[%s] reset to chroot fd dir failed\n", __func__);
            return !in_chroot;
        }
        chroot (".");
        in_chroot = FALSE;
    }
    return !in_chroot;
}

void run_hooks_before_chroot()
{
    run_hooks(&before_chroot_info);
    setup_monitor_extract_progress();
}

void run_hooks_in_chroot()
{
    ensure_we_can_find_in_chroot_hooks();
    if (enter_chroot() == FALSE) {
        installer_terminate();
    }
    run_hooks(&in_chroot_info);
}

void run_hooks_after_chroot()
{
    if (break_chroot() == FALSE) {
        installer_terminate();
    }
    run_hooks(&after_chroot_info);
}

void update_hooks_progress(HookInfo* info)
{
    if (info == &before_chroot_info) {
    // extract squashfs is in before chroot info, special treat it with monitor_extract_progress()
        return;
    }
    double ratio = (info->current_job_num + 1) * 1.0 /
                   (g_list_length(g_list_first(info->jobs)) - 1);
    g_assert(ratio > 0 && ratio <= 1);
    double p = info->progress_begin + 
               (info->progress_end - info->progress_begin) * ratio;
    g_assert(p > 0 && p <= 100);
    update_install_progress((int)p);
}

void run_one_by_one(GPid pid, gint status, HookInfo* info)
{
    g_message("[%s]\n", __func__);
    GError* error = NULL;

    if (pid != -1) {
        g_spawn_close_pid(pid);
        g_spawn_check_exit_status(status, &error);
        if (error != NULL) {
            g_warning("[%s] run hook(%s) failed: %s\n", __func__,
                     (char*)g_list_nth_data(g_list_first(info->jobs),
                                            info->current_job_num),
                     error->message);
            g_error_free(error);
            break_chroot();
            installer_terminate();
            return;
        }
    }

    if (info->jobs->data == NULL) {
        g_message("[%s] jobs is empty\n", __func__);
        g_list_free_full(g_list_first(info->jobs), g_free);
        enter_next_stage();
        return;
    }

    gint std_out, std_err;
    GPid child_pid;

    char* argv[2];
    argv[0] = info->jobs->data;
    argv[1] = 0;

    info->current_job_num = g_list_index(g_list_first(info->jobs),
                                         info->jobs->data);
    info->jobs = g_list_next(info->jobs);
    g_spawn_async(info->jobs_path, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                  NULL, NULL, &child_pid, &error);
    if (error != NULL) {
        g_warning("[%s] can't spawn %s: %s\n", __func__, argv[0],
                  error->message);
        g_error_free(error);
        return;
    }

    g_child_watch_add(child_pid, (GChildWatchFunc)run_one_by_one, info);
    update_hooks_progress(info);
}

void run_hooks(HookInfo* info)
{
    const char* path = info->jobs_path;
    GError* error = NULL;
    GDir* dir = g_dir_open(path, 0, &error);
    if (error != NULL) {
        g_warning("[%s] can't exec_hoosk %s: %s\n", __func__, path,
                  error->message);
        g_error_free(error);
        return;
    }

    const char* job_name = NULL;
    while (job_name = g_dir_read_name(dir)) {
        if (g_str_has_suffix(job_name, ".job")) {
            info->jobs = g_list_append(info->jobs,
                (gpointer)g_build_filename(path, job_name, NULL));
        }
    }
    g_dir_close(dir);
    chdir(path);

    info->jobs = g_list_sort(info->jobs, (GCompareFunc)strcmp);

    //append the end guard element so that run_one_by_one can free the GList
    info->jobs = g_list_append(info->jobs, NULL);

    run_one_by_one(-1, 0, info);
}

enum {
    EXTRACT_PROGRESS_NONE,

    EXTRACT_PROGRESS_BASE,
    EXTRACT_PROGRESS_BASE_END,

    EXTRACT_PROGRESS_LANG,
    EXTRACT_PROGRESS_LANG_END
};

static int read_progress(const char* path)
{
    char* contents = NULL;
    if (g_file_get_contents(path, &contents, NULL, NULL)) {
        char* endptr = NULL;
        double v = g_strtod(contents, &endptr);
        g_free(contents);
        if (endptr == NULL) {
            return -1;
        }
        if (v < 0 || v > 100) {
            g_warning("[%s] invalid progress value(%d) read from %s(%s)\n",
                      __func__, (int)v, path, contents);
            return 0;
        }
        return v;
    } else {
        g_free(contents);
        return -1;
    }
}

#define PROGRESS_LOG_BASE "/tmp/deepin-installer/unsquashfs_base_progress"
#define PROGRESS_LOG_LANG "/tmp/deepin-installer/unsquashfs_lang_progress"

static gboolean monitor_extract_progress(HookInfo* info)
{
    static int stage = EXTRACT_PROGRESS_NONE;

    int v = 0;
    switch (stage) {
    case EXTRACT_PROGRESS_NONE:
        v = read_progress(PROGRESS_LOG_BASE);
        if (v != -1) {
        stage = EXTRACT_PROGRESS_BASE;
        }
        return TRUE;
    case EXTRACT_PROGRESS_BASE:
        v = read_progress(PROGRESS_LOG_BASE);
        if (v >= 100) {
        stage = EXTRACT_PROGRESS_BASE_END;
        }

        double ratio = v / 100.0;
        //extract lang pack use 10% time
        update_install_progress(info->progress_begin +
                                (info->progress_end - 10 ) * ratio);
        return TRUE;
    case EXTRACT_PROGRESS_BASE_END:
        v = read_progress(PROGRESS_LOG_LANG);
        if (v != -1) {
            stage = EXTRACT_PROGRESS_LANG;
        }
        return TRUE;
    case EXTRACT_PROGRESS_LANG:
        v = read_progress(PROGRESS_LOG_LANG);
        if (v >= 100) {
            stage = EXTRACT_PROGRESS_LANG_END;
        }

        update_install_progress(info->progress_end - 10 + 10 * ratio);
        return TRUE;
    case EXTRACT_PROGRESS_LANG_END:
        g_message("[%s] END Monitor_extract_progress\n", __func__);
        return FALSE;
    }
}

void setup_monitor_extract_progress()
{
    g_remove(PROGRESS_LOG_BASE);
    g_remove(PROGRESS_LOG_LANG);
    g_timeout_add_seconds(1, (GSourceFunc)monitor_extract_progress,
                          &before_chroot_info);
}

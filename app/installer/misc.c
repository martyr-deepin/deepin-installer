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

#include "misc.h"
#include "part_util.h"
#include "fs_util.h"
#include "info.h"
#include "hooks.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <parted/parted.h>
#include <unistd.h>

#define LOG_FILE_PATH           "/tmp/installer.log"


static GFile* _get_gfile_from_gapp(GDesktopAppInfo* info);

static GList *filelist = NULL;

static gboolean 
mount_procfs ()
{
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar *dev_target = NULL;
    gchar *devpts_target = NULL;
    gchar *proc_target = NULL;
    gchar *sys_target = NULL;
    gchar *mount_dev = NULL;
    gchar *mount_devpts = NULL;
    gchar *mount_proc = NULL;
    gchar *mount_sys = NULL;

    dev_target = g_strdup_printf ("%s/dev", TARGET);
    devpts_target = g_strdup_printf ("%s/dev/pts", TARGET);
    proc_target = g_strdup_printf ("%s/proc", TARGET);
    sys_target = g_strdup_printf ("%s/sys", TARGET);

    mount_dev = g_strdup_printf ("mount -v --bind /dev %s/dev", TARGET);
    mount_devpts = g_strdup_printf ("mount -vt devpts devpts %s/dev/pts", TARGET);
    mount_proc = g_strdup_printf ("mount -vt proc proc %s/proc", TARGET);
    mount_sys = g_strdup_printf ("mount -vt sysfs sysfs %s/sys", TARGET);

    guint dev_before = get_mount_target_count (dev_target);
    g_spawn_command_line_sync (mount_dev, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount dev %s\n", error->message);
        goto out;
    }
    guint dev_after = get_mount_target_count (dev_target);
    if (dev_after != dev_before + 1) {
        g_warning ("mount procfs:mount dev not changed\n");
        goto out;
    }
    
    guint devpts_before = get_mount_target_count (devpts_target);
    g_spawn_command_line_sync (mount_devpts, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount devpts %s\n", error->message);
        goto out;
    }
    guint devpts_after  = get_mount_target_count (devpts_target);
    if (devpts_after != devpts_before + 1) {
        g_warning ("mount procfs:mount devpts not changed\n");
        goto out;
    }

    guint proc_before = get_mount_target_count (proc_target);
    g_spawn_command_line_sync (mount_proc, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount proc %s\n", error->message);
        goto out;
    }
    guint proc_after = get_mount_target_count (proc_target);
    if (proc_after != proc_before + 1) {
        g_warning ("mount procfs:mount proc not changed\n");
        goto out;
    }

    guint sys_before = get_mount_target_count (sys_target);
    g_spawn_command_line_sync (mount_sys, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount sys %s\n", error->message);
        goto out;
    }
    guint sys_after = get_mount_target_count (sys_target);
    if (sys_after != sys_before + 1) {
        g_warning ("mount procfs:mount sys not changed\n");
        goto out;
    }
    ret = TRUE;
    goto out;

out:
    g_free (dev_target);
    g_free (devpts_target);
    g_free (proc_target);
    g_free (sys_target);

    g_free (mount_dev);
    g_free (mount_devpts);
    g_free (mount_proc);
    g_free (mount_sys);
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
    return ret;
}

JS_EXPORT_API
gboolean installer_chroot_target ()
{
    write_installer_conf("/target/etc/deepin-installer.conf");

    gboolean ret = FALSE;
    if (!mount_procfs ()) {
        goto out;
    }

    extern int chroot_fd;
    if ((chroot_fd = open (".", O_RDONLY)) < 0) {
        g_warning ("chroot:set chroot fd failed\n");
        goto out;
    }

    extern gboolean in_chroot;
    if (chroot (TARGET) == 0) {
	chdir("/"); //change to an valid directory
        in_chroot = TRUE;
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to %s falied:%s\n", TARGET, strerror (errno));
    }
    goto out;

out:
    if (ret) {
        emit_progress ("chroot", "finish");
    } else {
        emit_progress ("chroot", "terminate");
    }
    return ret;
}

//unmount after break chroot
static void 
unmount_target ()
{
    g_debug ("finish install:unmount target\n");
    extern gboolean in_chroot;
    if (in_chroot) {
        g_warning ("unmount mount:in chroot\n");
        return;
    }

    gchar *umount_sys_cmd = g_strdup_printf ("umount -l %s/sys", TARGET);
    gchar *umount_proc_cmd = g_strdup_printf ("umount -l %s/proc", TARGET);
    gchar *umount_devpts_cmd = g_strdup_printf ("umount -l %s/dev/pts", TARGET);
    gchar *umount_dev_cmd = g_strdup_printf ("umount -l %s/dev", TARGET);
    gchar *umount_target_cmd = g_strdup_printf ("umount -l %s", TARGET);

    g_spawn_command_line_sync (umount_sys_cmd, NULL, NULL, NULL, NULL);
    g_spawn_command_line_sync (umount_proc_cmd, NULL, NULL, NULL, NULL);
    g_spawn_command_line_sync (umount_devpts_cmd, NULL, NULL, NULL, NULL);
    g_spawn_command_line_sync (umount_dev_cmd, NULL, NULL, NULL, NULL);
    extern GList *mounted_list;
    int i;
    for (i = 0 ; i < g_list_length (mounted_list); i++) {
        gchar *umount_cmd = g_strdup_printf ("umount -l %s", (gchar *) g_list_nth_data (mounted_list, i));
        g_spawn_command_line_sync (umount_cmd, NULL, NULL, NULL, NULL);
        g_free (umount_cmd);
    }
    g_spawn_command_line_sync (umount_target_cmd, NULL, NULL, NULL, NULL);

    while (g_file_test (TARGET, G_FILE_TEST_IS_DIR)) {
        if (g_rmdir (TARGET) != 0) {
            g_spawn_command_line_sync (umount_target_cmd, NULL, NULL, NULL, NULL);
            g_usleep (1000);
        }
    }

    g_list_free_full (mounted_list, g_free);
    g_free (umount_sys_cmd);
    g_free (umount_proc_cmd);
    g_free (umount_devpts_cmd);
    g_free (umount_dev_cmd);
    g_free (umount_target_cmd);
}

static
GFile* _get_gfile_from_gapp(GDesktopAppInfo* info)
{
    return g_file_new_for_commandline_arg(g_desktop_app_info_get_filename(info));
}


void
finish_install_cleanup () 
{
    g_message ("finish install cleanup\n");
    static gboolean cleaned = FALSE;
    if (cleaned) {
        g_warning ("finish install cleanup:already cleaned\n");
        return;
    }
    cleaned = TRUE;

    run_hooks_in_chroot();
    
    extern gboolean in_chroot;
    extern int chroot_fd;

    if (in_chroot) {

        if (fchdir (chroot_fd) < 0) {
            g_warning ("finish install:reset to chroot fd dir failed\n");
        } else {
            int i = 0;
            for (i = 0; i < 1024; i++) {
                chdir ("..");
            }
        }
        chroot (".");
        in_chroot = FALSE;
    }
    unmount_target ();
    ped_device_free_all ();
}

JS_EXPORT_API 
void  installer_show_log ()
{
    gchar *cmd = g_strdup_printf ("xdg-open %s\n", LOG_FILE_PATH);
    g_spawn_command_line_async (cmd, NULL);
    g_free (cmd);
}



JS_EXPORT_API
char* installer_get_default_lang_pack()
{
    char* contents = NULL;
    if (!g_file_get_contents("/proc/cmdline", &contents, NULL, NULL)) {
	return g_strdup("en_US");
    }

    gchar* begin = g_strstr_len(contents, -1, "locale=");
    if (begin == NULL) {
	g_free(contents);
	return g_strdup("en_US");
    }
    g_free(contents);

    int end = 0;
    for (; begin[end] != ' ' && begin[end] != '.' && begin[end] != '\0'; end++);
    if (end == 0) {
	return g_strdup("en_US");
    }


    char* new_str = g_strdup(begin);
    new_str[end] = '\0';
    char* ret = g_strdup(new_str+ 7);
    g_free(new_str);
    return ret;
}

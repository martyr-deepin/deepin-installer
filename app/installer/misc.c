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

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <parted/parted.h>
#include "misc.h"
#include "part_util.h"
#include "fs_util.h"
#include "../../lib/dentry/entry.h"

#define PACKAGES_LIST_PATH      RESOURCE_DIR"/installer/blacklist.ini"
#define LOG_FILE_PATH           "/tmp/installer.log"
#define SCRIPTS_PATH    "/usr/share/deepin-installer/post-install"

extern int chroot(const char *path);
extern int fchdir(int fd);
extern int chdir(const char *path);


static GFile* _get_gfile_from_gapp(GDesktopAppInfo* info);
static ArrayContainer _normalize_array_container(ArrayContainer pfs);

extern ArrayContainer dentry_list_files(GFile* f);
extern char* dentry_get_name(Entry* e);
extern Entry* dentry_create_by_path(const char* path);
extern void dentry_copy (ArrayContainer fs, GFile* dest);



static GList *filelist = NULL;

static gboolean 
mount_procfs ()
{
    gboolean ret = FALSE;
    GError *error = NULL;
    extern const gchar* target;
    gchar *dev_target = NULL;
    gchar *devpts_target = NULL;
    gchar *proc_target = NULL;
    gchar *sys_target = NULL;
    gchar *mount_dev = NULL;
    gchar *mount_devpts = NULL;
    gchar *mount_proc = NULL;
    gchar *mount_sys = NULL;

    if (target == NULL) {
        g_warning ("mount procfs:target is NULL\n");
        goto out;
    }
    dev_target = g_strdup_printf ("%s/dev", target);
    devpts_target = g_strdup_printf ("%s/dev/pts", target);
    proc_target = g_strdup_printf ("%s/proc", target);
    sys_target = g_strdup_printf ("%s/sys", target);

    mount_dev = g_strdup_printf ("mount -v --bind /dev %s/dev", target);
    mount_devpts = g_strdup_printf ("mount -vt devpts devpts %s/dev/pts", target);
    mount_proc = g_strdup_printf ("mount -vt proc proc %s/proc", target);
    mount_sys = g_strdup_printf ("mount -vt sysfs sysfs %s/sys", target);

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
    gboolean ret = FALSE;
    if (!mount_procfs ()) {
        goto out;
    }

    extern const gchar* target;
    if (target == NULL) {
        g_warning ("chroot:target is NULL\n");
        goto out;
    }
    extern int chroot_fd;
    if ((chroot_fd = open (".", O_RDONLY)) < 0) {
        g_warning ("chroot:set chroot fd failed\n");
        goto out;
    }

    extern gboolean in_chroot;
    if (chroot (target) == 0) {
        in_chroot = TRUE;
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to %s falied:%s\n", target, strerror (errno));
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
    extern const gchar *target;
    if (target == NULL) {
        g_warning ("unmount mount:target is NULL\n");
        return;
    } 
    extern gboolean in_chroot;
    if (in_chroot) {
        g_warning ("unmount mount:in chroot\n");
        return;
    }

    gchar *umount_sys_cmd = g_strdup_printf ("umount -l %s/sys", target);
    gchar *umount_proc_cmd = g_strdup_printf ("umount -l %s/proc", target);
    gchar *umount_devpts_cmd = g_strdup_printf ("umount -l %s/dev/pts", target);
    gchar *umount_dev_cmd = g_strdup_printf ("umount -l %s/dev", target);
    gchar *umount_target_cmd = g_strdup_printf ("umount -l %s", target);

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

    while (g_file_test (target, G_FILE_TEST_IS_DIR)) {
        if (g_rmdir (target) != 0) {
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


static ArrayContainer _normalize_array_container(ArrayContainer pfs)
{
    GPtrArray* array = g_ptr_array_new();

    GFile** _array = pfs.data;
    for(size_t i=0; i<pfs.num; i++) {
        if (G_IS_DESKTOP_APP_INFO(_array[i])) {
            g_ptr_array_add(array, _get_gfile_from_gapp(((GDesktopAppInfo*)_array[i])));
        } else {
            g_ptr_array_add(array, g_object_ref(_array[i]));
        }
    }

    ArrayContainer ret;
    ret.num = pfs.num;
    ret.data = g_ptr_array_free(array, FALSE);
    return ret;
}


static void
excute_scripts()
{
    
    extern gboolean in_chroot;
    extern const gchar* target;
    if (!in_chroot) {
        g_warning ("excute_scripts:not in chroot\n");
        return;
    }
    
    ArrayContainer fs;
    GFile* src = g_file_new_for_path(SCRIPTS_PATH);
    fs = dentry_list_files(src);
    g_object_unref(src);
    g_assert(fs.num > 1);
    
    if(fs.num == 0){
        return;
    }
    
    //1.copy to target
    GFile* dest = g_file_new_for_path(target);
    dentry_copy(fs,dest);
    g_object_unref(dest);
    
    //2.excute
    const ArrayContainer _fs = _normalize_array_container(fs);
    GFile** files = _fs.data;
    for (size_t i=0; i<_fs.num; i++) {
        GFile *f = files[i];
        gchar *name = dentry_get_name(f);
        g_message("excute_scripts:script name :%s.",name);
        
        GError *error = NULL;
        const gchar *cmd = g_strdup_printf ("chroot %s /bin/bash -c \"./%s\"", target, name);
        g_message("excute_scripts:cmd :%s.",cmd);
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
        if (error != NULL) {
            g_warning ("excute_scripts:excute failed:%s\n", error->message);
            g_error_free (error);
            error = NULL;
        }
        
        g_free(name);
        g_object_unref(f);
    }
    g_free(_fs.data);

}


static void
fix_networkmanager ()
{
    extern gboolean in_chroot;
    if (!in_chroot) {
        g_warning ("fix networkmanager:not in chroot\n");
        return;
    }


    GError *error = NULL;
    const gchar *cmd = "sed -i 's/managed=false/managed=true/g' /etc/NetworkManager/NetworkManager.conf";
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("fix networkmanager:%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}

static void
remove_packages ()
{
    extern gboolean in_chroot;
    if (!in_chroot) {
        g_warning ("remove packages:not in chroot\n");
        return ;
    }

    GError *error = NULL;
    gchar *cmd = NULL;
    gchar *contents = NULL;
    gchar **strarray = NULL;
    gchar *packages = NULL;

    if (!g_file_test (PACKAGES_LIST_PATH, G_FILE_TEST_EXISTS)) {
        g_warning ("remove packages:%s not exists\n", PACKAGES_LIST_PATH);
        goto out;
    }
    g_file_get_contents (PACKAGES_LIST_PATH, &contents, NULL, &error);
    if (error != NULL) {
        g_warning ("remove packages:get packages list %s\n", error->message);
        goto out;
    }
    if (contents == NULL) {
        g_warning ("remove packages:contents NULL\n");
        goto out;
    }
    strarray = g_strsplit (contents, "\n", -1);
    if (strarray == NULL) {
       g_warning ("remove packages:strarray NULL\n"); 
       goto out;
    }
    packages = g_strjoinv (" ", strarray);
    if (packages == NULL) {
        g_warning ("remove packages:packages NULL\n");
        goto out;
    }

    if (g_file_test ("/var/lib/apt/lock", G_FILE_TEST_EXISTS)) {
       g_unlink ("/var/lib/apt/lock"); 
    }
    
    cmd = g_strdup_printf ("apt-get remove -y %s", packages);
    g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("remove packages:%s\n", error->message);
    }
    goto out;

out:
    g_free (cmd);
    g_free (contents);
    g_strfreev (strarray);
    g_free (packages);
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
}

void
finish_install_cleanup () 
{
    g_message ("finish install cleanup\n");
    /*excute_scripts();*/
    static gboolean cleaned = FALSE;
    if (cleaned) {
        g_warning ("finish install cleanup:already cleaned\n");
        return;
    }
    cleaned = TRUE;

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("finish install:target is NULL\n");
    } 
    extern gboolean in_chroot;
    extern int chroot_fd;

    if (in_chroot) {
        fix_networkmanager ();
        /*excute_scripts();*/
        remove_packages ();
        if (fchdir (chroot_fd) < 0) {
            g_warning ("finish install:reset to chroot fd dir failed\n");
        } else {
            int i = 0;
            for (i = 0; i < 1024; i++) {
                chdir ("..");
            }
            chroot (".");
        }
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


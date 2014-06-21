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

#include "grub.h"
#include "part_util.h"
#include "misc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*#define EFI_BOOT_MGR            RESOURCE_DIR"/installer/efibootmgr.deb"*/
/*#define GRUB_COMMON             RESOURCE_DIR"/installer/grub-common.deb"*/
/*#define GRUB2_COMMON            RESOURCE_DIR"/installer/grub2-common.deb"*/
/*#define GRUB_EFI_AMD64_BIN      RESOURCE_DIR"/installer/grub-efi-amd64-bin.deb"*/
/*#define GRUB_EFI_AMD64          RESOURCE_DIR"/installer/grub-efi-amd64.deb" */

/*static void*/
/*install_grub_efi_amd64 ()*/
/*{*/
    /*g_spawn_command_line_sync ("apt-get remove -y grub-pc grub2-common", NULL, NULL, NULL, NULL);*/
    /*g_warning ("----------------remove grub-pc grub2-common finish-------------------------\n");*/

    /*gchar *bootmgr = g_strdup_printf ("dpkg -i %s", EFI_BOOT_MGR);*/
    /*g_spawn_command_line_sync (bootmgr, NULL, NULL, NULL, NULL);*/
    /*g_free (bootmgr);*/
    /*g_warning ("----------------install efi boot mgr finish-------------------------\n");*/

    /*gchar *grub_common = g_strdup_printf ("dpkg -i %s", GRUB_COMMON);*/
    /*g_spawn_command_line_sync (grub_common, NULL, NULL, NULL, NULL);*/
    /*g_free (grub_common);*/
    /*g_warning ("----------------install grub-common finish-------------------------\n");*/

    /*gchar *grub2_common = g_strdup_printf ("dpkg -i %s", GRUB2_COMMON);*/
    /*g_spawn_command_line_sync (grub2_common, NULL, NULL, NULL, NULL);*/
    /*g_free (grub2_common);*/
    /*g_warning ("----------------install grub2-common finish-------------------------\n");*/

    /*gchar *bin = g_strdup_printf ("dpkg -i %s", GRUB_EFI_AMD64_BIN);*/
    /*g_spawn_command_line_sync (bin, NULL, NULL, NULL, NULL);*/
    /*g_free (bin);*/
    /*g_warning ("----------------install grub efi amd64 bin finish-------------------------\n");*/

    /*gchar *efi = g_strdup_printf ("dpkg -i %s", GRUB_EFI_AMD64);*/
    /*g_spawn_command_line_sync (efi, NULL, NULL, NULL, NULL);*/
    /*g_free (efi);*/
    /*g_warning ("----------------install grub efi amd64 finish-------------------------\n");*/
/*}*/

/*static gpointer*/
/*thread_update_grub (gpointer data)*/
/*{*/
    /*struct GrubHandler *handler = (struct GrubHandler *) data;*/
    /*gboolean ret = FALSE;*/
    /*gchar *path = NULL;*/
    /*gchar *grub_install = NULL;*/
    /*GError *error = NULL;*/

    /*if (handler == NULL || handler->uuid == NULL) {*/
        /*g_warning ("update grub:destination uuid NULL\n");*/
        /*goto out;*/
    /*}*/
    /*if (g_str_has_prefix (handler->uuid, "disk")) {*/
        /*path = installer_get_disk_path (handler->uuid);*/
    /*} else if (g_str_has_prefix (handler->uuid, "part")) {*/
        /*path = installer_get_partition_path (handler->uuid);*/
    /*} else {*/
        /*g_warning ("update grub:invalid uuid %s\n", handler->uuid);*/
        /*goto out;*/
    /*}*/

    /*if (handler->uefi) {*/
        /*install_grub_efi_amd64 ();*/
        /*emit_progress ("bootloader", "96%");*/
        /*grub_install = g_strdup_printf ("grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=linuxdeepin2014 --recheck --debug");*/
    /*} else {*/
        /*emit_progress ("bootloader", "96%");*/
        /*grub_install = g_strdup_printf ("grub-install --no-floppy --force %s", path);*/
    /*}*/

    /*g_spawn_command_line_sync (grub_install, NULL, NULL, NULL, &error);*/
    /*if (error != NULL) {*/
        /*g_warning ("update grub:grub-install %s\n", error->message);*/
        /*goto out;*/
    /*}*/
    /*emit_progress ("bootloader", "97%");*/

    /*if (handler->uefi) {*/
        /*g_spawn_command_line_sync ("grub-mkconfig -o /boot/grub/grub.cfg", NULL, NULL, NULL, &error);*/
        /*if (error != NULL) {*/
            /*g_warning ("update grub:update grub for uefi %s\n", error->message);*/
            /*goto out;*/
        /*}*/
    /*} else {*/
        /*if (g_file_test ("/usr/lib/deepin-daemon/grub2", G_FILE_TEST_IS_EXECUTABLE)) {*/
            /*if (!g_file_test ("/boot/grub/grub.cfg", G_FILE_TEST_EXISTS)) {*/
                /*g_creat ("/boot/grub/grub.cfg", 0444);*/
            /*}*/

            /*extern gchar *xrandr_size;*/
            /*gchar *grub_theme_cmd = NULL;*/
            /*if (xrandr_size != NULL) {*/
                /*grub_theme_cmd = g_strdup_printf ("/usr/lib/deepin-daemon/grub2 --debug --setup --gfxmode %s", xrandr_size);*/
            /*} else {*/
                /*grub_theme_cmd = g_strdup ("/usr/lib/deepin-daemon/grub2 --debug --setup --gfxmode 1024x768");*/
            /*}*/

            /*g_spawn_command_line_sync (grub_theme_cmd, NULL, NULL, NULL, &error);*/
            /*if (error != NULL) {*/
                /*g_warning ("update grub:update with style %s\n", error->message);*/
                /*g_error_free (error);*/
                /*error = NULL;*/
            /*}*/
            /*g_free (grub_theme_cmd);*/
        /*} */
        /*g_spawn_command_line_sync ("update-grub", NULL, NULL, NULL, &error);*/
        /*if (error != NULL) {*/
            /*g_warning ("update grub:update grub %s\n", error->message);*/
            /*goto out;*/
        /*}*/
    /*}*/
    /*emit_progress ("bootloader", "98%");*/
    /*ret = TRUE;*/
    /*goto out;*/

/*out:*/
    /*if (handler->uuid != NULL) {*/
        /*g_free ((gchar *)handler->uuid);*/
    /*}*/
    /*if (handler != NULL) {*/
        /*g_free (handler);*/
    /*}*/
    /*g_free (path);*/
    /*g_free (grub_install);*/
    /*if (error != NULL) {*/
        /*g_error_free (error);*/
        /*error = NULL;*/
    /*}*/

    /*finish_install_cleanup ();*/

    /*if (ret) {*/
        /*emit_progress ("bootloader", "finish");*/
    /*} else {*/
        /*emit_progress ("bootloader", "terminate");*/
    /*}*/
    /*return NULL;*/
/*}*/

/*JS_EXPORT_API */
/*void old_installer_update_bootloader (const gchar *uuid, gboolean uefi)*/
/*{*/
    /*if (uuid == NULL) {*/
        /*g_warning ("update bootloader:invalid device\n");*/
        /*emit_progress ("bootloader", "terminate");*/
        /*return;*/
    /*}*/
    /*struct GrubHandler *handler = g_new0 (struct GrubHandler, 1);*/
    /*handler->uuid = g_strdup (uuid);*/
    /*handler->uefi = uefi;*/

    /*GThread *thread = g_thread_new ("bootloader", (GThreadFunc) thread_update_grub, (gpointer) handler);*/
    /*g_thread_unref (thread);*/
/*}*/

JS_EXPORT_API 
void installer_update_bootloader (const gchar *uuid, gboolean uefi)
{
    if (uuid == NULL) {
        g_warning ("update bootloader:invalid device\n");
        emit_progress ("bootloader", "terminate");
        return;
    }
    char* path = NULL;
    if (g_str_has_prefix (uuid, "disk")) {
	path = installer_get_disk_path (uuid);
    } else if (g_str_has_prefix (uuid, "part")) {
	path = installer_get_partition_path (uuid);
    } else {
	g_warning ("update grub:invalid uuid %s\n", uuid);
    }
    write_bootloader_info(path, uefi);
    g_free(path);
    /*finish_install_cleanup ();*/
    emit_progress ("bootloader", "finish");
}

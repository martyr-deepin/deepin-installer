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

#ifndef __MISC_H
#define __MISC_H

#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "utils.h"
#include "jsextension.h"

struct PasswdHandler {
    gchar *username;
    gchar *password;
    gchar *hostname;
    GPid pid;
    GIOChannel *in_channel;
    GIOChannel *out_channel;
    guint child_watch_id;
    guint stdout_watch_id;
};

JS_EXPORT_API JSObjectRef installer_get_system_users ();

JS_EXPORT_API void installer_create_user (const gchar *username, const gchar *hostname, const gchar *password);

gboolean add_user (const gchar *username);

gboolean set_user_home (const gchar *username);

gboolean set_user_password (struct PasswdHandler *handler);

gboolean set_group (const gchar *username);

gboolean write_hostname (const gchar *hostname);

void installer_reboot ();

JS_EXPORT_API JSObjectRef installer_get_keyboard_layouts (); 

JS_EXPORT_API JSObjectRef installer_get_layout_variants (const gchar *layout_name); 

JS_EXPORT_API JSObjectRef installer_get_current_layout_variant ();

JS_EXPORT_API void installer_set_keyboard_layout_variant (const gchar *layout, const gchar *variant);

JS_EXPORT_API JSObjectRef installer_get_timezone_list ();


JS_EXPORT_API void installer_set_timezone (const gchar *timezone);

JS_EXPORT_API void installer_extract_squashfs ();

JS_EXPORT_API gboolean installer_mount_procfs ();

JS_EXPORT_API gboolean installer_chroot_target ();

JS_EXPORT_API void installer_copy_whitelist ();

#endif

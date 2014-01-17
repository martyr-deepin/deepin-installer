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

#include "base.h"

struct GrubHandler {
    const gchar *uuid;
    gboolean uefi;
};

void installer_reboot ();

JS_EXPORT_API JSObjectRef installer_get_timezone_list ();

JS_EXPORT_API void installer_set_timezone (const gchar *timezone);

JS_EXPORT_API gboolean installer_mount_procfs ();

JS_EXPORT_API gboolean installer_chroot_target ();

JS_EXPORT_API void installer_copy_whitelist ();

JS_EXPORT_API void installer_update_bootloader (const gchar *uuid, gboolean uefi);

void finish_install_cleanup ();

JS_EXPORT_API void  installer_draw_background (JSValueRef canvas, const gchar *path);

#endif

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

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "base.h"

JS_EXPORT_API JSObjectRef installer_get_keyboard_layouts (); 

JS_EXPORT_API JSObjectRef installer_get_layout_variants (const gchar *layout_name); 

JS_EXPORT_API JSObjectRef installer_get_current_layout_variant ();

JS_EXPORT_API void installer_set_keyboard_layout_variant (const gchar *layout, const gchar *variant);

#endif

/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "base.h"

//combines layout with layout name and variant name
JS_EXPORT_API gchar *installer_get_layout_description (const gchar *layout);

JS_EXPORT_API JSObjectRef installer_get_keyboard_layouts (); 

JS_EXPORT_API JSObjectRef installer_get_layout_variants (const gchar *layout_name); 

JS_EXPORT_API JSObjectRef installer_get_current_layout_variant ();

JS_EXPORT_API void installer_set_keyboard_layout_variant (const gchar *layout, const gchar *variant);

JS_EXPORT_API double installer_keyboard_detect_read_step (gchar *step);

JS_EXPORT_API JSObjectRef installer_keyboard_detect_get_symbols ();

JS_EXPORT_API gchar* installer_keyboard_detect_get_present ();

JS_EXPORT_API gchar* installer_keyboard_detect_get_not_present ();

JS_EXPORT_API JSObjectRef installer_keyboard_detect_get_keycodes ();

JS_EXPORT_API gchar* installer_keyboard_detect_get_result ();

JS_EXPORT_API void installer_set_layout(const gchar* layout);

#endif

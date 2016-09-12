/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef _GREETER_UTIL_H
#define _GREETER_UTIL_H

#include <glib.h>
#include <lightdm.h>
#include "jsextension.h"

JS_EXPORT_API JSObjectRef greeter_get_users ();

JS_EXPORT_API gchar* greeter_get_user_icon (const gchar* name);

JS_EXPORT_API gchar* greeter_get_user_realname (const gchar* name);

JS_EXPORT_API gboolean greeter_user_need_password (const gchar *name);

JS_EXPORT_API gchar* greeter_get_default_user ();

JS_EXPORT_API gchar* greeter_get_user_session (const gchar *name);

JS_EXPORT_API gboolean greeter_is_hide_users ();

JS_EXPORT_API gboolean greeter_is_support_guest ();

JS_EXPORT_API gboolean greeter_is_guest_default ();

JS_EXPORT_API void greeter_draw_user_background (JSValueRef canvas, const gchar *username);

JS_EXPORT_API gchar* greeter_get_date ();

JS_EXPORT_API gboolean greeter_detect_capslock ();

JS_EXPORT_API gboolean greeter_get_user_session_on (const gchar *username);

#endif

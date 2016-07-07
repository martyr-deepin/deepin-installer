/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "jsextension.h"
#include <lightdm.h>

JS_EXPORT_API JSObjectRef greeter_get_sessions ();

JS_EXPORT_API gchar* greeter_get_session_name (const gchar *key);

JS_EXPORT_API gchar* greeter_get_session_icon (const gchar *key);

JS_EXPORT_API gchar* greeter_get_default_session ();

JS_EXPORT_API gboolean greeter_get_can_suspend ();

JS_EXPORT_API gboolean greeter_get_can_hibernate ();

JS_EXPORT_API gboolean greeter_get_can_restart ();

JS_EXPORT_API gboolean greeter_get_can_shutdown ();

JS_EXPORT_API gboolean greeter_run_suspend ();

JS_EXPORT_API gboolean greeter_run_hibernate ();

JS_EXPORT_API gboolean greeter_run_restart ();

JS_EXPORT_API gboolean greeter_run_shutdown ();

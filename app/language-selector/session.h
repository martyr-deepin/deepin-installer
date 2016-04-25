/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 *              bluth <yuanchenglu001@gmail.com>
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

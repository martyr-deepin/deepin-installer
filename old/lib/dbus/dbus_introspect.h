/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __DBUS_INTROSPECTOR__
#define __DBUS_INTROSPECTOR__
#include <gio/gio.h>
#include <JavaScriptCore/JSBase.h>

JSObjectRef get_dbus_object(JSContextRef ctx, GDBusConnection* con, const char* name, const char* path, const char* iface, JSValueRef* exception);
void reset_dbus_infos();
GDBusMessage* watch_signal(GDBusConnection* connection, GDBusMessage *msg, gboolean incoming, gpointer no_use);
void add_watch(GDBusConnection* con, const char* path, const char* ifc, const char* member);
#endif


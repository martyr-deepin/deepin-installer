/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __DBUS_OBJECT_INFO__
#define __DBUS_OBJECT_INFO__
#include <glib.h>
#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSObjectRef.h>

struct DBusObjectInfo {
    GDBusConnection* connection;
    JSClassRef klass;
    JSObjectRef obj;
    char* name;
    char* path;
    char* iface;

    GHashTable* methods;
    GHashTable* properties;
    GHashTable* signals;
}; 

struct Method {
    char* name;
    GSList* signature_in;
    GSList* signature_out;
};

struct Signal {
    char* name;
    GSList* signature;
};

#include <JavaScriptCore/JSObjectRef.h>
struct Property {
    char* name;
    GSList* signature;
    JSPropertyAttributes access;
};


struct DBusObjectInfo* 
build_object_info(GDBusConnection* con, const char *name, const char* path, const char *interface);
void dbus_object_info_free(struct DBusObjectInfo* info);

#endif

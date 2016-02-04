/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <glib.h>
#include <JavaScriptCore/JavaScript.h>


#include "jsextension.h"
#include "dbus_introspect.h"

JS_EXPORT_API JSValueRef dbus_sys_object(const char* bus_name,
                                         const char* object_path,
                                         const char* interface,
                                         JSData* js)
{
    static GDBusConnection* sys_con = NULL;
    if (sys_con == NULL) {
        GError *error = NULL;
        sys_con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
        if (error != NULL) {
            g_error("[%s] ERROR:%s\n", __func__, error->message);
            g_error_free(error);
        }
        g_assert(sys_con != NULL);
        g_dbus_connection_add_filter(sys_con, watch_signal, NULL, NULL);
    }
    JSValueRef value = get_dbus_object(get_global_context(), sys_con,
                                       bus_name, object_path, interface,
                                       js->exception);
    if (value == NULL) {
        return NULL;
    }
    return value;
}

JS_EXPORT_API
JSValueRef dbus_sys(const char* bus_name, JSData* js)
{
    char** segs = g_strsplit(bus_name, ".", -1);
    char* r = g_strjoinv("/", segs);
    g_strfreev(segs);
    char* path = g_strdup_printf("/%s", r);
    g_free(r);

    JSValueRef obj = dbus_sys_object(bus_name, path, bus_name, js);
    g_free(path);
    return obj;
}


JS_EXPORT_API
JSValueRef dbus_session_object(const char* bus_name,
                               const char* object_path,
                               const char* interface,
                               JSData* js)
{
    static GDBusConnection* session_con = NULL;
    if (session_con == NULL) {
        GError *error = NULL;
        session_con = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
        if (error != NULL) {
            g_error("[%s] error: %s\n", __func__, error->message);
            g_error_free(error);
        }
        g_assert(session_con!= NULL);
        g_dbus_connection_add_filter(session_con, watch_signal, NULL, NULL);
    }
    JSValueRef value = get_dbus_object(get_global_context(), session_con,
                                       bus_name, object_path, interface,
                                       js->exception);
    if (value == NULL) {
        return NULL;
    }
    return value;
}

JS_EXPORT_API
JSValueRef dbus_session(const char* bus_name, JSData* js)
{
    char** segs = g_strsplit(bus_name, ".", -1);
    char* r = g_strjoinv("/", segs);
    g_strfreev(segs);
    char* path = g_strdup_printf("/%s", r);
    g_free(r);

    JSValueRef obj = dbus_session_object(bus_name, path, bus_name, js);
    g_free(path);
    return obj;
}

void dbus_reload()
{
    reset_dbus_infos();
}


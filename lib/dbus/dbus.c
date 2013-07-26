#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <JavaScriptCore/JavaScript.h>


#include "jsextension.h"
#include "dbus_introspect.h"

PRIVATE gboolean init = FALSE;
PRIVATE DBusGConnection* sys_con = NULL;
PRIVATE DBusGConnection* session_con = NULL;


void dbus_init()
{
    dbus_g_thread_init();
    init = TRUE;
}

JS_EXPORT_API
JSValueRef dbus_sys_object(
        const char* bus_name,
        const char* object_path,
        const char* interface,
        JSData* js)
{
    if (!init) dbus_init();

    if (sys_con == NULL) {
        GError *error = NULL;
        sys_con = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
        if (error != NULL) {
            g_warning("ERROR:%s\n", error->message);
            g_error_free(error);
        }
        g_assert(sys_con != NULL);
    }
    JSValueRef value = get_dbus_object(js->ctx, sys_con,
            bus_name, object_path, interface);
    if (value == NULL) {
        js_fill_exception(js->ctx, js->exception, "Can't dynamic build this dbus interface)");
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
JSValueRef dbus_session_object(
        const char* bus_name,
        const char* object_path,
        const char* interface,
        JSData* js)
{ 
    if (!init) dbus_init();

    if (session_con == NULL) {
        GError *error = NULL;
        session_con = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
        if (error != NULL) {
            g_warning("ERROR:%s\n", error->message);
            g_error_free(error);
        }
        g_assert(session_con != NULL);
    }
    JSValueRef value = get_dbus_object(js->ctx, session_con,
            bus_name, object_path, interface);
    if (value == NULL) {
        js_fill_exception(js->ctx, js->exception, "Can't dynamic build this dbus interface)");
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

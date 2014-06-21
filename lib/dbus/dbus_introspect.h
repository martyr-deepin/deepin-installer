#ifndef __DBUS_INTROSPECTOR__
#define __DBUS_INTROSPECTOR__
#include <gio/gio.h>
#include <JavaScriptCore/JSBase.h>

JSObjectRef get_dbus_object(JSContextRef ctx, GDBusConnection* con, const char* name, const char* path, const char* iface, JSValueRef* exception);
void reset_dbus_infos();
GDBusMessage* watch_signal(GDBusConnection* connection, GDBusMessage *msg, gboolean incoming, gpointer no_use);
void add_watch(GDBusConnection* con, const char* path, const char* ifc, const char* member);
#endif


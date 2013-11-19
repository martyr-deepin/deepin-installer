#ifndef __DBUS_OBJECT_INFO__
#define __DBUS_OBJECT_INFO__
#include <glib.h>
#include <dbus/dbus.h>
#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSObjectRef.h>

struct DBusObjectInfo {
    DBusConnection* connection;
    JSClassRef klass;
    JSObjectRef obj;
    char* server;
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
build_object_info(DBusGConnection* con, const char *server,
        const char* path, const char *interface);
void dbus_object_info_free(struct DBusObjectInfo* info);

#endif

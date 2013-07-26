#ifndef __DBUS_JS_CONVERT__
#define __DBUS_JS_CONVERT__

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <glib.h>

char* jsstring_to_cstr(JSContextRef ctx, JSStringRef js_string);
char* jsvalue_to_cstr(JSContextRef ctx, JSValueRef js_value);

gboolean js_to_dbus(JSContextRef ctx, const JSValueRef jsvalue, 
        DBusMessageIter *iter, const char* sig, JSValueRef *exception);

JSValueRef dbus_to_js(JSContextRef ctx, DBusMessageIter *iter);

#endif

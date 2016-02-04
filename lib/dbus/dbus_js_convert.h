/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __DBUS_JS_CONVERT__
#define __DBUS_JS_CONVERT__

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <glib.h>

char* jsstring_to_cstr(JSContextRef ctx, JSStringRef js_string);
char* jsvalue_to_cstr(JSContextRef ctx, JSValueRef js_value);

GVariant* js_to_dbus(JSContextRef ctx, const JSValueRef jsvalue, const GVariantType* sig, JSValueRef *exception);
JSValueRef dbus_to_js(JSContextRef ctx, GVariant *);

GVariantType* gslit_to_varianttype(GSList* l);
#endif

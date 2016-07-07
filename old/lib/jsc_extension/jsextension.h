/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *
 * Author:      snyh <snyh@snyh.org>
 * Maintainer:  snyh <snyh@snyh.org>
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
#ifndef __JS_EXTENSION__
#define __JS_EXTENSION__
#include "config.h"
#include <JavaScriptCore/JavaScript.h>
#include <glib.h>

typedef struct JSData {
    JSValueRef* exception;
    void* webview;
} JSData;

typedef struct _ArrayContainer {
    void* data;
    size_t num;
} ArrayContainer;


void js_value_protect(JSValueRef v);
void js_value_unprotect(JSValueRef v);

void init_js_extension(JSGlobalContextRef context, void* webview);
void destroy_js_extension();

/* Be careful use GRAB_CTX/UNGRAB_CTX */
void GRAB_CTX();
void UNGRAB_CTX();

/*  utils function *  */

void js_fill_exception(JSContextRef ctx, JSValueRef* excp, const char* format, ...);

JSGlobalContextRef get_global_context();

JSValueRef jsvalue_null();

JSValueRef jsvalue_from_number(JSContextRef, double number);
JSValueRef jsvalue_from_cstr(JSContextRef, const char* str);
JSValueRef json_from_cstr(JSContextRef, const char* json_str);
char* jsvalue_to_cstr(JSContextRef, JSValueRef);
char* jsstring_to_cstr(JSContextRef, JSStringRef);

typedef void* (*NObjectRef)(void*);
typedef void (*NObjectUnref)(void*);
/* c code should use this method and unref obj if you own one reference to obj*/
JSObjectRef create_nobject(JSContextRef ctx, void* obj, NObjectRef ref, NObjectUnref unref);

/* decrement the reference of obj, mainly used by jsc_gen.py when return an new create Pointer */
JSObjectRef create_nobject_and_own(JSContextRef ctx, void* obj, NObjectRef ref, NObjectUnref unref);

void* jsvalue_to_nobject(JSContextRef, JSValueRef);

gboolean jsvalue_instanceof(JSContextRef ctx, JSValueRef test, const char *klass);

void js_post_message(const char* name, JSValueRef json);
void js_post_signal(const char* name);

JSObjectRef json_create();
void json_append_value(JSObjectRef json, const char* key, JSValueRef value);
void json_append_string(JSObjectRef json, const char* key, const char* value);
void json_append_number(JSObjectRef json, const char* key, double value);
void json_append_nobject(JSObjectRef json, const char* key, void* value, NObjectRef ref, NObjectUnref unref);

JSObjectRef json_array_create();
void json_array_insert(JSObjectRef json, gsize i, JSValueRef value);
void json_array_insert_nobject(JSObjectRef json, gsize i, void* value, NObjectRef ref, NObjectUnref unref);



#endif



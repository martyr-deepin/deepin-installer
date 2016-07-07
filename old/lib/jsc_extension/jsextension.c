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
#include "jsextension.h"
#include <glib.h>

static GRecMutex __ctx_lock;
void GRAB_CTX() { g_rec_mutex_trylock(&__ctx_lock); }
void UNGRAB_CTX() { g_rec_mutex_unlock(&__ctx_lock); }

void js_fill_exception(JSContextRef ctx,
                       JSValueRef* excp,
                       const char* format,
                       ...)
{
    va_list args;
    va_start (args, format);
    char* str = g_strdup_vprintf(format, args);
    va_end(args);

    JSStringRef string = JSStringCreateWithUTF8CString(str);
    JSValueRef exc_str = JSValueMakeString(ctx, string);
    JSStringRelease(string);
    g_free(str);

    *excp= JSValueToObject(ctx, exc_str, NULL);
}

JSValueRef jsvalue_from_number(JSContextRef ctx, double number)
{
    return JSValueMakeNumber(ctx, number);
}

JSValueRef jsvalue_from_cstr(JSContextRef ctx, const char* str)
{
    JSStringRef jsstr = JSStringCreateWithUTF8CString(str);
    JSValueRef r = JSValueMakeString(ctx, jsstr);
    JSStringRelease(jsstr);
    return r;
}

JSValueRef jsvalue_null()
{
    return JSValueMakeNull(get_global_context());
}

char* jsstring_to_cstr(JSContextRef ctx, JSStringRef js_string)
{
    (void)ctx;
    size_t len = JSStringGetMaximumUTF8CStringSize(js_string);
    char *c_str = g_new(char, len);
    JSStringGetUTF8CString(js_string, c_str, len);
    return c_str;
}

char* jsvalue_to_cstr(JSContextRef ctx, JSValueRef jsvalue)
{
    if (!JSValueIsString(ctx, jsvalue)) {
        return NULL;
    }
    JSStringRef js_string = JSValueToStringCopy(ctx, jsvalue, NULL);
    char* cstr = jsstring_to_cstr(ctx, js_string);
    JSStringRelease(js_string);

    return cstr;
}

gboolean jsvalue_instanceof(JSContextRef ctx,
                            JSValueRef test,
                            const char *klass)
{
  JSStringRef property = JSStringCreateWithUTF8CString(klass);
  JSObjectRef ctor = JSValueToObject(ctx,
        JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx),
                            property, NULL),
        NULL);
  JSStringRelease(property);
  return JSValueIsInstanceOfConstructor(ctx, test, ctor, NULL);
}

PRIVATE gboolean _js_value_unprotect(JSValueRef v)
{
    JSValueUnprotect(get_global_context(), v);
    return FALSE;
}
void js_value_unprotect(JSValueRef v)
{
    g_main_context_invoke(NULL, (GSourceFunc)_js_value_unprotect, (gpointer)v);
}

PRIVATE gboolean _js_value_protect(JSValueRef v)
{
    JSValueProtect(get_global_context(), v);
    return FALSE;
}

void js_value_protect(JSValueRef v)
{
    g_main_context_invoke(NULL, (GSourceFunc)_js_value_protect, (gpointer)v);
}

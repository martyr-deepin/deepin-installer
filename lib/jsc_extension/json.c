#include "jsextension.h"

typedef struct {
    JSObjectRef json;
    const char *key;
    JSValueRef value;
} JsonCall;

JSObjectRef json_create()
{
    return JSObjectMake(get_global_context(), NULL, NULL);
}

gboolean __internal_append (JsonCall *call)
{
    JSStringRef js_key = JSStringCreateWithUTF8CString(call->key);
    JSObjectSetProperty(get_global_context(), call->json, js_key, call->value, kJSPropertyAttributeNone, NULL);
    JSStringRelease(js_key);
    g_free (call);
    return FALSE;
}

void json_append_value(JSObjectRef json, const char* key, JSValueRef value)
{
    JsonCall *call = g_new0 (JsonCall, 1);
    call->json = json;
    call->key = key;
    call->value = value;
    g_main_context_invoke (NULL, (GSourceFunc) __internal_append, call);
    //JSContextRef ctx = get_global_context();
    //JSStringRef js_key = JSStringCreateWithUTF8CString(key);
    //JSObjectSetProperty(ctx, json, js_key, value, kJSPropertyAttributeNone, NULL);
    //JSStringRelease(js_key);
}

void json_append_string(JSObjectRef json, const char* key, const char* value)
{

    JSContextRef ctx = get_global_context();
    JSValueRef js_value = jsvalue_from_cstr(ctx, value);
    json_append_value(json, key, js_value);
}

void json_append_number(JSObjectRef json, const char* key, double value)
{
    JSContextRef ctx = get_global_context();
    json_append_value(json, key, JSValueMakeNumber(ctx, value));
}

void json_append_nobject(JSObjectRef json, const char* key, void* value, NObjectRef ref, NObjectUnref unref)
{
    JSContextRef ctx = get_global_context();
    JSObjectRef js_value = create_nobject(ctx, value, ref, unref);
    json_append_value(json, key, js_value);
}

void json_append_nobject_a(JSObjectRef json, const char* key, void* values[], gsize size, NObjectRef ref, NObjectUnref unref)
{
    JSContextRef ctx = get_global_context();

    JSObjectRef js_value = json_array_create();
    for (gsize i=0; i<size; i++) {
        JSObjectRef item = create_nobject(ctx, values[i], ref, unref);
        json_array_insert(js_value, i, item);
    }

    json_append_value(json, key, js_value);
}

JSObjectRef json_array_create()
{
    JSContextRef ctx = get_global_context();
    return JSObjectMakeArray(ctx, 0, NULL, NULL);
}
void json_array_insert(JSObjectRef json, gsize i, JSValueRef value)
{
    JSContextRef ctx = get_global_context();
    JSObjectSetPropertyAtIndex(ctx, json, i, value, NULL);
}
void json_array_insert_nobject(JSObjectRef json, gsize i, void* value, NObjectRef ref, NObjectUnref unref)
{
    JSContextRef ctx = get_global_context();
    JSObjectRef js_value = create_nobject(ctx, value, ref, unref);
    json_array_insert(json, i, js_value);
}

JSValueRef json_from_cstr(JSContextRef ctx, const char* json_str)
{
    JSStringRef str = JSStringCreateWithUTF8CString(json_str);
    JSValueRef json = JSValueMakeFromJSONString(ctx, str);
    JSStringRelease(str);
    if (json == NULL) {
        g_error("This should not appear, please report to the author with the error message: \n  %s \n", json_str);
        g_assert(json != NULL);
    }
    return json;
}


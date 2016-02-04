/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dbus_js_convert.h"
#include "jsextension.h"

gboolean filter_function_child(JSContextRef ctx, JSValueRef jsvalue, int i)
{
    JSObjectRef p =JSValueToObject(ctx,
            JSObjectGetPropertyAtIndex(ctx, (JSObjectRef)jsvalue, i, NULL),
            NULL);
    if (p == NULL) {
        return FALSE;
    }
    return JSObjectIsFunction(ctx, p);
}

gboolean filter_array_child(JSContextRef ctx,
                            JSPropertyNameArrayRef array,
                            int index)
{
    JSStringRef key = JSPropertyNameArrayGetNameAtIndex(array, index);
    char* c_key = jsstring_to_cstr(ctx, key);
    char* endptr = NULL;
    g_ascii_strtoll(c_key, &endptr, 10);
    if (endptr == c_key) {
        return TRUE;
    } else {
        g_free(c_key);
        return FALSE;
    }
}

GVariant* js_to_dbus(JSContextRef ctx,
                     const JSValueRef jsvalue,
                     const GVariantType* sig,
                     JSValueRef *exception)
{
    if (g_variant_type_is_array(sig)) {
        GVariantBuilder builder;
        g_variant_builder_init(&builder, sig);
        JSPropertyNameArrayRef array = JSObjectCopyPropertyNames(ctx,
                (JSObjectRef)jsvalue);

        const GVariantType* child_sig = g_variant_type_element(sig);

        if (g_variant_type_is_dict_entry(child_sig)) {
            const GVariantType* key_sig = g_variant_type_first(child_sig);
            const GVariantType* value_sig = g_variant_type_next(key_sig);
            for (size_t i=0; i < JSPropertyNameArrayGetCount(array); i++) {
                if (filter_function_child(ctx, jsvalue, i)) {
                    continue;
                }

                g_variant_builder_open(&builder, child_sig);
                JSValueRef key = JSValueMakeString(ctx,
                        JSPropertyNameArrayGetNameAtIndex(array, i));
                JSValueRef value = JSObjectGetPropertyAtIndex(ctx,
                        (JSObjectRef)jsvalue, i, NULL);
                g_variant_builder_add_value(&builder,
                        js_to_dbus(ctx, key, key_sig, exception));
                g_variant_builder_add_value(&builder,
                        js_to_dbus(ctx, value, value_sig, exception));
                g_variant_builder_close(&builder);
            }
            return g_variant_builder_end(&builder);

        } else {
            GVariantBuilder builder;
            g_variant_builder_init(&builder, sig);
            JSPropertyNameArrayRef array = JSObjectCopyPropertyNames(ctx,
                    (JSObjectRef)jsvalue);
            const GVariantType* child_sig = g_variant_type_element(sig);
            for (size_t i=0; i < JSPropertyNameArrayGetCount(array); i++) {
                if (filter_array_child(ctx, array, i)) {
                    continue;
                }
                g_variant_builder_add_value(&builder,
                    js_to_dbus(ctx,
                        JSObjectGetPropertyAtIndex(ctx,
                                                   (JSObjectRef)jsvalue, i,
                                                   NULL),
                        child_sig, exception));
            }
            JSPropertyNameArrayRelease(array);
            return g_variant_builder_end(&builder);
        }
    } else if (g_variant_type_is_tuple(sig)) {
        GVariantBuilder builder;
        g_variant_builder_init(&builder, sig);
        JSPropertyNameArrayRef array = JSObjectCopyPropertyNames(ctx,
                (JSObjectRef)jsvalue);
        const GVariantType* current_sig = g_variant_type_first(sig);
        for (size_t i=0; i < JSPropertyNameArrayGetCount(array); i++) {
            if (filter_array_child(ctx, array, i)) {
                continue;
            }
            g_variant_builder_add_value(&builder,
                js_to_dbus(ctx,
                    JSObjectGetPropertyAtIndex(ctx, (JSObjectRef)jsvalue, i,
                                               NULL),
                    current_sig, exception));
            current_sig = g_variant_type_next(current_sig);
        }
        JSPropertyNameArrayRelease(array);
        return g_variant_builder_end(&builder);
    } else {
        switch (g_variant_type_peek_string(sig)[0]) {
            case 'y':
                return g_variant_new_byte(JSValueToNumber(ctx, jsvalue,
                                                          exception));
            case 'n':
                return g_variant_new_int16(JSValueToNumber(ctx, jsvalue,
                                                           exception));
            case 'q':
                return g_variant_new_uint16(JSValueToNumber(ctx, jsvalue,
                                                            exception));
            case 'i':
                return g_variant_new_int32(JSValueToNumber(ctx, jsvalue,
                                                           exception));
            case 'u':
                return g_variant_new_uint32(JSValueToNumber(ctx, jsvalue,
                                                            exception));
            case 'x':
                return g_variant_new_int64(JSValueToNumber(ctx, jsvalue,
                                                           exception));
            case 't':
                return g_variant_new_uint64(JSValueToNumber(ctx, jsvalue,
                                                            exception));
            case 'd':
                return g_variant_new_double(JSValueToNumber(ctx, jsvalue,
                                                            exception));
            case 'h':
                return g_variant_new_handle(JSValueToNumber(ctx, jsvalue,
                                                            exception));
            case 'b':
                return g_variant_new_boolean(JSValueToBoolean(ctx, jsvalue));

            case 's': {
                    char* v = jsvalue_to_cstr(ctx, jsvalue);
                    GVariant* r = g_variant_new_string(v);
                    g_free(v);
                    return r;
                }

            case 'v': {
                    //TODO:
                    /*g_variant_new_variant()*/
                    g_assert_not_reached();
                }
        }
    }
    g_assert_not_reached();
}

JSValueRef dbus_to_js(JSContextRef ctx, GVariant *dbus)
{
    JSValueRef jsvalue = NULL;
    GVariantClass type = g_variant_classify(dbus);
    switch (type) {
        case G_VARIANT_CLASS_STRING:
        case G_VARIANT_CLASS_OBJECT_PATH:
        case G_VARIANT_CLASS_SIGNATURE: {
                JSStringRef js_string = JSStringCreateWithUTF8CString(
                      g_variant_get_string(dbus, NULL));
                jsvalue = JSValueMakeString(ctx, js_string);
                JSStringRelease(js_string);
                return jsvalue;
            }

        case G_VARIANT_CLASS_BYTE:
            return JSValueMakeNumber(ctx, g_variant_get_byte(dbus));

        case G_VARIANT_CLASS_DOUBLE:
            return JSValueMakeNumber(ctx, g_variant_get_double(dbus));

        case G_VARIANT_CLASS_INT16:
            return JSValueMakeNumber(ctx, g_variant_get_int16(dbus));

        case G_VARIANT_CLASS_UINT16:
            return JSValueMakeNumber(ctx, g_variant_get_uint16(dbus));

        case G_VARIANT_CLASS_INT32:
            return JSValueMakeNumber(ctx, g_variant_get_int32(dbus));

        case G_VARIANT_CLASS_UINT32:
            return JSValueMakeNumber(ctx, g_variant_get_uint32(dbus));

        case G_VARIANT_CLASS_INT64:
            return JSValueMakeNumber(ctx, g_variant_get_int64(dbus));

        case G_VARIANT_CLASS_UINT64:
            return JSValueMakeNumber(ctx, g_variant_get_uint64(dbus));

        case G_VARIANT_CLASS_BOOLEAN:
            return JSValueMakeBoolean(ctx, g_variant_get_boolean(dbus));

        case G_VARIANT_CLASS_HANDLE:
            g_warning("[%s] does not support FD type\n", __func__);
            return JSValueMakeNumber(ctx, g_variant_get_uint32(dbus));

        case G_VARIANT_CLASS_VARIANT: {
                GVariant* v = g_variant_get_variant(dbus);
                jsvalue = dbus_to_js(ctx, v);
                g_variant_unref(v);
                return jsvalue;
            }

        case G_VARIANT_CLASS_DICT_ENTRY:
            /*g_assert_not_reached();*/
            break;

        case G_VARIANT_CLASS_ARRAY: {
                if (g_variant_type_is_dict_entry(
                        g_variant_type_element(g_variant_get_type(dbus)))) {
                    jsvalue = JSObjectMake(ctx, NULL, NULL);
                    for (size_t i=0; i<g_variant_n_children(dbus); i++) {
                        GVariant *dic = g_variant_get_child_value(dbus, i);
                        GVariant *key= g_variant_get_child_value (dic, 0);
                        GVariant *value = g_variant_get_child_value (dic, 1);

                        JSValueRef js_key = dbus_to_js(ctx, key);
                        JSValueRef js_value = dbus_to_js(ctx, value);

                        JSStringRef key_str = JSValueToStringCopy(ctx, js_key,
                                                                  NULL);
                        JSObjectSetProperty(ctx, (JSObjectRef)jsvalue,
                                            key_str, js_value, 0, NULL);
                        JSStringRelease(key_str);

                        g_variant_unref(key);
                        g_variant_unref(value);
                        g_variant_unref(dic);
                    }
                    return jsvalue;
                } else {
                    int n = g_variant_n_children(dbus);
                    JSValueRef *args = g_new(JSValueRef, n);
                    for (int i=0; i < n; i++) {
                        GVariant* v = g_variant_get_child_value(dbus, i);
                        args[i] = dbus_to_js(ctx, v);
                        g_variant_unref(v);
                    }
                    jsvalue = JSObjectMakeArray(ctx, n, args, NULL);
                    g_free(args);
                    return jsvalue;
                }
            }

        case G_VARIANT_CLASS_TUPLE: {
                int n = g_variant_n_children(dbus);
                jsvalue = JSObjectMakeArray(ctx, 0, NULL, NULL);
                for (int i=0; i < n; i++) {
                    GVariant* v = g_variant_get_child_value(dbus, i);
                    JSObjectSetPropertyAtIndex(ctx, (JSObjectRef)jsvalue,
                                               i, dbus_to_js(ctx, v), NULL);
                    g_variant_unref(v);
                }
                return jsvalue;
            }

        case G_VARIANT_CLASS_MAYBE:
            g_assert_not_reached();
    }
    g_warning("[%s] does not  support signature type:%c\n", __func__, type);
    return JSValueMakeUndefined(ctx);
}

GVariantType* gslit_to_varianttype(GSList* l)
{
    GString* str = g_string_new("(");
    while (l != NULL) {
        g_string_append(str, l->data);
        l = g_slist_next(l);
    }
    g_string_append(str, ")");
    return g_variant_type_new(g_string_free(str, FALSE));
}

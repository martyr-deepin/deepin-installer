/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "utils.h"
#include "dbus_introspect.h"
#include "dbus_object_info.h"
#include "dbus_js_convert.h"
#include "jsextension.h"

void dbus_object_info_free(struct DBusObjectInfo* info);

static GHashTable *__sig_callback_hash = NULL; // hash of (path:ifc:sig_name@unique_name  ---> (hash of callbackid---> callback))
static GHashTable *__objs_cache = NULL;

void reset_dbus_infos()
{
    if (__sig_callback_hash) {
        g_hash_table_remove_all(__sig_callback_hash);
    }
    if (__objs_cache) {
        g_hash_table_remove_all(__objs_cache);
    }
}

typedef int SIGNAL_CALLBACK_ID;

struct SignalInfo {
    JSObjectRef callback;
    GVariant* body;
};
struct AsyncInfo {
    JSObjectRef on_ok;
    JSObjectRef on_error;
    GDBusConnection* connection;
};

struct ObjCacheKey {
    GDBusConnection* connection;
    const char* bus_name;
    const char* path;
    const char* iface;
};

guint key_hash(struct ObjCacheKey* key)
{
    return g_str_hash(key->bus_name) + g_str_hash(key->path) +
        g_str_hash(key->iface) + g_direct_hash(key->connection);
}

guint key_equal(struct ObjCacheKey* a, struct ObjCacheKey* b)
{
    char* a_str = g_strdup_printf("%s%s%s%p",
            a->bus_name, a->path, a->iface, a->connection);
    char* b_str = g_strdup_printf("%s%s%s%p",
            b->bus_name, b->path, b->iface, a->connection);
    int ret = g_strcmp0(a_str, b_str);
    g_free(a_str);
    g_free(b_str);
    return ret;
}

gboolean handle_signal_callback(struct SignalInfo* info)
{
    g_assert(info->callback != NULL);

    if (info->body == NULL) {
        JSObjectCallAsFunction(get_global_context(), info->callback, NULL,
                               0, NULL, NULL);
    } else {
        int num = g_variant_n_children(info->body);

        JSValueRef *params = g_new(JSValueRef, num);
        for (int i=0; i<num; i++) {
            GVariant* item = g_variant_get_child_value(info->body, i);
            params[i] = dbus_to_js(get_global_context(), item);
            g_variant_unref(item);
        }
        JSObjectCallAsFunction(get_global_context(), info->callback, NULL,
                               num, params, NULL);

        g_free(params);
        g_variant_unref(info->body);
    }
    g_free(info);
    return FALSE;
}

GDBusMessage* watch_signal(GDBusConnection* connection,
                           GDBusMessage *msg,
                           gboolean incoming,
                           gpointer no_use G_GNUC_UNUSED)
{
    if (!incoming) {
        return msg;
    }

    if (g_dbus_message_get_message_type(msg) != G_DBUS_MESSAGE_TYPE_SIGNAL) {
        return msg;
    }

    if (__sig_callback_hash == NULL) {
        return msg;
    }

    const char* iface = g_dbus_message_get_interface(msg);
    const char* s_name = g_dbus_message_get_member(msg);
    const char* path = g_dbus_message_get_path(msg);

    char* key = g_strdup_printf("%s:%s:%s@%s", path, iface, s_name,
                                g_dbus_connection_get_unique_name(connection));
    GHashTable* cbs_info  = g_hash_table_lookup(__sig_callback_hash, key);
    g_free(key);

    if (cbs_info == NULL) {
        return msg;
    } else {
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, cbs_info);
        JSObjectRef callback;

        while (g_hash_table_iter_next(&iter, NULL, (gpointer)&callback)) {
            struct SignalInfo* sig_info  = g_new0(struct SignalInfo, 1);
            GVariant* body = g_dbus_message_get_body(msg);
            sig_info->callback =callback;
            if (body) {
                sig_info->body= g_variant_ref(body);
            }
            g_main_context_invoke(NULL, (GSourceFunc)handle_signal_callback,
                                  sig_info);
        }

        return msg;
    }
}


SIGNAL_CALLBACK_ID add_signal_callback(struct DBusObjectInfo *info,
                                       struct Signal *sig,
                                       JSObjectRef func)
{
    g_assert(sig != NULL);
    g_assert(func != NULL);

    if (__sig_callback_hash == NULL) {
        __sig_callback_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
              NULL, (GDestroyNotify)g_hash_table_destroy);
    }

    char* key = g_strdup_printf("%s:%s:%s@%s", info->path, info->iface,
            sig->name, g_dbus_connection_get_unique_name(info->connection));

    g_debug("[%s] key: %s\n", __func__, key);
    GHashTable *cbs = g_hash_table_lookup(__sig_callback_hash, key);
    if (cbs == NULL) {
        cbs = g_hash_table_new_full(g_direct_hash, g_direct_equal,
              NULL, (GDestroyNotify)js_value_unprotect);
        g_hash_table_insert(__sig_callback_hash, key, cbs);
    } else {
        g_free(key);
    }

    js_value_protect(func);
    SIGNAL_CALLBACK_ID id = (SIGNAL_CALLBACK_ID)GPOINTER_TO_INT(func);
    g_debug("%u", id);
    g_hash_table_insert(cbs, GINT_TO_POINTER((int)id), func);
    return id;
}



static
JSValueRef signal_connect(JSContextRef ctx,
                          JSObjectRef function G_GNUC_UNUSED,
                          JSObjectRef this,
                          size_t argumentCount,
                          const JSValueRef arguments[],
                          JSValueRef *exception)
{
    if (argumentCount != 2 ) {
        js_fill_exception(ctx, exception, "connect must have two params");
        return NULL;
    }
    struct DBusObjectInfo* obj_info = JSObjectGetPrivate(this);

    if (!JSValueIsString(ctx, arguments[0])) {
        js_fill_exception(ctx, exception,
                          "the first params must the signal name");
        return NULL;
    }

    char* s_name = jsvalue_to_cstr(ctx, arguments[0]);
    struct Signal *signal = g_hash_table_lookup(obj_info->signals, s_name);
    if (signal == NULL) {
        char* desc = g_strdup_printf("dbus(\"%s:%s:%s\") hasn't signal \"%s\"",
                                     obj_info->name,
                                     obj_info->path,
                                     obj_info->iface,
                                     s_name);
        js_fill_exception(ctx, exception, desc);
        g_free(desc);
        goto errout;
    }

    JSObjectRef callback = JSValueToObject(ctx, arguments[1], NULL);
    if (!JSObjectIsFunction(ctx, callback)) {
        js_fill_exception(ctx, exception,
                          "the params two must be an function!");
        goto errout;
    }

    SIGNAL_CALLBACK_ID id = add_signal_callback(obj_info, signal, callback);
    if (id == -1) {
        js_fill_exception(ctx, exception,
              "you have aleady watch the signal with this callback?");
        goto errout;
    }
    add_watch(obj_info->connection, obj_info->path, obj_info->iface, s_name);
    g_free(s_name);

    return JSValueMakeNumber(ctx, id);

errout:
    g_free(s_name);
    return NULL;
}

static
JSValueRef signal_disconnect(JSContextRef ctx,
                            JSObjectRef function G_GNUC_UNUSED,
                            JSObjectRef this,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    struct DBusObjectInfo* info = JSObjectGetPrivate(this);

    if (argumentCount != 2) {
        js_fill_exception(ctx, exception,
                          "Disconnet_signal need tow paramters!");
        return NULL;
    }

    char* sig_name = jsvalue_to_cstr(ctx, arguments[0]);
    char* key = g_strdup_printf("%s:%s:%s@%s", info->path, info->iface,
          sig_name, g_dbus_connection_get_unique_name(info->connection));
    g_debug("[%s] remove signal callback: %s\n", __func__, key);
    g_free(sig_name);
    GHashTable *cbs = g_hash_table_lookup(__sig_callback_hash, key);
    g_free(key);

    if (cbs == NULL) {
        g_debug("[%s] no callback\n", __func__);
        js_fill_exception(ctx, exception, "This signal hasn't connected!");
        return NULL;
    }
    SIGNAL_CALLBACK_ID cb_id = (SIGNAL_CALLBACK_ID)GPOINTER_TO_INT(arguments[1]);
    g_debug("[%s] cb_id: %u\n", __func__, cb_id);
    if (!g_hash_table_remove(cbs, GINT_TO_POINTER(cb_id))) {
        js_fill_exception(ctx, exception, "This signal hasn't connected!");
        return NULL;
    }

    return JSValueMakeNull(ctx);
}

static
JSValueRef signal_emit(JSContextRef ctx,
                       JSObjectRef function G_GNUC_UNUSED,
                       JSObjectRef this G_GNUC_UNUSED,
                       size_t argumentCount G_GNUC_UNUSED,
                       const JSValueRef arguments[] G_GNUC_UNUSED,
                       JSValueRef *exception)
{
    /*obj_info;*/
    /*signal_name;*/
    /*signal_signature;*/
    /*arguments;*/
    js_fill_exception(ctx, exception, "Not Implement signal emit");
    return NULL;
}


void async_info_free(struct AsyncInfo* info)
{
    if (info->on_error) {
        JSValueUnprotect(get_global_context(), info->on_error);
    }
    if (info->on_ok) {
        JSValueUnprotect(get_global_context(), info->on_ok);
    }
    g_free(info);
}

void async_callback(GObject *source G_GNUC_UNUSED, GAsyncResult* res,
                    struct AsyncInfo *info)
{
    GError* error = NULL;
    GVariant* r = g_dbus_connection_call_finish(info->connection, res, &error);
    if (error != NULL) {
        if (info->on_error != NULL) {
            JSObjectCallAsFunction(get_global_context(), info->on_error,
                                   NULL, 0, NULL, NULL);
        }
        async_info_free(info);
        return;
    } else {
        int num = g_variant_n_children(r);

        JSValueRef *params = g_new(JSValueRef, num);
        for (int i=0; i<num; i++) {
            GVariant* item = g_variant_get_child_value(r, i);
            params[i] = dbus_to_js(get_global_context(), item);
            g_variant_unref(item);
        }
        if (info->on_ok) {
            JSObjectCallAsFunction(get_global_context(), info->on_ok,
                                   NULL, num, params, NULL);
        }

        g_free(params);
        g_variant_unref(r);
        async_info_free(info);
        return;
    }
}

bool dynamic_set(JSContextRef ctx, JSObjectRef object,
                 JSStringRef propertyName, JSValueRef jsvalue,
                 JSValueRef* exception)
{
    struct DBusObjectInfo* obj_info = JSObjectGetPrivate(object);
    GError* error = NULL;

    char* prop_name = jsstring_to_cstr(ctx, propertyName);
    struct Property *p = g_hash_table_lookup(obj_info->properties, prop_name);

    GVariantType* sig = g_variant_type_new(p->signature->data);
    g_dbus_connection_call_sync(obj_info->connection,
                                obj_info->name,
                                obj_info->path,
                                "org.freedesktop.DBus.Properties",
                                "Set",
                                g_variant_new("(ssv)",
                                    obj_info->iface, prop_name,
                                    js_to_dbus(ctx, jsvalue, sig, exception)),
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    g_variant_type_free(sig);
    g_free(prop_name);

    if (error != NULL) {
        char* err_str = g_strdup_printf("synamic_set:%s\n", error->message);
        js_fill_exception(ctx, exception, err_str);
        g_free(err_str);
        g_error_free(error);
        return FALSE;
    }
    return TRUE;
}

bool has_property(JSContextRef ctx, JSObjectRef object,
                  JSStringRef propertyName)
{
    struct DBusObjectInfo* obj_info = JSObjectGetPrivate(object);
    char* prop_name = jsstring_to_cstr(ctx, propertyName);
    GDBusProxy* proxy = g_dbus_proxy_new_sync(obj_info->connection,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              NULL,
                                              obj_info->name,
                                              obj_info->path,
                                              obj_info->iface,
                                              NULL,
                                              NULL);

    gboolean has = FALSE;

    gchar** names = g_dbus_proxy_get_cached_property_names(proxy);
    if (names != NULL) {
        for (guint i=0; i < g_strv_length(names); i++) {
            if (g_strcmp0(names[i], prop_name) == 0) {
                has = TRUE;
                break;
            }
        }
        g_strfreev(names);
    }
    g_free(prop_name);
    return has;
}

JSValueRef dynamic_get(JSContextRef ctx,
                       JSObjectRef object,
                       JSStringRef propertyName,
                       JSValueRef* exception)
{
    struct DBusObjectInfo* obj_info = JSObjectGetPrivate(object);
    GError* error = NULL;

    GVariantType* sig_out = g_variant_type_new("(v)");
    char* prop_name = jsstring_to_cstr(ctx, propertyName);

    GVariant * v = g_dbus_connection_call_sync(obj_info->connection,
                          obj_info->name,
                          obj_info->path,
                          "org.freedesktop.DBus.Properties",
                          "Get",
                          g_variant_new("(ss)", obj_info->iface, prop_name),
                          sig_out,
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          &error);

    g_free(prop_name);
    g_variant_type_free(sig_out);

    if (error != NULL) {
        char* err_str = g_strdup_printf("dyanmic_get:%s\n", error->message);
        js_fill_exception(ctx, exception, err_str);
        g_free(err_str);
        g_error_free(error);
        return NULL;
    } else {
        GVariant* arg0 = g_variant_get_child_value(v, 0);
        JSValueRef ret = dbus_to_js(ctx,  arg0);
        g_variant_unref(arg0);
        g_variant_unref(v);
        return ret;
    }
}

static
JSValueRef dynamic_function(JSContextRef ctx,
                            JSObjectRef function,
                            JSObjectRef this,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception)
{
    JSValueRef ret;
    gboolean async = TRUE;
    JSObjectRef ok_callback = NULL;
    JSObjectRef error_callback = NULL;

    struct DBusObjectInfo* obj_info = JSObjectGetPrivate(this);

    JSStringRef name_str = JSStringCreateWithUTF8CString("name");
    JSValueRef js_func_name = JSObjectGetProperty(ctx, function,
                                                  name_str, NULL);
    JSStringRelease(name_str);

    char* func_name = jsvalue_to_cstr(ctx, js_func_name);
    if (g_str_has_suffix(func_name, "_sync")) {
        async = FALSE;
        func_name[strlen(func_name)-5] = '\0';
    }

    struct Method *m = g_hash_table_lookup(obj_info->methods, func_name);
    g_assert(obj_info->methods != NULL);

    GSList* sigs_in = m->signature_in;
    int i = argumentCount - g_slist_length(sigs_in);
    if (async) {
        if (i == 1) {
            ok_callback = JSValueToObject(ctx, arguments[--argumentCount],
                                          NULL);
            if (!ok_callback || !JSObjectIsFunction(ctx, ok_callback)) {
                js_fill_exception(ctx, exception,
                                  "the parmas's must be the ok callback");
                return NULL;
            }
        } else if (i == 2) {
            error_callback = JSValueToObject(ctx, arguments[--argumentCount],
                                             NULL);
            if (!error_callback || !JSObjectIsFunction(ctx, error_callback)) {
                js_fill_exception(ctx, exception,
                                  "last parmas's must be the error callback");
                return NULL;
            }
            ok_callback = JSValueToObject(ctx, arguments[--argumentCount], NULL);
            if (!ok_callback || !JSObjectIsFunction(ctx, ok_callback)) {
                js_fill_exception(ctx, exception,
                                  "the parmas's must be the ok callback");
                return NULL;
            }
        } else if (i != 0) {
            js_fill_exception(ctx, exception, "Signature didn't mached");
            return NULL;
        }
    } else {
        if (i != 0) {
            js_fill_exception(ctx, exception, "Signature didn't mached");
            return NULL;
        }
    }


    GVariantBuilder args;
    g_variant_builder_init(&args, G_VARIANT_TYPE_TUPLE);
    for (guint i=0; i<argumentCount; i++) {
        GVariantType* sig = g_variant_type_new(g_slist_nth_data(sigs_in, i));
        GVariant* v = js_to_dbus(ctx, arguments[i], sig, exception);
        g_variant_builder_add_value(&args, v);
        g_variant_type_free(sig);
    }

    GVariantType* sigs_out = gslit_to_varianttype(m->signature_out);
    if (async) {
        ret = JSValueMakeUndefined(ctx);

        struct AsyncInfo *info = g_new0(struct AsyncInfo, 1);
        info->connection = obj_info->connection;
        if (error_callback) {
            JSValueProtect(get_global_context(), error_callback);
            info->on_error = error_callback;
        }
        if (ok_callback) {
            JSValueProtect(get_global_context(), ok_callback);
            info->on_ok = ok_callback;
        }
        g_dbus_connection_call(obj_info->connection,
                               obj_info->name,
                               obj_info->path,
                               obj_info->iface,
                               func_name,
                               g_variant_builder_end(&args),
                               sigs_out,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               (GAsyncReadyCallback)async_callback,
                               info);
    } else {
        GError* error = NULL;
        GVariant * v = g_dbus_connection_call_sync(obj_info->connection,
                                                obj_info->name,
                                                obj_info->path,
                                                obj_info->iface,
                                                func_name,
                                                g_variant_builder_end(&args),
                                                sigs_out,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
        if (error != NULL) {
            js_fill_exception(ctx, exception, error->message);
            g_error_free(error);
            return NULL;
        }
        if (g_variant_n_children(v) == 1) {
            GVariant* arg0 = g_variant_get_child_value(v, 0);
            ret = dbus_to_js(ctx, arg0);
            g_variant_unref(arg0);
        } else {
            ret = dbus_to_js(ctx, v);
        }
        g_variant_unref(v);
    }
    g_variant_type_free(sigs_out);

    g_free(func_name);

    return ret;
}

JSClassRef get_cache_class(struct DBusObjectInfo* obj_info G_GNUC_UNUSED)
{
    //TODO: build cache;
    return NULL;
}

void obj_finalize(JSObjectRef obj)
{
    struct DBusObjectInfo *info = JSObjectGetPrivate(obj);
    g_assert(info != NULL);
}

JSObjectRef build_dbus_object(JSContextRef ctx, struct ObjCacheKey *key)
{
    struct DBusObjectInfo* obj_info = build_object_info(key->connection,
                                                        key->bus_name,
                                                        key->path,
                                                        key->iface);

    if (obj_info == NULL) {
        //can't build object info
        return NULL;
    }

    guint num_of_prop = g_hash_table_size(obj_info->properties);
    g_hash_table_size(obj_info->signals);

    // async_funs +  sync_funs + connect + emit + NULL
    JSStaticFunction* static_funcs = g_new0(JSStaticFunction, 4);

    JSStaticValue* static_values = g_new0(JSStaticValue, num_of_prop + 1);

    static_funcs[0].name = "connect";
    static_funcs[0].callAsFunction = signal_connect;
    static_funcs[0].attributes = kJSPropertyAttributeReadOnly;

    static_funcs[1].name = "dis_connect";
    static_funcs[1].callAsFunction = signal_disconnect;
    static_funcs[1].attributes = kJSPropertyAttributeReadOnly;

    static_funcs[2].name = "emit";
    static_funcs[2].callAsFunction = signal_emit;
    static_funcs[2].attributes = kJSPropertyAttributeReadOnly;

    GList *props = g_hash_table_get_keys(obj_info->properties);
    for (guint i = 0; i < num_of_prop; i++) {
        const char *p_name = g_list_nth_data(props, i);
        struct Property *prop = g_hash_table_lookup(obj_info->properties,
                                                    p_name);

        static_values[i].name = prop->name;
        static_values[i].attributes = prop->access;

        //default read write
        if (prop->access == kJSPropertyAttributeNone)  {
            static_values[i].setProperty = dynamic_set;
        }

        static_values[i].getProperty = dynamic_get;
    }

    GString *class_name = g_string_new(NULL);
    g_string_printf(class_name, "%s_%s_%s", obj_info->name, obj_info->path,
                    obj_info->iface);
    JSClassDefinition class_def = {
        0,
        kJSClassAttributeNone,
        class_name->str,
        NULL,
        static_values,
        static_funcs,
        NULL,
        NULL,//obj_finalize,
        has_property,
        dynamic_get,
        dynamic_set,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
    g_string_free(class_name, FALSE);

    obj_info->klass = JSClassCreate(&class_def);

    obj_info->obj = JSObjectMake(ctx, obj_info->klass, obj_info);

    guint num_of_func = g_hash_table_size(obj_info->methods);
    GList *funcs = g_hash_table_get_keys(obj_info->methods);
    for (guint i = 0; i < num_of_func; i++) {
        JSStringRef f_name = JSStringCreateWithUTF8CString(
              g_list_nth_data(funcs, i));
        JSObjectSetProperty(ctx, obj_info->obj, f_name,
              JSObjectMakeFunctionWithCallback(ctx, f_name, dynamic_function),
              kJSPropertyAttributeReadOnly, NULL);
        JSStringRelease(f_name);


        char* tmp = g_strdup_printf("%s_sync",
                                    (char*)g_list_nth_data(funcs, i));
        JSStringRef f_name_sync = JSStringCreateWithUTF8CString(tmp);
        g_free(tmp);
        JSObjectSetProperty(ctx, obj_info->obj, f_name_sync,
                JSObjectMakeFunctionWithCallback(ctx, f_name_sync,
                                                 dynamic_function),
                kJSPropertyAttributeReadOnly, NULL);
        JSStringRelease(f_name_sync);
    }
    return obj_info->obj;
}

JSObjectRef get_dbus_object(JSContextRef ctx,
                            GDBusConnection* con,
                            const char* bus_name,
                            const char* path,
                            const char* iface,
                            JSValueRef* exception)
{
    if (bus_name == NULL || path == NULL ||  iface == NULL) {
        char* err_str = g_strdup_printf("can't build dbus object by %s:%s:%s\n",
                                        bus_name, path, iface);
        js_fill_exception(ctx, exception, err_str);
        g_free(err_str);
        return NULL;
    }
    if (__objs_cache == NULL) {
        __objs_cache = g_hash_table_new_full((GHashFunc)key_hash,
              (GEqualFunc)key_equal, NULL,
              (GDestroyNotify)dbus_object_info_free);
    }
    struct ObjCacheKey key;
    key.connection = con;
    key.bus_name = bus_name;
    key.path = path;
    key.iface = iface;

    JSObjectRef obj = g_hash_table_lookup(__objs_cache, &key);
    if (obj == NULL) {
        obj = build_dbus_object(ctx, &key);
        if (obj == NULL) {
            js_fill_exception(ctx, exception, "can't build_dbus_object");
        }
    }
    return obj;
}


void add_watch(GDBusConnection* con, const char* path, const char* ifc,
               const char* member)
{
    GDBusProxy* proxy = g_dbus_proxy_new_sync(con,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              NULL,
                                              "org.freedesktop.DBus",
                                              "/org/freedesktop/DBus",
                                              "org.freedesktop.DBus",
                                              NULL,
                                              NULL);
    char* rule = g_strdup_printf(
            "eavesdrop=true,type=signal,path=%s,interface=%s,member=%s",
            path, ifc, member);
    g_dbus_proxy_call_sync(proxy,
                           "AddMatch",
                           g_variant_new("(s)", rule),
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           NULL);
    g_free(rule);
    g_object_unref(proxy);
}

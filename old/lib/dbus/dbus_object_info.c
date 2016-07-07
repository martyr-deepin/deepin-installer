/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <string.h>
#include <glib.h>

#include "utils.h"
#include "dbus_object_info.h"

static struct DBusObjectInfo *c_obj_info;
static struct Method *c_method;
static struct Signal *c_signal;
static struct Property *c_property;

enum State {
    S_NONE,
    S_PENDING,
    S_METHOD,
    S_SIGNAL,
    S_PROPERTY
} state = S_NONE;

static
void method_free(gpointer data)
{
    struct Method* m = (struct Method*)data;
    g_free(m->name);
    g_slist_free_full(m->signature_in, g_free);
    g_slist_free_full(m->signature_out, g_free);
    g_free(data);
}

static
void signal_free(gpointer data)
{
    struct Signal* s = (struct Signal*)data;
    g_free(s->name);
    g_slist_free_full(s->signature, g_free);
    g_free(data);
}

static
void property_free(gpointer data)
{
    struct Property* p = (struct Property*)data;
    g_free(p->name);
    g_slist_free_full(p->signature, g_free);
    g_free(data);
}

static
void parse_property(const gchar **names, const gchar **values)
{
    const gchar **n_c = names;
    const gchar **v_c = values;

    while (*n_c) {
        if (g_strcmp0(*n_c, "type") == 0) {
            c_property->signature = g_slist_append(c_property->signature,
                                                   g_strdup(*v_c));
        }
        if (g_strcmp0(*n_c, "name") == 0) {
            c_property->name = g_strdup(*v_c);
        }

        if (g_strcmp0(*n_c, "access") == 0) {
            if (g_strcmp0(*v_c, "read") == 0) {
                c_property->access = kJSPropertyAttributeReadOnly;
            } else if (g_strcmp0(*v_c, "readwrite") == 0) {
                c_property->access = kJSPropertyAttributeNone;
            } else {
                g_assert_not_reached();
            }
        }
        n_c++;
        v_c++;
    }
}

static
void parse_signal(const gchar **names, const gchar **values)
{
    const gchar **n_c = names;
    const gchar **v_c = values;

    while (*n_c) {
        if (g_strcmp0(*n_c, "type") == 0) {
            c_signal->signature =
                g_slist_append((GSList*)c_signal->signature, g_strdup(*v_c));
            return;
        }
        n_c++;
        v_c++;
    }
}

static
void parse_parms(const gchar **names, const gchar **values)
{
    const gchar **n_c = names;
    const gchar **v_c = values;
    gboolean in = TRUE;
    const gchar *type = NULL;

    while (*n_c) {
        if (g_strcmp0(*n_c, "type") == 0) {
            type = *v_c;
        }
        if (g_strcmp0(*n_c, "direction") == 0) {
            if (g_strcmp0(*v_c, "in") == 0) {
                in = TRUE;
            } else {
                in = FALSE;
            }
        }
        n_c++;
        v_c++;
    }

    if (type) {
        if (in)  {
            c_method->signature_in =
                g_slist_append(c_method->signature_in, g_strdup(type));
        } else {
            c_method->signature_out =
                g_slist_append(c_method->signature_out, g_strdup(type));
        }
    }
}

static
void parse_start(GMarkupParseContext* context G_GNUC_UNUSED,
                const gchar *element_name,
                const gchar **attribute_names,
                const gchar **attribute_values,
                gpointer user_data G_GNUC_UNUSED,
                GError **error G_GNUC_UNUSED)
{
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;

    if (state == S_NONE && g_strcmp0(element_name, "interface") == 0) {
        while (*name_cursor) {
            if (g_strcmp0(*name_cursor, "name") == 0 &&
                g_strcmp0(*value_cursor, c_obj_info->iface) == 0) {
                state = S_PENDING;
                return;
            }
            name_cursor++;
            value_cursor++;
        }
    }

    if (state != S_NONE) {
        if (g_strcmp0(element_name, "method") == 0) {
            state = S_METHOD;
            c_method = g_new0(struct Method, 1);
            while (g_strcmp0(*name_cursor, "name") != 0) {
                name_cursor++;
                value_cursor++;
            }
            c_method->name = g_strdup(*value_cursor);
            return;
        }
        if (g_strcmp0(element_name, "signal") == 0) {
            state = S_SIGNAL;
            c_signal = g_new0(struct Signal, 1);
            while (g_strcmp0(*name_cursor, "name") != 0) {
                name_cursor++;
                value_cursor++;
            }
            c_signal->name = g_strdup(*value_cursor);
            return;
        }
        if (g_strcmp0(element_name, "property") == 0) {
            state = S_PROPERTY;
            c_property = g_new0(struct Property, 1);
            parse_property(attribute_names, attribute_values);
            return;
        }
    }

    switch (state) {
        case S_METHOD:
            parse_parms(attribute_names, attribute_values);
            break;

        case S_SIGNAL:
            parse_signal(attribute_names, attribute_values);
            break;

        // fall through, avoid warning
        case S_NONE:
        case S_PENDING:
        case S_PROPERTY:
            break;
    }
}

static
void parse_end(GMarkupParseContext *context G_GNUC_UNUSED,
               const gchar* element_name,
               gpointer user_data G_GNUC_UNUSED,
               GError **error G_GNUC_UNUSED)
{
    if (g_strcmp0(element_name, "interface") == 0) {
        state = S_NONE;
    }
    if (state == S_METHOD && g_strcmp0(element_name, "method") == 0) {
        state = S_PENDING;
        g_hash_table_insert(c_obj_info->methods, c_method->name, c_method);
    }
    if (state == S_PROPERTY && g_strcmp0(element_name, "property") == 0) {
        state = S_PENDING;
        g_hash_table_insert(c_obj_info->properties, c_property->name, c_property);
    }

    if (state == S_SIGNAL && g_strcmp0(element_name, "signal") == 0) {
        state = S_PENDING;
        g_hash_table_insert(c_obj_info->signals, c_signal->name, c_signal);
    }
}

static
void build_current_object_info(const char* xml,
                               const char* interface G_GNUC_UNUSED)
{
    g_assert(xml != NULL);
    static GMarkupParser parser = {
        .start_element = parse_start,
        .end_element = parse_end,
        .text = NULL,
        .passthrough = NULL,
        .error = NULL
    };

    GMarkupParseContext *context = g_markup_parse_context_new(&parser, 0,
                                                              NULL, NULL);
    GError* error = NULL;
    if (g_markup_parse_context_parse(context, xml, strlen(xml), &error)
            == FALSE) {
        g_warning("[%s] introspect's xml content error, %s, xml: %s!\n",
                  __func__, error->message, xml);
        g_clear_error(&error);
    }
    g_markup_parse_context_free(context);
}

struct DBusObjectInfo* build_object_info(GDBusConnection* con,
                                         const char *name,
                                         const char* path,
                                         const char *interface)
{
    struct DBusObjectInfo *obj_info = g_new(struct DBusObjectInfo, 1);

    c_obj_info = obj_info;
    obj_info->connection = con;
    obj_info->methods = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              NULL, method_free);
    obj_info->properties = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                 NULL, property_free);
    obj_info->signals = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              NULL, signal_free);
    obj_info->name = g_strdup(name);
    obj_info->path = g_strdup(path);
    obj_info->iface = g_strdup(interface);

    GError* error = NULL;
    GDBusProxy* proxy = g_dbus_proxy_new_sync(con,
                                      G_DBUS_PROXY_FLAGS_NONE,
                                      NULL,
                                      name,
                                      path,
                                      "org.freedesktop.DBus.Introspectable",
                                      NULL,
                                      &error);
    if (error != NULL) {
        g_warning("[%s] create proxy error: %s\n", __func__, error->message);
        g_error_free(error);
        return NULL;
    }
    g_assert(G_IS_DBUS_PROXY(proxy));

    GVariant* args = g_dbus_proxy_call_sync(proxy,
                                            "Introspect",
                                            NULL,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);
    g_assert(G_IS_DBUS_PROXY(proxy));

    if (error != NULL) {
        g_warning("[%s] call Introspect error: %s\n", __func__,
                  error->message);
        g_error_free(error);
        return NULL;
    }

    GVariant* r = g_variant_get_child_value(args, 0);
    const char* info_xml = g_variant_get_string(r, NULL);
    build_current_object_info(info_xml, interface);
    c_obj_info = NULL;
    g_variant_unref(r);
    g_variant_unref(args);

    g_object_unref(proxy);

    return obj_info;
}

void dbus_object_info_free(struct DBusObjectInfo* info)
{
    /*JSClassRelease(info->klass);*/
    //TOOD free jsobjec?
    g_free(info->name);
    g_free(info->path);
    g_free(info->iface);
    g_hash_table_unref(info->methods);
    g_hash_table_unref(info->properties);
    g_hash_table_unref(info->signals);
    g_free(info);
}

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <string.h>
#include <glib.h>

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
            c_property->signature = g_slist_append(c_property->signature, g_strdup(*v_c));
        }
        if (g_strcmp0(*n_c, "name") == 0) {
            c_property->name = g_strdup(*v_c);
        }

        if (g_strcmp0(*n_c, "access") == 0) {
            if (g_strcmp0(*v_c, "read") == 0)
                c_property->access = kJSPropertyAttributeReadOnly;
            else if (g_strcmp0(*v_c, "readwrite") == 0)
                c_property->access = kJSPropertyAttributeNone;
            else
                g_assert_not_reached();
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
            if (g_strcmp0(*v_c, "in") == 0)
                in = TRUE;
            else
                in = FALSE;
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
void parse_start(GMarkupParseContext* context,
        const gchar *element_name,
        const gchar **attribute_names,
        const gchar **attribute_values,
        gpointer user_data,
        GError **error)
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
void parse_end(GMarkupParseContext *context,
        const gchar* element_name, gpointer user_data, GError **error)
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
void build_current_object_info(const char* xml, const char* interface)
{
    g_assert(xml != NULL);
    static GMarkupParser parser = {
        .start_element = parse_start,
        .end_element = parse_end,
        .text = NULL,
        .passthrough = NULL,
        .error = NULL
    };

    GMarkupParseContext *context = g_markup_parse_context_new(&parser, 0, NULL, NULL);
    if (g_markup_parse_context_parse(context, xml, strlen(xml), NULL)
            == FALSE) {
        g_warning("introspect's xml content error!\n");
    }
    g_markup_parse_context_free(context);
}


struct DBusObjectInfo* build_object_info(
        DBusGConnection* con,
        const char *server, const char* path,
        const char *interface)
{
    struct DBusObjectInfo *obj_info = g_new(struct DBusObjectInfo, 1);
    c_obj_info = obj_info;
    obj_info->connection = dbus_g_connection_get_connection(con);
    obj_info->methods = g_hash_table_new_full(g_str_hash, g_str_equal,
          NULL, method_free);
    obj_info->properties = g_hash_table_new_full(g_str_hash, g_str_equal,
          NULL, property_free);
    obj_info->signals = g_hash_table_new_full(g_str_hash, g_str_equal,
          NULL, signal_free);
    obj_info->server = g_strdup(server);
    obj_info->path = g_strdup(path);
    obj_info->iface = g_strdup(interface);


    char* info_xml = NULL;
    DBusGProxy *proxy = dbus_g_proxy_new_for_name(con,
            server, path,
            "org.freedesktop.DBus.Introspectable");
    if (!dbus_g_proxy_call(proxy, "Introspect", NULL, G_TYPE_INVALID,
            G_TYPE_STRING, &info_xml, G_TYPE_INVALID)) {
        g_warning("Error When fetcho introspectable infomation.");
    }
    g_object_unref(proxy);

    if (info_xml == NULL)
        return NULL;

    build_current_object_info(info_xml, interface);
    g_free(info_xml);
    c_obj_info = NULL;
    return obj_info;
}

void dbus_object_info_free(struct DBusObjectInfo* info)
{
    /*JSClassRelease(info->klass);*/
    //TOOD free jsobjec?
    g_free(info->server);
    g_free(info->path);
    g_free(info->iface);
    g_hash_table_unref(info->methods);
    g_hash_table_unref(info->properties);
    g_hash_table_unref(info->signals);
    g_free(info);
}

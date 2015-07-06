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
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <gio/gio.h>
#include <sys/stat.h>
#include "utils.h"
#include "xdg_misc.h"
#include "jsextension.h"
#include "dwebview.h"
#include "dcore.h"
#include "pixbuf.h"


#define DESKTOP_SCHEMA_ID "com.deepin.dde.desktop"
#define DOCK_SCHEMA_ID "com.deepin.dde.dock"
#define SCHEMA_KEY_ENABLED_PLUGINS "enabled-plugins"

static GSettings* desktop_gsettings = NULL;
GHashTable* enabled_plugins = NULL;
GHashTable* plugins_state = NULL;

enum PluginState {
    DISABLED_PLUGIN,
    ENABLED_PLUGIN,
    UNKNOWN_PLUGIN
};

//TODO run_command support variable arguments

JS_EXPORT_API
char* dcore_get_theme_icon(const char* name, double size)
{
    return icon_name_to_path_with_check_xpm(name, size);
}


JS_EXPORT_API
char* dcore_get_name_by_appid(const char* id)
{
    GDesktopAppInfo* a = guess_desktop_file(id);
    if (a != NULL) {
        char* name = g_strdup(g_app_info_get_name(G_APP_INFO(a)));
        g_object_unref(a);
        return name;
    }
    return g_strdup("");
}

#define IS_DIR(path) g_file_test(path, G_FILE_TEST_IS_DIR)

gboolean is_plugin(char const* path)
{
    char* info_file_path = g_build_filename(path, "info.ini", NULL);
    gboolean _is_plugin = g_file_test(info_file_path, G_FILE_TEST_EXISTS);
    g_free(info_file_path);

    return _is_plugin;
}

void _init_state(gpointer key,
                 gpointer value G_GNUC_UNUSED,
                 gpointer user_data)
{
    g_hash_table_replace((GHashTable*)user_data,
                         g_strdup(key),
                         GINT_TO_POINTER(DISABLED_PLUGIN));
}

gchar * get_schema_id(GSettings* gsettings)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(gsettings), "schema-id", &value);
    char * schema_id = g_strdup(g_value_get_string(&value));
    g_value_unset(&value);
    return schema_id;
}

void get_enabled_plugins(GSettings* gsettings, char const* key)
{
    char * schema_id = get_schema_id(gsettings);
    char const* id_prefix = NULL;
    if (g_str_has_suffix(schema_id, "desktop")) {
        id_prefix = "desktop:";
    }

    g_free(schema_id);
    g_assert(id_prefix != NULL);

    char** values = g_settings_get_strv(gsettings, key);
    for (int i = 0; values[i] != NULL; ++i) {
        g_hash_table_add(enabled_plugins,
                         g_strconcat(id_prefix, values[i], NULL));
        g_hash_table_replace(plugins_state,
                             g_strconcat(id_prefix, values[i], NULL),
                             GINT_TO_POINTER(ENABLED_PLUGIN));
    }

    g_strfreev(values);
}

JS_EXPORT_API
void dcore_init_plugins(char const* app_name)
{
    GSettings* gsettings = NULL;
    if (desktop_gsettings == NULL) {
        desktop_gsettings = g_settings_new(DESKTOP_SCHEMA_ID);
    }

    if (0 == g_strcmp0(app_name, "desktop")) {
        gsettings = desktop_gsettings;
    }

    if (plugins_state == NULL) {
        plugins_state = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    }

    g_hash_table_foreach(plugins_state, _init_state, plugins_state);

    if (enabled_plugins != NULL) {
        g_hash_table_unref(enabled_plugins);
    }

    enabled_plugins = g_hash_table_new_full(g_str_hash, g_str_equal,
                                            g_free, NULL);
    get_enabled_plugins(gsettings, SCHEMA_KEY_ENABLED_PLUGINS);
}

void scan_plugin_dir(char const* path, char const* app_name, JSObjectRef array)
{
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        return;
    }

    JSContextRef ctx = get_global_context();
    GDir* dir = g_dir_open(path, 0, NULL);
    if (dir != NULL) {
        const char* file_name = NULL;
        for (int i=0; NULL != (file_name = g_dir_read_name(dir)); ) {
            char* full_path = g_build_filename(path, file_name, NULL);

            if (IS_DIR(full_path) && is_plugin(full_path)) {
                char* js_name = g_strconcat(file_name, ".js", NULL);
                char* js_path = g_build_filename(full_path, js_name, NULL);
                g_free(js_name);

                char* key = g_strconcat(app_name, ":", file_name, NULL);
                if (g_hash_table_contains(enabled_plugins, key)) {

                    JSValueRef v = jsvalue_from_cstr(ctx, js_path);
                    json_array_insert(array, i++, v);
                }

                g_free(key);
                g_free(js_path);
            }

            g_free(full_path);
        }

        g_dir_close(dir);
    }
}

JS_EXPORT_API
JSValueRef dcore_get_plugins(const char* app_name)
{
    JSObjectRef array = json_array_create();

    char* path = g_build_filename(RESOURCE_DIR, app_name, "plugin", NULL);
    scan_plugin_dir(path, app_name, array);
    g_free(path);

    // TBD
    path = g_build_filename(getenv("HOME"), ".dde-plugin",
                            app_name, "plugin", NULL);
    scan_plugin_dir(path, app_name, array);
    g_free(path);

    return array;
}

void create_strv(gpointer key G_GNUC_UNUSED,
                 gpointer value,
                 gpointer user_data)
{
    char* pos = strchr((char*)value, ':');
    char* plugin_name = g_strdup(pos + 1);
    g_ptr_array_add((GPtrArray*)user_data, plugin_name);
}

void enable_plugin(GSettings* gsettings, char const* id, gboolean value)
{
    if (value && !g_hash_table_contains(enabled_plugins, id)) {
        g_hash_table_add(enabled_plugins, g_strdup(id));
        g_hash_table_replace(plugins_state, g_strdup(id),
                             GINT_TO_POINTER(ENABLED_PLUGIN));
    } else if (!value) {
        g_hash_table_remove(enabled_plugins, id);
        g_hash_table_replace(plugins_state, g_strdup(id),
                             GINT_TO_POINTER(DISABLED_PLUGIN));
    }

    GPtrArray* values = g_ptr_array_new_with_free_func(g_free);
    g_hash_table_foreach(enabled_plugins, create_strv, (gpointer)values);
    g_ptr_array_add(values, NULL);
    g_settings_set_strv(gsettings, SCHEMA_KEY_ENABLED_PLUGINS,
                        (char const* const*)values->pdata);
    g_ptr_array_unref(values);
    g_settings_sync();
}

JS_EXPORT_API
void dcore_enable_plugin(char const* id, gboolean value)
{
    GSettings* gsettings = NULL;
    char* pos = strchr(id, ':');
    char* app_name = g_strndup(id, pos - id);

    if (0 == g_strcmp0(app_name, "desktop")) {
        gsettings = desktop_gsettings;
    }

    g_free(app_name);

    enable_plugin(gsettings, id, value);
}

void trans_to_js_array(char** strv, gsize length, JSObjectRef json)
{
    JSContextRef ctx = get_global_context();
    for (guint i = 0; strv[i] != NULL || i < length; ++i) {
        json_array_insert(json, i, jsvalue_from_cstr(ctx, strv[i]));
    }
}

JS_EXPORT_API
JSValueRef dcore_get_plugin_info(char const* path)
{
    char* info_file_path = g_build_filename(path, "info.ini", NULL);
    GKeyFile* info_file = g_key_file_new();
    g_key_file_load_from_file(info_file, info_file_path,
                              G_KEY_FILE_NONE, NULL);
    g_free(info_file_path);

    JSObjectRef json = json_create();
    char* id = g_key_file_get_string(info_file, "Plugin", "ID", NULL);
    json_append_string(json, "ID", id == NULL ? "" : id);
    g_free(id);

    char* name = g_key_file_get_string(info_file, "Plugin", "name", NULL);
    json_append_string(json, "name", name == NULL ? "" : name);
    g_free(name);

    char* description = g_key_file_get_string(info_file, "Plugin",
                                              "description", NULL);
    json_append_string(json, "description",
                       description == NULL ? "" : description);
    g_free(description);

    int width = g_key_file_get_integer(info_file, "Plugin", "width", NULL);
    json_append_number(json, "width", width);

    int height = g_key_file_get_integer(info_file, "Plugin", "height", NULL);
    json_append_number(json, "height", height);

    GError* error = NULL;
    double x = g_key_file_get_double(info_file, "Plugin", "x", &error);
    if (error) {
        json_append_value(json, "x", jsvalue_null());
        g_error_free(error);
    } else {
        json_append_number(json, "x", x);
    }

    error = NULL;
    double y = g_key_file_get_double(info_file, "Plugin", "y", &error);
    if (error) {
        json_append_value(json, "y", jsvalue_null());
        g_error_free(error);
    } else {
        json_append_number(json, "y", y);
    }

    char* type = g_key_file_get_string(info_file, "Plugin", "type", NULL);
    json_append_string(json, "type", type == NULL ? "" : type);
    g_free(type);

    char* author = g_key_file_get_string(info_file, "Author", "author", NULL);
    json_append_string(json, "author", author == NULL ? "" : author);
    g_free(author);

    char* email = g_key_file_get_string(info_file, "Author", "email", NULL);
    json_append_string(json, "email", email == NULL ? "" : email);
    g_free(email);

    char* textdomain = g_key_file_get_string(info_file, "Locale",
                                             "textdomain", NULL);
    json_append_string(json, "textdomain",
                       textdomain == NULL ? "" : textdomain);
    g_free(textdomain);

    gsize length = 0;
    char** js = g_key_file_get_string_list(info_file, "Resource", "js",
                                           &length, NULL);
    JSObjectRef js_arr = json_array_create();
    trans_to_js_array(js, length, js_arr);
    json_append_value(json, "js", js_arr);
    g_strfreev(js);

    char** css = g_key_file_get_string_list(info_file, "Resource", "css",
                                            &length, NULL);
    JSObjectRef css_arr = json_array_create();
    trans_to_js_array(css, length, css_arr);
    json_append_value(json, "css", css_arr);
    g_strfreev(css);

    char** screenshot = g_key_file_get_string_list(info_file, "Resource",
                                                   "screenshot", &length,
                                                   NULL);
    JSObjectRef ss_arr = json_array_create();
    trans_to_js_array(screenshot, length, ss_arr);
    json_append_value(json, "screenshot", ss_arr);
    g_strfreev(screenshot);

    g_key_file_free(info_file);

    return json;
}

JS_EXPORT_API
void dcore_new_window(const char* url, const char* title, double w, double h)
{
    g_warning("Don't use the function now!");
    GtkWidget* container = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(container), title);
    gtk_widget_set_size_request(container, (int)w, (int)h);
    GtkWidget *webview = d_webview_new_with_uri(url);
    gtk_container_add(GTK_CONTAINER(container), webview);
    gtk_widget_show_all(container);
}

JS_EXPORT_API
gboolean dcore_open_browser(char const* origin_uri)
{
    if (origin_uri == NULL || origin_uri[0] == '\0') {
        return FALSE;
    }

    char* uri = g_uri_unescape_string(origin_uri,
                                      G_URI_RESERVED_CHARS_ALLOWED_IN_PATH);
    char* scheme = g_uri_parse_scheme(uri);
    if (scheme == NULL) {
        char* uri_without_scheme = uri;
        uri = g_strconcat("http://", uri_without_scheme, NULL);
        g_free(uri_without_scheme);
    }
    g_free(scheme);

    gboolean launch_result = g_app_info_launch_default_for_uri(uri, NULL,
                                                               NULL);
    g_free(uri);

    return launch_result;
}

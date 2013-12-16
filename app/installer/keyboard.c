/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 * Maintainer:  Long Wei <yilang2007lw@gamil.com>
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

#include "keyboard.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>

static XklConfigRec *config_rec = NULL;
static GHashTable *layout_variants_hash = NULL;
static GHashTable *layout_desc_hash = NULL;

static void 
_foreach_variant (XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    XklConfigItem *layout = (XklConfigItem *) data;

    GList *variants = g_list_copy (g_hash_table_lookup (layout_variants_hash, layout->name));
    variants = g_list_append (variants, g_strdup (item->name));
    g_hash_table_replace (layout_variants_hash, g_strdup (layout->name), variants);

    gchar *key = g_strdup_printf ("%s,%s", layout->name, item->name);
    gchar *value = g_strdup_printf ("%s,%s", layout->description, item->description);
    g_hash_table_insert (layout_desc_hash, key, value);
}

static void 
_foreach_layout(XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    GList *variants = NULL;
    g_hash_table_insert (layout_variants_hash, g_strdup (item->name), variants);
    g_hash_table_insert (layout_desc_hash, g_strdup (item->name), g_strdup (item->description));
    xkl_config_registry_foreach_layout_variant(config, 
                                               item->name,
                                               _foreach_variant, 
                                               (gpointer) item);
}

void
init_keyboard_layouts () 
{
    static gboolean inited = FALSE;
    if (inited) {
        g_warning ("init keyboard layouts:already inited\n");
    }
    inited = TRUE;

    layout_variants_hash = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                                  (GEqualFunc) g_str_equal, 
                                                  (GDestroyNotify) g_free, 
                                                  (GDestroyNotify) g_list_free);

    layout_desc_hash = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                                  (GEqualFunc) g_str_equal, 
                                                  (GDestroyNotify) g_free, 
                                                  (GDestroyNotify) g_list_free);

    Display *dpy = NULL;
    XklEngine *engine = NULL;
    XklConfigRegistry *cfg_reg = NULL;
    
    dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning ("init keyboard layouts: XOpenDisplay\n");
        goto out;
    }

    engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("init keyboard layouts: xkl engine get instance\n");
        goto out;
    }

    config_rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config_rec, engine);
    if (config_rec == NULL) {
        g_warning ("init keyboard layouts: xkl config rec\n");
        goto out;
    }

    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("init keyboard layouts: xkl config registry get instance\n");
        goto out;
    }
    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("init keyboard layouts: xkl config registry load\n");
        goto out;
    }

    xkl_config_registry_foreach_layout(cfg_reg, _foreach_layout, NULL);
    goto out;

out:
    if (cfg_reg != NULL) {
        g_object_unref (cfg_reg);
    }
    if (engine != NULL) {
        g_object_unref (engine);
    }
    if (dpy != NULL) {
        XCloseDisplay (dpy);
    }
}

JS_EXPORT_API 
gchar *installer_get_layout_description (const gchar *layout)
{
    gchar *desc = NULL;
    if (layout_desc_hash == NULL) {
        g_warning ("get layout description:layout desc hash NULL\n");
        init_keyboard_layouts ();
    }
    desc = g_strdup (g_hash_table_lookup (layout_desc_hash, layout));
    if (desc == NULL) {
        desc = g_strdup (layout);
    }

    return desc;
}

JS_EXPORT_API 
JSObjectRef installer_get_keyboard_layouts ()
{
    JSObjectRef layouts = json_array_create ();
    if (layout_variants_hash == NULL) {
        init_keyboard_layouts ();
    }

    gsize index = 0;
    GList *keys = g_hash_table_get_keys (layout_variants_hash);

    for (index = 0; index < g_list_length (keys); index++) {
        gchar *layout = g_strdup (g_list_nth_data (keys, index));
        json_array_insert (layouts, index, jsvalue_from_cstr (get_global_context (), layout));
        g_free (layout);
    }

    return layouts;
}

JS_EXPORT_API 
JSObjectRef installer_get_layout_variants (const gchar *layout_name) 
{
    JSObjectRef layout_variants = json_array_create ();
    if (layout_variants_hash == NULL) {
        g_warning ("get layout variants:layout variants hash NULL\n");
        init_keyboard_layouts ();
    }

    gsize index = 0;
    GList *variants = (GList *) g_hash_table_lookup (layout_variants_hash, layout_name);

    for (index = 0; index < g_list_length (variants); index++) {
        gchar *variant = g_strdup (g_list_nth_data (variants, index));
        json_array_insert (layout_variants, index, jsvalue_from_cstr (get_global_context (), variant));
        g_free (variant);
    }

    return layout_variants;
}

JS_EXPORT_API
JSObjectRef installer_get_current_layout_variant ()
{
    JSObjectRef current = json_create ();

    if (config_rec == NULL) {
        g_warning ("get current layout variant:xkl config null\n");
        init_keyboard_layouts ();
        return current;
    }

    gchar **layouts = g_strdupv (config_rec->layouts);
    gchar **variants = g_strdupv (config_rec->variants);

    JSObjectRef layout_array = json_array_create ();
    JSObjectRef variant_array = json_array_create ();

    gsize index = 0;
    for (index = 0; index < g_strv_length (layouts); index++) {
        json_array_insert (layout_array, index, jsvalue_from_cstr (get_global_context (), layouts[index]));
    }
    json_append_value (current, "layouts", (JSValueRef) layout_array);

    for (index = 0; index < g_strv_length (variants); index++) {
        json_array_insert (variant_array, index, jsvalue_from_cstr (get_global_context (), variants[index]));
    }
    json_append_value (current, "variants", (JSValueRef) variant_array);

    g_strfreev (layouts);
    g_strfreev (variants);

    return current;
}

JS_EXPORT_API 
void installer_set_keyboard_layout_variant (const gchar *layout, const gchar *variant)
{
    gchar *contents = NULL;
    gchar **lines = NULL;
    gchar **new = NULL;
    gchar *result = NULL;
    gboolean succeed = FALSE;
    GError *error = NULL;

    g_file_get_contents ("/etc/default/keyboard", &contents, NULL, &error);
    if (error != NULL) {
        g_warning ("set keyboard:get file contents\n");
        goto out;
    }

    lines = g_strsplit (contents, "\n", -1);
    if (lines == NULL) {
        g_warning ("set keyboard:lines NULL\n");
        goto out;
    }
    new = g_new0 (gchar *, g_strv_length (lines) + 1);
    int i;
    for (i = 0; i < g_strv_length (lines); i++) {
        //g_warning ("line %d:%s\n", i, lines[i]);
        if (g_str_has_prefix (lines[i], "XKBLAYOUT=")) {
            new[i] = g_strdup_printf ("XKBLAYOUT=\"%s\"", layout != NULL ? layout : "");
        } else if (g_str_has_prefix (lines[i], "XKBVARIANT=")) {
            new[i] = g_strdup_printf ("XKBVARIANT=\"%s\"", variant != NULL ? variant: "");
        } else {
            new[i] = g_strdup (lines[i]);
        }
    }
    result = g_strjoinv ("\n", new);
    if (result == NULL) {
        g_warning ("set keyboard:result NULL\n");
        goto out;
    }
    g_file_set_contents ("/etc/default/keyboard", result, -1, &error);
    if (error != NULL) {
        g_warning ("set keyboard:set file contents\n");
        goto out;
    }
    succeed = TRUE;
    goto out;

out:
    g_free (contents);
    g_strfreev (lines);
    g_strfreev (new);
    g_free (result);
    if (error != NULL) {
        g_error_free (error);
    }
    if (!succeed) {
        g_warning ("set keyboard failed, just skip\n");
    }
    emit_progress ("keyboard", "finish");
}

/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "keyboard.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>
#include "utils.h"

#define KEYBOARD_DETECT_FILE    RESOURCE_DIR"/pc105.tree"
/**
 setxkbmap -query
 can get current keyboard layout
 **/
XklConfigRec *config_rec = NULL;
GHashTable *layout_variants_hash = NULL;
GHashTable *layout_desc_hash = NULL;

enum StepType {
    UNKNOWN,
    PRESS_KEY,
    KEY_PRESENT,
    KEY_PRESENT_P,
    RESULT
};

static gchar *pc105_contents = NULL;
static gchar *current_step = NULL;
static GList *symbols = NULL;
static GHashTable *keycodes = NULL;
static gchar *present = NULL;
static gchar *not_present = NULL;
static gchar *result = NULL;

static void 
_foreach_variant (XklConfigRegistry *config, const XklConfigItem *item,
                  gpointer data)
{
    if (item == NULL || data == NULL) {
        g_warning ("[%s]: some param NULL\n", __func__);
        return;
    }
    XklConfigItem *layout = (XklConfigItem *) data;

    GList *variants = g_list_copy(
        g_hash_table_lookup(layout_variants_hash, layout->name));
    variants = g_list_append (variants, g_strdup (item->name));
    g_hash_table_replace(layout_variants_hash, g_strdup(layout->name),
                         variants);

    gchar *key = g_strdup_printf("%s,%s", layout->name, item->name);
    gchar *value = g_strdup_printf("%s,%s", layout->description,
                                   item->description);
    g_hash_table_insert (layout_desc_hash, key, value);
}

static void 
_foreach_layout(XklConfigRegistry *config, const XklConfigItem *item,
                gpointer data)
{
    if (config == NULL || item == NULL) {
        g_warning ("[%s]: some param NULL\n", __func__);
        return;
    }
    GList *variants = NULL;
    g_hash_table_insert(layout_variants_hash, g_strdup (item->name), variants);
    g_hash_table_insert(layout_desc_hash, g_strdup(item->name),
                        g_strdup(item->description));
    xkl_config_registry_foreach_layout_variant(config, item->name,
                                               _foreach_variant, 
                                               (gpointer) item);
}

void
init_keyboard_layouts () 
{
    static gboolean inited = FALSE;
    if (inited) {
        g_warning ("[%s]: already inited\n", __func__);
    }
    inited = TRUE;

    layout_variants_hash = g_hash_table_new_full((GHashFunc) g_str_hash, 
                                                 (GEqualFunc) g_str_equal, 
                                                 (GDestroyNotify) g_free, 
                                                 (GDestroyNotify) g_list_free);

    layout_desc_hash = g_hash_table_new_full((GHashFunc) g_str_hash, 
                                             (GEqualFunc) g_str_equal, 
                                             (GDestroyNotify) g_free, 
                                             (GDestroyNotify) g_list_free);

    Display *dpy = NULL;
    XklEngine *engine = NULL;
    XklConfigRegistry *cfg_reg = NULL;
    
    dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning("[%s] display is NULL\n", __func__);
        goto out;
    }

    engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("[%s]: xkl engine is NULL\n", __func__);
        goto out;
    }

    config_rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config_rec, engine);
    if (config_rec == NULL) {
        g_warning ("[%s]: xkl config rec is NULL\n", __func__);
        goto out;
    }

    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("[%s]: cfg_reg is NULL\n", __func__);
        goto out;
    }
    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("[%s]: xkl config registry load\n", __func__);
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
        g_warning ("[%s]: layout desc hash NULL\n", __func__);
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
    GRAB_CTX ();
    JSObjectRef layouts = json_array_create ();
    if (layout_variants_hash == NULL) {
        init_keyboard_layouts ();
    }

    gsize index = 0;
    GList *keys = g_hash_table_get_keys (layout_variants_hash);

    for (index = 0; index < g_list_length (keys); index++) {
        gchar *layout = g_strdup (g_list_nth_data (keys, index));
        json_array_insert(layouts, index,
                          jsvalue_from_cstr(get_global_context(), layout));
        g_free (layout);
    }

    UNGRAB_CTX ();
    return layouts;
}

JS_EXPORT_API 
JSObjectRef installer_get_layout_variants (const gchar *layout_name) 
{
    GRAB_CTX ();
    JSObjectRef layout_variants = json_array_create ();
    if (layout_name == NULL) {
        g_warning ("[%s]: layout is NULL\n", __func__);
        return layout_variants;
    }
    if (layout_variants_hash == NULL) {
        g_warning ("[%s]: layout variants hash NULL\n", __func__);
        init_keyboard_layouts ();
    }

    gsize index = 0;
    GList *variants = (GList *) g_hash_table_lookup(layout_variants_hash,
                                                    layout_name);

    for (index = 0; index < g_list_length(variants); index++) {
        gchar *variant = g_strdup (g_list_nth_data(variants, index));
        json_array_insert(layout_variants, index,
                          jsvalue_from_cstr(get_global_context(), variant));
        g_free (variant);
    }
    UNGRAB_CTX ();

    return layout_variants;
}

JS_EXPORT_API
JSObjectRef installer_get_current_layout_variant ()
{
    GRAB_CTX ();
    JSObjectRef current = json_create ();

    if (config_rec == NULL) {
        g_warning("[%s]: config_rec is NULL\n", __func__);
        init_keyboard_layouts ();
    }

    gchar **layouts = g_strdupv (config_rec->layouts);
    gchar **variants = g_strdupv (config_rec->variants);

    JSObjectRef layout_array = json_array_create ();
    JSObjectRef variant_array = json_array_create ();

    gsize index = 0;
    for (index = 0; index < g_strv_length (layouts); index++) {
        json_array_insert(layout_array, index,
                          jsvalue_from_cstr(get_global_context(),
                                            layouts[index]));
    }
    json_append_value (current, "layouts", (JSValueRef) layout_array);

    for (index = 0; index < g_strv_length (variants); index++) {
        json_array_insert(variant_array, index,
                          jsvalue_from_cstr(get_global_context(),
                                            variants[index]));
    }
    json_append_value (current, "variants", (JSValueRef) variant_array);

    g_strfreev (layouts);
    g_strfreev (variants);
    UNGRAB_CTX ();

    return current;
}

static void
init_keyboard_detect ()
{
    static gboolean inited = FALSE;
    if (inited) {
        g_warning("[%s]: already inited\n", __func__);
        return;
    }
    inited = TRUE;

    gsize length;
    GError *error = NULL;

    if (!g_file_get_contents(KEYBOARD_DETECT_FILE, &pc105_contents, &length,
                             &error)) {
        g_warning("[%s]: detect_file: %s, error: %s\n", __func__,
                  KEYBOARD_DETECT_FILE, error->message);
        g_error_free (error);
    }
}

JS_EXPORT_API 
double installer_keyboard_detect_read_step (gchar *step)
{
    g_debug("[%s]: step: %s, current_step: %s\n", __func__, step,
            current_step);
    if (step == NULL) {
        g_warning("[%s]: step NULL\n", __func__);
        return 0;
    }
    if (pc105_contents == NULL) {
       init_keyboard_detect (); 
    }
    if (current_step != NULL) {
        if (result != NULL) {
            g_warning("[%s]: already done\n", __func__);
        }
    }

    enum StepType t = UNKNOWN;
    if (keycodes != NULL) {
        g_hash_table_destroy (keycodes);
        keycodes = NULL;
    }
    if (symbols != NULL) {
        g_list_free_full (symbols, g_free);
        symbols = NULL;
    }
    if (result != NULL) {
        g_free (result);
        result = NULL;
    }

    gchar **lines = g_strsplit (pc105_contents, "\n", -1);
    guint i = 0;
    guint size = g_strv_length (lines);
    gboolean found = FALSE;
    for (i = 0; i < size ; i++) {
        if (g_str_has_prefix (lines[i], "STEP ")) {
            if (!found) {
                //g_warning ("prefix step:read over %s\n", lines[i]);
                if (current_step != NULL) {
                    g_free (current_step);
                    current_step = NULL;
                }
                current_step = g_strstrip (g_strdup (lines[i] + 5));
                if (g_strcmp0 (current_step, step) == 0) {
                    found = TRUE;
                }
            } else {
                //g_warning ("prefix step:read end at %s\n", lines[i]);
                goto out;
            }
        } 

        if (g_strcmp0 (current_step, step) != 0) {
            //g_warning ("current step %s not equal target step %s with line->%s\n", current_step, step, lines[i]);
            continue;

        } else if (g_str_has_prefix (lines[i], "PRESS ")) {
            g_debug("[%s] prefix PRESS: %s\n", __func__, lines[i]);
            if (t == UNKNOWN) {
                t = PRESS_KEY;
            } 
            if (t != PRESS_KEY) {
                g_warning("[%s]: invalid step goto PRESS\n", __func__);
            }
            symbols = g_list_append(symbols,
                                    g_strstrip(g_strdup(lines[i] + 6)));

        } else if (g_str_has_prefix (lines[i], "CODE ")) {
            g_debug("[%s] prefix CODE: %s\n", __func__, lines[i]);
            if (t != PRESS_KEY) {
                g_warning("[%s]: invalid step goto CODE\n", __func__);
            }
            gchar **items = g_strsplit (lines[i], " ", -1);
            gchar *key = g_strdup(items[1]);
            gchar *value = g_strdup(items[2]);
            g_strfreev (items);

            if (keycodes == NULL) {
                keycodes = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                 g_free, g_free);
            }
            g_hash_table_insert (keycodes, key, value);

        } else if (g_str_has_prefix (lines[i], "FIND ")) {
            g_debug("[%s] prefix FIND: %s\n", __func__, lines[i]);
            if (t == UNKNOWN) {
                t = KEY_PRESENT;
            } else {
                g_warning("[%s]: invalid step goto FIND\n", __func__);
            }
            symbols = g_list_append(symbols,
                                    g_strstrip(g_strdup(lines[i] + 5)));

        } else if (g_str_has_prefix (lines[i], "FINDP ")) {
            g_debug("[%s] prefix FINDP: %s\n", __func__, lines[i]);
            if (t == UNKNOWN) {
                t = KEY_PRESENT_P;
            } else {
                g_warning("[%s]: invalid step goto FINDP\n", __func__);
            }
            symbols = g_list_append(symbols,
                                    g_strstrip(g_strdup(lines[i] + 6)));

        } else if (g_str_has_prefix (lines[i], "YES ")) {
            g_debug("[%s] prefix YES: %s\n", __func__, lines[i]);
            if (t != KEY_PRESENT_P && t != KEY_PRESENT) {
                g_warning("[%s]: invalid step goto YES\n", __func__);
            }
            if (present != NULL) {
                g_free (present);
                present = NULL;
            }
            present = g_strstrip (g_strdup (lines[i] + 4));

        } else if (g_str_has_prefix (lines[i], "NO ")) {
            g_debug("[%s] prefix NO: s\n", __func__, lines[i]);
            if (t != KEY_PRESENT_P && t != KEY_PRESENT) {
                g_warning("[%s]: invalid step goto NO\n", __func__);
            }
            if (not_present != NULL) {
                g_free (not_present);
                not_present = NULL;
            }
            not_present = g_strstrip (g_strdup (lines[i] + 3));

        } else if (g_str_has_prefix (lines[i], "MAP ")) {
            g_debug("[%s] prefix MAP: %s\n", __func__, lines[i]);
            if (t == UNKNOWN) {
                t = RESULT;
            }
            result = g_strstrip (g_strdup (lines[i] + 4));
            goto out;
        }
    }
out:
    if (lines != NULL) {
        g_strfreev (lines);
    }
    return t;
}

JS_EXPORT_API 
JSObjectRef installer_keyboard_detect_get_symbols ()
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();
    guint i = 0;
    for (i = 0; i < g_list_length (symbols); i++) {
        gchar * symbol = g_strdup (g_list_nth_data (symbols, i));
        json_array_insert(array, i,
                          jsvalue_from_cstr(get_global_context(), symbol));
        g_free (symbol);
    }
    UNGRAB_CTX ();
    return array;
}

JS_EXPORT_API 
gchar* installer_keyboard_detect_get_present ()
{
    return g_strdup (present);
}

JS_EXPORT_API
gchar* installer_keyboard_detect_get_not_present ()
{
    return g_strdup (not_present);
}

JS_EXPORT_API
JSObjectRef installer_keyboard_detect_get_keycodes ()
{
    GRAB_CTX ();
    JSObjectRef json = json_create ();
    if (keycodes == NULL) {
        return json;
    }
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, keycodes);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        gchar *skey = g_strdup (key);
        gchar *svalue = g_strdup (value);
        json_append_string (json, skey, svalue);
        g_free (skey);
        g_free (svalue);
    }
    UNGRAB_CTX ();
    return json;
} 
JS_EXPORT_API 
gchar* installer_keyboard_detect_get_result ()
{
    return g_strdup (result);
}

JS_EXPORT_API
void installer_set_layout(const gchar* layout)
{
    const gchar* cmd = g_strdup_printf("/usr/bin/setxkbmap -layout %s", 
                                       layout);
    spawn_command_sync(cmd,TRUE);
}

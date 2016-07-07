/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "greeter_util.h"
#include "user.h"
#include "mutils.h"

static GList *users = NULL;

JS_EXPORT_API
JSObjectRef greeter_get_users ()
{
    JSObjectRef array = json_array_create ();

    LightDMUser *user = NULL;
    guint i;

    if (users == NULL) {
        LightDMUserList *user_list = lightdm_user_list_get_instance ();
        if (user_list == NULL) {
            g_warning("[%s]: user list is NULL\n", __func__);
            return array;
        }

        users = lightdm_user_list_get_users (user_list);
    }

    for (i = 0; i < g_list_length (users); ++i) {
        gchar *username = NULL;

        user = (LightDMUser *) g_list_nth_data (users, i);
        username = g_strdup (lightdm_user_get_name (user));

        json_array_insert(array, i, jsvalue_from_cstr(get_global_context(),
                                                      g_strdup(username)));

        g_free (username);
    }

    return array;
}

JS_EXPORT_API
gchar* greeter_get_user_icon (const gchar* name)
{
    return get_user_icon (name);
}

JS_EXPORT_API
gchar* greeter_get_user_realname (const gchar* name)
{
    return get_user_realname (name);
}

JS_EXPORT_API
gboolean greeter_user_need_password (const gchar *name)
{
    return is_need_pwd (name);
}

JS_EXPORT_API
gchar* greeter_get_default_user ()
{
    gchar *username = NULL;
    extern LightDMGreeter *greeter;
    extern GKeyFile *greeter_keyfile;

    username = g_strdup(g_key_file_get_value(greeter_keyfile,
                                             "deepin-greeter", "last-user",
                                             NULL));
    if (username == NULL) {
        username = g_strdup (lightdm_greeter_get_select_user_hint (greeter));
    }

    return username;
}

JS_EXPORT_API
gchar* greeter_get_user_session (const gchar *name)
{
    gchar *session = NULL;
    LightDMUserList *user_list = NULL;
    LightDMUser *user = NULL;

    user_list = lightdm_user_list_get_instance ();
    if (user_list == NULL) {
        g_warning("[%s]: user list is NULL\n", __func__);
        return NULL;
    }

    user = lightdm_user_list_get_user_by_name (user_list, name);
    if (user == NULL) {
        g_warning("[%s]: user for %s is NULL\n", __func__, name);
        return NULL;
    }

    session = g_strdup (lightdm_user_get_session (user));

    return session;
}

JS_EXPORT_API
gboolean greeter_is_hide_users ()
{
    extern LightDMGreeter *greeter;

    return lightdm_greeter_get_hide_users_hint (greeter);
}

JS_EXPORT_API
gboolean greeter_is_support_guest ()
{
    extern LightDMGreeter *greeter;

    return lightdm_greeter_get_has_guest_account_hint (greeter);
}

JS_EXPORT_API
gboolean greeter_is_guest_default ()
{
    extern LightDMGreeter *greeter;

    return lightdm_greeter_get_select_guest_hint (greeter);
}

JS_EXPORT_API
void greeter_draw_user_background (JSValueRef canvas, const gchar *username)
{
    draw_user_background (canvas, username);
}

JS_EXPORT_API
gchar* greeter_get_date ()
{
    return get_date_string ();
}

JS_EXPORT_API
gboolean greeter_detect_capslock ()
{
    return is_capslock_on ();
}


LightDMUser* lightdm_user_get_by_username(const gchar *name)
{
    LightDMUserList *user_list = NULL;
    LightDMUser *user = NULL;

    user_list = lightdm_user_list_get_instance ();
    if (user_list == NULL) {
        g_warning("[%s]: user list is NULL\n", __func__);
        return NULL;
    }

    user = lightdm_user_list_get_user_by_name (user_list, name);
    if (user == NULL) {
        g_warning("[%s]: user for %s is NULL\n", __func__, name);
        return NULL;
    }
    
    return user;
}


JS_EXPORT_API
gboolean greeter_get_user_session_on(const gchar *username)
{
    g_message("[%s] username: %s\n", __func__, username);
    extern LightDMGreeter *greeter;
    const gchar *user_lock_path = NULL;
    const gchar *username_authentication = NULL;
    gboolean result = FALSE;
    LightDMUser *user = NULL;

    user = lightdm_user_get_by_username(username);
    result = lightdm_user_get_logged_in(user);
    if (result){
        g_warning("[%s] lightdm_user_get_logged_in return TRUE\n", __func__);
    } else {
        g_warning("[%s] lightdm_user_get_logged_in return FALSE\n", __func__);
    }

    user_lock_path = g_strdup_printf("%s%s", username, ".dlock.app.deepin");
    if (app_is_running (user_lock_path)) {
        /*result = TRUE;*/
        g_warning("[%s] user_lock_path is_running, login_on is TRUE\n",
                  __func__);
    }

    username_authentication = lightdm_greeter_get_authentication_user(greeter);
    if (g_strcmp0 (username, username_authentication) == 0) {
        /*result = TRUE;*/
        g_warning("[%s] username_authentication is %s, login_on is TRUE\n",
                  __func__, username);
    }

    g_warning("[%s] result is %d.\n", __func__, result);
    return result;
}

JS_EXPORT_API
void greeter_set_language(char* lang)
{
    g_message("[%s] language: %s\n", __func__, lang);
    GError *error = NULL;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                      0,
                                      NULL,
                                      "com.deepin.helper.LanguageSelector",
                                      "/com/deepin/helper/LanguageSelector",
                                      "com.deepin.helper.LanguageSelector",
                                      NULL,
                                      &error);
    if (error != NULL) {
        g_warning("[%s] on com.deepin.helper.LanguageHelper failed: %s\n",
                  __func__, error->message);
        g_error_free(error);
    }

    if (proxy != NULL) {
        GVariant* params = NULL;
        GVariant* retval = g_dbus_proxy_call_sync(proxy, "Set",
                                                  g_variant_new("(s)", lang),
                                                  G_DBUS_CALL_FLAGS_NONE,
                                                  -1, NULL, NULL);
        if (retval != NULL) {
            g_variant_unref(retval);
        }
        g_object_unref(proxy);
    }
}

/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2011 ~ 2013 Long Wei
 *
 * Author:      Long Wei <yilang2007lw@gmail.com>
 *              bluth <yuanchenglu001@gmail.com>
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

#include "session.h"

static GList *sessions = NULL;

JS_EXPORT_API
JSObjectRef greeter_get_sessions ()
{
    JSObjectRef array = json_array_create ();

    guint i;

    if (sessions == NULL) {
        sessions = lightdm_get_sessions ();
    }

    for (i = 0; i < g_list_length (sessions); ++i) {
        gchar *key = NULL;
        LightDMSession *session = (LightDMSession *) g_list_nth_data (sessions, i);

        key = g_strdup (lightdm_session_get_key (session));
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context (), g_strdup (key)));

        g_free (key);
    }

    return array;
}

LightDMSession*
find_session_by_key(const gchar *key)
{
    LightDMSession *ret = NULL;
    guint i;

    if (sessions == NULL) {
        sessions = lightdm_get_sessions ();
    }

    for (i = 0; i < g_list_length (sessions); i++) {
        LightDMSession *session = (LightDMSession *) g_list_nth_data (sessions, i);

        if (session != NULL) {
            gchar *session_key = g_strdup (lightdm_session_get_key (session));
            if (g_strcmp0 (key, session_key) == 0) {
                ret = session;

            } else {
                continue;
            }
            g_free (session_key);

        } else {
            continue;
        }
    }

    return ret;
}

JS_EXPORT_API
gchar* greeter_get_session_name (const gchar *key)
{
    gchar *name = NULL;
    LightDMSession *session = NULL;

    session = find_session_by_key (key);
    if (session != NULL) {
        name = g_strdup (lightdm_session_get_name (session));

    } else {
        g_warning ("get session name:session is NULL\n");
    }

    return name;
}

JS_EXPORT_API
gchar* greeter_get_session_icon (const gchar *key)
{
    gchar* icon = NULL;

    gchar *session = g_ascii_strdown (key, -1);

    if (session == NULL) {
        g_warning ("get session icon:session is NULL\n");
        icon = g_strdup ("unkown");

    } else if (g_str_has_prefix (session, "gnome")){
        icon = g_strdup ("gnome");

    } else if (g_str_has_prefix (session, "deepin")){
        icon = g_strdup ("deepin");

    } else if (g_str_has_prefix (session, "kde")){
        icon = g_strdup ("kde");

    } else if (g_str_has_prefix (session, "ubuntu")){
        icon = g_strdup ("ubuntu");

    } else if (g_str_has_prefix (session, "xfce")){
        icon = g_strdup ("xfce");

    } else if (g_str_has_prefix (session, "lxde")){
        icon = g_strdup ("lxde");
    } else if (g_str_has_prefix (session, "enlightenment")){
        icon = g_strdup ("enlightenment");
    } else if (g_str_has_prefix (session, "fluxbox")){
        icon = g_strdup ("fluxbox");

    } else {
        icon = g_strdup ("unkown");
    }

    g_free (session);

    return icon;
}

JS_EXPORT_API
gchar* greeter_get_default_session ()
{
    gchar *key = NULL;

    extern LightDMGreeter *greeter;
    guint i;

    gchar* session_name = g_strdup (lightdm_greeter_get_default_session_hint (greeter));
    if (session_name != NULL) {

        if (sessions == NULL) {
            sessions = lightdm_get_sessions ();
        }

        for (i = 0; i < g_list_length (sessions); ++i) {

            LightDMSession *session = (LightDMSession *) g_list_nth_data (sessions, i);
            gchar *name = g_strdup (lightdm_session_get_name (session));

            if (g_strcmp0 (session_name, name) == 0) {
                key = g_strdup (lightdm_session_get_key (session));
                g_free (name);
                break;

            } else {
                g_free (name);
                continue;
            }
        }
    }
    g_free (session_name);

    return key;
}

JS_EXPORT_API
gboolean greeter_get_can_suspend ()
{
    return lightdm_get_can_suspend ();
}

JS_EXPORT_API
gboolean greeter_get_can_hibernate ()
{
    return lightdm_get_can_hibernate ();
}

JS_EXPORT_API
gboolean greeter_get_can_restart ()
{
    return lightdm_get_can_restart ();
}

JS_EXPORT_API
gboolean greeter_get_can_shutdown ()
{
    return lightdm_get_can_shutdown ();
}

JS_EXPORT_API
gboolean greeter_run_suspend ()
{
    return lightdm_suspend (NULL);
}

JS_EXPORT_API
gboolean greeter_run_hibernate ()
{
    return lightdm_hibernate (NULL);
}

JS_EXPORT_API
gboolean greeter_run_restart ()
{
    return lightdm_restart (NULL);
}

JS_EXPORT_API
gboolean greeter_run_shutdown ()
{
    return lightdm_shutdown (NULL);
}


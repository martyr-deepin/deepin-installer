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

#include "misc.h"
#include "part_util.h"
#include <pwd.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>

extern struct passwd* getpwent (void);
extern void endpwent (void);

JSObjectRef layouts;
JSObjectRef layout_variants;

void copy_file (const gchar *src, const gchar *dest)
{
  g_printf ("copy file\n");  

}

JS_EXPORT_API 
JSObjectRef installer_get_system_users()
{
    JSObjectRef array = json_array_create ();

    struct passwd *user;
    gchar *username = NULL;
    int i = 0;

    while ((user = getpwent ()) != NULL){
        username = g_strdup (user->pw_name);
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), username));
        i++;
        g_free (username);
    }

    endpwent ();

    return array;
}

JS_EXPORT_API 
void installer_create_user (const gchar *username, const gchar *hostname, const gchar *password)
{
    g_printf ("create user\n");
}

JS_EXPORT_API 
void installer_reboot ()
{
    GError *error = NULL;
    GDBusProxy *ck_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            "org.freedesktop.ConsoleKit",
            "/org/freedesktop/ConsoleKit/Manager",
            "org.freedesktop.ConsoleKit.Manager",
            NULL,
            &error);

    g_assert (ck_proxy != NULL);
    if (error != NULL) {
        g_warning ("installer reboot: ck proxy %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    GVariant *can_restart_var = g_dbus_proxy_call_sync (ck_proxy,
                                "CanRestart",
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

    g_assert (can_restart_var != NULL);
    if (error != NULL) {
        g_warning ("installer reboot: CanRestart %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    gboolean can_restart = FALSE;
    g_variant_get (can_restart_var, "(b)", &can_restart);

    g_variant_unref (can_restart_var);

    if (can_restart) {
        g_dbus_proxy_call (ck_proxy,
                           "Restart",
                           NULL,
                           G_DBUS_CALL_FLAGS_NONE,
                           -1,
                           NULL,
                           NULL,
                           NULL);
    } else {
        g_warning ("installer reboot: can restart is false\n");
    }

    g_object_unref (ck_proxy);
}

static void 
foreach_variant (XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    gsize* index = (gsize*) data;
    g_warning ("foreach variant index:%d\n", *index);

    JSValueRef variant = jsvalue_from_cstr (get_global_context (), item->name);

    json_array_insert (layout_variants, *index, variant);

    *index = *index + 1;
}

static void 
foreach_layout(XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    gsize* index = (gsize*) data;
    g_debug ("foreach layout index:%d\n", *index);

    JSValueRef layout = jsvalue_from_cstr (get_global_context (), item->name);

    json_array_insert (layouts, *index, layout);

    *index = *index + 1;
}

JS_EXPORT_API 
JSObjectRef installer_get_keyboard_layouts ()
{
    layouts = json_array_create ();

    Display *dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning ("get keyboard layouts: XOpenDisplay\n");
        return layouts;
    }

    XklEngine *engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("get keyboard layouts: xkl engine get instance\n");
        return layouts;
    }

    XklConfigRegistry *cfg_reg = NULL;
    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("get keyboard layouts: xkl config registry get instance\n");
        return layouts;
    }

    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("get keyboard layouts: xkl config registry load\n");
        return layouts;
    }

    gsize index = 0;
    xkl_config_registry_foreach_layout(cfg_reg, foreach_layout, &index);

    g_object_unref (engine);
    g_object_unref (cfg_reg);
    XCloseDisplay (dpy);

    return layouts;
}

JS_EXPORT_API 
JSObjectRef installer_get_layout_variants (const gchar *layout_name) 
{

    layout_variants = json_array_create ();

    Display *dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning ("get layout variants: XOpenDisplay\n");
        return layout_variants;
    }

    XklEngine *engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("get layout variants: xkl engine get instance\n");
        return layout_variants;
    }

    XklConfigRegistry *cfg_reg = NULL;
    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("get layout variants: xkl config registry get instance\n");
        return layout_variants;
    }

    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("get layout variants: xkl config registry load\n");
        return layout_variants;
    }

    gsize index = 0;
    xkl_config_registry_foreach_layout_variant(cfg_reg, layout_name, foreach_variant, &index);

    g_object_unref (engine);
    g_object_unref (cfg_reg);
    XCloseDisplay (dpy);

    return layout_variants;

}

void write_hostname (const gchar *hostname)
{
    GError *error = NULL;

    if (hostname == NULL) {
        g_warning ("write hostname:hostname is NULL\n");
        return ;
    }

    extern const gchar* target;
    if (target == NULL) {
        g_warning ("write hostname:target is NULL\n");
        return ;
    }

    gchar *hostname_file = g_strdup_printf ("%s/etc/hostname", target);

    g_file_set_contents (hostname_file, hostname, -1, &error);
    if (error != NULL) {
        g_warning ("write hostname: set hostname file %s contents failed\n", hostname_file);
        g_error_free (error);
    }
    error = NULL;
    g_free (hostname_file);

    gchar *hosts_file = g_strdup_printf ("%s/etc/hosts", target);
    const gchar *lh = "127.0.0.1  localhost\n";
    const gchar *lha = g_strdup_printf ("127.0.1.1  %s\n", hostname);
    const gchar *ip6_comment = "\n# The following lines are desirable for IPv6 capable hosts\n";
    const gchar *loopback = "::1     ip6-localhost ip6-loopback\n";
    const gchar *localnet = "fe00::0 ip6-localnet\n";
    const gchar *mcastprefix = "ff00::0 ip6-mcastprefix\n";
    const gchar *allnodes = "ff02::1 ip6-allnodes\n";
    const gchar *allrouters = "ff02::2 ip6-allrouters\n";

    gchar *hosts_content = g_strconcat (lh, lha, ip6_comment, loopback, localnet, mcastprefix, allnodes, allrouters, NULL);
    g_file_set_contents (hosts_file, hosts_content, -1, &error);
    if (error != NULL) {
        g_warning ("write hostname: set hosts file %s contents failed\n", hosts_file);
        g_error_free (error);
    }
    error = NULL;
    g_free (hosts_file);
    g_free (hosts_content);
}

void mount_procfs ()
{
    GError *error = NULL;
    gint status = -1;

    extern const gchar* target;
    if (target == NULL) {
        g_warning ("mount procfs:target is NULL\n");
        return ;
    }

    gchar *mount_dev = g_strdup_printf ("mount -v --bind /dev %s/dev", target);
    g_spawn_command_line_sync (mount_dev, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount dev %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    if (status != 0) {
        g_warning ("mount procfs:mount dev failed\n");
    }
    g_free (mount_dev);

    gchar *mount_devpts = g_strdup_printf ("mount -vt devpts devpts %s/dev/pts", target);
    g_spawn_command_line_sync (mount_devpts, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount devpts %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    if (status != 0) {
        g_warning ("mount procfs:mount devpts failed\n");
    }
    g_free (mount_devpts);

    gchar *mount_proc = g_strdup_printf ("mount -vt proc proc %s/proc", target);
    g_spawn_command_line_sync (mount_proc, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount proc %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    if (status != 0) {
        g_warning ("mount procfs:mount proc failed\n");
    }
    g_free (mount_proc);

    gchar *mount_sys = g_strdup_printf ("mount -vt sysfs sysfs %s/sys", target);
    g_spawn_command_line_sync (mount_sys, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount sys %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    if (status != 0) {
        g_warning ("mount procfs:mount sys failed\n");
    }
    g_free (mount_sys);

    //gchar *mount_shm = g_strdup_printf ("mount -vt tmpfs shm %s/dev/shm", target);
}

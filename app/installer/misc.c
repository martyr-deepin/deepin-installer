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
#include "fs_util.h"
#include <pwd.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <libxklavier/xklavier.h>
#include <gio/gunixinputstream.h>

extern struct passwd* getpwent (void);
extern void endpwent (void);
extern int chroot (const char *path);
extern int lchown (const char *path, uid_t owner, gid_t group);

XklConfigRec *config = NULL;
GHashTable *layout_variants_hash = NULL;
static GList *timezone_list = NULL;
static GList *filelist = NULL;

JS_EXPORT_API 
JSObjectRef installer_get_system_users()
{
    JSObjectRef array = json_array_create ();

    struct passwd *user = g_new0 (struct passwd, 1);
    gchar *username = NULL;
    int i = 0;

    while ((user = getpwent ()) != NULL){
        username = g_strdup (user->pw_name);
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), username));
        i++;
        g_free (username);
    }

    endpwent ();
    g_free (user);

    return array;
}

JS_EXPORT_API 
gboolean installer_create_user (const gchar *username, const gchar *hostname, const gchar *password)
{
    gboolean ret = FALSE;

    GError *error = NULL;
    gint status = -1;

    gchar **groups = g_new0 (gchar*, 15);
    groups[0] = g_strdup ("cdrom");
    groups[1] = g_strdup ("floppy");
    groups[2] = g_strdup ("dialout");
    groups[3] = g_strdup ("audio");
    groups[4] = g_strdup ("video");
    groups[5] = g_strdup ("plugdev");
    groups[6] = g_strdup ("sambashare");
    groups[7] = g_strdup ("admin");
    groups[8] = g_strdup ("wheel");
    groups[9] = g_strdup ("netdev");
    groups[10] = g_strdup ("lp");
    groups[11] = g_strdup ("scanner");
    groups[12] = g_strdup ("lpadmin");
    groups[13] = g_strdup ("sudo");

    gchar *useradd_cmd = g_strdup_printf ("useradd -U -m --skel /etc/skel --shell /bin/bash %s", username);
    g_spawn_command_line_sync (useradd_cmd, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("create user:useradd %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    if (status != 0) {
        g_warning ("create user:user add failed\n");
        g_free (useradd_cmd);
        return ret;
    }
    g_free (useradd_cmd);

    //struct passwd *user = g_new0 (struct passwd, 1);
    //uid_t uid;
    //gid_t gid;
    //gchar *home = NULL;
    //while ((user = getpwent ()) != NULL) {
    //    if (g_strcmp0 (username, user->pw_name) == 0) {
    //        home = g_strdup (user->pw_dir);        
    //        uid = user->pw_uid;
    //        gid = user->pw_gid;
    //        break;
    //    }
    //}
    //endpwent ();
    //g_free (user);

    //if (home == NULL) {
    //    g_warning ("create user:get user home failed\n");

    //} else {
    //    if (lchown (home, uid, gid) != 0) {
    //        g_warning ("create user:lchown failed\n");
    //    }
    //    g_free (home);
    //}
    gchar *chown_cmd = g_strdup_printf ("chown -hR %s:%s /home/%s", username, username, username);
    g_spawn_command_line_sync (chown_cmd, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("create user:chown %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    if (status != 0) {
        g_warning ("create user:chown failed\n");
    }
    g_free (chown_cmd);

    //fix me, fix set user passwd
    gchar *passwd_cmd = g_strdup_printf ("echo %s\n %s\n | passwd %s", password, password, username); 
    //g_printf ("create user:passwd cmd %s\n", passwd_cmd);
    g_spawn_command_line_sync (passwd_cmd, NULL, NULL, &status, &error);
    if (error != NULL) {
        g_warning ("create user:passwd %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;
    if (status != 0) {
        g_warning ("create user:set user password failed\n");
        g_free (passwd_cmd);
        return ret;
    }
    g_free (passwd_cmd);

    while (*groups != NULL) {
        gchar *groupadd_cmd = g_strdup_printf ("groupadd -r -f %s", *groups);
        g_printf ("create user:groupadd cmd %s\n", groupadd_cmd);

        g_spawn_command_line_sync (groupadd_cmd, NULL, NULL, &status, &error);
        if (error != NULL) {
            g_warning ("create user:groupadd %s\n", error->message);
            g_error_free (error);
        }
        error = NULL;
        if (status != 0) {
            g_warning ("create user:group add failed for %s\n", *groups);
            g_free (groupadd_cmd);
            groups++;
            continue;
        }
        g_free (groupadd_cmd);

        gchar *gpasswd_cmd = g_strdup_printf ("gpasswd --add %s %s", username, *groups);
        g_printf ("create user:gpasswd cmd %s\n", gpasswd_cmd);

        g_spawn_command_line_sync (gpasswd_cmd, NULL, NULL, &status, &error);
        if (error != NULL) {
            g_warning ("create user:gpasswd %s\n", error->message);
            g_error_free (error);
        }
        error = NULL;
        if (status != 0) {
            g_warning ("create user:gpasswd failed for %s\n", *groups);
            g_free (gpasswd_cmd);
            groups++;
            continue;
        }
        g_free (gpasswd_cmd);

        groups++;
    }

    //fix me, can't free groups
    //g_strfreev (groups);

    if (! write_hostname (hostname)) {
        g_warning ("create user:write hostname failed\n");
        return ret;
    }

    ret = TRUE;

    return ret;
}

gboolean 
write_hostname (const gchar *hostname)
{
    gboolean ret = FALSE;

    GError *error = NULL;

    if (hostname == NULL) {
        g_warning ("write hostname:hostname is NULL\n");
        return ret;
    }

    //extern const gchar* target;
    //if (target == NULL) {
    //    g_warning ("write hostname:target is NULL\n");
    //    return ret;
    //}

    //gchar *hostname_file = g_strdup_printf ("%s/etc/hostname", target);
    gchar *hostname_file = g_strdup ("/etc/hostname");

    g_file_set_contents (hostname_file, hostname, -1, &error);
    if (error != NULL) {
        g_warning ("write hostname: set hostname file %s contents failed\n", hostname_file);
        g_error_free (error);
        g_free (hostname_file);
        return ret;
    }
    error = NULL;
    g_free (hostname_file);

    //gchar *hosts_file = g_strdup_printf ("%s/etc/hosts", target);

    gchar *hosts_file = g_strdup ("/etc/hosts");
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
        g_free (hosts_file);
        g_free (hosts_content);
        return ret;
    }
    error = NULL;
    g_free (hosts_file);
    g_free (hosts_content);

    ret = TRUE;

    return ret;
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
_foreach_variant (XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    const gchar *layout = (const gchar *)data;

    //fix me:free glist memory
    GList *variants = g_list_copy (g_hash_table_lookup (layout_variants_hash, layout));
    variants = g_list_append (variants, g_strdup (item->name));

    g_hash_table_replace (layout_variants_hash, g_strdup (layout), variants);
}

static void 
_foreach_layout(XklConfigRegistry *config, const XklConfigItem *item, gpointer data)
{
    const gchar *layout = item->name;
    GList *variants = NULL;
    g_hash_table_insert (layout_variants_hash, g_strdup (layout), variants);

    xkl_config_registry_foreach_layout_variant(config, g_strdup (layout), _foreach_variant, (gpointer) layout);
}

void
init_keyboard_layouts () 
{
    layout_variants_hash = g_hash_table_new_full ((GHashFunc) g_str_hash, (GEqualFunc) g_str_equal, (GDestroyNotify) g_free, (GDestroyNotify) g_list_free);

    Display *dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning ("init keyboard layouts: XOpenDisplay\n");
        return ;
    }

    XklEngine *engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("init keyboard layouts: xkl engine get instance\n");
        return ;
    }

    config = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config, engine);
    if (config == NULL) {
        g_warning ("init keyboard layouts: xkl config rec\n");
        return ;
    }

    XklConfigRegistry *cfg_reg = NULL;
    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("init keyboard layouts: xkl config registry get instance\n");
        return ;
    }

    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("init keyboard layouts: xkl config registry load\n");
        return ;
    }

    xkl_config_registry_foreach_layout(cfg_reg, _foreach_layout, NULL);

    g_object_unref (engine);
    g_object_unref (cfg_reg);
    XCloseDisplay (dpy);
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

    if (config == NULL) {
        g_warning ("get current layout variant: config is NULL\n");
        init_keyboard_layouts ();
    }

    g_assert (config != NULL);

    gchar **layouts = g_strdupv (config->layouts);
    gchar **variants = g_strdupv (config->variants);

    JSObjectRef layout_array = json_array_create ();
    JSObjectRef variant_array = json_array_create ();

    gsize index = 0;
    for (index = 0; index < sizeof(layouts)/sizeof(gchar*); index++) {
        json_array_insert (layout_array, index, jsvalue_from_cstr (get_global_context (), layouts[index]));
    }
    json_append_value (current, "layouts", (JSValueRef) layout_array);

    for (index = 0; index < sizeof(variants)/sizeof(gchar*); index++) {
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
    if (config == NULL) {
        g_warning ("set keyboard layout variant:xkl config null\n");
        init_keyboard_layouts ();
    }
    g_assert (config != NULL);

    gchar **layouts = g_new0 (char *, 2);
    layouts[0] = g_strdup (layout);

    if (layouts == NULL) {
        g_warning ("set keyboard layout variant:must specify layout\n");
        return ;
    }
    xkl_config_rec_set_layouts (config, (const gchar **)layouts);

    gchar **variants = g_new0 (char *, 2);
    variants[0] = g_strdup (variant);

    if (variants != NULL) {
        xkl_config_rec_set_variants (config, (const gchar **)variants);
    }

    g_strfreev (layouts);
    g_strfreev (variants);
}

void walk_directory (const gchar *root, void *callback (const gchar *))
{
    GFile *source_dir = NULL;
    GFileInfo *info = NULL;
    GError *error = NULL;

    source_dir = g_file_new_for_path (root);
    info = g_file_query_info (source_dir, "standard::", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
    if (error != NULL) {
        g_warning ("walk directory:query info %s\n", error->message);
        g_error_free (error);
        return ;
    }
    error = NULL;

    if (g_file_info_get_file_type (info) == G_FILE_TYPE_REGULAR) {
        //g_warning ("walk directory: add file %s\n", root);
        callback (root);

    } else if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY) {

        GFileEnumerator *enumerator = g_file_enumerate_children (source_dir, "standard::type", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
        if (error != NULL) {
            g_warning ("walk directory:enumerate children %s\n", error->message);
            g_error_free (error);
        }
        error = NULL;

        GFileInfo *child_info = g_file_enumerator_next_file (enumerator, NULL, &error);
        if (error != NULL) {
            g_warning ("walk directory:enumerate next file%s\n", error->message);
            g_error_free (error);
        }
        error = NULL;

        while (child_info != NULL) {
            gchar *path = g_strdup_printf ("%s/%s", root, g_file_info_get_name (child_info));

            callback ((const gchar *) path);
            walk_directory (path, callback);

            g_free (path);
            child_info = g_file_enumerator_next_file (enumerator, NULL, &error);
            if (error != NULL) {
                g_warning ("walk directory:enumerate next in while %s\n", error->message);
                g_error_free (error);
            }
            error = NULL;
        }
        //g_object_unref (child_info);
        g_object_unref (enumerator);

    } else {
        g_debug ("walk directory:current not support\n");
        return ;
    }

    g_object_unref (info);
    g_object_unref (source_dir);
}

void *
walk_timezones (const gchar *path)
{
    GFile *zone_dir = g_file_new_for_path ("/usr/share/zoneinfo");
    GFile *zone = g_file_new_for_path (path);

    gchar *relative = g_file_get_relative_path (zone_dir, zone);
    if (relative == NULL) {
        g_warning ("walk timezones: relative is NULL\n");
    } else if (g_str_has_prefix (relative, "posix") || g_str_has_prefix (relative, "right")) {
        g_debug ("walk timezones: ignore files under posix and right\n");
    } else {
        //g_warning ("walk timezones: %s\n", relative);
        timezone_list = g_list_append (timezone_list, g_strdup (relative));
    }

    g_free (relative);
    g_object_unref (zone);
    g_object_unref (zone_dir);
}

JS_EXPORT_API 
JSObjectRef installer_get_timezone_list ()
{
    JSObjectRef timezones = json_array_create ();

    walk_directory ("/usr/share/zoneinfo", walk_timezones);

    gsize index = 0;
    for (index = 0; index < g_list_length (timezone_list); index++) {
        json_array_insert (timezones, index, jsvalue_from_cstr (get_global_context (), (gchar *)g_list_nth_data (timezone_list, index)));
    }

    return timezones;
}

JS_EXPORT_API 
gboolean installer_set_timezone (const gchar *timezone)
{
    gboolean ret = FALSE;

    GError *error = NULL;
    //extern const gchar *target;
    //if (target == NULL) {
    //    g_warning ("set timezone:target is NULL\n");
    //    return ret;
    //}

    //gchar *timezone_file = g_strdup_printf ("%s/etc/timezone", target);

    gchar *timezone_file = g_strdup ("/etc/timezone");
    g_file_set_contents (timezone_file, timezone, -1, &error);
    if (error != NULL) {
        g_warning ("set timezone:write timezone %s\n", error->message);
        g_error_free (error);
        g_free (timezone_file);
        return ret;
    }
    error = NULL;
    g_free (timezone_file);

    //gchar *zoneinfo_file = g_strdup_printf ("%s/etc/timezone/%s", target, timezone);

    gchar *zoneinfo_file = g_strdup_printf ("/etc/timezone/%s", timezone);
    gchar *zone_content = NULL;
    gsize length;

    g_file_get_contents (zoneinfo_file, &zone_content, &length, &error);
    if (error != NULL) {
        g_warning ("set timezone:read /etc/timezone %s\n", error->message);
        g_error_free (error);
        g_free (zoneinfo_file);
        return ret;
    }
    error = NULL;
    g_free (zoneinfo_file);
    
    if (zone_content == NULL) {
        g_warning ("set timezone:read /etc/timezone/%s failed\n", timezone);
        return ret;
    }

    //gchar *localtime_file = g_strdup_printf ("%s/etc/localtime", target);

    gchar *localtime_file = g_strdup ("/etc/localtime");
    g_file_set_contents (localtime_file, zone_content, -1, &error);
    if (error != NULL) {
        g_warning ("set timezone:write localtime %s\n", error->message);
        g_error_free (error);
        g_free (zone_content);
        g_free (localtime_file);
        return ret;
    }
    error = NULL;

    g_free (zone_content);
    g_free (localtime_file);

    //fix me, set /etc/default/rcS content"
    ret = TRUE;

    return ret;
}

void *
walk_copy (const gchar *path)
{
    GFile *file = g_file_new_for_path (path);
    filelist = g_list_append (filelist, g_file_dup (file));
    g_object_unref (file);
}

//fix me, insert the copy file blacklist 
static GList*
get_source_file_list (const gchar *source_root)
{
    walk_directory (source_root, walk_copy);

    return filelist;
}

static GFile* 
get_coordinate_target (const gchar *source_root, GFile *src)
{
    GFile *coo_target = NULL;

    GFile *source_dir = NULL;
    GFile *target_dir = NULL;
    GError *error = NULL;

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("get coordinate target:target is NULL\n");
        return coo_target;
    }

    source_dir = g_file_new_for_path (source_root); 
    if (source_dir == NULL) {
        g_warning ("get coordinate target:get source root file %s failed\n", source_root);
        return coo_target;
    }

    gchar *relative_path = g_file_get_relative_path (source_dir, src);
    if (relative_path == NULL) {
        g_warning ("get coordinate target:get relative path failed\n");
        return coo_target;
    }

    target_dir = g_file_new_for_path (target);
    if (target_dir == NULL) {
        g_warning ("get coordinate target:get target file %s failed\n", target);
        return coo_target;
    }
    coo_target = g_file_resolve_relative_path (target_dir, relative_path);

    g_free (relative_path);
    g_object_unref (target_dir);
    g_object_unref (source_dir);

    return coo_target;
}

static gint
get_total_size ()
{
    g_printf ("get total size\n");
}

void
progress_callback (goffset current_num_bytes, goffset total_num_bytes, gpointer user_data)
{
    g_printf ("progress callback\n");
}

void
finish_callback (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_printf ("finish callback\n");
}

//discard as can't read 3.6G file into memory
JS_EXPORT_API 
void installer_copy_file (const gchar *source_root)
{
    GError *error = NULL;

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("get coordinate target:target is NULL\n");
        return ;
    }

    filelist = get_source_file_list (source_root);
    if (filelist == NULL) {
        g_warning ("copy file:get source file list failed\n");
    }

    for (int index = 0; index < g_list_length (filelist); index++) {
        GFile *src = g_file_dup (g_list_nth_data (filelist, index));
        GFile *dest = get_coordinate_target (source_root, src); 
        if (dest == NULL) {
            g_warning ("copy file:get coordinate target failed\n");
        }

        GFileInfo *info = g_file_query_info (src, "standard::type", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
        if (error != NULL) {
            g_warning ("copy file:query src info failed\n");
            g_error_free (error);
        }
        error = NULL;

        GFileType type = g_file_info_get_file_type (info);
        if (type == G_FILE_TYPE_DIRECTORY) {

            if (g_file_query_exists (dest, NULL)) {
                continue;
            } else {
                GFile *parent = g_file_get_parent (dest);
                if (parent == NULL) {
                   g_file_make_directory_with_parents (dest, NULL, &error);
                   if (error != NULL) {
                        g_warning ("copy file:make directory with parents failed %s\n", error->message);
                        g_error_free (error);
                   }
                } else {
                   g_file_make_directory (dest, NULL, &error);
                   if (error != NULL) {
                        g_warning ("copy file:make directory failed %s\n", error->message);
                        g_error_free (error);
                   }
                }
                error = NULL;
                g_object_unref (parent);
            }
        } else {
            //fix me :only copy regular and symlink file??? file permissions???
            g_file_copy_async (src, 
                               dest, 
                               G_FILE_COPY_OVERWRITE | G_FILE_COPY_TARGET_DEFAULT_PERMS, 
                               0, 
                               NULL,
                               (GFileProgressCallback) progress_callback,
                               g_file_dup (dest),
                               (GAsyncReadyCallback) finish_callback,
                               g_file_dup (dest)
                        );
        }

        g_object_unref (info);
        g_object_unref (src);
        g_object_unref (dest);
    }
}

static void
watch_extract_child (GPid pid, gint status, gpointer data)
{
    if (data == NULL) {
        g_warning ("watch extract child:arg data is NULL\n");
    }

    gint* timeout_id = (gint *) (data);
    if (*timeout_id > 0) {
        g_source_remove (*timeout_id);
        g_free (timeout_id);
    } else {
        g_warning ("watch extract child:timeout id less than 0\n");
    }

    g_spawn_close_pid (pid);
}

static gboolean 
cb_out_watch (GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar **progress = (gchar **) data;

    gchar *string;
    gsize  size;

    if (cond == G_IO_HUP) {
        g_io_channel_unref (channel);
        return FALSE;
    }

    g_io_channel_read_line (channel, &string, &size, NULL, NULL);

    gchar *match = get_matched_string (string, "\\d{1,3}%");
    //g_printf ("cb out watch:%s\n", string);
    //g_printf ("cb out watch:match->              %s\n", match);
    if (match == NULL) {
        g_debug ("cb out watch:line without extract progress\n");
    }
    *progress = g_strdup (match);

    g_free (match);
    g_free (string);

    return TRUE;
}

static gboolean
cb_err_watch (GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if (cond == G_IO_HUP) {
        g_io_channel_unref (channel);
        return FALSE;
    }

    g_io_channel_read_line (channel, &string, &size, NULL, NULL);
    //fix me, parse error here
    g_printf ("cb err watch:%s\n", string); 

    g_free (string);

    return TRUE;
}

static gboolean
cb_timeout (gpointer data)
{
    gchar **progress = (gchar **)data;
    static gchar *report_progress = "0%";

    if (*progress != NULL) {
        if (g_strcmp0 (*progress, report_progress) == 0) {
            g_debug ("cb timeout:progress not changed\n");
            return TRUE;
        } else {
            report_progress = *progress;
            g_printf ("cb timeout: emit extract progress:%s\n", *progress);
            //emit_progress ("extract", *progress);
            if (g_strcmp0 ("100%", *progress) == 0) {
                g_printf ("cb timeout:extract finish\n");
                g_strfreev (progress);
                return FALSE;
            }
        }
    } else {
        g_warning ("cb timeout:progress null\n");
    }

    return TRUE;
}

//gpointer 
//extract_squashfs (gpointer data)
JS_EXPORT_API
void installer_extract_squashfs ()
{
    if (g_find_program_in_path ("unsquashfs") == NULL) {
        g_warning ("extract squashfs: unsquashfs not installed\n");
        return ;
    }

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("extract squash fs:target is NULL\n");
        return ;
    }

    gchar **argv = g_new0 (gchar *, 6);
    argv[0] = g_strdup ("unsquashfs");
    argv[1] = g_strdup ("-f");
    argv[2] = g_strdup ("-d");
    argv[3] = g_strdup (target);
    argv[4] = g_strdup ("/cdrom/casper/filesystem.squashfs");

    gint std_output;
    gint std_error;
    GError *error = NULL;
    GPid pid;
    GIOChannel *out_channel = NULL;
    GIOChannel *err_channel = NULL;

    gint* timeout_id = g_new0 (gint, 1);

    gchar** progress = g_new0 (gchar*, 1);

    g_spawn_async_with_pipes (NULL,
                              argv,
                              NULL,
                              G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
                              NULL,
                              NULL,
                              &pid,
                              NULL,
                              &std_output,
                              &std_error,
                              &error);
    if (error != NULL) {
        g_warning ("extract squashfs:spawn async pipes %s\n", error->message);
        g_error_free (error);
    }
    error = NULL;

    out_channel = g_io_channel_unix_new (std_output);
    err_channel = g_io_channel_unix_new (std_error);

    g_io_add_watch_full (out_channel, G_PRIORITY_LOW, G_IO_IN | G_IO_HUP, (GIOFunc) cb_out_watch, progress, (GDestroyNotify) g_io_channel_unref);
    g_io_add_watch_full (err_channel, G_PRIORITY_LOW, G_IO_IN | G_IO_HUP, (GIOFunc) cb_err_watch, progress, (GDestroyNotify) g_io_channel_unref);
    //g_io_add_watch (out_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_out_watch, progress);
    //g_io_add_watch (err_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_err_watch, progress);

    *timeout_id = g_timeout_add (100, (GSourceFunc) cb_timeout, progress);
    //g_printf ("extract squashfs:timout id %d\n", *timeout_id);
    g_child_watch_add (pid, (GChildWatchFunc) watch_extract_child, timeout_id);

    g_strfreev (argv);
}

//JS_EXPORT_API 
//void installer_extract_squashfs ()
//{
//    GThread *thread = g_thread_new ("extract", (GThreadFunc) extract_squashfs, NULL);
//
//    g_thread_unref (thread);
//}

JS_EXPORT_API
gboolean installer_mount_procfs ()
{
    gboolean ret = FALSE;

    GError *error = NULL;
    gint status = -1;

    extern const gchar* target;
    if (target == NULL) {
        g_warning ("mount procfs:target is NULL\n");
        return ret;
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
        g_free (mount_dev);
        return ret;
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
        g_free (mount_devpts);
        return ret;
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
        g_free (mount_proc);
        return ret;
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
        g_free (mount_sys);
        return ret;
    }
    g_free (mount_sys);

    ret = TRUE;

    return ret;
    //gchar *mount_shm = g_strdup_printf ("mount -vt tmpfs shm %s/dev/shm", target);
}

JS_EXPORT_API
gboolean installer_chroot_target ()
{
    gboolean ret = FALSE;

    extern const gchar* target;
    if (target == NULL) {
        g_warning ("chroot:target is NULL\n");
        return ret;
    }

    if (chroot (target) == 0) {
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to %s falied\n", target);
    }

    return ret;
}


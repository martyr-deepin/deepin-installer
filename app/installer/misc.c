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
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/sysinfo.h>
#include <ftw.h>

#define WHITE_LIST_PATH RESOURCE_DIR"/installer/whitelist.ini"

extern struct passwd* getpwent (void);
extern void endpwent (void);
extern int chroot (const char *path);
extern int lchown (const char *path, uid_t owner, gid_t group);

XklConfigRec *config = NULL;
GHashTable *layout_variants_hash = NULL;
static GList *timezone_list = NULL;
static GList *filelist = NULL;

#define BUFSIZE 64

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

static void
free_passwd_handler (struct PasswdHandler *handler)
{
    GError *error = NULL;

    g_free (handler->username);
    g_free (handler->password);
    g_free (handler->hostname);
    
    if (handler->child_watch_id != 0) {
        g_source_remove (handler->child_watch_id);
        handler->child_watch_id = 0;
    }

    if (handler->child_watch_id != 0) {
        g_source_remove (handler->child_watch_id);
        handler->child_watch_id = 0;
    }

    if (handler->in_channel != NULL) {
        if (g_io_channel_shutdown (handler->in_channel, TRUE, &error) != G_IO_STATUS_NORMAL) {
            g_warning ("create user:shutdown in channel: %s", error->message);
            g_error_free (error);
        }
        error = NULL;
        g_io_channel_unref (handler->in_channel);
        handler->in_channel = NULL;
    }

    if (handler->out_channel != NULL) {
        if (g_io_channel_shutdown (handler->out_channel, TRUE, &error) != G_IO_STATUS_NORMAL) {
            g_warning ("create user:shutdown out channel: %s", error->message);
            g_error_free (error);
        }
        error = NULL;
        g_io_channel_unref (handler->out_channel);
        handler->out_channel = NULL;
    }

    if (handler->stdout_watch_id != 0) {
        g_source_remove (handler->stdout_watch_id);
        handler->stdout_watch_id = 0;
    }

    if (handler->pid != -1) {
        g_spawn_close_pid (handler->pid);
        handler->pid = -1;
    }
    g_free (handler);
}

static gpointer
thread_create_user (gpointer data)
{
    struct PasswdHandler *handler = (struct PasswdHandler *) data;
    
    if (!add_user (handler->username)) {
        g_warning ("create user:add user failed\n");
    }

    if (!set_user_home (handler->username)) {
        g_warning ("create user:set user home failed\n");
    }

    if (!set_group (handler->username)) {
        g_warning ("create user:set group failed\n");
    }
    
    if (!write_hostname (handler->hostname)) {
        g_warning ("create user:write hostname failed\n");
    }
    if (!set_user_password (handler)) {
        g_warning ("create user:set user password failed\n");
    }
}

JS_EXPORT_API 
void installer_create_user (const gchar *username, const gchar *hostname, const gchar *password)
{
    struct PasswdHandler *handler = g_new0 (struct PasswdHandler, 1);
    handler->username = g_strdup (username);
    handler->password = g_strdup (password);
    handler->hostname = g_strdup (hostname);
    handler->pid = -1;
    handler->in_channel = NULL;
    handler->out_channel = NULL;
    handler->child_watch_id = 0;
    handler->stdout_watch_id = 0;

    GThread *user_thread = g_thread_new ("user", (GThreadFunc) thread_create_user, handler);
    g_thread_unref (user_thread);
}

gboolean 
add_user (const gchar *username)
{
    GError *error = NULL;
    gchar *useradd_cmd = g_strdup_printf ("useradd -U -m --skel /etc/skel --shell /bin/bash %s", username);

    g_spawn_command_line_sync (useradd_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("create user:useradd %s\n", error->message);
        g_error_free (error);
        error = NULL;
        g_free (useradd_cmd);
        return FALSE;
    }
    g_free (useradd_cmd);

    return TRUE;
}

gboolean 
set_user_home (const gchar *username)
{
    GError *error = NULL;
    gchar *chown_cmd = g_strdup_printf ("chown -hR %s:%s /home/%s", username, username, username);

    g_spawn_command_line_sync (chown_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("create user:chown %s\n", error->message);
        g_error_free (error);
        error = NULL;
        g_free (chown_cmd);
        return FALSE;
    }
    g_free (chown_cmd);

    return TRUE;
}

static void
watch_passwd_child (GPid pid, gint status, struct PasswdHandler *handler)
{
    g_printf ("watch password child:set password finish\n");
    free_passwd_handler (handler);
    if (status == -1) {
        emit_progress ("user", "terminate");
    } else {
        emit_progress ("user", "finish");
    }
}

static gboolean
passwd_out_watch (GIOChannel *channel, GIOCondition cond, struct PasswdHandler *handler)
{
    static int write_count = 0;
    if (write_count > 1) {
        return False;
    }

    gchar buf[BUFSIZE];
    memset (buf, 0, BUFSIZE);
    GError *error = NULL;        

    if (g_io_channel_read_chars (channel, buf, BUFSIZE, NULL, &error) != G_IO_STATUS_NORMAL) {
        g_warning ("passwd out watch:read error %s", error->message);
        g_error_free (error);
        return TRUE;
    }
    error = NULL;
    g_debug ("passwd out watch: read %s\n", buf);

    gchar *passwd = g_strdup_printf ("%s\n", handler->password);
    g_debug ("passwd out watch:write password %s\n", handler->password);
    if (passwd != NULL) {
        if (g_io_channel_write_chars (handler->in_channel, passwd, -1, NULL, &error) != G_IO_STATUS_NORMAL) {
            g_warning ("passwd out watch:write %s to channel: %s", passwd, error->message);
            g_error_free (error);
        }
        error = NULL;
        g_free (passwd);
        write_count = write_count + 1;
    }

    return TRUE;
}

static void
ignore_sigpipe (gpointer data)
{
    signal (SIGPIPE, SIG_IGN);    
}

gboolean 
set_user_password (struct PasswdHandler *handler)
{
    g_printf ("set user password");
    gboolean ret = FALSE;

    gchar **argv = g_new0 (gchar *, 3);
    argv[0] = g_strdup ("passwd");
    argv[1] = g_strdup (handler->username);

    gint std_in, std_out, std_err;
    GError *error = NULL;

    g_spawn_async_with_pipes (NULL,
                              argv,
                              NULL,
                              G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
                              ignore_sigpipe,
                              NULL,
                              &handler->pid,
                              &std_in,
                              &std_out,
                              &std_err,
                              &error);
    g_strfreev (argv);
    if (error != NULL) {
        g_warning ("set user password:spawn async pipes %s\n", error->message);
        g_error_free (error);
        free_passwd_handler (handler);
        return ret;
    }
    error = NULL;

    g_debug ("dup stderr to stdout");
    if ((dup2 (std_err, std_out)) == -1) {
        g_warning ("set user password:dup %s\n", strerror (errno));
        if (handler->pid != -1) {
            extern int kill (pid_t pid, int sig);
            kill (handler->pid, 9);
        }
        free_passwd_handler (handler);
        return ret;
    }

    handler->in_channel = g_io_channel_unix_new (std_in);
    handler->out_channel = g_io_channel_unix_new (std_out);

    if (g_io_channel_set_encoding (handler->in_channel, NULL, &error) != G_IO_STATUS_NORMAL ||
        g_io_channel_set_encoding (handler->out_channel, NULL, &error) != G_IO_STATUS_NORMAL ||
        g_io_channel_set_flags (handler->in_channel, G_IO_FLAG_NONBLOCK, &error) != G_IO_STATUS_NORMAL ||
        g_io_channel_set_flags (handler->out_channel, G_IO_FLAG_NONBLOCK, &error) != G_IO_STATUS_NORMAL ) {

        g_warning ("set user password: set io channel encoding or flags failed\n");
        if (handler->pid != -1) {
            extern int kill (pid_t pid, int sig);
            kill (handler->pid, 9);
        }
        free_passwd_handler (handler);
    }

    g_io_channel_set_buffered (handler->in_channel, FALSE);
    g_io_channel_set_buffered (handler->out_channel, FALSE);
    //g_warning ("watch io channel for set password");

    handler->stdout_watch_id = g_io_add_watch (handler->out_channel, G_IO_IN | G_IO_HUP, (GIOFunc) passwd_out_watch, handler);
    handler->child_watch_id = g_child_watch_add (handler->pid, (GChildWatchFunc) watch_passwd_child, handler);

    ret = TRUE;

    return ret;
}

gboolean 
set_group (const gchar *username)
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
    int i ;
    
    for (i = 0; i < 14; i++) {
        gchar *groupadd_cmd = g_strdup_printf ("groupadd -r -f %s", groups[i]);
        g_debug ("create user:groupadd cmd %s\n", groupadd_cmd);

        g_spawn_command_line_sync (groupadd_cmd, NULL, NULL, &status, &error);
        if (error != NULL) {
            g_warning ("create user:groupadd %s\n", error->message);
            g_error_free (error);
        }
        error = NULL;
        if (status != 0) {
            g_warning ("create user:group add failed for %s\n", groups[i]);
            g_free (groupadd_cmd);
            continue;
        }
        g_free (groupadd_cmd);

        gchar *gpasswd_cmd = g_strdup_printf ("gpasswd --add %s %s", username, groups[i]);
        g_debug ("create user:gpasswd cmd %s\n", gpasswd_cmd);

        g_spawn_command_line_sync (gpasswd_cmd, NULL, NULL, &status, &error);
        if (error != NULL) {
            g_warning ("create user:gpasswd %s\n", error->message);
            g_error_free (error);
        }
        error = NULL;
        if (status != 0) {
            g_warning ("create user:gpasswd failed for %s\n", groups[i]);
            g_free (gpasswd_cmd);
            continue;
        }
        g_free (gpasswd_cmd);
    }
    g_strfreev (groups);

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
        g_free ((gchar *)lha);
        g_free (hosts_file);
        g_free (hosts_content);
        return ret;
    }
    error = NULL;
    g_free ((gchar *)lha);
    g_free (hosts_file);
    g_free (hosts_content);

    ret = TRUE;

    return ret;
}

void installer_reboot ()
{
    GError *error = NULL;
    GDBusProxy *ck_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
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

    xkl_config_registry_foreach_layout_variant(config, 
                                               g_strdup (layout), 
                                               _foreach_variant, 
                                               (gpointer) layout);
}

void
init_keyboard_layouts () 
{
    g_printf ("init keyboard layouts");
    layout_variants_hash = g_hash_table_new_full ((GHashFunc) g_str_hash, 
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

    config = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config, engine);
    if (config == NULL) {
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
out:
    if (engine != NULL) {
        g_object_unref (engine);
    }
    if (cfg_reg != NULL) {
        g_object_unref (cfg_reg);
    }
    if (dpy != NULL) {
        XCloseDisplay (dpy);
    }
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
        g_warning ("get current layout variant: config is NULL, init it\n");
        init_keyboard_layouts ();
    }
    if (config == NULL) {
        g_warning ("get current layout variant:xkl config null after init\n");
        return current;
    }

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
        g_warning ("set keyboard layout variant:xkl config null, init it\n");
        init_keyboard_layouts ();
    }
    if (config == NULL) {
        g_warning ("set keyboard layout variant:xkl config null after init\n");
        goto out;
    }

    gchar **layouts = g_new0 (char *, 2);
    layouts[0] = g_strdup (layout);

    if (layouts == NULL) {
        g_warning ("set keyboard layout variant:must specify layout\n");
        goto out;
    }
    xkl_config_rec_set_layouts (config, (const gchar **)layouts);

    gchar **variants = g_new0 (char *, 2);
    variants[0] = g_strdup (variant);

    if (variants != NULL) {
        xkl_config_rec_set_variants (config, (const gchar **)variants);
    }

    g_strfreev (layouts);
    g_strfreev (variants);
    emit_progress ("keyboard", "finish");
out:
    g_warning ("set keyboard layout variant failed, just skip this step");
    emit_progress ("keyboard", "finish");
}

JS_EXPORT_API
JSObjectRef installer_get_timezone_list ()
{
    JSObjectRef timezones = json_array_create ();
    gsize index = 0;
    GError *error = NULL;

    GFile *file = g_file_new_for_path ("/usr/share/zoneinfo/zone.tab");
    if (!g_file_query_exists (file, NULL)) {
        g_warning ("get timezone list:zone.tab not exists\n");
        return timezones;
    }

    GFileInputStream *input = g_file_read (file, NULL, &error);
    if (error != NULL){
        g_warning ("get timezone list:read zone.tab error->%s", error->message);
        g_error_free (error);
        g_object_unref (file);
        return timezones;
    }
    error = NULL;

    GDataInputStream *data_input = g_data_input_stream_new ((GInputStream *) input);
    if (data_input == NULL) {
        g_warning ("get timezone list:get data input stream failed\n");
        g_object_unref (input);
        return timezones;
    }
    
    char *data = (char *) 1;
    while (data) {
        gsize length = 0;
        data = g_data_input_stream_read_line (data_input, &length, NULL, &error);
        if (error != NULL) {
            g_warning ("get timezone list:read line error");
            g_error_free (error);
            continue;
        }
        error = NULL;
        if (data != NULL) {
            if (g_str_has_prefix (data, "#")){
                g_debug ("get timezone list:comment line, just pass");
                continue;
            } else {
                gchar **line = g_strsplit (data, "\t", -1);
                if (line == NULL) {
                    g_warning ("get timezone list:split %s failed\n", data);
                } else {
                    json_array_insert (timezones, index, jsvalue_from_cstr (get_global_context (), line[2]));
                    index++;
                }
                g_strfreev (line);
            }
        } else {
            break;
        }
    }

    g_object_unref (data_input);
    g_object_unref (input);
    g_object_unref (file);

    return timezones;
}

static gpointer
thread_set_timezone (gpointer data)
{
    gchar *timezone = (gchar *) data;
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar *timezone_file = NULL;
    gchar *zoneinfo_path = NULL;
    gchar *localtime_path = NULL;
    GFile *zoneinfo_file = NULL;
    GFile *localtime_file = NULL;

    if (timezone == NULL) {
        g_warning ("set timezone:timezone NULL\n");
        goto out;
    }
    timezone_file = g_strdup ("/etc/timezone");
    g_file_set_contents (timezone_file, timezone, -1, &error);
    if (error != NULL) {
        g_warning ("set timezone:write timezone %s\n", error->message);
        goto out;
    }
    zoneinfo_path = g_strdup_printf ("/usr/share/zoneinfo/%s", timezone);
    localtime_path = g_strdup ("/etc/localtime");
    zoneinfo_file = g_file_new_for_path (zoneinfo_path);
    localtime_file = g_file_new_for_path (localtime_path);
    g_file_copy (zoneinfo_file, localtime_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("set timezone:cp %s to /etc/localtime %s\n", zoneinfo_path, error->message);
        goto out;
    }
    ret = TRUE;
    goto out;

out:
    g_free (timezone);
    g_free (timezone_file);
    g_free (zoneinfo_path);
    g_free (localtime_path);
    if (error != NULL) {
        g_error_free (error);
    }
    if (zoneinfo_file != NULL) {
        g_object_unref (zoneinfo_file);
    }
    if (localtime_file != NULL) {
        g_object_unref (localtime_file);
    }
    if (ret) {
        emit_progress ("timezone", "finish");
    } else {
        g_warning ("set timezone failed, just skip this step");
        emit_progress ("timezone", "finish");
    }
    return NULL;
}

JS_EXPORT_API 
void installer_set_timezone (const gchar *timezone)
{
    GThread *thread = g_thread_new ("timezone", (GThreadFunc) thread_set_timezone, g_strdup (timezone));
    g_thread_unref (thread);
}

static int 
copy_file_cb (const char *path, const struct stat *sb, int typeflag)
{
    extern const gchar *target;
    if (path == NULL) {
        return 1;
    }
    static GFile *target_f; 
    if (target_f == NULL) {
        target_f = g_file_new_for_path (target);
    }
    static GFile *target_squash_f;
    if (target_squash_f == NULL) {
        gchar *squash = g_strdup_printf ("%s/squashfs", target);
        target_squash_f = g_file_new_for_path (squash);
        g_free (squash);
    }

    GFile *src = g_file_new_for_path (path);
    gchar *origin = g_file_get_relative_path (target_squash_f, src);
    if (origin == NULL) {
        g_warning ("copy file cb:origin NULL for %s\n", path);
    }
    GFile *dest = g_file_resolve_relative_path (target_f, origin);
    g_free (origin);

    g_warning ("copy file from %s to %s\n", g_file_get_path (src), g_file_get_path (dest));

    g_file_copy (src, dest, G_FILE_COPY_OVERWRITE | G_FILE_COPY_NOFOLLOW_SYMLINKS,  NULL, NULL, NULL, NULL);

    g_object_unref (src);
    g_object_unref (dest);

    return 0;
}

JS_EXPORT_API
void installer_extract_iso ()
{
    gboolean succeed = FALSE;
    extern const gchar *target;
    GError *error = NULL;
    gchar *target_iso;
    gchar *mount_cmd;
    gchar *umount_cmd;

    if (target == NULL) {
        g_warning ("extract iso:target NULL\n");
        goto out;
    }

    target_iso = g_strdup_printf ("%s/squashfs", target);
    if (g_mkdir_with_parents (target_iso, 0755) == -1) {
        g_warning ("extract iso:mkdir failed\n");
        goto out;
    }

    mount_cmd = g_strdup_printf ("mount /cdrom/casper/filesystem.squashfs %s", target_iso);
    g_spawn_command_line_sync (mount_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("extract iso:mount squashfs->%s\n", error->message);
        goto out;
    }

    ftw (target_iso, copy_file_cb, 65536);

    umount_cmd  = g_strdup_printf ("umount %s", target_iso);
    g_spawn_command_line_sync (umount_cmd, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("extract iso:umount squashfs->%s\n", error->message);
        goto out;
    }
    succeed = TRUE;
    goto out;
out:
    g_free (mount_cmd);
    g_free (umount_cmd);
    g_free (target_iso);
    if (error != NULL) {
        g_error_free (error);
    }
    if (succeed) {
        emit_progress ("extract", "finish");
    } else {
        emit_progress ("extract", "terminate");
    }
}

static guint
get_cpu_num ()
{
    guint num = 0;
    gchar *output = NULL;
    const gchar *cmd = "sh -c \"cat /proc/cpuinfo |grep processor |wc -l\"";
    GError *error = NULL;

    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get cpu num:%s\n", error->message);
        g_error_free (error);
    }
    if (output == NULL) {
        return num;
    }
    num = g_strtod (g_strstrip (output), NULL);

    g_free (output);
    return num;
}

static void
watch_extract_child (GPid pid, gint status, gpointer data)
{
    GError *error = NULL;
    if (data == NULL) {
        g_warning ("watch extract child:arg data is NULL\n");
    }
    g_spawn_check_exit_status (status, &error);
    if (error != NULL) {
        g_warning ("watch extract child:error->%s\n", error->message);
        g_error_free (error);
        emit_progress ("extract", "terminate");
    } else {
        g_printf ("watch extract child:extract finish\n");
        emit_progress ("extract", "finish");
    }

    guint* cb_ids = (guint *) data;
    if (cb_ids[0] > 0) {
        g_source_remove (cb_ids[0]);
        cb_ids[0] = 0;
    }
    if (cb_ids[1] > 0) {
        g_source_remove (cb_ids[1]);
        cb_ids[1] = 0;
    }
    if (cb_ids[2] > 0) {
        g_source_remove (cb_ids[2]);
        cb_ids[2] = 0;
    }
    g_free (cb_ids);

    g_spawn_close_pid (pid);
}

static gboolean 
cb_out_watch (GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar **progress = (gchar **) data;

    gchar *string;
    gsize  size;

    if (cond == G_IO_HUP) {
        //g_printf ("cb out watch: io hup\n");
        g_io_channel_unref (channel);
        return FALSE;
    }

    g_io_channel_read_line (channel, &string, &size, NULL, NULL);
    //g_printf ("cb out watch:%s***cb out watch finish\n", string);
    gchar *match = get_matched_string (string, "\\d{1,3}%");
    //g_printf ("cb out watch:match->              %s\n", match);
    if (match == NULL) {
        g_debug ("cb out watch:line without extract progress\n");
    } else {
        if (*progress != NULL) {
            g_free (*progress);
            *progress = NULL;
        }
        *progress = g_strdup (match);
    }

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

//will stop automaticly when extract finish
static gboolean
cb_timeout (gpointer data)
{
    gchar **progress = (gchar **)data;

    if (progress != NULL) {
        if (*progress != NULL) {
            g_printf ("cb timeout: emit extract progress:%s\n", *progress);
            emit_progress ("extract", *progress);
        } else {
            g_warning ("cb timeout:*progress null\n");
        }
    } else {
        g_warning ("cb timeout:progress null\n");
    }

    return TRUE;
}


JS_EXPORT_API
void installer_extract_squashfs ()
{
    g_warning ("extract squashfs");
    if (g_find_program_in_path ("unsquashfs") == NULL) {
        g_warning ("extract squashfs: unsquashfs not installed\n");
        emit_progress ("extract", "terminate");
        return;
    }

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("extract squash fs:target is NULL\n");
        emit_progress ("extract", "terminate");
        return;
    }
    const gchar *iso = "/cdrom/casper/filesystem.squashfs";
    if (!g_file_test (iso, G_FILE_TEST_EXISTS)) {
        g_warning ("extract squashfs:iso not exists\n");
        emit_progress ("extract", "terminate");
        return;
    }

    guint processors = get_cpu_num ();
    guint puse = 1;
    if (processors > 2) {
        puse = processors / 2;
    }

    gchar **argv = g_new0 (gchar *, 12);
    argv[0] = g_strdup ("unsquashfs");
    argv[1] = g_strdup ("-f");
    argv[2] = g_strdup ("-p");
    argv[3] = g_strdup_printf ("%d", puse);
    argv[4] = g_strdup ("-da");
    argv[5] = g_strdup_printf ("%d", 32);
    argv[6] = g_strdup ("-fr");
    argv[7] = g_strdup_printf ("%d", 32);
    argv[8] = g_strdup ("-d");
    argv[9] = g_strdup (target);
    argv[10] = g_strdup ("/cdrom/casper/filesystem.squashfs");

    gint std_output;
    gint std_error;
    GError *error = NULL;
    GPid pid;
    GIOChannel *out_channel = NULL;
    GIOChannel *err_channel = NULL;

    guint* cb_ids = g_new0 (guint, 3);
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

    cb_ids[0] = g_io_add_watch_full (out_channel,
                                     G_PRIORITY_LOW, 
                                     G_IO_IN | G_IO_HUP, 
                                     (GIOFunc) cb_out_watch, 
                                     progress, 
                                     (GDestroyNotify) g_io_channel_unref);

    cb_ids[1] = g_io_add_watch_full (err_channel, 
                                     G_PRIORITY_LOW, 
                                     G_IO_IN | G_IO_HUP, 
                                     (GIOFunc) cb_err_watch, 
                                     progress, 
                                     (GDestroyNotify) g_io_channel_unref);
    //g_io_add_watch (out_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_out_watch, progress);
    //g_io_add_watch (err_channel, G_IO_IN | G_IO_HUP, (GIOFunc) cb_err_watch, progress);

    cb_ids[2] = g_timeout_add (2000, (GSourceFunc) cb_timeout, progress);
    g_child_watch_add (pid, (GChildWatchFunc) watch_extract_child, cb_ids);
    g_strfreev (argv);
}

JS_EXPORT_API
gboolean installer_mount_procfs ()
{
    gboolean ret = FALSE;
    GError *error = NULL;
    extern const gchar* target;
    gchar *dev_target = NULL;
    gchar *devpts_target = NULL;
    gchar *proc_target = NULL;
    gchar *sys_target = NULL;
    gchar *mount_dev = NULL;
    gchar *mount_devpts = NULL;
    gchar *mount_proc = NULL;
    gchar *mount_sys = NULL;

    if (target == NULL) {
        g_warning ("mount procfs:target is NULL\n");
        goto out;
    }
    dev_target = g_strdup_printf ("%s/dev", target);
    devpts_target = g_strdup_printf ("%s/dev/pts", target);
    proc_target = g_strdup_printf ("%s/proc", target);
    sys_target = g_strdup_printf ("%s/sys", target);

    mount_dev = g_strdup_printf ("mount -v --bind /dev %s/dev", target);
    mount_devpts = g_strdup_printf ("mount -vt devpts devpts %s/dev/pts", target);
    mount_proc = g_strdup_printf ("mount -vt proc proc %s/proc", target);
    mount_sys = g_strdup_printf ("mount -vt sysfs sysfs %s/sys", target);

    guint dev_before = get_mount_target_count (dev_target);
    g_spawn_command_line_sync (mount_dev, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount dev %s\n", error->message);
        goto out;
    }
    guint dev_after = get_mount_target_count (dev_target);
    if (dev_after != dev_before + 1) {
        g_warning ("mount procfs:mount dev not changed\n");
        goto out;
    }
    
    guint devpts_before = get_mount_target_count (devpts_target);
    g_spawn_command_line_sync (mount_devpts, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount devpts %s\n", error->message);
        goto out;
    }
    guint devpts_after  = get_mount_target_count (devpts_target);
    if (devpts_after != devpts_before + 1) {
        g_warning ("mount procfs:mount devpts not changed\n");
        goto out;
    }

    guint proc_before = get_mount_target_count (proc_target);
    g_spawn_command_line_sync (mount_proc, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount proc %s\n", error->message);
        goto out;
    }
    guint proc_after = get_mount_target_count (proc_target);
    if (proc_after != proc_before + 1) {
        g_warning ("mount procfs:mount proc not changed\n");
        goto out;
    }

    guint sys_before = get_mount_target_count (sys_target);
    g_spawn_command_line_sync (mount_sys, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("mount procfs:mount sys %s\n", error->message);
        goto out;
    }
    guint sys_after = get_mount_target_count (sys_target);
    if (sys_after != sys_before + 1) {
        g_warning ("mount procfs:mount sys not changed\n");
        goto out;
    }
    ret = TRUE;
    goto out;

out:
    g_free (dev_target);
    g_free (devpts_target);
    g_free (proc_target);
    g_free (sys_target);

    g_free (mount_dev);
    g_free (mount_devpts);
    g_free (mount_proc);
    g_free (mount_sys);
    if (error != NULL) {
        g_error_free (error);
    }
    return ret;
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
    extern int chroot_fd;
    if ((chroot_fd = open (".", O_RDONLY)) < 0) {
        g_warning ("chroot:set chroot fd failed\n");
        return ret;
    }

    extern gboolean in_chroot;
    if (chroot (target) == 0) {
        in_chroot = TRUE;
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to %s falied:%s\n", target, strerror (errno));
    }

    emit_progress ("chroot", "finish");
    return ret;
}

//copy whitelist file just after extract and before chroot
JS_EXPORT_API 
void installer_copy_whitelist ()
{
    GError *error = NULL;
    gchar *cmd = NULL;
    gchar *contents = NULL;
    gchar **strarray = NULL;

    extern const gchar *target;
    if (target == NULL) {
        g_warning ("copy whitelist:target NULL\n");
        goto out;
    }

    if (!g_file_test (WHITE_LIST_PATH, G_FILE_TEST_EXISTS)) {
        g_warning ("copy whitelist:%s not exists\n", WHITE_LIST_PATH);
        goto out;
    }
    g_file_get_contents (WHITE_LIST_PATH, &contents, NULL, &error);
    if (error != NULL) {
        g_warning ("copy whitelist:get packages list %s\n", error->message);
        g_error_free (error);
        goto out;
    }
    if (contents == NULL) {
        g_warning ("copy whitelist:contents NULL\n");
        goto out;
    }
    strarray = g_strsplit (contents, "\n", -1);
    if (strarray == NULL) {
       g_warning ("copy whitelist:strarray NULL\n"); 
       goto out;
    }
    guint count = g_strv_length (strarray);
    g_printf ("copy whitelist:file count %d\n", count);
    guint index = 0;
    for (index = 0; index < count; index++) {
        gchar *item = g_strdup (strarray[index]);
        g_printf ("copy whitelist:start copy file %s\n", item);
        if (!g_file_test (item, G_FILE_TEST_EXISTS)) {
            g_warning ("copy whitelist:file %s not exists\n", item);
            g_free (item);
            continue;
        }
        GFile *src = g_file_new_for_path (item);
        gchar *dest_path = g_strdup_printf ("%s%s", target, item);
        GFile *dest = g_file_new_for_path (dest_path);

        g_file_copy (src, dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);

        g_free (item);
        g_free (dest_path);
        g_object_unref (src);
        g_object_unref (dest);
        if (error != NULL) {
            g_warning ("copy whiltelist:file %s error->%s\n", item, error->message);
            g_error_free (error);
        }
    }
    goto out;

out:
    g_free (cmd);
    g_free (contents);
    g_strfreev (strarray);
    if (error != NULL) {
        g_error_free (error);
    }
}

JS_EXPORT_API 
double installer_get_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("get memory size:%s\n", strerror (errno));
        return 0;
    }

    return info.totalram;
}

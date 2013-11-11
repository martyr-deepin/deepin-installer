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

JS_EXPORT_API 
gboolean installer_create_user (const gchar *username, const gchar *hostname, const gchar *password)
{
    gboolean ret = FALSE;

    GError *error = NULL;

    if (!add_user (username)) {
        g_warning ("create user:add user failed\n");
        //return ret;
    }

    if (!set_user_home (username)) {
        g_warning ("create user:set user home failed\n");
        //return ret;
    }

    if (!set_group (username)) {
        g_warning ("create user:set group failed\n");
        //return ret;
    }
    
    if (!write_hostname (hostname)) {
        g_warning ("create user:write hostname failed\n");
        //return ret;
    }

    struct PasswdHandler *handler = g_new0 (struct PasswdHandler, 1);
    handler->username = g_strdup (username);
    handler->password = g_strdup (password);
    handler->pid = -1;
    handler->in_channel = NULL;
    handler->out_channel = NULL;
    handler->child_watch_id = 0;
    handler->stdout_watch_id = 0;

    set_user_password (handler);
    free_passwd_handler (handler);

    ret = TRUE;

    return ret;
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
    g_printf ("passwd out watch: read %s\n", buf);

    gchar *passwd = g_strdup_printf ("%s\n", handler->password);
    //g_printf ("passwd out watch:write password %s\n", handler->password);
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
        g_printf ("create user:groupadd cmd %s\n", groupadd_cmd);

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
        g_printf ("create user:gpasswd cmd %s\n", gpasswd_cmd);

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

    xkl_config_registry_foreach_layout_variant(config, 
                                               g_strdup (layout), 
                                               _foreach_variant, 
                                               (gpointer) layout);
}

void
init_keyboard_layouts () 
{
    layout_variants_hash = g_hash_table_new_full ((GHashFunc) g_str_hash, 
                                                  (GEqualFunc) g_str_equal, 
                                                  (GDestroyNotify) g_free, 
                                                  (GDestroyNotify) g_list_free);

    Display *dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
        g_warning ("init keyboard layouts: XOpenDisplay\n");
        return ;
    }

    XklEngine *engine = xkl_engine_get_instance (dpy);
    if (engine == NULL) {
        g_warning ("init keyboard layouts: xkl engine get instance\n");
        XCloseDisplay (dpy);
        return ;
    }

    config = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (config, engine);
    if (config == NULL) {
        g_warning ("init keyboard layouts: xkl config rec\n");
        g_object_unref (engine);
        XCloseDisplay (dpy);
        return ;
    }

    XklConfigRegistry *cfg_reg = NULL;
    cfg_reg = xkl_config_registry_get_instance (engine);
    if (cfg_reg == NULL) {
        g_warning ("init keyboard layouts: xkl config registry get instance\n");
        g_object_unref (engine);
        XCloseDisplay (dpy);
        return ;
    }

    if (!xkl_config_registry_load(cfg_reg, TRUE)) {
        g_warning ("init keyboard layouts: xkl config registry load\n");
        g_object_unref (engine);
        g_object_unref (cfg_reg);
        XCloseDisplay (dpy);
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

JS_EXPORT_API 
gboolean installer_set_timezone (const gchar *timezone)
{
    gboolean ret = FALSE;

    GError *error = NULL;

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

    gchar *zoneinfo_path = g_strdup_printf ("/usr/share/zoneinfo/%s", timezone);
    gchar *localtime_path = g_strdup ("/etc/localtime");
    GFile *zoneinfo_file = g_file_new_for_path (zoneinfo_path);
    GFile *localtime_file = g_file_new_for_path (localtime_path);
    g_file_copy (zoneinfo_file, localtime_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("set timezone:cp /etc/localtime %s\n", error->message);
        g_free (zoneinfo_path);
        g_free (localtime_path);
        g_object_unref (zoneinfo_file);
        g_object_unref (localtime_file);
        return ret;
    }
    error = NULL;
    g_free (zoneinfo_path);
    g_free (localtime_path);
    g_object_unref (zoneinfo_file);
    g_object_unref (localtime_file);
    //fix me, set /etc/default/rcS content"
    ret = TRUE;

    return ret;
}

static void
watch_extract_child (GPid pid, gint status, gpointer data)
{
    if (data == NULL) {
        g_warning ("watch extract child:arg data is NULL\n");
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

    g_printf ("watch extract child:extract finish\n");
    //emit_progress ("extract", *progress);
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
            //emit_progress ("extract", *progress);
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

    cb_ids[2] = g_timeout_add (1000, (GSourceFunc) cb_timeout, progress);
    g_child_watch_add (pid, (GChildWatchFunc) watch_extract_child, cb_ids);

    g_strfreev (argv);
}

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

JS_EXPORT_API 
void installer_show_help ()
{
    GError *error = NULL;

    if (g_find_program_in_path ("installerhelp") == NULL) {
        g_warning ("installerhelp not found\n");
        return ;
    }

    g_spawn_command_line_async ("installerhelp", &error);
    if (error != NULL) {
        g_warning ("run installerhelp:%s\n", error->message);
        g_error_free (error);
    }
}

JS_EXPORT_API 
void installer_hide_help ()
{
    GError *error = NULL;

    gchar *cmd = g_strdup ("pkill -9 installerhelp");
    g_spawn_command_line_async (cmd, &error);
    if (error != NULL) {
        g_warning ("hide installer help:%s\n", error->message);
        g_error_free (error);
    }
    g_free (cmd);
}

JS_EXPORT_API 
gboolean installer_is_help_running ()
{
    gboolean running = FALSE;
    GError *error = NULL;
    gchar *output = NULL;

    gchar *cmd = g_strdup ("sh -c \"ps aux |grep installerhelp\"");
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error); 
    g_free (cmd);
    if (error != NULL) {
        g_warning ("is help running:%s\n", error->message);
        g_error_free (error);
    }
    gchar **items = g_strsplit (output, "\n", -1);
    if (g_strv_length (items) > 3) {
        running = TRUE;
    }
    g_strfreev (items);
    g_free (output);

    return running;
}

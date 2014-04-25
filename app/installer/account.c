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

#include "account.h"

extern struct passwd* getpwent (void);
extern void endpwent (void);
extern int lchown (const char *path, uid_t owner, gid_t group);

#define BUFSIZE 64

JS_EXPORT_API 
JSObjectRef installer_get_system_users()
{
    GRAB_CTX ();
    JSObjectRef array = json_array_create ();

    struct passwd *user;
    gchar *username = NULL;
    int i = 0;

    while ((user = getpwent ()) != NULL){
        if (user->pw_uid >= 1000 || g_strcmp0 ("deepin", user->pw_name) == 0) {
            continue;
        }
        username = g_strdup (user->pw_name);
        json_array_insert (array, i, jsvalue_from_cstr (get_global_context(), username));
        i++;
        g_free (username);
    }
    endpwent ();
    UNGRAB_CTX ();

    return array;
}

static void
free_passwd_handler (struct PasswdHandler *handler)
{
    if (handler == NULL) {
        return;
    }
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
        emit_progress ("user", "terminate");
        return NULL;
    }

    if (!set_user_home (handler->username)) {
        g_warning ("create user:set user home failed\n");
        emit_progress ("user", "terminate");
        return NULL;
    }

    if (!set_group (handler->username)) {
        g_warning ("create user:set group failed\n");
        emit_progress ("user", "terminate");
        return NULL;
    }
    
    if (!write_hostname (handler->hostname)) {
        g_warning ("create user:write hostname failed\n");
        emit_progress ("user", "terminate");
        return NULL;
    }
    if (!set_user_password (handler)) {
        g_warning ("create user:set user password failed\n");
        emit_progress ("user", "terminate");
        return NULL;
    }
    emit_progress ("user", "finish");
    return NULL;
}

JS_EXPORT_API 
void installer_create_user (const gchar *username, const gchar *hostname, const gchar *password)
{
    if (username == NULL || hostname == NULL || password == NULL) {
        g_warning ("create user:invalid username-> %s or hostname->%s or password->%s\n", username, hostname, password);
        emit_progress ("user", "terminate");
        return;
    }
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
    if (username == NULL) {
        return FALSE;
    }
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
    if (username == NULL) {
        return FALSE;
    }
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
    g_debug ("watch password child:set password finish\n");
    free_passwd_handler (handler);
    if (status == -1) {
        emit_progress ("user", "terminate");
    }
}

static gboolean
passwd_out_watch (GIOChannel *channel, GIOCondition cond, struct PasswdHandler *handler)
{
    static int write_count = 0;
    if (write_count > 1 || channel == NULL) {
        return FALSE;
    }

    gchar buf[BUFSIZE];
    memset (buf, 0, BUFSIZE);
    GError *error = NULL;        

    if (g_io_channel_read_chars (channel, buf, BUFSIZE, NULL, &error) != G_IO_STATUS_NORMAL) {
        g_warning ("passwd out watch:read error %s", error->message);
        g_error_free (error);
        error = NULL;
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
            error = NULL;
        }
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
    g_debug ("set user password");
    gboolean ret = FALSE;
    if (handler == NULL) {
        return ret;
    }

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

    if (username == NULL) {
        return ret;
    }

    gchar **groups = g_new0 (gchar*, 15);
    groups[0] = g_strdup ("cdrom");
    groups[1] = g_strdup ("floppy");
    groups[2] = g_strdup ("dialout");
    groups[3] = g_strdup ("audio");
    groups[4] = g_strdup ("video");
    groups[5] = g_strdup ("plugdev");
    groups[6] = g_strdup ("sambashare");
    groups[7] = g_strdup ("adm");
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
            error = NULL;
        }
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
            error = NULL;
        }
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
        return ret;
    }

    gchar *hostname_file = g_strdup ("/etc/hostname");

    g_file_set_contents (hostname_file, hostname, -1, &error);
    if (error != NULL) {
        g_warning ("write hostname: set hostname file %s contents failed\n", hostname_file);
        g_error_free (error);
        error = NULL;
        g_free (hostname_file);
        return ret;
    }
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
        error = NULL;
        g_free ((gchar *)lha);
        g_free (hosts_file);
        g_free (hosts_content);
        return ret;
    }
    g_free ((gchar *)lha);
    g_free (hosts_file);
    g_free (hosts_content);

    ret = TRUE;

    return ret;
}

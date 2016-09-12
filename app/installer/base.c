/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <sys/sysinfo.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "base.h"
#include "ped_utils.h"
#include "dwebview.h"

int server_sockfd;

gboolean installer_is_running ()
{
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0';
    int path_size = g_sprintf (server_addr.sun_path+1, "%s",
                               "installer.app.deepin");
    server_addr.sun_family = AF_UNIX;
    server_len = 1 + path_size + offsetof(struct sockaddr_un, sun_path);

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (0 == bind(server_sockfd, (struct sockaddr *) & server_addr, server_len)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

gchar* get_matched_string (const gchar *target, const gchar *regex_string)
{
    g_return_val_if_fail(target != NULL, NULL);
    g_return_val_if_fail(regex_string != NULL, NULL);

    GError *error = NULL;
    GRegex* regex = g_regex_new (regex_string, 0, 0, &error);
    if (error != NULL) {
        g_warning ("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        return NULL;
    }

    gchar *result = NULL;

    GMatchInfo *match_info = NULL;
    g_regex_match (regex, target, 0, &match_info);
    if (g_match_info_matches (match_info)) {
        result = g_match_info_fetch (match_info, 1);
    } else {
        g_debug("[%s]: failed in `%s` to find `%s`\n", __func__, target,
                regex_string);
    }

    g_match_info_free (match_info);
    g_regex_unref (regex);

    return result;
}

gchar* get_matched_string_old (const gchar *target, const gchar *regex_string)
{
    gchar *result = NULL;
    GError *error = NULL;
    GRegex *regex = NULL;
    GMatchInfo *match_info = NULL;

    if (target == NULL || regex_string == NULL) {
        g_warning ("[%s]: paramemter NULL\n", __func__);
        return NULL;
    }

    regex = g_regex_new (regex_string, 0, 0, &error);
    if (error != NULL) {
        g_warning ("get matched string:%s\n", error->message);
        g_error_free (error);
        error = NULL;
        return NULL;
    }

    g_regex_match (regex, target, 0, &match_info);
    if (g_match_info_matches (match_info)) {
        result = g_match_info_fetch (match_info, 0);

    } else {
        g_debug("[%s]: failed in `%s` to find `%s`\n", __func__, target,
                regex_string);
    }

    g_match_info_free (match_info);
    g_regex_unref (regex);

    return result;
}

JS_EXPORT_API 
double installer_get_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("[%s]: %s\n", __func__, strerror (errno));
        return 0;
    }

    return info.totalram * info.mem_unit;
}

JS_EXPORT_API 
double installer_get_keycode_from_keysym (double keysym)
{
    Display *dpy = XOpenDisplay (0);
    if (dpy == NULL) {
        g_warning ("[%s] display is NULL\n", __func__);
        return 0;
    }
    KeyCode code = XKeysymToKeycode (dpy, (KeySym) keysym);
    XCloseDisplay (dpy);
    return code;
}

JS_EXPORT_API 
gboolean installer_detect_capslock ()
{
    gboolean capslock_flag = FALSE;

    Display *d = XOpenDisplay (0);
    guint n;

    if (d) {
        XkbGetIndicatorState (d, XkbUseCoreKbd, &n);
        if ((n & 1)) {
            capslock_flag = TRUE;
        }
    }
    XCloseDisplay (d);

    return capslock_flag;
}

double get_free_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("[%s] error: %s\n", __func__, strerror (errno));
        return 0;
    }

    return info.freeram * info.mem_unit;
}

guint
get_cpu_num ()
{
    guint num = 0;
    gchar *output = NULL;
    const gchar *cmd = "sh -c \"cat /proc/cpuinfo |grep processor |wc -l\"";
    GError *error = NULL;

    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    if (output == NULL) {
        return num;
    }
    num = g_strtod (g_strstrip (output), NULL);

    g_free (output);
    return num;
}

void 
unmount_partition_by_device (const gchar *path)
{
    if (path == NULL) {
        g_warning ("[%s] error: path NULL\n", __func__);
        return;
    }
    gchar *mp = NULL;
    while ((mp = get_partition_mount_point (path)) != NULL) {
        gchar *cmd = NULL;
        if (g_strcmp0 (mp, "swap") == 0) {
            cmd = g_strdup_printf ("swapoff %s", path);
        } else {
            cmd = g_strdup_printf ("umount -l %s", mp);
        }
	      GError* error = NULL;
        g_spawn_command_line_sync(cmd, NULL, NULL, NULL, &error);
        if (error != NULL) {
            g_warning("[%s] failed (cmd: %s) : %s\n", __func__, cmd,
                      error->message);
            g_error_free(error);
        }
        g_free (cmd);
        g_free (mp);
    }
}

guint 
get_mount_target_count (const gchar *target)
{
    guint count = 0;
    gchar *findcmd = NULL;
    gchar *output = NULL;
    gchar **array = NULL;
    GError *error = NULL;

    if (target == NULL) {
        g_warning ("[%s]: target NULL\n", __func__);
        goto out;
    }

    findcmd = g_strdup_printf ("findmnt --target %s", target);
    g_spawn_command_line_sync (findcmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s]:run %s error->%s\n", __func__, findcmd,
                   error->message);
        goto out;
    }
    if (output == NULL || output == "") {
        g_warning ("[%s]: output invalid\n", __func__);
        goto out;
    }
    array = g_strsplit (output, "\n", -1);
    if (array == NULL) {
        g_warning ("[%s]: array NULL\n", __func__);
        goto out;
    }
    count = g_strv_length (array);
    if (count > 2) {
        count = count - 2;
    } else {
        count = 0;
    }
    goto out;
out:
    g_free (findcmd);
    g_free (output);
    if (array != NULL) {
        g_strfreev (array);
    }
    if (error != NULL) {
        g_error_free (error);
        error = NULL;
    }
    return count;
}

gchar *
get_partition_uuid (const gchar *path)
{
    gchar *uuid = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("[%s]: path %s does not exist\n", __func__, path);
        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s UUID -o value %s", path);
    g_spawn_command_line_sync (cmd, &uuid, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (cmd);

    return uuid;
}

gchar *
get_partition_label (const gchar *path)
{
    gchar *label = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("[%s]: path %s does not exist", __func__, path);
        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s LABEL -o value %s", path);
    g_spawn_command_line_sync (cmd, &label, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("[%s] error: %s\n", __func__, error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (cmd);

    return label;
}

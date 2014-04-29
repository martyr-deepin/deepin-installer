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

#include <sys/sysinfo.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "base.h"
#include "dwebview.h"

int server_sockfd;

gboolean installer_is_running ()
{
    socklen_t server_len;
    struct sockaddr_un server_addr;

    server_addr.sun_path[0] = '\0';
    int path_size = g_sprintf (server_addr.sun_path+1, "%s", "installer.app.deepin");
    server_addr.sun_family = AF_UNIX;
    server_len = 1 + path_size + offsetof(struct sockaddr_un, sun_path);

    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (0 == bind(server_sockfd, (struct sockaddr *)&server_addr, server_len)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void emit_progress (const gchar *step, const gchar *progress)
{
    GRAB_CTX ();
    if (step == NULL || progress == NULL) {
        g_warning ("emit progress:invalid step->%s or progress->%s\n", step, progress);
    }
    JSObjectRef message = json_create ();
    json_append_string (message, "stage", step);
    json_append_string (message, "progress", progress);
    js_post_message ("progress", message);
    UNGRAB_CTX ();
}

gchar *
get_matched_string (const gchar *target, const gchar *regex_string) 
{
    gchar *result = NULL;
    GError *error = NULL;
    GRegex *regex = NULL;
    GMatchInfo *match_info = NULL;

    if (target == NULL || regex_string == NULL) {
        g_warning ("get matched string:paramemter NULL\n");
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
        g_warning ("get matched string:failed in %s to find %s\n", target, regex_string);
    }

    g_match_info_free (match_info);
    g_regex_unref (regex);

    return result;
}

gchar *
get_xrandr_size ()
{
    gchar *size = NULL;
    GError *error = NULL;
    
    const gchar *cmd = "sh -c \"xrandr | grep '*' | awk '{print $1}'\"";
    gchar *output = NULL;
    g_spawn_command_line_sync (cmd, &output, NULL, NULL, &error); 
    if (error != NULL) {
        g_warning ("get xrandr size:%s\n", error->message);
        g_error_free (error);
        error = NULL;
        g_free (output);
        return g_strdup("1024x768");
    }
    gchar **lines = g_strsplit(output, "\n", -1);
    if (lines != NULL) {
        size = g_strdup(g_strstrip(lines[0]));
    } else {
        size = g_strdup("1024x768");
    }
    g_strfreev (lines);
    g_free(output);

    return size;
}

JS_EXPORT_API 
double installer_get_memory_size ()
{
    struct sysinfo info;
    if (sysinfo (&info) != 0) {
        g_warning ("get memory size:%s\n", strerror (errno));
        return 0;
    }

    return info.totalram * info.mem_unit;
}

JS_EXPORT_API 
double installer_get_keycode_from_keysym (double keysym)
{
    Display *dpy = XOpenDisplay (0);
    if (dpy == NULL) {
        g_warning ("get keycode from keysym:XOpenDisplay\n");
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
        g_warning ("get free memory size:%s\n", strerror (errno));
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
        g_warning ("get cpu num:%s\n", error->message);
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

gchar *
get_partition_mount_point (const gchar *path)
{
    gchar *mp = NULL;
    gchar *swap_cmd = NULL;
    gchar *swap_output = NULL;
    gchar *cmd = NULL;
    GError *error = NULL;

    if (path == NULL || !g_file_test (path, G_FILE_TEST_EXISTS)) {
        g_warning ("get partition mount point:invalid path %s\n", path);
        return mp;
    }

    swap_cmd = g_strdup_printf ("sh -c \"cat /proc/swaps |grep %s |awk '{print $1}'\"", path);
    g_spawn_command_line_sync (swap_cmd, &swap_output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition mount point:run swap cmd error->%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (swap_cmd);

    if (swap_output != NULL && g_strcmp0 (g_strstrip(swap_output), path) == 0) {
        return g_strdup ("swap");
    }

    cmd = g_strdup_printf ("findmnt -k -f -n -o TARGET -S %s", path);
    g_spawn_command_line_sync (cmd, &mp, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition mount point:run cmd error->%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (cmd);
    if (mp != NULL) {
        mp = g_strstrip (mp);
        if (g_strcmp0 (mp, "") == 0) {
            g_free (mp);
            return NULL;
        }
    }

    return mp;
}

void 
unmount_partition_by_device (const gchar *path)
{
    if (path == NULL) {
        g_warning ("unmount partition by device:path NULL\n");
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
        g_spawn_command_line_async (cmd, NULL);
        g_free (cmd);
        g_free (mp);
        g_usleep (1000);
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
        g_warning ("get target mount count:target NULL\n");
        goto out;
    }

    findcmd = g_strdup_printf ("findmnt --target %s", target);
    g_spawn_command_line_sync (findcmd, &output, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get target mount count:run %s error->%s\n", findcmd, error->message);
        goto out;
    }
    if (output == NULL || output == "") {
        g_warning ("get target mount count:output invalid\n");
        goto out;
    }
    array = g_strsplit (output, "\n", -1);
    if (array == NULL) {
        g_warning ("get target mount count:array NULL\n");
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
        g_warning ("get partition uuid:path %s not exists", path);
        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s UUID -o value %s", path);
    g_spawn_command_line_sync (cmd, &uuid, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition uuid:%s\n", error->message);
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
        g_warning ("get partition label:path %s not exists", path);
        return NULL;
    }

    cmd = g_strdup_printf ("blkid -s LABEL -o value %s", path);
    g_spawn_command_line_sync (cmd, &label, NULL, NULL, &error);
    if (error != NULL) {
        g_warning ("get partition label:%s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
    g_free (cmd);

    return label;
}

JS_EXPORT_API 
void  installer_draw_background (JSValueRef canvas, const gchar *path)
{
    GError *error = NULL;
    cairo_t* cr =  NULL;
    GdkPixbuf *pixbuf = NULL;

    cr = fetch_cairo_from_html_canvas (get_global_context(), canvas);
    if (!g_file_test (path, G_FILE_TEST_EXISTS) || g_access (path, R_OK) != 0) {
        g_warning ("draw background:invalid path %s\n", path); 
        cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
        goto draw;
    }

    pixbuf = gdk_pixbuf_new_from_file_at_scale (path, 752, 450, FALSE, &error);
    if (error != NULL) {
        g_warning ("draw background:get pixbuf failed");
        g_error_free (error);
        error = NULL;
        cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
    } else {
        gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
    }
    goto draw;

draw:
    cairo_paint (cr);
    canvas_custom_draw_did (cr, NULL);
    if (pixbuf != NULL) {
        g_object_unref (pixbuf);
    }
}

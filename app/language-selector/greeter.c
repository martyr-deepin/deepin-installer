/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <gtk/gtk.h>
#include <cairo-xlib.h>
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <lightdm.h>
#include <unistd.h>
#include <glib.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <X11/XKBlib.h>

#include "jsextension.h"
#include "dwebview.h"
#include "i18n.h"
#include "utils.h"
#include "mutils.h"
#include "display_info.h"

#include "user.h"
#include "session.h"
#include "greeter_util.h"
#include "theme.h"

#define GREETER_HTML_PATH "file://"RESOURCE_DIR"/language-selector/index.html"
#define LANG_PATH RESOURCE_DIR"/language-selector/support_language_list.ini"

// Absolute path to log file.
// TODO(xushaohua): move log to /var/log/deepin-lightdm-language-selector.log
const char* kLogPath = "/tmp/deepin-lightdm-language-selector.log";

LightDMGreeter *greeter;
GKeyFile *greeter_keyfile;
gchar* greeter_file;

PRIVATE GtkWidget* bg_window = NULL;
static GtkWidget* container = NULL;

PRIVATE struct DisplayInfo rect_screen;
PRIVATE struct DisplayInfo rect_workarea;

struct AuthHandler {
    gchar *username;
    gchar *password;
    gchar *session;
};

struct AuthHandler *handler;

char** get_lang_groups()
{
    GKeyFile* key = g_key_file_new();
    gboolean load = g_key_file_load_from_file(key, LANG_PATH,
                                              G_KEY_FILE_NONE, NULL);
    gsize len;
    char** list = g_key_file_get_groups(key,&len);
    g_message("[%s] length:%d, load: %d\n", __func__, (int)len, load);
    g_key_file_unref(key);
    return list;
}

JS_EXPORT_API
JSObjectRef greeter_get_local_list()
{
    GKeyFile* key = g_key_file_new();
    gboolean load = g_key_file_load_from_file(key, LANG_PATH,
                                              G_KEY_FILE_NONE, NULL);
    gsize len;
    char** list = g_key_file_get_groups(key,&len);
    g_message("[%s] length: %d, load: %d\n", __func__, (int)len, load);
    JSObjectRef array = json_array_create();
    for (guint i = 0;i < len; i++)
    {
        g_message("[%s] list: %d: %s\n", __func__, i, list[i]);
        gchar* name = g_key_file_get_string(key,list[i],"name",NULL);
        gchar* local = g_key_file_get_string(key,list[i],"local",NULL);
        g_message("[%s] name: %s, local: %s\n", __func__, name, local);

        JSObjectRef json = json_create();
        json_append_string(json,"name",name);
        json_append_string(json,"local",local);
        g_free(name);
        g_free(local);
        json_array_insert(array,i,json);
    }
    g_strfreev(list);
    g_key_file_unref(key);
    return array;
}

JS_EXPORT_API
JSObjectRef greeter_get_lang_list()
{
    GKeyFile* key = g_key_file_new();
    gboolean load = g_key_file_load_from_file(key, LANG_PATH,
                                              G_KEY_FILE_NONE, NULL);
    gsize len;
    char** list = g_key_file_get_groups(key,&len);
    g_message("[%s] length: %d, load: %d\n", __func__, (int)len, load);
    JSObjectRef array = json_array_create();
    for (guint i = 0;i < len; i++)
    {
        g_message("[%s] list: %d: %s\n", __func__, i, list[i]);
        gchar* name = g_key_file_get_string(key,list[i],"name",NULL);

        JSObjectRef json = json_create();
        json_append_string(json,"name",name);
        json_append_string(json,"lang",list[i]);
        g_message("[%s] name: %s, lang: %s\n", __func__, name, list[i]);
        g_free(name);
        json_array_insert(array,i,json);
    }
    g_strfreev(list);
    g_key_file_unref(key);
    return array;
}

JS_EXPORT_API
char* greeter_get_lang_by_name(gchar* lang_name)
{
    GKeyFile* key = g_key_file_new();
    gboolean load = g_key_file_load_from_file(key, LANG_PATH,
                                              G_KEY_FILE_NONE, NULL);
    gsize len;
    char** list = g_key_file_get_groups(key, &len);
    g_message("[%s] length: %d,load: %d\n", __func__, (int)len, load);
    gchar* local = NULL;
    for (guint i = 0;i < len; i++) {
        gchar* name = g_key_file_get_string(key,list[i], "name", NULL);
        if (g_str_equal(lang_name,name)){
            local = list[i];
            g_message("[%s] list[%d]: name: %s, local: %s\n", __func__, i,
                      name, local);
        }
        g_free(name);
    }
    g_strfreev(list);
    g_key_file_unref(key);
    return g_strdup(local);
}

JS_EXPORT_API
void greeter_spawn_command_sync (const char* command, gboolean sync){
    GError *error = NULL;
    const gchar *cmd = g_strdup_printf ("%s", command);
    g_message("[%s] cmd: %s, sync: %d\n", __func__, cmd, sync);
    if(sync){
        g_spawn_command_line_sync (cmd, NULL, NULL, NULL, &error);
    }else{
        g_spawn_command_line_async (cmd, &error);
    }
    if (error != NULL) {
        g_warning("[%s] %s failed: %s\n", __func__, cmd, error->message);
        g_error_free (error);
        error = NULL;
    }
}

JS_EXPORT_API
char* greeter_get_resources_dir()
{
    return RESOURCE_DIR;
}


JS_EXPORT_API
char* greeter_get_theme()
{
    return get_theme_config();
}

static void
free_auth_handler (struct AuthHandler *handler)
{
    if (handler == NULL) {
        return ;
    }

    if (handler->username != NULL) {
        g_free (handler->username);
    }

    if (handler->password != NULL) {
        g_free (handler->password);
    }

    if (handler->session != NULL) {
        g_free (handler->session);
    }

    if (handler != NULL) {
        g_free (handler);
    }
}

static void
start_authentication (struct AuthHandler *handler)
{
    gchar *username = g_strdup (handler->username);
    g_warning("[%s]: %s\n", __func__, username);

    if (g_strcmp0 (username, "guest") == 0) {
        lightdm_greeter_authenticate_as_guest (greeter, NULL);
        g_warning("[%s] for guest\n", __func__);
    } else {
        lightdm_greeter_authenticate (greeter, username, NULL);
    }

    g_free (username);
}

static void
respond_authentication(LightDMGreeter *greeter,
                       const gchar *text G_GNUC_UNUSED,
                       LightDMPromptType type)
{
    g_message("[%s]\n", __func__);
    gchar *respond = NULL;

    if (type == LIGHTDM_PROMPT_TYPE_QUESTION) {
        respond = g_strdup (handler->username);
    }  else if (type == LIGHTDM_PROMPT_TYPE_SECRET) {
        respond = g_strdup (handler->password);
    } else {
        g_warning("[%s] failed: invalid prompt type\n", __func__);
        return ;
    }
    g_warning("[%s] respond: %s\n", __func__, respond);

    lightdm_greeter_respond (greeter, respond, NULL);

    g_free (respond);
}

static void
set_last_user (const gchar* username)
{
    gchar *data;
    gsize length;

    g_key_file_set_value(greeter_keyfile, "deepin-greeter", "last-user",
                         g_strdup(username));
    data = g_key_file_to_data (greeter_keyfile, &length, NULL);
    g_file_set_contents (greeter_file, data, length, NULL);

    g_free (data);
}

static void
start_session (LightDMGreeter *greeter)
{
    g_warning("[%s]\n", __func__);

    gchar *session = g_strdup (handler->session);
    set_last_user (handler->username);
    /*keep_user_background (handler->username);*/
    kill_user_lock (handler->username, handler->password);

    if (!lightdm_greeter_start_session_sync (greeter, session, NULL)) {
        g_warning("[%s] session %s failed\n", __func__, session);
        g_free (session);
        free_auth_handler (handler);
    } else {
        g_warning("[%s] %s succeed\n", __func__, session);
        g_key_file_free (greeter_keyfile);
        g_free (greeter_file);
        g_free (session);
        free_auth_handler (handler);
    }
}

static void
authenticated_complete(LightDMGreeter *greeter)
{
    g_message("[%s] real_time: %ld\n", __func__, g_get_real_time());
    if (!lightdm_greeter_get_is_authenticated (greeter)) {
        g_warning("[%s] auth-failed\n", __func__);
        JSObjectRef error_message = json_create();
        json_append_string(error_message, "error", _("Invalid Password"));
        js_post_message("auth-failed", error_message);
        return;
    }
    g_message("[%s] auth-succeed\n", __func__);
    js_post_signal("auth-succeed");
    start_session(greeter);
}

JS_EXPORT_API
gboolean greeter_start_session(const gchar *username, const gchar *password,
                               const gchar *session)
{
    gboolean ret = FALSE;

    if (handler != NULL) {
        free_auth_handler (handler);
    }

    handler = g_new0 (struct AuthHandler, 1);
    handler->username = g_strdup (username);
    handler->password = g_strdup (password);
    handler->session = g_strdup (session);

    if (lightdm_greeter_get_is_authenticated (greeter)) {
        g_warning("[%s] :already authenticated\n", __func__);
        /*start_session (handler);*/
        ret = TRUE;
    } else if (lightdm_greeter_get_in_authentication (greeter)) {
        if (g_strcmp0(username,
              lightdm_greeter_get_authentication_user(greeter)) == 0) {
            g_message("[%s]: current user in authentication\n", __func__);
         //   respond_authentication (handler);
        } else {
            g_message("[%s]: other user in authentication\n", __func__);
          //  lightdm_greeter_cancel_authentication (greeter);
        }
    } else {
        g_message("[%s]: start authenticated\n", __func__);
        start_authentication (handler);
        ret = TRUE;
    }

    return ret;
}

static gboolean
monitors_extend()
{
    g_message("[%s]\n", __func__);
    GdkScreen* screen = gdk_screen_get_default();
    if (screen == NULL) {
        return FALSE;
    }
    gint len = gdk_screen_get_n_monitors(screen);
    g_message("[%s] len: %d\n", __func__, len);
    if (len < 2) {
        return TRUE;
    }

    gchar* xrandr_primary = NULL;
    gchar* xrandr_extend = "";
    for (int i = 0; i < len; i++){
        gchar* name = gdk_screen_get_monitor_plug_name(screen,i);
        g_message("[%s]: name[%d]: %s", __func__, i, name);
        if (i == 0){
            xrandr_primary = g_strdup_printf(" %s --auto",name);
        } else {
            gchar* extend = g_strdup_printf(" --right-of %s",name);
            gchar* xrandr_extend_tmp = g_strdup(xrandr_extend);
            xrandr_extend = NULL;
            xrandr_extend = g_strconcat(xrandr_extend_tmp,extend,NULL);
            g_free(extend);
            g_free(xrandr_extend_tmp);
        }
        g_free(name);
    }
    gchar* cmd = g_strconcat("/usr/bin/xrandr --output", xrandr_primary,
                             xrandr_extend, NULL);
    g_free(xrandr_primary);
    g_free(xrandr_extend);

    gboolean result = spawn_command_sync(cmd,FALSE);
    g_message("[%s] cmd: %s, succeed: %d\n", __func__, cmd, result);
    g_free(cmd);
    return result;
}

void move_pointer_to_center(struct DisplayInfo info)
{
    GdkWindow* window = gtk_widget_get_window(container);
    GdkDisplay* display = gdk_window_get_display(window);
    GdkScreen* screen = gdk_display_get_default_screen (display);
    GdkDeviceManager* manager = gdk_display_get_device_manager(display);
    GList* devices = gdk_device_manager_list_devices(manager,
                                                     GDK_DEVICE_TYPE_MASTER);
    GdkDevice* device = NULL;
    for (GList* dev = devices; dev != NULL; dev = dev->next) {
        device = GDK_DEVICE(dev->data);
        if (gdk_device_get_source(device) != GDK_SOURCE_MOUSE) {
            continue;
        }
        gdk_device_warp(device, screen, info.x + info.width / 2,
                        info.y + info.height / 2);
    }
    g_list_free(devices);
}

static gboolean
monitors_set_cb ()
{
    gint len = update_monitors_num();
    g_message("[%s] monitors len: %d\n", __func__, len);
    if (len > 1){
        update_screen_info(&rect_screen);
        bg_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        draw_background_by_theme(bg_window,NULL,rect_screen);
    }

    container = create_web_container (FALSE, TRUE);
    gtk_window_set_decorated (GTK_WINDOW (container), FALSE);

    GtkWidget* webview = d_webview_new_with_uri (GREETER_HTML_PATH);
    gtk_container_add (GTK_CONTAINER(container), GTK_WIDGET (webview));
    g_signal_connect(webview, "draw", G_CALLBACK(erase_background), NULL);

    set_theme_background(container,webview);

    gtk_widget_realize (webview);
    gtk_widget_realize (container);

    if (len > 1) {
      move_pointer_to_center(rect_workarea);
    }

    GdkWindow* gdkwindow = gtk_widget_get_window (container);
    GdkRGBA rgba = { 0, 0, 0, 0.0 };
    gdk_window_set_background_rgba (gdkwindow, &rgba);
    gdk_window_set_skip_taskbar_hint (gdkwindow, TRUE);
    gdk_window_set_keep_above (gdkwindow, TRUE);
    gdk_window_set_accept_focus(gdkwindow,TRUE);
    GdkCursor* cursor = gdk_cursor_new_for_display (gdk_display_get_default(),
                                                    GDK_LEFT_PTR);
    gdk_window_set_cursor (gdk_get_default_root_window (), cursor);
    g_object_unref(cursor);

    gtk_widget_show_all (container);

    /*turn_numlock_on ();*/

    return FALSE;
}

static gboolean
init_monitors ()
{
    g_message("[%s]\n", __func__);
    gboolean xrandr = monitors_extend();
    g_message("[%s] succeed, xrandr: %d\n", __func__, xrandr);
    if (xrandr){
        g_timeout_add(500, (GSourceFunc)monitors_set_cb, NULL);
        return FALSE;
    } else {
        return TRUE;
    }
}

static void
init_lightdm ()
{
    g_message("[%s]\n", __func__);
    greeter = lightdm_greeter_new ();
    g_assert (greeter);

    g_signal_connect(greeter, "show-prompt",
                     G_CALLBACK(respond_authentication), NULL);
    //g_signal_connect(greeter, "show-message", G_CALLBACK(show_message_cb), NULL);
    g_signal_connect(greeter, "authentication-complete",
                     G_CALLBACK(authenticated_complete), NULL);
    //g_signal_connect(greeter, "autologin-timer-expired", G_CALLBACK(autologin_timer_expired_cb), NULL);

    if (!lightdm_greeter_connect_sync(greeter, NULL)){
        g_warning("[%s] connect greeter failed\n", __func__);
        exit (EXIT_FAILURE);
    }

    gchar *greeter_dir = g_build_filename(g_get_user_cache_dir(),
                                          "lightdm", NULL);

    if (g_mkdir_with_parents (greeter_dir, 0755) < 0){
        greeter_dir = "/var/cache/lightdm";
    }

    greeter_file = g_build_filename (greeter_dir, "deepin-greeter", NULL);
    g_free (greeter_dir);

    greeter_keyfile = g_key_file_new ();
    g_key_file_load_from_file(greeter_keyfile, greeter_file,
                              G_KEY_FILE_NONE, NULL);
}

static void redirect_log()
{
    g_message ("[%s]: log path is: %s", __func__, kLogPath);
    int log_file = open(kLogPath, O_RDWR| O_CREAT| O_TRUNC, 0644);
    if (log_file == -1) {
        perror("redirect_log failed!");
        return;
    }
    if (dup2(log_file, 1) == -1) {
        perror("Failed to redirect stdout!");
    }
    if (dup2(log_file, 2) == -1) {
        perror("Failed to redirect stderr!");
    }
}

int main (int argc, char **argv)
{
    redirect_log();
    g_message("[%s] greeter main\n", __func__);
    g_setenv("G_MESSAGES_DEBUG", "all", TRUE);

    init_i18n ();
    init_theme();
    gtk_init (&argc, &argv);

    init_lightdm();

    g_timeout_add(50,(GSourceFunc)init_monitors,NULL);

    gtk_main ();
    return 0;
}

/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "user.h"
#include "mutils.h"

GDBusProxy *get_user_proxy (const gchar *username)
{
    GDBusProxy *user_proxy = NULL;

    GDBusProxy *account_proxy = NULL;
    GError *error = NULL;

    account_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                  G_DBUS_PROXY_FLAGS_NONE,
                                                  NULL,
                                                  "org.freedesktop.Accounts",
                                                  "/org/freedesktop/Accounts",
                                                  "org.freedesktop.Accounts",
                                                  NULL,
                                                  &error);

    if (error != NULL) {
        g_warning("[%s]: account proxy %s\n", __func__, error->message);
        g_error_free (error);
        return user_proxy;
    }
    error = NULL;

    GVariant* user_path_var = g_dbus_proxy_call_sync(account_proxy,
                                                     "FindUserByName",
                                                     g_variant_new("(s)",
                                                                   username),
                                                     G_DBUS_CALL_FLAGS_NONE,
                                                     -1,
                                                     NULL,
                                                     &error);

    if (error != NULL) {
        g_warning("[%s]: FindUserByName %s\n", __func__, error->message);
        g_error_free (error);
        g_object_unref (account_proxy);
        return user_proxy;
    }
    error = NULL;
    g_object_unref (account_proxy);

    gchar * user_path = NULL;
    g_variant_get (user_path_var, "(o)", &user_path);
    if (user_path == NULL) {
        g_warning("[%s]: user path is NULL\n", __func__);
        g_variant_unref (user_path_var);
        return user_proxy;
    }

    user_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                               G_DBUS_PROXY_FLAGS_NONE,
                                               NULL,
                                               "org.freedesktop.Accounts",
                                               user_path,
                                               "org.freedesktop.Accounts.User",
                                               NULL,
                                               &error);
    if (error != NULL) {
        g_warning("[%s]: user proxy %s\n", __func__, error->message);
        g_error_free (error);
    }
    error = NULL;

    g_free (user_path);
    g_variant_unref (user_path_var);

    return user_proxy;
}

gchar *
get_user_icon (const gchar *username)
{
    gchar *icon = NULL;

    GDBusProxy *user_proxy = NULL;
    GVariant *user_icon_var = NULL;

    user_proxy = get_user_proxy (username);
    if (user_proxy == NULL) {
        g_warning("[%s]: get user proxy failed\n", __func__);
        return icon;
    }

    user_icon_var = g_dbus_proxy_get_cached_property (user_proxy, "IconFile");
    if (user_icon_var == NULL) {
        g_warning("[%s]: user icon var is NULL\n", __func__);
        g_object_unref (user_proxy);
        return icon;
    }

    icon = g_variant_dup_string (user_icon_var, NULL);

    if (!g_file_test(icon, G_FILE_TEST_EXISTS) || g_access(icon, R_OK) != 0) {
        g_warning("[%s]: %s not exists or not readable\n", __func__, icon);
        icon = NULL;
    }

    g_variant_unref (user_icon_var);
    g_object_unref (user_proxy);

    return icon;
}

gchar *
get_user_background (const gchar *username)
{
    gchar *background = NULL;

    GDBusProxy *user_proxy = NULL;
    GVariant *background_var = NULL;

    user_proxy = get_user_proxy (username);
    if (user_proxy == NULL) {
        g_warning ("get user background:get user proxy failed\n");
        background = g_strdup("/usr/share/backgrounds/default_background.jpg");
        return background;
    }

    background_var = g_dbus_proxy_get_cached_property(user_proxy,
                                                      "BackgroundFile");
    if (background_var == NULL) {
        g_warning("[%s]: background var is NULL\n", __func__);
        g_object_unref (user_proxy);
        return background;
    }

    background = g_variant_dup_string (background_var, NULL);

    if (!g_file_test(background, G_FILE_TEST_EXISTS) ||
        g_access(background, R_OK) != 0) {
        g_debug("[%s]: %s not exists or not readable\n", __func__, background);
        background = g_strdup("/usr/share/backgrounds/default_background.jpg");
    }

    g_variant_unref (background_var);
    g_object_unref (user_proxy);

    return background;
}

gchar *
get_user_realname (const gchar *username)
{
    gchar *realname = NULL;

    GDBusProxy *user_proxy = NULL;
    GVariant *realname_var = NULL;

    user_proxy = get_user_proxy (username);
    if (user_proxy == NULL) {
        g_warning("[%s]: get user proxy failed\n", __func__);
        return realname;
    }

    realname_var = g_dbus_proxy_get_cached_property (user_proxy, "RealName");
    if (realname_var == NULL) {
        g_warning("[%s]: realname var is NULL\n", __func__);
        g_object_unref (user_proxy);
        return realname;
    }

    realname  = g_variant_dup_string (realname_var, NULL);

    g_variant_unref (realname_var);
    g_object_unref (user_proxy);

    return realname;
}

gboolean
is_need_pwd (const gchar *username)
{
    gboolean needed = TRUE;

    GDBusProxy *lock_proxy = NULL;
    GVariant *lock_need_pwd = NULL;
    GError *error = NULL;
    g_message("[%s] username: %s", __func__, username);
    if (g_ascii_strncasecmp ("Guest", username, 5) == 0) {
        g_warning("[%s] guest user don't need password\n", __func__);
        needed = FALSE;

        return needed;
    }

    lock_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                               G_DBUS_PROXY_FLAGS_NONE,
                                               NULL,
                                               "com.deepin.dde.lock",
                                               "/com/deepin/dde/lock",
                                               "com.deepin.dde.lock",
                                               NULL,
                                               &error);

    if (error != NULL) {
        g_warning("[%s] lock proxy error: %s\n", __func__, error->message);
        g_error_free (error);
        return needed;
    }
    error = NULL;

    lock_need_pwd  = g_dbus_proxy_call_sync(lock_proxy,
                                            "NeedPwd",
                                            g_variant_new ("(s)", username),
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,
                                            NULL,
                                            &error);

    if (error != NULL) {
        g_warning("[%s] need pwd var %s\n", __func__, error->message);
        g_error_free (error);
        g_object_unref (lock_proxy);
        return needed;
    }

    g_variant_get (lock_need_pwd, "(b)", &needed);

    g_variant_unref (lock_need_pwd);
    g_object_unref (lock_proxy);

    return needed;
}

void
draw_user_background (JSValueRef canvas, const gchar *username)
{
    GError *error = NULL;

    gchar* image_path = get_user_background (username);
    if (image_path == NULL) {
        g_warning("[%s] image path is NULL\n", __func__);
        return ;
    }

    gint height = gdk_screen_get_height (gdk_screen_get_default ());
    gint width = gdk_screen_get_width (gdk_screen_get_default ());

    cairo_t* cr =  fetch_cairo_from_html_canvas(get_global_context(), canvas);

    GdkPixbuf *image_pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path,
        width, height, FALSE, &error);

    if (error != NULL) {
        g_warning("[%s] get lockfile pixbuf failed\n", __func__);
        g_error_free (error);

        cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
        cairo_paint (cr);

    } else {
        gdk_cairo_set_source_pixbuf (cr, image_pixbuf, 0, 0);
        cairo_paint (cr);
    }

    canvas_custom_draw_did (cr, NULL);
    g_object_unref (image_pixbuf);
    g_free (image_path);
}

static cairo_surface_t *
create_root_surface (GdkScreen *screen)
{
    gint number, width, height;
    Display *display;
    Pixmap pixmap;
    cairo_surface_t *surface;

    number = gdk_screen_get_number (screen);
    width = gdk_screen_get_width (screen);
    height = gdk_screen_get_height (screen);

    /* Open a new connection so with Retain Permanent so the pixmap remains when the greeter quits */
    gdk_flush ();

    display = XOpenDisplay(gdk_display_get_name(
          gdk_screen_get_display(screen)));
    if (!display) {
        g_warning("[%s] failed to create root pixmap\n", __func__);
        return NULL;
    }

    XSetCloseDownMode (display, RetainPermanent);
    pixmap = XCreatePixmap(display, RootWindow(display, number), width,
                           height, DefaultDepth(display, number));
    XCloseDisplay (display);

    /* Convert into a Cairo surface */
    surface = cairo_xlib_surface_create(GDK_SCREEN_XDISPLAY (screen),
        pixmap, GDK_VISUAL_XVISUAL(gdk_screen_get_system_visual(screen)),
        width, height);

    /* Use this pixmap for the background */
    XSetWindowBackgroundPixmap(GDK_SCREEN_XDISPLAY (screen),
        RootWindow(GDK_SCREEN_XDISPLAY(screen), number),
        cairo_xlib_surface_get_drawable(surface));


    return surface;
}

void
keep_user_background (const gchar *username)
{
    GdkPixbuf *background_pixbuf = NULL;
    GdkRGBA background_color;
    GdkRectangle monitor_geometry;

    gchar *bg_path = get_user_background (username);

    if (!gdk_rgba_parse (&background_color, bg_path)) {
        background_pixbuf = gdk_pixbuf_new_from_file (bg_path, NULL);
        if (!background_pixbuf) {
           g_warning("[%s] failed to load background: %s\n", __func__,
                     bg_path);
        }
    }
    g_free (bg_path);

    // !!! since 3.10 gdk_display_get_n_screens is deprecated
    // and the number of screens is always 1.
    GdkScreen* screen = gdk_screen_get_default();
    cairo_surface_t *surface;
    cairo_t *c;
    int monitor;

    surface = create_root_surface (screen);
    c = cairo_create (surface);

    for (monitor = 0; monitor < gdk_screen_get_n_monitors(screen); monitor++) {
        gdk_screen_get_monitor_geometry (screen, monitor, &monitor_geometry);

        if (background_pixbuf) {
            GdkPixbuf *pixbuf = gdk_pixbuf_scale_simple(background_pixbuf,
                monitor_geometry.width, monitor_geometry.height,
                GDK_INTERP_BILINEAR);

            gdk_cairo_set_source_pixbuf(c, pixbuf, monitor_geometry.x,
                                        monitor_geometry.y);
            g_object_unref (pixbuf);
        } else {
            gdk_cairo_set_source_rgba (c, &background_color);
        }

        cairo_paint (c);
        cairo_surface_flush (surface);
        XFlush (gdk_x11_get_default_xdisplay ());
    }

    cairo_destroy (c);

    if (background_pixbuf) {
        g_object_unref (background_pixbuf);
    }
}

void
kill_user_lock (const gchar *username, const gchar *password)
{
    gchar *user_lock_path = NULL;
    GDBusProxy *lock_proxy = NULL;
    GError *error = NULL;

    user_lock_path = g_strdup_printf("%s%s", username, ".dlock.app.deepin");

    if (app_is_running (user_lock_path)) {
        g_warning("[%s]: found user had locked\n", __func__);

        lock_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                   G_DBUS_PROXY_FLAGS_NONE,
                                                   NULL,
                                                   "com.deepin.dde.lock",
                                                   "/com/deepin/dde/lock",
                                                   "com.deepin.dde.lock",
                                                   NULL,
                                                   &error);

        if (error != NULL) {
            g_warning("[%s]: connect com.deepin.dde.lock failed\n", __func__);
            g_error_free (error);
        }
        error = NULL;

        g_dbus_proxy_call_sync(lock_proxy,
                               "ExitLock",
                               g_variant_new("(ss)", username, password),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               &error);

        if (error != NULL) {
            g_warning("[%s]: unlock failed\n", __func__);
            g_error_free (error);
        }
        error = NULL;

        g_object_unref (lock_proxy);
    }

    g_free (user_lock_path);
}


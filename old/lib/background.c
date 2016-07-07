/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "background.h"
#include <gdk/gdkx.h>
#include <cairo/cairo-xlib.h>
#include "X_misc.h"

gboolean background_info_draw_callback(GtkWidget* w, cairo_t* cr,
                                       BackgroundInfo* info)
{
    if (info->bg != NULL) {
        g_mutex_lock(&info->m);
        cairo_set_source_surface(cr, info->bg, 0, 0);
        cairo_paint_with_alpha(cr, info->alpha);
        g_mutex_unlock(&info->m);
    }

    GList* children = gtk_container_get_children(GTK_CONTAINER(w));
    for (GList* l = children; l != NULL; l = l->next) {
        GtkWidget* child = GTK_WIDGET(l->data);
        if (gtk_widget_get_visible(child)) {
            gdk_cairo_set_source_window(cr,
                                        gtk_widget_get_window(child), 0, 0);
            cairo_paint(cr);
        }
    }
    g_list_free(children);

    return TRUE;
}

void background_info_set_background_by_drawable(BackgroundInfo* info,
                                                guint32 drawable)
{
    gint x, y;
    guint border,depth, width=0, height=0;
    Display* dpy = gdk_x11_get_default_xdisplay();
    gdk_error_trap_push();
    //TODO:
    //we shoul use xatom_name window to set events instead of root window
    //because the monitors changed signal will came before root window rect changed
    //so the Xroot window rect maybe keep old rect in update_bg function and in Display DBus signal "PrimaryChanged"
    Window root;
    XGetGeometry(dpy, drawable, &root, &x, &y, &width, &height, &border,
                 &depth);
    g_debug("[%s] width: %d, height: %d, x: %d y: %d\n", __func__,
            width, height, x, y);
    if (gdk_error_trap_pop()) {
        g_warning("[%s] invalid drawable %d \n", __func__, drawable);
        return;
    }

    g_mutex_lock(&info->m);
    if (info->bg != NULL) {
        cairo_surface_destroy(info->bg);
        info->bg = NULL;
    }
    info->bg = cairo_xlib_surface_create(dpy, drawable,
        gdk_x11_visual_get_xvisual(gdk_visual_get_system()), width, height);
    g_mutex_unlock(&info->m);

    if (gtk_widget_get_realized(info->container)) {
        gdk_window_invalidate_rect(gtk_widget_get_window(info->container),
                                   NULL, TRUE);
    }
}

void background_info_set_background_by_file(BackgroundInfo* info,
                                            const char* file)
{
    g_message("[%s] file, :%s", __func__, file);
    GError* error = NULL;

    GdkScreen *screen;
    screen = gtk_window_get_screen (GTK_WINDOW (info->container));
    gint w = gdk_screen_get_width(screen);
    gint h = gdk_screen_get_height(screen);
    g_message("[%s] gdk_screen_get_width:%d, height: %d\n", __func__, w, h);
    GdkPixbuf* pb = gdk_pixbuf_new_from_file_at_scale(file,w,h,FALSE, &error);
    if (error != NULL) {
        g_warning("[%s] failed: %s\n", __func__, error->message);
        g_error_free(error);
        return;
    }
    g_mutex_lock(&info->m);
    if (info->bg != NULL) {
        cairo_surface_destroy(info->bg);
        info->bg = NULL;
    }
    info->bg = gdk_cairo_surface_create_from_pixbuf(pb, 1,
        gtk_widget_get_window(info->container));
    g_mutex_unlock(&info->m);
    g_object_unref(pb);
    if (gtk_widget_get_realized(info->container)) {
        gdk_window_invalidate_rect(gtk_widget_get_window(info->container),
                                   NULL, TRUE);
    }
}

void background_info_change_alpha(BackgroundInfo* info, double alpha)
{
    info->alpha = alpha;
    if (gtk_widget_get_realized(info->container)) {
        gdk_window_invalidate_rect(gtk_widget_get_window(info->container),
                                   NULL, TRUE);
    }
}

void background_info_clear(BackgroundInfo* info)
{
    if (info->bg != NULL) {
        cairo_surface_destroy(info->bg);
    }
    g_mutex_clear(&info->m);
    g_free(info);
}


BackgroundInfo* create_background_info(GtkWidget* container, GtkWidget* child)
{
    g_message("[%s]\n", __func__);
    BackgroundInfo* info = g_new0(BackgroundInfo, 1);
    g_mutex_init(&info->m);
    info->alpha = 1;
    info->container = container;

    if (child != NULL){
        gtk_widget_realize (child);
        gdk_window_set_composited(gtk_widget_get_window(child), TRUE);
    }
    g_signal_connect(container, "draw",
                     G_CALLBACK(background_info_draw_callback), info);
    gtk_widget_realize (container);
    GdkRGBA color = {0,0,0,0};
    gdk_window_set_background_rgba(gtk_widget_get_window(container), &color);

    return info;
}

static Atom _BG_ATOM = 0;

Drawable get_blurred_background()
{
    gulong n_item;
    gpointer data = get_window_property(gdk_x11_get_default_xdisplay(),
                                        GDK_ROOT_WINDOW(), _BG_ATOM, &n_item);
    if (data == NULL) {
        return 0;
    }
    Drawable bg = X_FETCH_32(data, 0);
    XFree(data);
    return bg;
}

GdkFilterReturn update_bg(XEvent* xevent, GdkEvent* event,
                          BackgroundInfo* info)
{
    (void)event;
    if (xevent->type == PropertyNotify) {
        if (((XPropertyEvent*)xevent)->atom == _BG_ATOM) {
            background_info_set_background_by_drawable(info,
                get_blurred_background());
        }
    }
    return GDK_FILTER_CONTINUE;
}

void setup_background(GtkWidget* container, GtkWidget* webview,
                      const char* xatom_name)
{
    _BG_ATOM = gdk_x11_get_xatom_by_name(xatom_name);

    BackgroundInfo* info = create_background_info(container, webview);
    background_info_set_background_by_drawable(info, get_blurred_background());

    //TODO:
    //we shoul use xatom_name window to set events instead of root window
    //because the monitors changed signal will came before root window rect changed
    //so the Xroot window rect maybe keep old rect in update_bg function
    gdk_window_set_events(gdk_get_default_root_window(),
                          GDK_PROPERTY_CHANGE_MASK);
    gdk_window_add_filter(gdk_get_default_root_window(),
                          (GdkFilterFunc)update_bg, info);
}

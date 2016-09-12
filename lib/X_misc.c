/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include "X_misc.h"
#include "dwebview.h"

void set_wmspec_desktop_hint (GdkWindow *window)
{
    GdkAtom atom = gdk_atom_intern ("_NET_WM_WINDOW_TYPE_DESKTOP", FALSE);

    gdk_property_change (window,  
                         gdk_atom_intern ("_NET_WM_WINDOW_TYPE", FALSE),
                         gdk_x11_xatom_to_atom (XA_ATOM), 32,
                         GDK_PROP_MODE_REPLACE, (guchar *) &atom, 1);
}

void set_wmspec_dock_hint(GdkWindow *window)
{
    GdkAtom atom = gdk_atom_intern ("_NET_WM_WINDOW_TYPE_DOCK", FALSE);

    gdk_property_change (window,
                         gdk_atom_intern ("_NET_WM_WINDOW_TYPE", FALSE),
                         gdk_x11_xatom_to_atom (XA_ATOM), 32,
                         GDK_PROP_MODE_REPLACE, (guchar *) &atom, 1);
}

void get_workarea_size(int* x, int* y, int* width, int* height)
{
    Display *dpy = gdk_x11_get_default_xdisplay();
    Atom property = XInternAtom(dpy, "_NET_WORKAREA", False);
    gulong items;
    gulong* data = get_window_property(dpy, GDK_ROOT_WINDOW(), property,
                                       &items);

    if (data && items == 4) {
        *x = X_FETCH_32(data, 0);
        *y = X_FETCH_32(data, 1);
        *width = X_FETCH_32(data, 2);
        *height = X_FETCH_32(data, 3);
        XFree(data);
    } else {
        x = y = width = height = 0;
    }
}

/* from libwnck/xutils.c, comes as LGPLv2+ */
static char *
latin1_to_utf8 (const char *latin1)
{
    GString *str;
    const char *p;

    str = g_string_new (NULL);

    p = latin1;
    while (*p) {
        g_string_append_unichar (str, (gunichar) *p);
        ++p;
    }

    return g_string_free (str, FALSE);
}

#include <gdk/gdkx.h>
/* derived from libwnck/xutils.c, comes as LGPLv2+ */
void get_wmclass (GdkWindow* xwindow, char **res_class, char **res_name)
{
    XClassHint ch;

    ch.res_name = NULL;
    ch.res_class = NULL;

    gdk_error_trap_push ();
    XGetClassHint(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                  GDK_WINDOW_XID(xwindow), &ch);
    gdk_error_trap_pop_ignored ();

    if (res_class) {
        *res_class = NULL;
    }

    if (res_name) {
        *res_name = NULL;
    }

    if (ch.res_name) {
        if (res_name) {
            *res_name = latin1_to_utf8 (ch.res_name);
        }

        XFree (ch.res_name);
    }

    if (ch.res_class) {
        if (res_class) {
            *res_class = latin1_to_utf8 (ch.res_class);
        }

        XFree (ch.res_class);
    }
}

enum {
    STRUT_LEFT = 0,
    STRUT_RIGHT = 1,
    STRUT_TOP = 2,
    STRUT_BOTTOM = 3,
    STRUT_LEFT_START = 4,
    STRUT_LEFT_END = 5,
    STRUT_RIGHT_START = 6,
    STRUT_RIGHT_END = 7,
    STRUT_TOP_START = 8,
    STRUT_TOP_END = 9,
    STRUT_BOTTOM_START = 10,
    STRUT_BOTTOM_END = 11
};


static Atom net_wm_strut_partial      = None;
void set_struct_partial(GdkWindow* gdk_window, guint32 orientation,
                        guint32 strut, guint32 strut_start,
                        guint32 strut_end)
{
    Display *display;
    Window   window;
    gulong   struts [12] = { 0, };

    g_return_if_fail (GDK_IS_WINDOW (gdk_window));

    display = GDK_WINDOW_XDISPLAY (gdk_window);
    window  = GDK_WINDOW_XID (gdk_window);

    if (net_wm_strut_partial == None)
        net_wm_strut_partial = XInternAtom(display, "_NET_WM_STRUT_PARTIAL",
                                           False);

    switch (orientation) {
        case ORIENTATION_LEFT:
            struts [STRUT_LEFT] = strut;
            struts [STRUT_LEFT_START] = strut_start;
            struts [STRUT_LEFT_END] = strut_end;
            break;

        case ORIENTATION_RIGHT:
            struts [STRUT_RIGHT] = strut;
            struts [STRUT_RIGHT_START] = strut_start;
            struts [STRUT_RIGHT_END] = strut_end;
            break;

        case ORIENTATION_TOP:
            struts [STRUT_TOP] = strut;
            struts [STRUT_TOP_START] = strut_start;
            struts [STRUT_TOP_END] = strut_end;
            break;

        case ORIENTATION_BOTTOM:
            struts [STRUT_BOTTOM] = strut;
            struts [STRUT_BOTTOM_START] = strut_start;
            struts [STRUT_BOTTOM_END] = strut_end;
            break;
    }

    gdk_error_trap_push ();
    XChangeProperty(display, window, net_wm_strut_partial, XA_CARDINAL, 32,
                    PropModeReplace, (guchar *) &struts, 12);
    gdk_error_trap_pop_ignored ();
}

void* get_window_property(Display* dsp, Window w, Atom pro, gulong* items)
{
    g_return_val_if_fail(pro != 0, NULL);
    Atom act_type;
    int act_format;
    gulong bytes_after;
    guchar* p_data = NULL;

    gdk_error_trap_push();
    int result = XGetWindowProperty(dsp, w, pro, 0, G_MAXULONG, FALSE,
                                    AnyPropertyType, &act_type, &act_format,
                                    items, &bytes_after, (void*)&p_data);
    int err = gdk_error_trap_pop();

    if (err != Success || result != Success) {
        g_warning("[%s] error, %d, %s\n", __func__, (int)w,
                  gdk_x11_get_xatom_name(pro));
        return NULL;
    } else {
        return p_data;
    }
}


gboolean has_atom_property(Display* dsp, Window w, Atom prop)
{
    gulong items;
    void* data = get_window_property(dsp, w, prop, &items);
    if (data == NULL) {
        return FALSE;
    } else {
        g_free(data);
        return TRUE;
    }
}

cairo_region_t* get_window_input_region(Display* dpy, Window w)
{
    int count = 0;
    int ordering = 0;
    XRectangle  *rects = XShapeGetRectangles(dpy, w, ShapeInput, &count,
                                             &ordering);
    cairo_region_t* reg = cairo_region_create();
    for (int i=0; i<count; i++) {
        cairo_rectangle_int_t rect = {rects[i].x, rects[i].y,
                                      rects[i].width, rects[i].height};
        cairo_region_union_rectangle(reg, &rect);
    }
    XFree(rects);
    return reg;
}

void get_atom_value_for_index(gpointer data, gulong n_item, gpointer res,
                              gulong index)
{
    *(gulong*)res = X_FETCH_32(data, index);
}

void get_atom_value_for_loop(gpointer data, gulong n_item, gpointer res,
                             gulong start_index)
{
    for (guint i = start_index; i < n_item; ++i) {
        ((gulong*)res)[i] = X_FETCH_32(data, i);
    }
}

gboolean get_atom_value_by_atom(Display* dsp, Window window_id, Atom atom,
                                gpointer res, CallbackFunc callback,
                                gulong index)
{
    gulong n_item;
    gpointer data = get_window_property(dsp, window_id, atom, &n_item);
    if (data == NULL) {
        return FALSE;
    }

    g_assert(callback != NULL);

    callback(data, n_item, res, index);

    XFree(data);
    return TRUE;
}

gboolean get_atom_value_by_name(Display* dsp, Window window_id,
                                const char* name, gpointer res,
                                CallbackFunc callback, gulong index)
{
    Atom atom = gdk_x11_get_xatom_by_name(name);
    return get_atom_value_by_atom(dsp, window_id, atom, res, callback, index);
}

static void _ensure_fullscreen_helper(GtkWidget* widget, GdkRectangle *alloc)
{
    int width = gdk_screen_width();
    int height = gdk_screen_height();
    if (alloc->x != 0 || alloc->y != 0 || alloc->width != width ||
        alloc->height != height) {
        gdk_window_move_resize(gtk_widget_get_window(widget), 0, 0,
                               width, height);
    }
}

void ensure_fullscreen(GtkWidget* widget)
{
    gtk_widget_set_size_request(widget, gdk_screen_width(),
                                gdk_screen_height());
    g_signal_connect(widget, "size-allocate",
                     (GCallback)_ensure_fullscreen_helper, NULL);
}

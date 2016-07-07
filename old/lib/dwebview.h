/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __DEEPIN_WEBVIEW__
#define __DEEPIN_WEBVIEW__

#include "config.h"
#include <glib.h>
#include <glib-object.h>
#include <webkit/webkit.h>
#include "jsextension.h"

G_BEGIN_DECLS

#define D_WEBVIEW_TYPE      (d_webview_get_type())
#define D_WEBVIEW(obj)      (G_TYPE_CHECK_INSTANCE_CAST((obj),\
            D_WEBVIEW_TYPE, DWebView))
#define D_WEBVIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass), \
            D_WEBVIEW_TYPE, DWebViewClass))
#define IS_D_WEBVIEW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
            D_WEBVIEW_TYPE))
#define IS_D_WEBVIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),\
            D_WEBVIEW_TYPE))

typedef struct _DWebView    DWebView;
typedef struct _DWebViewClass   DWebViewClass;

struct _DWebView {
    WebKitWebView parent;
};

struct _DWebViewClass {
    WebKitWebViewClass parent_class;
};


GtkWidget* create_web_container(bool normal, bool above);
GtkWidget* d_webview_new();
GtkWidget* d_webview_new_with_uri();
gboolean erase_background(GtkWidget* widget, cairo_t *cr, gpointer data);


// custom webkit's function
extern void canvas_custom_draw_did(cairo_t *cr, const cairo_rectangle_t* rect);
extern cairo_t* fetch_cairo_from_html_canvas(JSContextRef ctx, JSValueRef v);
GdkWindow* webkit_web_view_get_forward_window(GtkWidget*);
void dwebview_show_inspector(GtkWidget* webview);


// auto reload when resource file has changed
void monitor_resource_file(const char* app, GtkWidget* webview);

G_END_DECLS


#endif

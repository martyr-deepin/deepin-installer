/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "pixbuf.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "bg_pixbuf.c"

static void change_corner_alpha (GdkPixbuf* pixbuf, float fa)
{
    int rowstride, n_channels;
    guchar *pixels, *p;
    guchar alpha;

    alpha = (guchar) (fa * 255);
    n_channels = gdk_pixbuf_get_n_channels (pixbuf);

    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    //the outline of the icon are stripped.
    //so the for corner coordinates are (1, 1), (1, 15), (15, 1), (15, 15)
    //p = pixels + y * rowstride + x * n_channels;
    if (G_LIKELY(gdk_pixbuf_get_has_alpha (pixbuf)&&(n_channels == 4))) {
        p = pixels + 1 * rowstride + 1 * n_channels;
        p[3] = alpha;
        p = pixels + 15 * rowstride + 1 * n_channels;
        p[3] = alpha;
        p = pixels + 1 * rowstride + 15 * n_channels;
        p[3] = alpha;
        p = pixels + 15 * rowstride + 15 * n_channels;
        p[3] = alpha;
    } else {
        int i;
        p = pixels + 1 * rowstride + 1 * n_channels;
        for (i=0;i<3; i++) {
            p[i] = p[i] * fa;
        }
        p = pixels + 15 * rowstride + 1 * n_channels;
        for (i=0;i<3; i++) {
            p[i] = p[i] * fa;
        }
        p = pixels + 1 * rowstride + 15 * n_channels;
        for (i=0;i<3; i++) {
            p[i] = p[i] * fa;
        }
        p = pixels + 15 * rowstride + 15 * n_channels;
        for (i=0;i<3; i++) {
            p[i] = p[i] * fa;
        }
    }
}

char* generate_directory_icon(const char* p1, const char* p2,
                              const char* p3, const char* p4)
{
#define write_to_canvas(dest, src, x, y) \
    change_corner_alpha (src, 0.1); \
    gdk_pixbuf_composite(src, dest, x+1, y+1, 17-2, 17-2, x, y, 1, 1, GDK_INTERP_HYPER, 255);

    GdkPixbuf *bg = gdk_pixbuf_new_from_inline(-1, dir_bg_4, TRUE, NULL);

    GError* error = NULL;
    g_assert(bg !=NULL);
    if (p1 != NULL) {
        error = NULL;
        GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_scale(p1, 17, -1,
                                                            TRUE, &error);
        if (error == NULL) {
            write_to_canvas(bg, icon, 6+1, 6);
            g_object_unref(icon);
        } else {
            g_warning("[%s] generate_directory_icon: %s\n",
                      __func__, error->message);
            g_warning("[%s] generate_directory_icon icon 1: %s fail\n",
                      __func__, p1);
            g_error_free (error);
        }
    }
    if (p2 != NULL) {
        error = NULL;
        GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_scale(p2, 17, -1,
                                                            TRUE, &error);
        if (error == NULL) {
            write_to_canvas(bg, icon, 6+17 + 1, 6);
            g_object_unref(icon);
        } else {
            g_warning("[%s] generate_directory_icon icon 2: %s fail\n",
                      __func__, p2);
            g_warning("[%s] generate_directory_icon: %s\n",
                      __func__, error->message);
            g_error_free (error);
        }
    }
    if (p3 != NULL) {
        error = NULL;
        GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_scale(p3, 17, -1,
                                                            TRUE, &error);
        if (error == NULL) {
            write_to_canvas(bg, icon, 6+1, 6+17);
            g_object_unref(icon);
        } else {
            g_warning("[%s] generate_directory_icon icon 3: %s fail\n",
                      __func__, p3);
            g_warning("[%s] generate_directory_icon 3: %s\n",
                      __func__, error->message);
            g_error_free (error);
        }
    }
    if (p4 != NULL) {
        error = NULL;
        GdkPixbuf* icon = gdk_pixbuf_new_from_file_at_scale(p4, 17, -1,
                                                            TRUE, &error);
        if (error == NULL) {
            write_to_canvas(bg, icon, 6+17 + 1, 6+17);
            g_object_unref(icon);
        } else {
            g_warning("[%s] generate_directory_icon icon 4: %s fail\n",
                      __func__, p4);
            g_warning("[%s] generate_directory_icon: %s\n",
                      __func__, error->message);
            g_error_free (error);
        }
    }

    gchar* buf = NULL;
    gsize size = 0;

    error = NULL;
    gdk_pixbuf_save_to_buffer(bg, &buf, &size, "png", &error, NULL);
    g_assert(buf != NULL);

    if (error != NULL) {
        g_warning("%s\n", error->message);
        g_error_free(error);
        g_free(buf);
        return NULL;
    }

    char* base64 = g_base64_encode((const guchar*)buf, size);
    g_free(buf);
    char* data = g_strdup_printf("data:image/png;base64,%s", base64);
    g_free(base64);

    return data;
}


char* get_data_uri_by_pixbuf(GdkPixbuf* pixbuf)
{
    gchar* buf = NULL;
    gsize size = 0;
    GError *error = NULL;

    gdk_pixbuf_save_to_buffer(pixbuf, &buf, &size, "png", &error, NULL);
    g_assert(buf != NULL);

    if (error != NULL) {
        g_warning("%s\n", error->message);
        g_error_free(error);
        g_free(buf);
        return NULL;
    }

    char* base64 = g_base64_encode((const guchar*)buf, size);
    g_free(buf);
    char* data = g_strconcat("data:image/png;base64,", base64, NULL);
    g_free(base64);

    return data;
}
char* pixbuf_to_canvas_data(GdkPixbuf* pixbuf)
{
    guchar* buf = NULL;
    int size = 0;

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int stride = gdk_pixbuf_get_rowstride(pixbuf);
    int pix_bit = stride / width;

    int offset = 0;
    buf = gdk_pixbuf_get_pixels_with_length(pixbuf, (guint*)&size);

    g_assert(buf != NULL);
    GString* string = g_string_sized_new(height * stride + 10);
    g_string_append_c(string, '[');

    if (pix_bit == 4) {
        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                offset = i * stride + j*4;
                g_string_append_printf(string, "%d,%d,%d,%d,",
                                       buf[offset], buf[offset+1],
                                       buf[offset+2], buf[offset+3]);
            }
        }
    } else if (pix_bit == 3) {
        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                offset = i * stride + j*3;
                g_string_append_printf(string, "%d,%d,%d,255,", buf[offset],
                                       buf[offset+1], buf[offset+2]);
            }
        }
    }

    g_string_overwrite(string, string->len-1, "]");
    return g_string_free(string, FALSE);
}

char* get_data_uri_by_path(const char* path)
{
    GError *error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(path, &error);
    if (error != NULL) {
        g_warning("[%s] error: %s\n", __func__, error->message);
        g_error_free(error);
        return NULL;
    }
    char* c = get_data_uri_by_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    return c;
}

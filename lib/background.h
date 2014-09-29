#ifndef __BACKGROUND__
#define __BACKGROUND__

#include <gtk/gtk.h>

typedef struct _BackgroundInfo {
    GtkWidget* container;
    cairo_surface_t* bg;
    double alpha;
    GMutex m;
} BackgroundInfo;

gboolean background_info_draw_callback(GtkWidget* w, cairo_t* cr, BackgroundInfo* info);
void background_info_set_background_by_drawable(BackgroundInfo* info, guint32 drawable);
void background_info_set_background_by_file(BackgroundInfo* info, const char* file);
void background_info_change_alpha(BackgroundInfo* info, double alpha);
BackgroundInfo* create_background_info(GtkWidget* container, GtkWidget* child);
void background_info_clear(BackgroundInfo* info);

void setup_background(GtkWidget* container, GtkWidget* webview,const char* xatom_name);
#endif

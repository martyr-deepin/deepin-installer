#ifndef DCORE_H
#define DCORE_H

#include <glib.h>

gboolean dcore_open_browser(char const* uri);
char* dcore_get_theme_icon(const char* name, double size);

#endif /* end of include guard: DCORE_H */


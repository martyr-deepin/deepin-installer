/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef _M_UTILS_H
#define _M_UTILS_H

#include <sys/socket.h>
#include <sys/un.h>
#include <glib.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>
#include <sys/types.h>
#include <X11/XKBlib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "i18n.h"

gboolean app_is_running (const char* path);

gboolean is_capslock_on ();

gchar* get_date_string ();

void turn_numlock_on ();

#endif

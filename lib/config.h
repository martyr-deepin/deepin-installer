/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef __DUI_DEBUG
#define PRIVATE 
#else
#define PRIVATE static
#endif

#define JS_EXPORT_API
#define DBUS_EXPORT_API

#endif

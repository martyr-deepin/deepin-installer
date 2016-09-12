/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <unistd.h>

void singleton(const char* name);
gboolean is_application_running(const char* path);

#endif /* end of include guard: _SINGLETON_H_ */


/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

void enter_next_stage();
void update_install_progress(int v);
void installer_terminate();
void installer_start_install();

#endif

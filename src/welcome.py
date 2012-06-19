#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012~2013 Deepin, Inc.
#               2012~2013 Long Wei
#
# Author:     Long Wei <yilang2007lw@gmail.com>
# Maintainer: Long Wei <yilang2007lw@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from dtk.ui.button import RadioButton
from dtk.ui.label import Label
from dtk.ui.frame import HorizontalFrame

import gtk

class Welcome(gtk.VBox):
    '''first page of installer'''
    def __init__(self):
        super(Welcome, self).__init__()

        welcome_label = Label("请选择安装方式:", text_size=15)
        self.complete_radio_button = RadioButton("清盘安装")
        self.complete_radio_button.set_active(True)
        complete_label = Label("这里会清除所有的磁盘数据！")

        self.update_radio_button=RadioButton("升级当前系统")

        self.advance_radio_button = RadioButton("高级安装")
        advance_label = Label("自己创建并调整分区！")

        welcome_label_frame=HorizontalFrame()
        welcome_label_frame.add(welcome_label)
        welcome_label_frame.set(1,0,0,0)
        
        main_table = gtk.Table(6, 2)
        main_table.set_row_spacing(0, 40)
        # main_table.set_row_spacing(2, 40)

        main_table.attach(welcome_label_frame, 0, 1, 0,1)
        main_table.attach(self.complete_radio_button, 1, 2, 1,2, yoptions=gtk.FILL)
        main_table.attach(complete_label, 1, 2, 2, 3, xpadding=40)
        main_table.attach(self.update_radio_button,1,2,3,4,ypadding=40)
        main_table.attach(self.advance_radio_button, 1, 2, 4,5 ,yoptions=gtk.FILL)
        main_table.attach(advance_label, 1, 2, 5, 6, xpadding=40)
        # self.add(main_table)
        self.pack_start(main_table, False, False,90)
        
    def is_complete_install(self):
        return self.complete_radio_button.get_active()
        
        
        
        

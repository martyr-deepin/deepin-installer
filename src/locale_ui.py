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

import gtk
from constant import LANGUAGE_LIST

class LocaleUI(gtk.EventBox):
    
    def __init__(self):
        super(LocaleUI,self).__init__()

        self.locale_box=gtk.VBox(False,0)
        self.locale_label=gtk.Label("语言/时区/键盘布局：")
        self.locale_hbox=gtk.HBox(False,0)

        self.language_table=gtk.Table(1,2,False)
        self.timezone_table=gtk.Table(1,2,False)
        self.keyboard_table=gtk.Table(1,2,False)

        self.language_label=gtk.Label("语言：")
        # self.language_combox=gtk.combo_box_new_text()
        self.language_combox=gtk.Combo()
        self.language_combox.entry.set_text("language")
        # self.language_combox.append_text("简体中文")
        # self.language_combox.append_text("繁体中文")
        # self.language_combox.append_text("English")
        self.language_list=LANGUAGE_LIST
        self.language_combox.set_popdown_strings(self.language_list)
        # self.language_combox.set_active(0)
        
        self.language_table.attach(self.language_label,0,1,0,1)
        # self.alignment_language_label=gtk.Alignment(0.1,0,0,0)
        # self.language_table.attach(self.alignment_language_label,0,1,0,1)

        self.language_table.attach(self.language_combox,1,2,0,1)

        self.timezone_label=gtk.Label("时区：")
        self.timezone_combox=gtk.combo_box_new_text()
        self.timezone_combox.append_text("中国北京")
        self.timezone_combox.append_text("USA")
        self.timezone_combox.set_active(0)
        self.timezone_table.attach(self.timezone_label,0,1,0,1)
        self.timezone_table.attach(self.timezone_combox,1,2,0,1)


        self.keyboard_label=gtk.Label("键盘布局：")
        self.keyboard_combox=gtk.combo_box_new_text()
        self.keyboard_combox.append_text("正常布局")
        self.keyboard_combox.append_text("变态布局")
        self.keyboard_combox.set_active(0)
        self.keyboard_table.attach(self.keyboard_label,0,1,0,1)
        self.keyboard_table.attach(self.keyboard_combox,1,2,0,1)

        
        self.locale_hbox.pack_start(self.language_table,True,False,1)
        # self.locale_hbox.pack_start(self.language_combox,True,False,1)
        self.locale_hbox.pack_start(self.timezone_table,True,False,1)
        self.locale_hbox.pack_start(self.keyboard_table,True,False,1)

        
        self.locale_box.pack_start(self.locale_label,True,False,1)
        self.locale_box.pack_start(self.locale_hbox,True,False,1)


        self.show_all()

       
       

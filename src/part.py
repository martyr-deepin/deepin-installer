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

# from part_util import *
import gtk
from dtk.ui.label import Label
from dtk.ui.combo import ComboBox,ComboBoxItem
from dtk.ui.button import Button
from dtk.ui.listview import ListView
from dtk.ui.frame import HorizontalFrame

class Part():
    '''Part UI'''
    def __init__(self):

        self.choose_disk_box=gtk.HBox()
        self.choose_disk_label=Label("选择硬盘")
        self.choose_disk_combo=ComboBox()
        self.choose_disk1=ComboBoxItem("/dev/sda",None)
        self.choose_disk2=ComboBoxItem("/dev/sdb",None)
        self.choose_disk_combo.add_item(self.choose_disk1)
        self.choose_disk_combo.add_item(self.choose_disk2)

        self.choose_disk_box.pack_start(self.choose_disk_label,True,True,4)
        self.choose_disk_box.pack_start(self.choose_disk_combo,True,True,4)


        self.partition_box=gtk.HBox(True)
        self.partition_btn1=Button("/dev/sda1")
        self.partition_btn2=Button("/dev/sda2")
        self.partition_btn3=Button("/dev/sda5")
        self.partition_box.pack_start(self.partition_btn1)
        self.partition_box.pack_start(self.partition_btn2)
        self.partition_box.pack_start(self.partition_btn3)

        
        self.part_listview_box=gtk.VBox()
        
        self.part_set_box=gtk.HBox()
        self.part_new_table_btn=Button("新建分区表")
        self.part_new_btn=Button("添加")
        self.part_edit_btn=Button("更改")
        self.part_delete_btn=Button("删除")
        self.part_recovery_btn=Button("还原")
        self.part_set_box.pack_start(self.part_new_table_btn,True,True,4)
        self.part_set_box.pack_start(self.part_new_btn,True,True,4)
        self.part_set_box.pack_start(self.part_edit_btn,True,True,4)
        self.part_set_box.pack_start(self.part_recovery_btn,True,True,4)
        
        self.part_step_box=gtk.HBox()
        self.page_label=Label("Page 5")
        self.page_label.set_text("第2页，共5页")
        self.page_frame=HorizontalFrame()
        self.page_frame.set(0.1,0,0,0)
        self.page_frame.set_padding(5,0,20,400)

        self.page_frame.add(self.page_label)

        self.ok_button=Button("确定")
        self.back_button=Button("后退")
        self.cancle_button=Button("退出")

        self.part_step_box.pack_start(self.page_frame,False,True,4)
        self.part_step_box.pack_end(self.ok_button,True,True,4)
        self.part_step_box.pack_end(self.back_button,True,True,4)
        self.part_step_box.pack_end(self.cancle_button,True,True,4)

        self.step_box_frame=HorizontalFrame()

        self.step_box_frame.set_padding(0,20,0,20)
        self.step_box_frame.add(self.part_step_box)
        
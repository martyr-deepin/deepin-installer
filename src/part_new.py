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

from dtk.ui.combo import ComboBox,ComboBoxItem
from dtk.ui.label import Label
from dtk.ui.titlebar import Titlebar
from dtk.ui.window import Window
from dtk.ui.button import Button
from dtk.ui.frame import HorizontalFrame

class PartNew(Window):
    '''create new partition UI'''
    def __init__(self):
        super(PartNew,self).__init__()

        self.set_size_request(304,308)
        self.titlebar=Titlebar(["close"],None,None,"新建分区",False,50)
        self.set_position(gtk.WIN_POS_CENTER)

        self.new_part_box=gtk.VBox()
        self.new_part_table=gtk.Table(5,2)
        self.new_part_table.set_row_spacings(15)


        self.part_type_label=Label("新分区类型：")
        self.part_type_combo=ComboBox()
        self.part_type_primary=ComboBoxItem("主分区",None)
        self.part_type_extended=ComboBoxItem("逻辑分区",None)
        self.part_type_logical=ComboBoxItem("扩展分区",None)
        self.part_type_combo.set_select_index(0)
        self.part_type_combo.set_items([self.part_type_primary,self.part_type_extended,self.part_type_logical])
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_type_label)
        self.new_part_table.attach(frame,0,1,0,1)
        self.new_part_table.attach(self.part_type_combo,1,2,0,1,xpadding=30)


        self.part_capacity_label=Label("新分区容量:")
        self.part_capacity_combo=ComboBox()
        part_capacity_small=ComboBoxItem("small",None)
        part_capacity_big=ComboBoxItem("big",None)
        self.part_capacity_combo.set_select_index(0)
        self.part_capacity_combo.set_items([part_capacity_small,part_capacity_big])
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_capacity_label)
        self.new_part_table.attach(frame,0,1,1,2)
        self.new_part_table.attach(self.part_capacity_combo,1,2,1,2,xpadding=30)


        self.part_location_label=Label("新分区位置:")
        self.part_location_combo=ComboBox()
        self.part_location_begin=ComboBoxItem("起始")
        self.part_location_end=ComboBoxItem("结束")
        self.part_location_combo.set_select_index(0)
        self.part_location_combo.set_items([self.part_location_begin,self.part_location_end])
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_location_label)
        self.new_part_table.attach(frame,0,1,2,3)
        self.new_part_table.attach(self.part_location_combo,1,2,2,3,xpadding=30)


        self.part_mp_label=Label("挂    载    点：")
        self.part_mp_combo=ComboBox()
        part_mp_root=ComboBoxItem("/",None)
        part_mp_home=ComboBoxItem("/home",None)
        part_mp_swap=ComboBoxItem("/swap",None)
        self.part_mp_combo.set_select_index(0)
        self.part_mp_combo.set_items([part_mp_root,part_mp_home,part_mp_swap])
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_mp_label)
        self.new_part_table.attach(frame,0,1,3,4)
        self.new_part_table.attach(self.part_mp_combo,1,2,3,4,xpadding=30)


        self.part_fs_label=Label("用              于：")
        self.part_fs_combo=ComboBox()
        self.part_fs_ext4=ComboBoxItem("ext4",None)
        self.part_fs_ext3=ComboBoxItem("ext3",None)
        self.part_fs_ext2=ComboBoxItem("ext2",None)
        self.part_fs_combo.set_select_index(0)
        self.part_fs_combo.set_items([self.part_fs_ext4,self.part_fs_ext3,self.part_fs_ext2])
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_fs_label)
        self.new_part_table.attach(frame,0,1,4,5)
        self.new_part_table.attach(self.part_fs_combo,1,2,4,5,xpadding=30)

        self.new_ok_btn=Button("确定")
        self.new_cancel_btn=Button("取消")
        self.new_btn_box=gtk.HBox()
        self.new_btn_box.pack_end(self.new_cancel_btn,False,False,10)
        self.new_btn_box.pack_end(self.new_ok_btn,False,False,10)

        self.new_cancel_btn.connect("clicked",lambda w:self.destroy())
        self.titlebar.close_button.connect("clicked", lambda w: self.destroy())

        self.new_part_box.pack_start(self.new_part_table,False,False,20)
        self.new_part_box.pack_end(self.new_btn_box,False,False,20)
        self.window_frame.add(self.titlebar)
        self.window_frame.add(self.new_part_box)

if __name__=="__main__":

    PartNew()
    gtk.main()
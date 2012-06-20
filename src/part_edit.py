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

from dtk.ui.combo import ComboBox
from dtk.ui.label import Label
from dtk.ui.titlebar import Titlebar
from dtk.ui.window import Window
from dtk.ui.button import Button
from part_util import global_part_util

class PartEdit(Window):
    '''create new partition UI'''
    def __init__(self,part_edit_ok_callback,selected_partition):
        super(PartEdit,self).__init__()
        self.part_util=global_part_util
        self.selected_partition=selected_partition
        self.on_ok_btn_clicked=part_edit_ok_callback

        self.set_size_request(304,189)
        self.titlebar=Titlebar(["close"],None,None,"编辑分区",False,50)
        self.set_position(gtk.WIN_POS_CENTER)

        self.edit_part_box=gtk.VBox()
        self.edit_part_table=gtk.Table(2,2)
        self.edit_part_table.set_row_spacings(15)


        self.part_mp_label=Label("挂载点：")
        self.part_mp_combo=ComboBox([("/",1),("/home",2),("/swap",3)],100)
        # part_mp_root=ComboBoxItem("/",None)
        # part_mp_home=ComboBoxItem("/home",None)
        # part_mp_swap=ComboBoxItem("/swap",None)
        # self.part_mp_combo.set_select_index(0)
        self.part_mp_combo.select_first_item()
        # self.part_mp_combo.set_items([part_mp_root,part_mp_home,part_mp_swap])
        self.edit_part_table.attach(self.part_mp_label,0,1,0,1,xpadding=30)
        self.edit_part_table.attach(self.part_mp_combo,1,2,0,1,xpadding=15)


        self.part_fs_label=Label("用    于：")
        self.part_fs_combo=ComboBox([("ext4",1),("ext3",2),("ext2",3)],100)
        # self.part_fs_combo=ComboBox()
        # self.part_fs_ext4=ComboBoxItem("ext4",None)
        # self.part_fs_ext3=ComboBoxItem("ext3",None)
        # self.part_fs_ext2=ComboBoxItem("ext2",None)
        # self.part_fs_combo.set_select_index(0)
        self.part_fs_combo.select_first_item()
        # self.part_fs_combo.set_items([self.part_fs_ext4,self.part_fs_ext3,self.part_fs_ext2])
        self.edit_part_table.attach(self.part_fs_label,0,1,1,2,xpadding=30)
        self.edit_part_table.attach(self.part_fs_combo,1,2,1,2,xpadding=15)

        self.edit_ok_btn=Button("确定")
        self.edit_cancel_btn=Button("取消")
        self.edit_btn_box=gtk.HBox()
        self.edit_btn_box.pack_end(self.edit_cancel_btn,False,False,10)
        self.edit_btn_box.pack_end(self.edit_ok_btn,False,False,10)

        self.edit_cancel_btn.connect("clicked",lambda w:self.destroy())
        self.titlebar.close_button.connect("clicked", lambda w: self.destroy())
        self.edit_ok_btn.connect("clicked",self.on_ok_btn_clicked)

        self.edit_part_box.pack_start(self.edit_part_table,False,False,20)
        self.edit_part_box.pack_end(self.edit_btn_box,False,False,20)
        self.window_frame.add(self.titlebar)
        self.window_frame.add(self.edit_part_box)

if __name__=="__main__":

    PartEdit()
    gtk.main()
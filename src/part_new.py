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
from dtk.ui.frame import HorizontalFrame
from dtk.ui.spin import SpinBox
from part_util import global_part_util

class PartNew(Window):
    '''create new partition UI'''
    def __init__(self,part_new_ok_callback,selected_disk,geom_item):
        super(PartNew,self).__init__()

        self.ok_btn_clicked=part_new_ok_callback
        self.current_disk=selected_disk
        self.current_geom_item=geom_item
        self.part_util = global_part_util

        self.set_size_request(304,308)
        self.titlebar=Titlebar(["close"],None,None,"新建分区",False,50)
        self.set_position(gtk.WIN_POS_CENTER)

        self.new_part_box=gtk.VBox()
        self.new_part_table=gtk.Table(5,2)
        self.new_part_table.set_row_spacings(15)


        self.part_type_label=Label("新分区类型：")
        self.part_type_combo=ComboBox([("主分区",1),("逻辑分区",2)])
        # self.part_type_combo.select_first_item()
        self.limit_2added_part_type()
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_type_label)
        self.new_part_table.attach(frame,0,1,0,1)
        self.new_part_table.attach(self.part_type_combo,1,2,0,1,xpadding=30)
        self.part_type_combo.connect("item-selected",self.on_part_type_combo_selected)

        self.part_capacity_label=Label("新分区容量(MB):")
        self.max_size=self.get_max_size()
        self.part_capacity_spin=SpinBox(self.max_size,10,self.max_size,10,55)

        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_capacity_label)

        self.new_part_table.attach(frame,0,1,1,2)
        self.new_part_table.attach(self.part_capacity_spin,1,2,1,2,xpadding=30)


        self.part_location_label=Label("新分区位置:")
        # self.part_location_combo=ComboBox([("起始",1),("结束",2)])
        self.part_location_combo=ComboBox([("start",1),("end",2)])
        self.part_location_combo.select_first_item()
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_location_label)
        self.new_part_table.attach(frame,0,1,2,3)
        self.new_part_table.attach(self.part_location_combo,1,2,2,3,xpadding=30)


        self.part_mp_label=Label("挂    载    点：")
        self.part_mp_combo=ComboBox([("/",1),("/home",2),("/swap",3)])
        self.part_mp_combo.select_first_item()
        frame=HorizontalFrame()
        frame.set_padding(0,0,30,10)
        frame.add(self.part_mp_label)
        self.new_part_table.attach(frame,0,1,3,4)
        self.new_part_table.attach(self.part_mp_combo,1,2,3,4,xpadding=30)


        self.part_fs_label=Label("用              于：")
        self.part_fs_combo=ComboBox([("ext4",1),("ext3",2),("ext2",3)])
        self.part_fs_combo.select_first_item()
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

        self.new_ok_btn.connect("clicked",self.ok_btn_clicked)
        self.new_cancel_btn.connect("clicked",lambda w:self.destroy())
        self.titlebar.close_button.connect("clicked", lambda w: self.destroy())

        self.new_part_box.pack_start(self.new_part_table,False,False,20)
        self.new_part_box.pack_end(self.new_btn_box,False,False,20)
        self.window_frame.add(self.titlebar)
        self.window_frame.add(self.new_part_box)

    def get_max_size(self):
        '''return max size user can create with the given part type,geometry'''
        return self.part_util.get_space_geom_size(self.current_disk,self.current_geom_item[1])
    
    def limit_2added_part_type(self):
        '''limit part_type to add partition'''
        if self.current_geom_item[0]!="freespace":
            print "error,must add partition use freespace area"
            return 

        prev_item=self.part_util.get_prev_geom_info_tab_item(self.current_disk,self.current_geom_item[1])
        next_item=self.part_util.get_next_geom_info_tab_item(self.current_disk,self.current_geom_item[1])
        if prev_item[0]!="freespace" and prev_item!=self.current_geom_item:
            prev_part=self.part_util.get_part_from_geom(self.current_disk,prev_item[1])
        else:
            prev_part=None
        if next_item[0]!="freespace" and next_item!=self.current_geom_item:
            next_part=self.part_util.get_part_from_geom(self.current_disk,next_item[1])
        else:
            next_part=None

        if len(self.part_util.get_disk_logical_list(self.current_disk))==0:
            if len(self.part_util.get_disk_primary_list(self.current_disk)) >=4:
                print "only add logical ok"
                # self.part_type_combo.droplist.insensitive_item(0)
            else:
                print "both primary/logical ok"
        else:
            if len(self.part_util.get_disk_primary_list(self.current_disk)) >=4:
                print "only add logical ok"
                # self.part_type_combo.droplist.insensitive_item(0)
            else:    
                if prev_part!=None and next_part!=None:
                    if prev_part.type==1 and next_part.type==1:
                        print "only add logical ok"
                        # self.part_type_combo.droplist.insensitive_item(0)
                    elif prev_part.type==0 and next_part.type==0:
                        print "only add primary ok"
                        # self.part_type_combo.droplist.insensitive_item(1)
                    else:
                        print "both primary/logical ok"
                elif prev_part!=None and next_part==None:
                    if prev_part.type==0:
                        print "only add primary ok"
                        # self.part_type_combo.droplist.insensitive_item(1)
                    else:
                        print "only add logical ok"
                        # self.part_type_combo.droplist.insensitive_item(0)
                elif prev_part==None and next_part!=None:
                    if next_part.type==0:
                        print "only add primary ok"
                        # self.part_type_combo.droplist.insensitive_item(1)
                    else:
                        print "only add logical ok"
                        # self.part_type_combo.droplist.insensitive_item(0)
                else:        
                    print "error,because the logical list not null"

    def on_part_type_combo_selected(self,arg1,arg2,arg3,arg4):
        '''on part_type_combo_selected'''
        ava_size=int(self.get_max_size())
        self.part_capacity_spin.set_upper(ava_size)
        self.part_capacity_spin.set_value(ava_size)


if __name__=="__main__":

    PartNew()
    gtk.main()
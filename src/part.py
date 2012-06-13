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
from dtk.ui.label import Label
from dtk.ui.combo import ComboBox,ComboBoxItem
from dtk.ui.button import Button,CheckButton
from dtk.ui.listview import ListView
from dtk.ui.frame import HorizontalFrame
from dtk.ui.scrolled_window import ScrolledWindow
from part_list_item import PartListItem
from part_util import PartUtil
from dtk.ui.utils import container_remove_all
import parted

class Part(gtk.VBox):
    '''Part UI'''
    def __init__(self):
        super(Part,self).__init__()

        self.selected_disk=None
        self.selected_disk_partitions=None
        self.selected_part=None
        self.part_util=PartUtil()

        #select disk box
        choose_disk_box=gtk.HBox()
        choose_disk_label=Label("选择硬盘:    ")
        disk_info=self.part_util.get_install_device_info()
        self.choose_disk_combo=ComboBox()
        for disk in disk_info.keys():
            disk_combo_item=ComboBoxItem(disk,None)
            self.choose_disk_combo.add_item(disk_combo_item)

        self.choose_disk_combo.set_select_index(0)    
        self.selected_disk=self.part_util.get_disk_from_path("/dev/sda")
        self.choose_disk_combo.connect("item-selected",self.on_disk_combo_selected)
        choose_disk_box.pack_start(choose_disk_label,True,True,4)
        choose_disk_box.pack_start(self.choose_disk_combo,True,True,4)

        #disk partitions button
        self.selected_disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        self.partition_box=gtk.HBox(True,4)
        self.primary_partition_box=gtk.HBox(True,4)
        self.extend_partition_box=gtk.HBox(True,4)
        self.partition_box.set_size_request(-1,35)
        self.partition_box.pack_start(self.primary_partition_box)
        self.partition_box.pack_start(self.extend_partition_box)
        self.selected_part_btn=None
        self.update_part_btn_box()

        #partitions list view
        part_listview_box=gtk.VBox()
        # part_listview_box.set_size_request(-1,280)
        self.part_listview_items=[]
        self.init_part_listview_items()    
        part_listview=ListView(
            [(lambda item:item.partition,cmp),
            (lambda item:item.fstype,cmp),
            (lambda item:item.mp,cmp),
            (lambda item:item.format,cmp),
            (lambda item:item.total_size,cmp),
            (lambda item:item.used_size,cmp),
            (lambda item:item.part_type,cmp)
             ]
            )
        # part_listview.set_expand_column(2)
        part_listview.add_titles(["分区","文件系统","挂载点","格式化","总容量","已用容量","类型"])
        part_listview.add_items(self.part_listview_items)
        # self.selected_part_list_item=part_listview.select_first_item()

        # part_listview.cell_widths=[100,60,100,60,60,60,100]
        part_scrolled_window=ScrolledWindow()
        part_scrolled_window.set_size_request(400,-1)
        part_scrolled_window.add_child(part_listview)
        part_listview_box.set_size_request(400,-1)
        part_listview_box.pack_start(part_scrolled_window)
        
        #partition admin buttons
        part_set_box=gtk.HBox(False,12)
        part_set_box.set_size_request(100,30)
        part_new_table_btn=Button("新建分区表")
        part_new_btn=Button("新建分区")
        part_edit_btn=Button("编辑分区")
        part_delete_btn=Button("删除分区")
        part_recovery_btn=Button("还原分区表")
        part_set_box.pack_start(part_new_table_btn,False,False,4)
        part_set_box.pack_start(part_new_btn,False,False,4)
        part_set_box.pack_start(part_edit_btn,False,False,4)
        part_set_box.pack_start(part_delete_btn,False,False,4)
        part_set_box.pack_start(part_recovery_btn,False,False,4)
        
        #pack above boxes
        part_frame=HorizontalFrame()
        part_box=gtk.VBox()
        # part_box.set_size_request(400,-1)
        part_box.pack_start(choose_disk_box,False,False,4)
        part_box.pack_start(self.partition_box,False,False,4)
        part_box.pack_start(part_listview_box,False,False,4)
        part_box.pack_start(part_set_box,False,False,4)
        part_frame.add(part_box)
        part_frame.set_padding(0,0,30,30)
        
        self.add(part_frame)

    def update_choose_disk_combo(self):
        '''update the item of choose disk'''
        pass

    def on_disk_combo_selected(self,widget,event):
        '''change disk,react to partitions display'''
        print "disk selected"
        self.selected_disk=self.part_util.get_disk_from_path(self.choose_disk_combo.get_current_item().get_label())
        self.update_part_btn_box()
        self.init_part_listview_items()

    def update_part_btn_box(self):
        '''when change disk,the partitions display changed'''

        self.disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        # container_remove_all(self.partition_box)
        container_remove_all(self.primary_partition_box)
        container_remove_all(self.extend_partition_box)
        for part in self.selected_disk_partitions:
            # print part.type
            part_btn=Button(part.path)
            part_btn.set_label(part.path)
            if part.type==0:
                self.primary_partition_box.pack_start(part_btn,False,False,1)
            elif part.type==1:
                self.extend_partition_box.pack_start(part_btn,False,False,1)
            # self.partition_box.pack_start(part_btn,False,False,1)
            else:
                print "extend"
            part_btn.connect("clicked",self.on_part_btn_clicked)    
            
        self.primary_partition_box.show_all()    
        self.extend_partition_box.show_all()    
        self.partition_box.show_all()
    
    def on_part_btn_clicked(self,widget):
        '''react to listview'''
        print "btn clicked"
        self.selected_part_btn=widget
        print widget.label
        
    def init_part_listview_items(self):
        '''update listview_items,mostly used when change disk or first load'''
        self.disk_partition_info=filter(lambda item:item[0].disk==self.selected_disk,self.part_util.disk_partition_info_tab)
        for item in self.disk_partition_info:
            part_list_item=PartListItem(str(item[0].path),str(item[4]),str(item[7]),str(item[5]),"8G","4G",item[2])
            # part_list_item=PartListItem(str(item[0].path),str(item[4]),str(item[7]),CheckButton(),"8G","4G",item[2])
            self.part_listview_items.append(part_list_item)

    def on_part_item_clicked(self):
        '''one part clicked'''
        pass

    def set_part_item_focus(self):
        '''set part_item_focus'''
        pass

    def update_selected_disk(self):
        pass

    def update_selected_disk_partitions(self):
        '''update the partitions list of the selected_disk'''
        self.selected_disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        return self.selected_disk_partitions

    def update_disk_primary_list(self):
        '''return primary partitions list of the selected_disk'''
        return filter(lambda item:item.type==0,self.selected_disk_partitions)
        
    def update_disk_logical_list(self):
        '''return logical partitions list of the selected_disk'''
        return filter(lambda item:item.type==1,self.selected_disk_partitions)

    def generate_part_path(self,disk,part_type):
        '''generate_part_path from view'''
        main_part_list=filter(lambda item:item.type==parted.PARTITION_NORMAL or item.type==parted.PARTITION_EXTENDED,
                              self.selected_disk.partitions)
        logical_part_list=filter(lambda item:item.type==parted.PARTITION_LOGICAL)

        if part_type=="primary":
            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                return disk.device.path+str(1)
            else:
                max_num=max(self.part_util.get_disk_partitions())
                return disk.device.part+str(max_num+1)

        elif part_type=="extend":
            for part in self.part_util.get_disk_partitions():
                if part.type==parted.PARTITION_EXTENDED:
                    print "can have only one extend "

            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                return disk.device.path+str(1)
            else:
                max_num=max(self.part_util.get_disk_partitions())
                print max_num
                return disk.device.part+str(max_num+1)

        elif part_type=="logical":        
            if len(logical_part_list)==0:
                return disk.device.path+str(5)
            else:
                max_num=max(self.part_util.get_disk_partitions())
                return disk.device.part+str(max_num+1)

        else:
            print "invalid part type"

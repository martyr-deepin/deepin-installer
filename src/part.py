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
from dtk.ui.scrolled_window import ScrolledWindow
from part_list_item import PartListItem
from part_util import PartUtil


class Part(gtk.VBox):
    '''Part UI'''
    def __init__(self):
        super(Part,self).__init__()

        self.part_util=PartUtil()

        choose_disk_box=gtk.HBox()
        choose_disk_label=Label("选择硬盘:    ")
        
        disk_info=self.part_util.get_install_device_info()
        self.choose_disk_combo=ComboBox()
        
        for disk in disk_info.keys():
            disk_combo_item=ComboBoxItem(disk,None)
            self.choose_disk_combo.add_item(disk_combo_item)
            self.choose_disk_combo.set_select_index(0)    


        self.choose_disk_combo.connect("item-selected",self.on_disk_combo_selected)
    
        choose_disk_box.pack_start(choose_disk_label,True,True,4)
        choose_disk_box.pack_start(self.choose_disk_combo,True,True,4)

        self.selected_disk=self.part_util.get_disk_from_path("/dev/sda")
        
        # self.disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)

        self.partition_box=gtk.HBox(True,4)

        part_listview_box=gtk.VBox()
        part_listview_box.set_size_request(-1,300)
        part_listview_items=[]
        self.disk_partition_info=self.part_util.disk_partition_info_tab
        for item in self.disk_partition_info:
            part_list_item=PartListItem(str(item[0].path),str(item[4]),str(item[7]),str(item[5]),"8G","4G",item[2])
            part_listview_items.append(part_list_item)


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
        # self.part_listview.set_expand_column(0)
        part_listview.add_titles(["分区","文件系统","挂载点","格式化","总容量","已用容量","类型"])
        part_listview.add_items(part_listview_items)
        part_listview.cell_widths=[100,60,60,60,60,60,100]

        part_scrolled_window=ScrolledWindow()
        part_scrolled_window.add_child(part_listview)
        part_listview_box.pack_start(part_scrolled_window)


        part_set_box=gtk.HBox(False,12)
        part_set_box.set_size_request(100,30)
        part_new_table_btn=Button("新建分区表")
        part_new_btn=Button("添加")
        part_edit_btn=Button("更改")
        part_delete_btn=Button("删除")
        part_recovery_btn=Button("还原")
        part_set_box.pack_start(part_new_table_btn,False,False,4)
        part_set_box.pack_start(part_new_btn,False,False,4)
        part_set_box.pack_start(part_edit_btn,False,False,4)
        part_set_box.pack_start(part_delete_btn,False,False,4)
        part_set_box.pack_start(part_recovery_btn,False,False,4)

        part_frame=HorizontalFrame()
        part_box=gtk.VBox()
        part_box.set_size_request(400,-1)
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

    def on_disk_combo_selected(self):
        '''change disk,react to partitions display'''
        self.selected_disk=self.part_util.get_disk_from_path(self.choose_disk_combo.get_current_item())

        self.update_part_btn_box()


    def update_part_btn_box(self):
        '''when change disk,the partitions display changed'''
        self.disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        
        for part in self.disk_partitions:
            part_btn=Button(part.path)
            self.partition_box.pack_start(part_btn,False,False,1)
            part_btn.connect("connect",self.on_part_btn_clicked)    

    def on_part_btn_clicked(self):
        ''''''
        pass

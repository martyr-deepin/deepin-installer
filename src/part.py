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
from part_util import global_part_util
from ui_utils import switch_box
from part_new import PartNew
from part_edit import PartEdit

class Part(gtk.VBox):
    '''Part UI'''
    def __init__(self):
        super(Part,self).__init__()

        self.selected_disk=None
        self.selected_disk_partitions=None
        self.selected_part=None
        self.part_util=global_part_util

        #select disk box
        choose_disk_box=gtk.HBox()
        choose_disk_label=Label("选择硬盘:    ")
        disk_info=self.part_util.get_install_device_info()
        self.choose_disk_combo=ComboBox()
        for disk in disk_info.keys():
            disk_combo_item=ComboBoxItem(disk,None)
            self.choose_disk_combo.add_item(disk_combo_item)

        self.choose_disk_combo.set_select_index(0)    
        # self.selected_disk=self.part_util.get_disk_from_path("/dev/sda")
        self.choose_disk_combo.connect("item-selected",self.on_disk_combo_selected)
        self.update_selected_disk()
        choose_disk_box.pack_start(choose_disk_label,True,True,4)
        choose_disk_box.pack_start(self.choose_disk_combo,True,True,4)

        #disk partitions button
        # self.selected_disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        self.partition_btns_container_box=gtk.VBox()
        self.selected_part_btn=None
        self.update_part_btn_box()

        #partitions list view
        self.part_listview_container_box=gtk.VBox()
        self.update_part_listview()

        
        #partition admin buttons
        part_set_box=gtk.HBox(False,12)
        part_set_box.set_size_request(100,30)
        part_new_table_btn=Button("新建分区表")
        part_new_btn=Button("新建分区")
        part_edit_btn=Button("编辑分区")
        part_delete_btn=Button("删除分区")
        part_recovery_btn=Button("还原分区表")

        part_new_table_btn.connect("clicked",self.on_new_table_clicked)
        part_new_btn.connect("clicked",self.on_part_new_btn_clicked)
        part_edit_btn.connect("clicked",self.on_part_edit_btn_clicked)
        part_delete_btn.connect("clicked",self.on_part_delete_btn_clicked)
        part_recovery_btn.connect("clicked",self.on_part_recovery_btn_clicked)

        part_set_box.pack_start(part_new_table_btn,False,False,4)
        part_set_box.pack_start(part_new_btn,False,False,4)
        part_set_box.pack_start(part_edit_btn,False,False,4)
        part_set_box.pack_start(part_delete_btn,False,False,4)
        part_set_box.pack_start(part_recovery_btn,False,False,4)
        
        #pack above boxes
        self.part_frame=HorizontalFrame()
        part_box=gtk.VBox()
        # part_box.set_size_request(400,-1)
        part_box.pack_start(choose_disk_box,False,False,4)
        part_box.pack_start(self.partition_btns_container_box,False,False,4)
        part_box.pack_start(self.part_listview_container_box,False,False,4)
        part_box.pack_start(part_set_box,False,False,4)
        self.part_frame.add(part_box)
        self.part_frame.set_padding(0,0,30,30)
        
        self.add(self.part_frame)


#here comes basic function for update UI data

    def update_selected_disk(self):
        '''changed the current used and displayed disk'''
        self.selected_disk=self.part_util.get_disk_from_path(self.choose_disk_combo.get_current_item().get_label())
        return self.selected_disk

    def update_selected_disk_partitions(self):
        '''update the partitions list of the selected_disk'''
        self.selected_disk_partitions=self.part_util.get_disk_partitions(self.selected_disk)
        return self.selected_disk_partitions

    def update_disk_primary_list(self):
        '''return primary partitions list of the selected_disk'''
        self.update_selected_disk_partitions()
        return filter(lambda item:item.type==0,self.selected_disk_partitions)
        
    def update_disk_logical_list(self):
        '''return logical partitions list of the selected_disk'''
        self.update_selected_disk_partitions()
        return filter(lambda item:item.type==1,self.selected_disk_partitions)

    def update_disk_part_path(self):
        '''update part path when had add or delete partition operation,just update the display'''
        pass

    def generate_disk_part_path(self,part_type):
        '''generate_part_path for new add partition,used by listview display and backend add operation'''
        main_part_list=self.part_util.get_disk_main_partitions(self.selected_disk)
        logical_part_list=self.part_util.get_disk_logical_partitions(self.selected_disk)
        self.new_part_path=""
        if part_type=="primary":
            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                self.new_part_path=self.selected_disk.device.path+str(1)
            else:
                max_num=max(main_part_list.path[-1])
                self.new_part_path=self.selected_disk.device.path+str(max_num+1)

        elif part_type=="extend":
            for part in main_part_list:
                if part.type==2:
                    print "can have only one extend "
                else:
                    continue

            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                self.new_part_path=self.selected_disk.device.path+str(1)
            else:
                max_num=max(main_part_list.path[-1])
                self.new_part_path=self.selected_disk.device.path+str(max_num+1)

        elif part_type=="logical":      
            for part in main_part_list:
                if part.type==2:
                    break
                else:
                    continue
            else:
                print "must have a extend part first"

            if len(logical_part_list)==0:
                self.new_part_path=self.selected_disk.device.path+str(5)
            else:
                max_num=max(logical_part_list.path[-1])
                self.new_part_path=self.selected_disk.device.path+str(max_num+1)
        else:
            print "invalid part type"
            
        return self.new_part_path    

    def update_choose_disk_combo(self):
        '''update the item of choose disk'''
        self.update_selected_disk()

    def update_part_btn_box(self):
        '''when change disk,the partitions display changed'''
        self.update_selected_disk_partitions()
        self.partition_box=gtk.HBox()
        self.partition_box.set_size_request(-1,35)
        total_width=1000
        total_length=self.selected_disk.device.length

        for part in self.selected_disk_partitions:
            part_btn=Button("B")
            part_btn.set_label(part.path)
            part_btn.min_width=total_width*((float)(part.geometry.length)/(float)(total_length))
            #need consider the order of partitions
            if part.type==0:
                self.partition_box.pack_start(part_btn,False,False,1)
            elif part.type==1:
                self.partition_box.pack_start(part_btn,False,False,1)
            else:
                # print "extend,pass"
                pass
            part_btn.connect("clicked",self.on_part_btn_clicked)    
            
        switch_box(self.partition_btns_container_box,self.partition_box)    
        
    def init_part_listview_items(self):
        '''update listview_items,mostly used when change disk or first load'''
        self.update_selected_disk()
        self.disk_partition_info=filter(lambda item:item[0].disk==self.selected_disk,self.part_util.disk_partition_info_tab)
        part_listview_items=[]
        for item in self.disk_partition_info:
            part_list_item=PartListItem(str(item[0].path),str(item[4]),str(item[7]),str(item[5]),"8G","4G",item[2])
            # part_list_item=PartListItem(str(item[0].path),str(item[4]),str(item[7]),CheckButton(),"8G","4G",item[2])
            part_listview_items.append(part_list_item)

        return part_listview_items
    
    def update_part_listview(self):
        '''reload listview because the listview_items changed'''
        self.update_selected_disk_partitions()
        part_listview_box=gtk.VBox()
        part_listview_box.set_size_request(-1,280)
        self.part_listview_items=self.init_part_listview_items()    
        self.part_listview=ListView(
            [(lambda item:item.partition,cmp),
            (lambda item:item.fstype,cmp),
            (lambda item:item.mp,cmp),
            (lambda item:item.format,cmp),
            (lambda item:item.total_size,cmp),
            (lambda item:item.used_size,cmp),
            (lambda item:item.part_type,cmp)
             ]
            )
        self.part_listview.set_expand_column(2)
        self.part_listview.add_titles(["分区","文件系统","挂载点","格式化","总容量","已用容量","类型"])
        self.part_listview.add_items(self.part_listview_items)
        
        # self.part_listview.cell_widths=[100,80,100,60,60,60,100]
        part_scrolled_window=ScrolledWindow()
        part_scrolled_window.add_child(self.part_listview)
        part_listview_box.add(part_scrolled_window)
        part_listview_box.show_all()
        switch_box(self.part_listview_container_box,part_listview_box)

    def add_part_2btn_box(self):
        '''add new added part to part_btn_box'''
        #need do actually add operation first
        self.update_part_btn_box()

    def add_part_2listview(self,part_listview_items):
        '''add new added partition to listview'''
        # need do autually add operation first
        # part_listview_items=[]
        # part_listview_item1=PartListItem("/dev/hda1","ext4","/test","True","8G","4G","primary")
        # part_listview_item2=PartListItem("/dev/hda2","ext4","/test","True","8G","4G","primary")
        # part_listview_items.append(part_listview_item1)
        # part_listview_items.append(part_listview_item2)
        self.part_listview.add_items(part_listview_items)

    def delete_part_from_listview(self):
        '''delete part from listview'''
        #need do real delete operation first
        self.update_part_btn_box()

    def delete_part_form_btn_box(self):
        '''delete part from btn box'''
        #need do real delete operation first
        self.update_part_listview()


#below comes directly operations by UI
    def on_disk_combo_selected(self,widget,event):
        '''change disk,react to partitions display'''
        self.selected_disk=self.update_selected_disk()
        self.update_part_btn_box()
        self.update_part_listview()
    
    def on_part_btn_clicked(self,widget):
        '''react to listview'''
        print "btn clicked"
        self.selected_part_btn=widget
        print widget.label

    def on_part_item_clicked(self):
        '''one part clicked'''
        pass

    def set_part_item_focus(self):
        '''set part_item_focus'''
        pass

    def on_part_new_table_clicked(self,widget):
        '''create new partition table of the selected disk'''
        print "do something first"
        self.update_part_btn_box()
        self.update_part_listview()

    def on_part_new_btn_clicked(self,widget):
        '''create new partition'''
        #update_part_btn_box and listview after add partition operation
        self.part_new=PartNew(self.on_part_new_ok_btn_clicked,self.selected_disk)
        self.part_new.show_all()

    def on_new_table_clicked(self,widget):
        '''clear data in current disk,create new partition table'''
        pass

    def on_part_new_ok_btn_clicked(self,widget):
        '''confirm to add new partition'''
        print "desire to add new partition"
        disk_path=self.selected_disk.device.path

        part_type=self.part_new.part_type_combo.get_current_item().get_label()
        part_type_dick={"","",""}
        part_type=part_type_dick[part_type]
        part_path=self.generate_disk_part_path(part_type)

        part_capacity=self.part_new.part_capacity_spin.get_value()
        part_fs=self.part_new.part_fs_combo.get_current_item().get_label()
        part_format=True
        part_format_str="True"
        part_name=None
        part_location=self.part_new.part_location_combo.get_current_item().get_label()

        part_mp=self.part_new.part_mp_combo.get_current_item().get_label()

        part_listview_items=[[part_path,part_fs,part_mp,part_format_str,part_capacity,"4G",part_type]]
        self.add_part_2listview(part_listview_items)
        self.part_util.add_disk_partition_info_tab(disk_path,part_type,part_capacity,part_fs,part_format,part_name,part_mp)

        # self.add_part_2btn_box()
        

    def on_part_edit_btn_clicked(self,widget):
        '''edit selected partition'''
        self.part_edit=PartEdit()
        self.part_edit.show_all()

    def on_part_edit_ok_btn_clicked(self,widget):
        '''confirm to edit partition'''
        #need also alter info in the backend table
        self.delete_part_from_listview()
        self.add_part_2listview()
        self.update_part_listview()

    def on_part_delete_btn_clicked(self,widget):
        '''delete partition'''
        #need also alter info in the backend table
        self.delete_part_from_listview()
        self.delete_part_form_btn_box()
        self.update_part_btn_box()
        self.update_part_listview()

    def backup_disk_part_table(self):
        '''backup disk part table just to no edit state'''
        pass

    def on_part_recovery_btn_clicked(self,widget):
        '''recovery part info to backup,consider frontend and backend'''
        self.update_part_btn_box()
        self.update_part_listview()

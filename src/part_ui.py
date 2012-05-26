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
from part_new_ui import PartNewUI
from part_edit_ui import PartEditUI

class PartUI(gtk.EventBox):
    '''parttion ui,include tips label,parttion table view,operation buttons '''

    def __init__(self):
        super(PartUI,self).__init__()

        '''partitions:used for tree view display:(plan to merge with part_form)
           partition,fstype,mountpoint,volumetag,size,isformat  '''
        self.partitions=[("/dev/sda1","ext3","/","root","8G","yes"),
                         ("/dev/sda2","ext2","/swap","swap","1G","yes"),
                         ("/dev/sda3","test","test","test","test","yes")
                        ]

        '''partitions_info:used for do partition'''
        self.partitions_info=[[]]

        '''partition_info:used for do partition:for one given partition'''
        self.partition_info=[]
        
        '''selected_device:deepend on user choice'''
        self.selected_device=""
        
        '''partition_info from part_new or service for part_edit;
        keys:device,partition,fs,mp,size,isformat,part_type,volume

        '''
        self.part_form={}

        self.part_vbox=gtk.VBox()
        self.part_disk_hbox=gtk.HBox()
        self.part_operation_hbox=gtk.HBox()

        '''tips label'''
        self.part_disk_label=gtk.Label("磁盘分区设置:")
        self.part_disk_choose_label=gtk.Label("请选择磁盘")
        self.part_disk_combox=gtk.combo_box_new_text()
        self.part_disk_combox.append_text("/dev/sda")
        self.part_disk_combox.append_text("/dev/sdb")
        self.part_disk_combox.append_text("/dev/sdc")
        self.part_disk_combox.set_active(0)
        self.part_auto_radiobtn=gtk.RadioButton(None,"自动分区")
        self.part_man_radiobtn=gtk.RadioButton(self.part_auto_radiobtn,"手动分区")

        self.part_disk_hbox.pack_start(self.part_disk_choose_label,False,False,1)
        self.part_disk_hbox.pack_start(self.part_disk_combox,False,False,1)
        self.part_disk_hbox.pack_start(self.part_auto_radiobtn,False,False,1)
        self.part_disk_hbox.pack_start(self.part_man_radiobtn,False,False,1)


        self.part_disk_label_alignment=gtk.Alignment(0,0,0,0)
        self.part_disk_label_alignment.add(self.part_disk_label)

        '''parttion table view'''
        self.part_tree_store=self.creat_part_tree_model()
        self.part_tree_view=gtk.TreeView(self.part_tree_store)
        self.part_tree_view.set_rules_hint(True)
        self.creat_part_tree_columns()


        '''operation parttion table view buttons '''
        self.part_operation_new=gtk.Button("新建")
        self.part_operation_edit=gtk.Button("修改")
        self.part_operation_delete=gtk.Button("删除")
        self.part_operation_recovery=gtk.Button("还原")
        self.part_operation_new.connect("clicked",self.on_new_btn_clicked)
        self.part_operation_edit.connect("clicked",self.on_edit_btn_clicked)
        self.part_operation_delete.connect("clicked",self.on_delete_btn_clicked)
        self.part_operation_recovery.connect("clicked",self.on_recovery_btn_clicked)

        self.part_operation_hbox.pack_start(self.part_operation_new,False,False,1)
        self.part_operation_hbox.pack_start(self.part_operation_edit,False,False,1)
        self.part_operation_hbox.pack_start(self.part_operation_delete,False,False,1)
        self.part_operation_hbox.pack_start(self.part_operation_recovery,False,False,1)
        

        '''pack tips label,parttion table view,operation buttons'''
        self.part_vbox.pack_start(self.part_disk_label_alignment,False,False,5)
        # self.part_vbox.pack_start(self.part_disk_label,False,False,0)
        self.part_vbox.pack_start(self.part_disk_hbox,False,False,3)
        self.part_vbox.pack_start(self.part_tree_view,False,False,0)
        self.part_vbox.pack_start(self.part_operation_hbox,False,False,0)

        self.set_btn_sensitive(self.part_operation_edit,False)
        self.part_tree_view.connect("row-activated",self.on_part_row_activated)


    def creat_part_tree_columns(self):
        '''creat part tree columns:partitions,filesystem,mountpoint,volume,size,isformat'''

        partition=gtk.TreeViewColumn()
        partition.set_title("partition")
        cell=gtk.CellRendererText()
        partition.pack_start(cell,True)
        partition.add_attribute(cell,"text",0)
        self.part_tree_view.append_column(partition)

        filesystem=gtk.TreeViewColumn()
        filesystem.set_title("filesystem")
        cell=gtk.CellRendererText()
        filesystem.pack_start(cell,True)
        filesystem.add_attribute(cell,"text",1)
        self.part_tree_view.append_column(filesystem)

        mountpoint=gtk.TreeViewColumn()
        mountpoint.set_title("mountpoint")
        cell=gtk.CellRendererText()
        mountpoint.pack_start(cell,True)
        mountpoint.add_attribute(cell,"text",2)
        self.part_tree_view.append_column(mountpoint)

        volume=gtk.TreeViewColumn()
        volume.set_title("volume")
        cell=gtk.CellRendererText()
        volume.pack_start(cell,True)
        volume.add_attribute(cell,"text",3)
        self.part_tree_view.append_column(volume)

        size=gtk.TreeViewColumn()
        size.set_title("size")
        cell=gtk.CellRendererText()
        size.pack_start(cell,True)
        size.add_attribute(cell,"text",4)
        self.part_tree_view.append_column(size)

        isformat=gtk.TreeViewColumn()
        isformat.set_title("isformat")
        cell=gtk.CellRendererText()
        isformat.pack_start(cell,True)
        isformat.add_attribute(cell,"text",5)
        self.part_tree_view.append_column(isformat)


    def creat_part_tree_model(self):
        '''create part tree model'''
        self.part_tree_store=gtk.TreeStore(str,str,str,str,str,str)
        
        # self.insert_part_tree_father()
        # self.insert_part_tree_child()
        return self.part_tree_store    

    def insert_part_tree_father(self):
        '''add primary partitions /extend to the table'''
        # for part in self.partitions:
        #     # self.part_tree_store.append(None,part[0],part[1],part[2],part[3],part[4],part[5])
        #     self.part_tree_store.append(None,part)
        self.get_part_form()
        part_form_view_list=[self.part_form['partition'],
                             self.part_form['fstype'],
                             self.part_form['partsize'],
                             self.part_form['mp'],
                             self.part_form['volume'],
                             self.part_form['isformat']
                             ]
        if self.part_form['parttype']=="primary":
            self.part_tree_store.append(None,part_form_view_list)

        elif self.part_form['parttype']=="extend":
            piter=self.part_tree_store.append(None,part_form_view_list)
            self.insert_part_tree_child(piter,part_form_view_list)
            
        else:
            pass

    def insert_part_tree_child(self,piter):
        '''add logic partitions to the extend partition table'''
        part_form_view_list=[self.part_form['partition'],
                             self.part_form['fstype'],
                             self.part_form['partsize'],
                             self.part_form['mp'],
                             self.part_form['volume'],
                             self.part_form['isformat']
                             ]
        if self.part_form['parttype']=="logic":
            self.part_tree_store.append(piter,part_form_view_list)

    def get_part_form(self):
        '''get part_form data from part_new_ui to add a partition'''
        for key in PartNewUI().part_new_form.keys():
            self.part_form[key]=PartNewUI().part_new_form[key]
        
        self.part_form['device']=self.get_selected_device()
        self.part_form['partition']="#this need to be completed!!!!!"
        return self.part_form

    def set_part_form(self):
        '''fill part_form data to service for part_edit_ui'''
        # self.get_part_from_view()
        # for key in PartEditUI()part_edit_form.keys():
        #     self.part_edit_form[key]=self.part_form[key]
        
    def get_part_from_view(self):
        '''get part data from tree view to fill part form'''
        ###this to be completed
        pass


    def add_part_to_view(self):
        '''add data from add button operation to the tree view'''
        pass

    def delete_part_from_view(self):
        '''delete data from tree view by press delete button'''
        pass
    

    def get_selected_device(self):
        '''get selected_device'''

        self.selected_device=self.part_disk_combox.get_active_text()
        return self.selected_device

    def on_part_row_activated(self,widget,column,event):
        '''when double clicked on the selected row'''
        PartEditUI()

    def on_new_btn_clicked(self,widget):
        PartNewUI()
        self.add_part_to_view()
                            
    def on_edit_btn_clicked(self,widget):
        PartEditUI()

    def on_delete_btn_clicked(self,widget):
        pass

    def on_recovery_btn_clicked(self,widget):
        pass

    def set_btn_sensitive(self,btn,sensitive):
        
        buttons=[self.part_operation_new,self.part_operation_edit,self.part_operation_delete,self.part_operation_recovery]
        
        if btn in buttons:
            btn.set_sensitive(sensitive)
        else:
            print "error occurs"
            
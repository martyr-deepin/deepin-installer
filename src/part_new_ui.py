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

class PartNewUI(gtk.Window):
    
    def __init__(self):
        super(PartNewUI,self).__init__()

        self.set_size_request(400,300)
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_title("new partition")
        
        self.part_new_form={}
        
        self.part_new_vbox=gtk.VBox(1)
        self.part_new_label=gtk.Label("new partition")
        
        self.part_type_hbox=gtk.HBox(1)
        self.part_type_label=gtk.Label("please choose part type:")
        self.part_type_combo=gtk.combo_box_new_text()
        self.part_type_combo.append_text("primary")
        self.part_type_combo.append_text("logic")
        self.part_type_combo.append_text("extend")
        self.part_type_hbox.pack_start(self.part_type_label)
        self.part_type_hbox.pack_start(self.part_type_combo)

        self.part_size_hbox=gtk.HBox(1)
        self.part_size_label=gtk.Label("please set part size")
        self.part_size_entry=gtk.Entry()
        self.part_size_hbox.pack_start(self.part_size_label)
        self.part_size_hbox.pack_start(self.part_size_entry)

        self.part_fstype_hbox=gtk.HBox(1)
        self.part_fstype_label=gtk.Label("please choose fs type:")
        self.part_fstype_combo=gtk.combo_box_new_text()
        self.part_fstype_combo.append_text("ext3")
        self.part_fstype_combo.append_text("ext4")
        self.part_fstype_combo.append_text("ext2")
        self.part_fstype_hbox.pack_start(self.part_fstype_label)
        self.part_fstype_hbox.pack_start(self.part_fstype_combo)

        self.part_volume_hbox=gtk.HBox(1)
        self.part_volume_label=gtk.Label("set tag:")
        self.part_volume_entry=gtk.Entry()
        self.part_volume_hbox.pack_start(self.part_volume_label)
        self.part_volume_hbox.pack_start(self.part_volume_entry)


        self.part_isformat_hbox=gtk.HBox(1)
        self.part_isformat_label=gtk.Label("is format:")
        self.part_isformat_checkbtn=gtk.CheckButton("is formattttt")
        self.part_isformat_hbox.pack_start(self.part_isformat_label)
        self.part_isformat_hbox.pack_start(self.part_isformat_checkbtn)
        # self.part_isformat_checkbtn.connect("toggled",self.on_format_toggled)
                       
        self.part_mp_hbox=gtk.HBox(1)
        self.part_mp_label=gtk.Label("please choose mount point:")
        self.part_mp_combo=gtk.combo_box_new_text()
        self.part_mp_combo.append_text("/")
        self.part_mp_combo.append_text("/swap")
        self.part_mp_combo.append_text("/home")
        self.part_mp_hbox.pack_start(self.part_mp_label)
        self.part_mp_hbox.pack_start(self.part_mp_combo)

        self.part_ok_hbox=gtk.HBox(1)
        self.part_ok_btn=gtk.Button("OK")
        self.part_cancel_btn=gtk.Button("Cancel")
        self.part_ok_hbox.pack_start(self.part_ok_btn)
        self.part_ok_hbox.pack_start(self.part_cancel_btn)

        self.part_new_vbox.pack_start(self.part_new_label)
        self.part_new_vbox.pack_start(self.part_type_hbox)
        self.part_new_vbox.pack_start(self.part_size_hbox)
        self.part_new_vbox.pack_start(self.part_fstype_hbox)
        self.part_new_vbox.pack_start(self.part_volume_hbox)
        self.part_new_vbox.pack_start(self.part_isformat_hbox)
        self.part_new_vbox.pack_start(self.part_mp_hbox)
        self.part_new_vbox.pack_start(self.part_ok_hbox)

        self.add(self.part_new_vbox)
        # self.connect("delete-event",self.delete_event)
        self.part_ok_btn.connect("clicked",self.on_ok_btn_clicked)
        self.part_cancel_btn.connect("clicked",self.on_cancle_btn_clicked)
        self.connect("destroy",lambda w: self.hide_all())
        self.show_all()
    
    def get_part_type(self):
        return self.part_type_combo.get_active_text()

    def get_part_fs(self):
        return self.part_fstype_combo.get_active_text()

    def get_part_size(self):
        return self.part_size_entry.get_text()

    def get_part_mp(self):
        return self.part_mp_combo.get_active_text()

    def get_part_volume(self):
        return self.part_volume_entry.get_text()

    def get_part_isformat(self):
        '''this may need fix'''
        # return self.part_isformat_checkbtn.is_toggled
        return True

    def set_part_new_form(self):
        '''put user new part data into part_new_form'''
        self.part_new_form['parttype']=self.get_part_type()
        self.part_new_form['fstype']=self.get_part_fs()
        self.part_new_form['partsize']=self.get_part_size()
        self.part_new_form['mp']=self.get_part_mp()
        self.part_new_form['volume']=self.get_part_volume()
        self.part_new_form['isformat']=self.get_part_isformat()

    def check_part_form(self):
        '''check user set validation'''
        pass

    # def on_format_toggled(self):
    #     self.part_isformat_checkbtn.is_toggled=True
        
    def on_ok_btn_clicked(self,widget):
        '''insert the partition to the tree view '''
        self.check_part_form()
        self.set_part_new_form()
       

    def on_cancle_btn_clicked(self,widget,event):
        self.delete_event(widget,event)

    def delete_event(self,widget,event):
        gtk.main_quit()
        return False

if __name__=="__main__":
    PartNewUI()
    gtk.main()
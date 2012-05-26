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

class PartEditUI(gtk.Window):
    
    def __init__(self):
        super(PartEditUI,self).__init__()

        self.set_size_request(400,300)
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_title("edit partition")

        self.part_new_vbox=gtk.VBox(1)
        self.part_new_label=gtk.Label("edit partition")
        
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
        self.part_new_vbox.pack_start(self.part_mp_hbox)
        self.part_new_vbox.pack_start(self.part_ok_hbox)

        self.add(self.part_new_vbox)
        self.connect("destroy",gtk.main_quit)
        self.show_all()

if __name__=="__main__":
    PartEditUI()
    gtk.main()
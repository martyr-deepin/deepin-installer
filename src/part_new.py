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
from dtk.ui.window import Window
from dtk.ui.combo import ComboBox,ComboBoxItem
from dtk.ui.label import Label

class PartNew():
    '''create new partition UI'''
    def __init__(self):
        self.window=Window()

        self.new_part_box=gtk.VBox()
        
        self.part_type_box=gtk.HBox()
        self.part_type_label=Label("新分区类型：")
        self.part_type_combo=ComboBox()
        self.part_type_primary=ComboBoxItem("主分区",None)
        self.part_type_extended=ComboBoxItem("逻辑分区",None)
        self.part_type_logical=ComboBoxItem("扩展分区",None)
        self.part_type_combo.add_item(self.part_type_primary)
        self.part_type_combo.add_item(self.part_type_extended)
        self.part_type_combo.add_item(self.part_type_logical)
        self.part_type_box.pack_start(self.part_type_label)
        self.part_type_box.pack_start(self.part_type_combo)

        self.part_capacity_box=gtk.Hbox()
        self.part_capacity_label=Label("新分区容量:")
        self.part_capacity_combo=ComboBox()
        self.part_capacity_box.pack_start(self.part_capacity_label)
        self.part_capacity_box.pack_start(self.part_capacity_combo)

        self.part_location_box=gtk.HBox()
        self.part_location_label=Label("新分区位置:")
        self.part_location_combo=ComboBox()
        self.part_location_begin=ComboBoxItem("起始")
        self.part_location_end=ComboBoxItem("结束")
        self.part_location_box.pack_start(self.part_location_label)
        self.part_location_box.pack_start(self.part_location_combo)


        self.part_mp_box=gtk.HBox()
        self.part_mp_label=Label("挂载点：")
        self.part_mp_combo=ComboBox()
        self.part_mp_box.pack_start(self.part_mp_label)
        self.part_mp_box.pack_start(self.part_mp_combo)

        self.part_fs_box=gtk.HBox()
        self.part_fs_label=Label("用于：")
        self.part_fs_combo=ComboBox()
        self.part_fs_ext4=ComboBoxItem("ext4",None)
        self.part_fs_combo.add_item(self.part_fs_ext4)
        self.part_fs_box.pack_start(self.part_fs_label)
        self.part_fs_box.pack_start(self.part_fs_combo)

        
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
from dtk.ui.combo import ComboBox,ComboItem
from dtk.ui.button import Button
from dtk.ui.listview import ListView


class Part():
    '''Part UI'''
    def __init__(self):

        self.choose_disk_box=gtk.HBox()
        self.choose_disk_label=Label("选择硬盘")
        self.choose_disk_combo=ComboBox()
        self.choose_disk1=ComboItem("/dev/sda",None)
        self.choose_disk2=ComboItem("/dev/sdb",None)
        self.choose_disk_combo.add_item(self.choose_disk1)
        self.choose_disk_combo.add_item(self.choose_disk2)

        self.choose_disk_box.pack_start(self.choose_disk_label,True,True,4)
        self.choose_disk_box.pack_start(self.choose_disk_combo,True,True,4)


        self.partition_box=gtk.HBox(True)
        self.partition_btn1=Button("/dev/sda1")
        self.partition_btn2=Button("/dev/sda2")
        self.partition_btn3=Button("/dev/sda5")
        self.partition_box.pack_start(self.partition_btn1)
        self.partition_box.pack_start(self.partition_btn2)
        self.partition_box.pack_start(self.partition_btn3)

        
        
        
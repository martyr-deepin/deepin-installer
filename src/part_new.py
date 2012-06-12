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

from dtk.ui.skin_config import skin_config
from dtk.ui.theme import Theme,ui_theme
from dtk.ui.utils import get_parent_dir
import os


# Init skin config.
skin_config.init_skin(
    "12",
    os.path.join(get_parent_dir(__file__, 2), "skin"),
    os.path.expanduser("~/.config/deepin-installer/skin"),
    os.path.expanduser("~/.config/deepin-installer/skin_config.ini"),
    )

# Create application theme.
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 2), "app_theme"),
    os.path.expanduser("~/.config/deepin-installer/theme")
    )

# Set theme.
skin_config.load_themes(ui_theme, app_theme)






import gtk

from dtk.ui.combo import ComboBox,ComboBoxItem
from dtk.ui.label import Label
from dtk.ui.popup_window import PopupWindow

class PartNew(PopupWindow):
    '''create new partition UI'''
    def __init__(self):
        
        super(PartNew,self).__init__()


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

        self.part_capacity_box=gtk.HBox()
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

        self.new_part_box.pack_start(self.part_type_box)
        self.new_part_box.pack_start(self.part_capacity_box)
        self.new_part_box.pack_start(self.part_location_box)
        self.new_part_box.pack_start(self.part_mp_box)
        self.new_part_box.pack_start(self.part_fs_box)


if __name__=="__main__":


    PartNew()
    gtk.main()
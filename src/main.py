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
from dtk.ui.application import Application
# from dtk.ui.label import Label
# from dtk.ui.frame import HorizontalFrame
# from dtk.ui.button import Button
from welcome import Welcome
from part import Part

class DeepinInstaller():
    '''deepin installer'''
    def __init__(self):
        self.application=Application("installer")
        self.application.set_default_size(756,520)
        self.application.add_titlebar(["min", "close"],None,"欢迎使用深度软件",None,False)
        self.application.titlebar.set_size_request(-1,50)

        self.content_box=gtk.VBox()
        self.content_box.set_size_request(-1,420)
        self.step_box=gtk.HBox()
        self.step_box.set_size_request(-1,50)

        self.content_box.pack_start(Welcome().install_frame,True,True,4)
        # self.step_box.pack_start(Welcome().step_box_frame,True,True,4)
        self.step_box.pack_start(Part().step_box_frame,True,True,4)

        self.application.main_box.pack_start(self.content_box)
        self.application.main_box.pack_start(self.step_box)
        self.application.window.enable_resize=False
        self.application.run()



if __name__=="__main__":
    di=DeepinInstaller()

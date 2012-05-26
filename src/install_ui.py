#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012~2013 Deepin, Inc.
#               2012~2013 Long Wei
#v
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
from deepin_demonstrate_ui import DeepinDemonstrateUI
from install_progress_ui import InstallProgressUI
from finish_restart_ui import FinishRestartUI

class InstallUI(gtk.Window):
    ''' Install demonstrate ,progress ...'''
    def __init__(self):
        super(InstallUI,self).__init__()

        self.install_ui_vbox=gtk.VBox(1)
        self.set_title("Welcome to use linux deepin")
        self.set_size_request(750,500)
        self.set_position(gtk.WIN_POS_CENTER)

        self.demonstrate_ui=DeepinDemonstrateUI().demonstrate_vbox
        self.install_progress_ui=InstallProgressUI().install_progress_vbox
        self.finish_restart_ui=FinishRestartUI().finish_restart_hbox

        self.install_ui_vbox.pack_start(self.demonstrate_ui,False,False,1)
        self.install_ui_vbox.pack_start(self.install_progress_ui,False,False,1)
        self.install_ui_vbox.pack_start(self.finish_restart_ui,False,False,1)

        self.add(self.install_ui_vbox)
        self.connect("destroy",gtk.main_quit)
        self.show_all()

if __name__=="__main__":
    InstallUI()
    gtk.main()

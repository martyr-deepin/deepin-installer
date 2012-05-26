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

from locale_ui import LocaleUI
from user_ui import UserUI
from part_ui import PartUI
from confirm_install_ui import ConfirmInstallUI
from setupinfo_msg_ui import SetupInfoMsgUI

class SetupInfoUI(gtk.Window):
    '''user interface to set locale,account,parttions'''

    def __init__(self):
        
        super(SetupInfoUI,self).__init__()
        self.set_size_request(750,500)
        self.set_title("Welcome to install linux deepin")
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_title("Welcome to install linux deepin")

        self.locale_ui=LocaleUI().locale_box
        self.user_ui=UserUI().user_vbox
        self.part_ui=PartUI().part_vbox
        self.msg_ui=SetupInfoMsgUI().setupinfo_msg_hbox
        self.confirm_ui=ConfirmInstallUI().confirm_install_alignment

        self.pack_box=gtk.VBox(False,1)
        self.pack_box.pack_start(self.locale_ui,False,True,14)
        self.pack_box.pack_start(self.user_ui,False,True,14)
        self.pack_box.pack_start(self.part_ui,False,True,14)
        self.pack_box.pack_start(self.msg_ui,False,True,1)
        self.pack_box.pack_start(self.confirm_ui,False,True,1)



        self.connect("destroy",gtk.main_quit)

        self.add(self.pack_box)
        self.show_all()

        

if __name__=="__main__":
    SetupInfoUI()    
    gtk.main()
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
from install_ui import InstallUI

class ConfirmInstallUI(gtk.EventBox):
    
    def __init__(self):
        super(ConfirmInstallUI,self).__init__()

        self.confirm_install_hbox=gtk.HBox(14)
        self.confirm_install_btn=gtk.Button("开始安装",stock=gtk.STOCK_OK)
        self.cancel_install_btn=gtk.Button("取消安装",stock=gtk.STOCK_CANCEL)

        self.confirm_install_hbox.add(self.confirm_install_btn)
        self.confirm_install_hbox.add(self.cancel_install_btn)

        self.confirm_install_alignment=gtk.Alignment(1,0,0,0)
        self.confirm_install_alignment.add(self.confirm_install_hbox)

        self.confirm_install_btn.connect("clicked",self.on_confirm_btn_clicked)
        self.cancel_install_btn.connect("clicked",gtk.main_quit)
        

    def on_confirm_btn_clicked(self,widget):
        InstallUI()


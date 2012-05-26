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

class InstallProgressUI(gtk.EventBox):

    def __init__(self):
        super(InstallProgressUI,self).__init__()

        self.install_progress_vbox=gtk.VBox(10)

        self.install_progress_pgb=gtk.ProgressBar()


        self.install_progress_label=gtk.Label("系统正在安装中，请稍候...")
        
        self.install_progress_vbox.pack_start(self.install_progress_label,True,True,50)
        self.install_progress_vbox.pack_start(self.install_progress_pgb,False,False,51)


    def get_progress_label_text(self):
        return self.install_progress_label.get_text()

    def set_progress_label_text(self,text):
        self.install_progress_label.set_text(text)

    def set_progress_label_show(self,Flag):

        if Flag:
            self.install_progress_label.show()
        else:
            self.install_progress_label.hide()

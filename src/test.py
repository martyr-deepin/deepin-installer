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
import gio

class CopyTest(gtk.Window):
    def __init__(self):
        super(CopyTest,self).__init__()
        self.set_size_request(300,200)
        self.set_title("Copy Test")
        self.set_position(gtk.WIN_POS_CENTER)
        self.vbox=gtk.VBox(False,1)
        
        self.progressbar=gtk.ProgressBar(None)

        self.src=gio.File(path="test.py")
        self.dst=gio.File(path="test_dst.py")

        gio.File.copy_async(self.dst,self.call_back,self.progress_callback,G_FILECOPY_OVERWRITE,G_PRIORITY_DEFAULT,NULL,user_data,call_back data,)

        self.vbox.pack_start(self.progressbar,True,True,5)
        self.add(self.vbox)
        self.connect("destroy",gtk.main_quit)
        self.show_all()

if __name__=="__main__":
    CopyTest()
    gtk.main()
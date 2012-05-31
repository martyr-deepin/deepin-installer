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
import glib

class CopyUtil():
    '''copy files'''
    def __init__(self,src,dst):
        self.src=gio.File(src)
        self.dst=gio.File(dst)
        info=self.src.query_info(gio.FILE_ATTRIBUTE_STANDARD_SIZE)
        self.total_num_bytes=info.get_attribute_uint64(gio.FILE_ATTRIBUTE_STANDARD_SIZE)
        print self.total_num_bytes

        self.window=gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_size_request(400,300)
        self.window.set_title("Copy Util")
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.vbox=gtk.VBox(False,1)
        self.progressbar=gtk.ProgressBar()
        self.progressbar.set_text("Copying......")
        
        self.vbox.pack_start(self.progressbar)
        self.window.add(self.vbox)
        self.window.show_all()
        self.window.connect("destroy",gtk.main_quit)


    def start_copy(self):
        self.src.copy_async(self.dst,self.finish_copy_callback,self.progress_copy_callback,gio.FILE_COPY_OVERWRITE,glib.PRIORITY_DEFAULT,None,self.progressbar,self.progressbar)
        gtk.main()

    def finish_copy_callback(self,srcfile,result,progressbar):
        self.src.copy_finish(result)
        self.progressbar.set_text("Finished Copy")
        self.progressbar.set_fraction(1.0)
        print "finish copy"


    def progress_copy_callback(self,current_num_bytes,total_num_bytes,progressbar):
        total_num_bytes=self.total_num_bytes

        self.progressbar.set_text(str(current_num_bytes/1024)+"K/"+str(total_num_bytes/1024)+"K")
        self.progressbar.set_fraction(current_num_bytes/total_num_bytes)



if __name__=="__main__":
    cu=CopyUtil("deepin.iso","deepin_copy.iso")
    cu.start_copy()


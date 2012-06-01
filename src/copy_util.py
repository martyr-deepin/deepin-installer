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

import gio
import os
import gtk
import glib

class CopyUtil():
    
    def __init__(self):
        self.source=None
        self.target=None
        self.progressbar=None
        self.copy_callback_data=""
        self.progress_callback_data=""
        self.total_bytes=0
        self.current_bytes=0
        self.end_bytes=0
        self.filelist=[]

    def get_source_gfilelist(self,source_path):
        '''get source gfilelist'''
        self.source=gio.File(source_path)
        source_type=self.source.query_info("standard::type",gio.FILE_QUERY_INFO_NOFOLLOW_SYMLINKS).get_file_type()
        if source_type==gio.FILE_TYPE_DIRECTORY:
            for dirpath,dirnames,filenames in os.walk(source_path):
                for dirname in dirnames:
                    fullpath=os.path.join(dirpath,dirname)
                    self.filelist.append(gio.File(fullpath))
                    
                for filename in filenames:
                    fullpath=os.path.join(dirpath,filename)
                    self.filelist.append(gio.File(fullpath))

        else:            
            self.filelist.append(self.source)
        return self.filelist    

    def get_total_num_bytes(self,filelist):
        '''return the size of source_path'''
        for f in filelist:
            fi=f.query_info(gio.FILE_ATTRIBUTE_STANDARD_SIZE)
            fsize=fi.get_attribute_uint64(gio.FILE_ATTRIBUTE_STANDARD_SIZE)
            self.total_bytes+=fsize

        return self.total_bytes
    
    def get_target_gfile(self,srcfile,target):
        '''get target gfile object'''
        relative_path=self.source.get_relative_path(srcfile)
        target_gfile=target.resolve_relative_path(relative_path)

        return target_gfile    

    def copy_list(self,filelist,target_path):
        '''copy all gfile object to the given target_path'''
        target=gio.File(target_path)
        target_type=target.query_info("standard::type",gio.FILE_QUERY_INFO_NOFOLLOW_SYMLINKS).get_file_type()

        if target_type==gio.FILE_TYPE_DIRECTORY:
            for f in filelist:
                target_gfile=self.get_target_gfile(f,target)

                if f.query_info("standard::type",gio.FILE_QUERY_INFO_NOFOLLOW_SYMLINKS).get_file_type()==gio.FILE_TYPE_DIRECTORY:
                    if target_gfile.query_exists(None):
                        continue
                    if not target_gfile.get_parent():
                        target_gfile.make_directory_with_parents()
                    else:
                        target_gfile.make_directory()
                else:        
                    if target_gfile.query_exists():
                        target_gfile.delete()
                    self.start_copy(f,target_gfile)

        else:
            print "must supply a directory to copy data"

    def start_copy(self,srcfile,dstfile):
        srcfile.copy_async(dstfile,self.finish_copy_callback,self.progress_copy_callback,
                           flags=gio.FILE_COPY_OVERWRITE,io_priority=glib.PRIORITY_DEFAULT,cancellable=None,
                           user_data=self.copy_callback_data,
                           progress_callback_data=self.progress_callback_data)
        # print "start copy "+srcfile.get_path()+" to "+dstfile.get_path()


    def finish_copy_callback(self,srcfile,result,copy_callback_data):
        '''finish copy a single file or directory'''
        print "finish copy:"+srcfile.get_path()
        srcfile.copy_finish(result)
        # self.progressbar.set_text("Finished Copy")
        # self.progressbar.set_fraction(1.0)

    def progress_copy_callback(self,current_num_bytes,end_num_bytes,progress_callback_data):
        # self.global_current_bytes=self.end_bytes+current_num_bytes
        # self.end_bytes+=end_num_bytes
        # self.progressbar.set_text(str(self.global_current_bytes/1024)+"K/"+str(self.total_bytes)+"K:"
        #                           +str(self.global_current_bytes*100/self.total_bytes*1024)+"%")
        # self.progressbar.set_fraction(self.global_current_bytes/self.total_bytes*1024)
        self.progressbar.set_text(str(current_num_bytes)+" / "+str(end_num_bytes))
        self.progressbar.set_fraction(current_num_bytes/end_num_bytes)
        print "current_num_bytes:"
        print current_num_bytes
        print "end_num_bytes:"
        print end_num_bytes

    def test_copy_single(self):
        self.window=gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_size_request(400,300)
        self.window.set_title("Copy Util")
        self.window.set_position(gtk.WIN_POS_CENTER)
        self.vbox=gtk.VBox(False,1)
        self.label=gtk.Label("Copy Util")

        self.progressbar=gtk.ProgressBar()
        self.progressbar.set_text("Copying......")
        
        self.vbox.pack_start(self.label)
        self.vbox.pack_start(self.progressbar)
        self.window.add(self.vbox)
        self.window.connect("destroy",gtk.main_quit)
        self.window.show_all()


        srcfile=gio.File("deepin.iso")
        dstfile=gio.File("deepin_copy.iso")

        self.start_copy(srcfile,dstfile)
        
if __name__=="__main__":
    cu=CopyUtil()
    # filelist=cu.get_source_gfilelist("/home/yilang/Project/deepin-installer")
    # cu.total_bytes=cu.get_total_num_bytes(filelist)

    # cu.copy_list(filelist,"/home/yilang/test")

    cu.test_copy_single()
    gtk.main()
    # cu.get_source_gfilelist("/home/yilang/Project/deepin-installer")
    
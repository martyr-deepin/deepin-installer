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

class DeepinDemonstrateUI(gtk.EventBox):
    '''Demonstrate linux deepin os,software,desktop,etc,,,'''
    def __init__(self):
        super(DeepinDemonstrateUI,self).__init__()

        self.demonstrate_vbox=gtk.VBox(0)
        try:
            
            self.demonstrate_welcom_pixbuf=gtk.gdk.pixbuf_new_from_file("test.png")
        except Exception,e:
            print e.message

        self.demonstrate_image_welcom=gtk.Image()
        self.demonstrate_image_welcom.set_from_pixbuf(self.demonstrate_welcom_pixbuf)


        self.demonstrate_togglebtn_welcom=gtk.ToggleButton("welcome")
        self.demonstrate_togglebtn_music=gtk.ToggleButton("music")
        self.demonstrate_togglebtn_film=gtk.ToggleButton("film")
        self.demonstrate_togglebtn_office=gtk.ToggleButton("office")
        self.demonstrate_togglebtn_desktop=gtk.ToggleButton("desktop")
        self.demonstrate_togglebtn_scrot=gtk.ToggleButton("scrot")
        self.demonstrate_togglebtn_ui=gtk.ToggleButton("ui")
        self.demonstrate_togglebtn_rss=gtk.ToggleButton("rss")




        self.demonstrate_togglebtn_hbox=gtk.HBox(True,0)
        self.demonstrate_togglebtn_alignment=gtk.Alignment(0.8,0,0,0)
        self.demonstrate_togglebtn_alignment.add(self.demonstrate_togglebtn_hbox)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_welcom)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_music)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_film)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_office)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_desktop)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_scrot)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_ui)
        self.demonstrate_togglebtn_hbox.pack_start(self.demonstrate_togglebtn_rss)



        self.demonstrate_vbox.pack_start(self.demonstrate_image_welcom,False,False,1)
        self.demonstrate_vbox.pack_start(self.demonstrate_togglebtn_alignment,False,False,0)

        self.show_all()





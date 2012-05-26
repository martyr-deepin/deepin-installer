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

import cairo
import gtk
import math


class Deepin(gtk.Window):
    '''Test for install progress'''
    
    def __init__(self):
        super(Deepin,self).__init__()
        self.set_size_request(750,500)
        self.set_position(gtk.WIN_POS_CENTER)
        self.set_title("Install Deepin Progress")

        self.darea=gtk.DrawingArea()
        self.darea.connect("expose-event",self.draw_text)
        
        self.add(self.darea)
        self.connect("destroy",gtk.main_quit)
        self.show_all()


    def draw_text(self,widget,event):
        
        cr=widget.window.cairo_create()
        cr.move_to(50,150)
        # cr.set_source_rgb(0.3,0.4,0.6)
        # cr.fill()
        cr.set_font_size(100)
        cr.text_path("Linux Deepin")
        cr.set_line_width(0.5)
        cr.set_source_rgb(0.3,0.4,0.6)
        cr.stroke_preserve()
        cr.set_source_rgb(1,1,1)
        cr.fill()

if __name__=="__main__":
    Deepin()
    gtk.main()

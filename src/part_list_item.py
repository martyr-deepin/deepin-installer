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


import gobject

from dtk.ui.constant import ALIGN_START
from dtk.ui.utils import get_content_size
from dtk.ui.constant import DEFAULT_FONT_SIZE,ALIGN_END
from dtk.ui.draw import draw_text,draw_pixbuf
from dtk.ui.theme import ui_theme
from part_util import global_part_util
from dtk.ui.button import CheckButton

class PartListItem(gobject.GObject):
    '''part list item'''
        
    __gsignals__ = {
        "redraw-request" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, ()),
    }

    def __init__(self,disk,geom_item,partition,mp,fstype,format,total_size,part_type):
        gobject.GObject.__init__(self)
        self.update(disk,geom_item,partition,mp,fstype,format,total_size,part_type)
        self.index = None
        
    def set_index(self, index):
        '''Update index.'''
        self.index = index
        
    def get_index(self):
        '''Get index.'''
        return self.index

    def update(self,disk,geom_item,partition,mp,fstype,format,total_size,part_type):
        '''update item'''
        self.disk=disk
        self.geom_item=geom_item
        self.partition=partition

        # print self.partition.disk
        self.part_path=global_part_util.disk_part_display_path[self.disk][self.partition]
        self.fstype=fstype
        self.mp=mp
        self.format=format
        self.total_size=total_size
        self.part_type=part_type

        # Calculate item size.
        self.partition_padding_x = 10
        self.partition_padding_y = 5
        (self.partition_width, self.partition_height) = get_content_size(self.part_path, DEFAULT_FONT_SIZE)
        
        self.fstype_padding_x = 10
        self.fstype_padding_y = 5
        (self.fstype_width, self.fstype_height) = get_content_size(self.fstype, DEFAULT_FONT_SIZE)

        self.mp_padding_x = 10
        self.mp_padding_y = 5
        (self.mp_width, self.mp_height) = get_content_size(self.mp, DEFAULT_FONT_SIZE)

        self.format_padding_x = 10
        self.format_padding_y = 5
        (self.format_width, self.format_height) = get_content_size(self.format, DEFAULT_FONT_SIZE)

        self.total_size_padding_x = 10
        self.total_size_padding_y = 5
        (self.total_size_width, self.total_size_height) = get_content_size(self.total_size, DEFAULT_FONT_SIZE)

        self.part_type_padding_x = 10
        self.part_type_padding_y = 5
        (self.part_type_width, self.part_type_height) = get_content_size(self.part_type, DEFAULT_FONT_SIZE)
        
    def emit_redraw_request(self):
        '''Emit redraw-request signal.'''
        self.emit("redraw-request")

    def get_column_sizes(self):
        '''Get sizes.'''
        return [(self.partition_width + self.partition_padding_x * 2,
                 self.partition_height + self.partition_padding_y * 2),
                (self.mp_width + self.mp_padding_x * 2, 
                 self.mp_height + self.mp_padding_y * 2),
                (self.fstype_width + self.fstype_padding_x * 2, 
                 self.fstype_height + self.fstype_padding_y * 2),
                (self.format_width + self.format_padding_x * 2, 
                 self.format_height + self.format_padding_y * 2),
                (self.total_size_width + self.total_size_padding_x * 2, 
                 self.total_size_height + self.total_size_padding_y * 2),
                (self.part_type_width + self.part_type_padding_x * 2, 
                 self.part_type_height + self.part_type_padding_y * 2),
                ]    
        
    def render_partition(self, cr, rect ,in_select , in_highlight):
        '''Render partition.'''
        rect.x += self.partition_padding_x
        rect.width -= self.partition_padding_x * 2
        render_text(cr, rect, self.part_path ,in_select , in_highlight)

    def render_fstype(self, cr, rect ,in_select , in_highlight):
        '''Render  fstype.'''
        rect.x += self.fstype_padding_x
        rect.width -= self.fstype_padding_x * 2
        render_text(cr, rect, self.fstype ,in_select , in_highlight)

    def render_mp(self, cr, rect ,in_select ,in_highlight):
        '''Render mp.'''
        rect.x += self.mp_padding_x
        rect.width -= self.mp_padding_x * 2
        render_text(cr, rect, self.mp ,in_select ,in_highlight)

    def render_format(self, cr, rect ,in_select ,in_highlight):
        '''Render  format.'''
        rect.x += self.format_padding_x
        rect.width -= self.format_padding_x * 2
        render_text(cr, rect, self.format ,in_select ,in_highlight)
        # self.render_toggle(cr, rect, self.format)

    def render_total_size(self, cr, rect ,in_select ,in_highlight):
        '''Render  total_size.'''
        rect.x += self.total_size_padding_x
        rect.width -= self.total_size_padding_x * 2
        render_text(cr, rect, self.total_size ,in_select ,in_highlight)

    def render_part_type(self, cr, rect ,in_select ,in_highlight):
        '''Render part_type.'''
        rect.width -=self.part_type_padding_x * 2
        render_text(cr, rect, self.part_type,in_select ,in_highlight ,align=ALIGN_END)
    
    def get_renders(self):
        '''Get render callbacks.'''
        return [self.render_partition,
                self.render_mp,
                self.render_fstype,
                self.render_format,
                self.render_total_size,
                self.render_part_type]

    def render_toggle(self,cr,rect,format=False):
        '''Render CheckBox button'''
        self.format_checkbtn=CheckButton()
        self.format_checkbtn.connect("toggled",self.on_format_toggled)
        
    def on_format_toggled(self,widget,event):
        '''on format button toggled'''
        if self.format==True:
            self.format=False
        elif self.format==False:
            self.format=True
        else:
            self.format=False

def render_text(cr, rect, content, in_select, in_highlight, align=ALIGN_START, font_size=DEFAULT_FONT_SIZE):
    '''Render text.'''
    if in_select or in_highlight:
        color = ui_theme.get_color("listItemSelectText").get_color()
    else:
        color = ui_theme.get_color("listItemText").get_color()
    draw_text(cr, content, 
              rect.x, rect.y, rect.width, rect.height,
              font_size, 
              color,
              alignment=align)
    
def render_image(cr, rect, image_path, x, y):
    '''Render image.'''
    draw_pixbuf(cr, ui_theme.get_pixbuf(image_path).get_pixbuf(), x, y)


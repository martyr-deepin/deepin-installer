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
from dtk.ui.skin_config import skin_config
from dtk.ui.theme import Theme,ui_theme
from dtk.ui.utils import get_parent_dir
import os


# Init skin config.
skin_config.init_skin(
    "12",
    os.path.join(get_parent_dir(__file__, 2), "skin"),
    os.path.expanduser("~/.config/deepin-installer/skin"),
    os.path.expanduser("~/.config/deepin-installer/skin_config.ini"),
    "deepin-installer-demo",
    "1.0"
    )

# Create application theme.
app_theme = Theme(
    os.path.join(get_parent_dir(__file__, 2), "app_theme"),
    os.path.expanduser("~/.config/deepin-installer/theme")
    )

# Set theme.
skin_config.load_themes(ui_theme, app_theme)

import gtk
from dtk.ui.application import Application
from dtk.ui.label import Label
from dtk.ui.frame import HorizontalFrame
from dtk.ui.button import Button
from dtk.ui.utils import container_remove_all

from welcome import Welcome
from part import Part
from account import Account
from ui_utils import switch_box

class DeepinInstaller():
    '''deepin installer'''
    def __init__(self):
        self.application=Application("installer")
        self.application.set_default_size(756,520)
        self.application.add_titlebar(["min", "close"],None,"       欢迎使用深度软件",None,False)
        self.application.titlebar.set_size_request(-1,50)

        self.content_box=gtk.VBox()
        self.content_box.set_size_request(-1,400)

        self.step_contain_box=gtk.HBox()
        self.step_contain_box.set_size_request(-1,50)

        self.step=1

        self.page_label=Label("Page 5")
        self.page_label.set_text("第%d步，共5步" % self.step)
        self.step_label=Label("安装步骤信息")
        self.page_frame=HorizontalFrame()
        self.page_frame.set(0.1,0,0,0)
        # self.page_frame.set_padding(10,0,60,300)
        self.page_frame.set_padding(10,0,60,40)
        self.page_frame.add(self.page_label)

        self.ok_button=Button("下一步")
        self.back_button=Button("上一步")
        self.cancle_button=Button("退出")

        self.step_box=gtk.HBox()
        self.step_box.pack_start(self.page_frame,False,False,4)
        # self.step_box.pack_start(self.step_label,False,False,4)
        self.step_box.pack_end(self.ok_button,False,False,4)
        self.step_box.pack_end(self.back_button,False,False,4)
        self.step_box.pack_end(self.cancle_button,False,False,4)

        self.step_box_frame=HorizontalFrame()
        self.step_box_frame.set_size_request(-1,50)

        self.step_box_frame.set_padding(0,20,5,10)
        self.step_box_frame.add(self.step_box)

        self.ok_button.connect("clicked",self.on_ok_btn_click)
        self.back_button.connect("clicked",self.on_back_btn_click)
        self.cancle_button.connect("clicked",lambda w:gtk.main_quit())

        self.step_contain_box.add(self.step_box_frame)

        self.show_widgets()
        self.application.main_box.pack_start(self.content_box,False,False,4)
        self.application.main_box.pack_end(self.step_contain_box,False,False,4)
        self.application.window.enable_resize=False
        self.application.run()
        
        
    def show_widgets(self):
        # print "show widgets step:"
        # print self.step
        if self.step < 1 or self.step > 5:
            print "invalid step"

        if self.step == 1:
            self.welcome_box = Welcome()
            self.back_button.set_no_show_all(True)
            container_remove_all(self.back_button)
            switch_box(self.content_box,self.welcome_box)

        elif self.step == 2:
            # self.application.titlebar.change_title("分区信息")
            # self.set_step_label("分区信息")
            self.part_box=Part()
            self.back_button.set_no_show_all(False)
            self.back_button.show_all()
            switch_box(self.content_box, self.part_box)
            
        elif self.step==3:
            # self.set_step_label("时区选择（键盘布局)")
            self.account_box=Account()
            switch_box(self.content_box,self.account_box)
        
    def update_page_label(self):    
        self.page_label.set_text("第%d步，共5步" % self.step)

    def set_step_label(self,text):
        '''update label description of current install step'''
        self.step_label.set_text(text)

    def on_ok_btn_click(self,widget):
        '''confirm to goto next step'''

        # if self.step==1:
        #     print "do nothing"
        # elif self.step==2:
        #     self.on_commit_part_operations()
        # elif self.step==3:
        #     print "do other operation"

        self.step=self.step+1
        self.update_page_label()
        self.show_widgets()

    def on_commit_part_operations(self):
        '''commit partition add/delete/edit operations'''
        pass

    def on_back_btn_click(self,widget):
        self.step=self.step-1
        self.update_page_label()
        self.show_widgets()

if __name__=="__main__":
    di=DeepinInstaller()

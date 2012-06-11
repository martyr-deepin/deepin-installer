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

from dtk.ui.button import RadioButton,Button
from dtk.ui.label import Label
from dtk.ui.frame import HorizontalFrame,VerticalFrame


import gtk

class Welcome():
    '''first page of installer'''
    def __init__(self):
        self.content_box=gtk.VBox()

        self.install_method_label=Label("请选择安装方式：",text_size=20)

        self.install_clear_btn=RadioButton("清盘安装")
        self.install_clear_describe=Label("          这将会清除硬盘上的全部数据")
        self.install_clear_box=gtk.VBox()
        self.install_clear_box.pack_start(self.install_clear_btn,True,True,4)
        self.install_clear_box.pack_start(self.install_clear_describe,True,True,4)
        self.install_clear_frame=HorizontalFrame()
        self.install_clear_frame.add(self.install_clear_box)
        self.install_clear_frame.set_padding(1,10,20,0)

        self.install_advance_btn=RadioButton("高级安装")
        self.install_advance_describe=Label("          自己创建并调整分区")
        self.install_advance_box=gtk.VBox()
        self.install_advance_box.pack_start(self.install_advance_btn,True,True,4)
        self.install_advance_box.pack_start(self.install_advance_describe,True,True,4)
        self.install_advance_frame=HorizontalFrame()
        self.install_advance_frame.add(self.install_advance_box)
        self.install_advance_frame.set_padding(10,1,20,0)                                    

        self.content_box.pack_start(self.install_method_label,True,True,40)
        self.content_box.pack_start(self.install_clear_frame)
        self.content_box.pack_start(self.install_advance_frame)


        self.install_frame=VerticalFrame()
        self.install_frame.add(self.content_box)
        self.install_frame.set(0.4,0.2,0,0)
        self.install_frame.set_padding(40,80,80,100)

        

        self.welcome_step_box=gtk.HBox()
        #add step/ok/cancel button
        self.page_label=Label("Page 5")
        self.page_label.set_text("第1页，共5页")
        self.page_frame=HorizontalFrame()
        self.page_frame.set(0.1,0,0,0)
        self.page_frame.set_padding(5,0,20,400)

        self.page_frame.add(self.page_label)

        self.ok_button=Button("确定")

        self.cancle_button=Button("退出")

        self.welcome_step_box.pack_start(self.page_frame,False,True,4)
        self.welcome_step_box.pack_end(self.ok_button,True,True,4)
        self.welcome_step_box.pack_end(self.cancle_button,True,True,4)

        self.step_box_frame=HorizontalFrame()

        self.step_box_frame.set_padding(0,20,0,20)
        self.step_box_frame.add(self.welcome_step_box)



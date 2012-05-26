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

class SetupInfoMsgUI(gtk.EventBox):
    ''' switch to rich text or image as tips to user?'''
    def __init__(self):
        super(SetupInfoMsgUI,self).__init__()

        self.setupinfo_msg_hbox=gtk.HBox(0)
        self.setupinfo_msg_label=gtk.Label("安装信息设置正确与否提示：")
        self.setupinfo_msg_hbox.add(self.setupinfo_msg_label)
        self.set_msg_label_show(False)


    def get_msg_label_text(self):
        return self.setupinfo_msg_label.get_text()

    def set_msg_label_text(self,str):
        self.setupinfo_msg_label.set_text(str)

    def set_msg_label_show(self,flag):
        if flag:
            self.setupinfo_msg_label.show()
        else:
            self.setupinfo_msg_label.hide()
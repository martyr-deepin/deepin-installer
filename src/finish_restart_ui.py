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

class FinishRestartUI(gtk.EventBox):

    def __init__(self):
        super(FinishRestartUI,self).__init__()

        self.finish_restart_hbox=gtk.HBox(0)
        
        self.finish_restart_label=gtk.Label("系统安装完成，是否立即重新启动")
        self.finish_restart_okbtn=gtk.Button(stock=gtk.STOCK_OK)
        self.finish_restart_cancelbtn=gtk.Button(stock=gtk.STOCK_CANCEL)

        self.finish_restart_hbox.pack_start(self.finish_restart_label,False,False,1)
        self.finish_restart_hbox.pack_start(self.finish_restart_okbtn,False,False,1)
        self.finish_restart_hbox.pack_start(self.finish_restart_cancelbtn,False,False,1)

if __name__=="__main__":
    pass

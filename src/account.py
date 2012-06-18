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
from dtk.ui.label import Label
from dtk.ui.entry import TextEntry
from dtk.ui.button import RadioButton
from dtk.ui.frame import HorizontalFrame

class Account(gtk.VBox):
    '''account ui'''
    def __init__(self):
        super(Account,self).__init__()

        account_tab=gtk.Table(5,3)
        account_tab.set_row_spacings(25)
        account_tab.set_col_spacings(15)

        self.username_label=Label("用户名：")
        self.username_entry=TextEntry("请输入用户名")
        self.username_entry.set_size(150,24)
        self.username_check=Label("image")
        account_tab.attach(self.username_label,0,1,0,1)
        account_tab.attach(self.username_entry,1,2,0,1)
        account_tab.attach(self.username_check,2,3,0,1)

        self.hostname_label=Label("计算机名：")
        self.hostname_entry=TextEntry("请输入计算机名")
        self.hostname_entry.set_size(100,24)
        self.hostname_check=Label("image")
        account_tab.attach(self.hostname_label,0,1,1,2)
        account_tab.attach(self.hostname_entry,1,2,1,2)
        account_tab.attach(self.hostname_check,2,3,1,2)

        self.password_label=Label("密  码：")
        self.password_entry=TextEntry("请输入密码")
        self.password_entry.set_size(100,24)
        self.password_check=Label("image")
        account_tab.attach(self.password_label,0,1,2,3)
        account_tab.attach(self.password_entry,1,2,2,3)
        account_tab.attach(self.password_check,2,3,2,3)

        self.confirm_password_label=Label("确认密码：")
        self.confirm_password_entry=TextEntry("请输入确认密码")
        self.confirm_password_entry.set_size(100,24)
        self.confirm_password_check=Label("image")
        account_tab.attach(self.confirm_password_label,0,1,3,4)
        account_tab.attach(self.confirm_password_entry,1,2,3,4)
        account_tab.attach(self.confirm_password_check,2,3,3,4)


        self.auto_login=RadioButton("自动登录")
        self.auth_login=RadioButton("登录时需要密码")
        account_tab.attach(self.auto_login,0,1,4,5)
        account_tab.attach(self.auth_login,1,3,4,5,xpadding=30)
        self.auto_login.set_active(True)

        account_frame=HorizontalFrame()
        account_frame.set(0.4,0.4,0,0)
        account_frame.set_padding(50,50,50,50)
        account_frame.add(account_tab)
        self.add(account_frame)
        # self.set_size_request(500,400)

if __name__=="__main__":
    pass
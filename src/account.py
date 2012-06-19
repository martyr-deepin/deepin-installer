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

from account_util import account_util

class Account(gtk.VBox):
    '''account ui'''
    def __init__(self):
        super(Account,self).__init__()

        account_tab=gtk.Table(5,3)
        account_tab.set_row_spacings(25)
        account_tab.set_col_spacings(15)

        self.username_label=Label("用  户  名：")
        self.username_entry=TextEntry("请输入用户名")
        self.username_entry.set_size(150,24)
        self.username_check=Label("image")
        account_tab.attach(self.username_label,0,1,0,1)
        account_tab.attach(self.username_entry,1,2,0,1)
        account_tab.attach(self.username_check,2,3,0,1)
        self.username_entry.entry.connect("press-return",self.on_username_return)
        self.username_entry.entry.connect("focus-in-event",self.on_username_focus_in)
        self.username_entry.entry.connect("focus-out-event",self.on_username_focus_out)
        self.username_entry.entry.connect("changed",self.on_username_changed)

        self.hostname_label=Label("计算机名：")
        self.hostname_entry=TextEntry("请输入计算机名")
        self.hostname_entry.set_size(100,24)
        self.hostname_check=Label("image")
        account_tab.attach(self.hostname_label,0,1,1,2)
        account_tab.attach(self.hostname_entry,1,2,1,2)
        account_tab.attach(self.hostname_check,2,3,1,2)
        self.hostname_entry.entry.connect("press-return",self.on_hostname_return)
        self.hostname_entry.entry.connect("focus-in-event",self.on_hostname_focus_in)
        self.hostname_entry.entry.connect("focus-out-event",self.on_hostname_focus_out)
        self.hostname_entry.entry.connect("changed",self.on_hostname_changed)

        self.password_label=Label("密         码：")
        self.password_entry=TextEntry("请输入密码")
        self.password_entry.set_size(100,24)
        self.password_check=Label("image")
        account_tab.attach(self.password_label,0,1,2,3)
        account_tab.attach(self.password_entry,1,2,2,3)
        account_tab.attach(self.password_check,2,3,2,3)
        self.password_entry.entry.connect("press-return",self.on_password_return)
        self.password_entry.entry.connect("focus-in-event",self.on_password_focus_in)
        self.password_entry.entry.connect("focus-out-event",self.on_password_focus_out)
        self.password_entry.entry.connect("changed",self.on_password_changed)

        self.confirm_password_label=Label("确认密码：")
        self.confirm_password_entry=TextEntry("请输入确认密码")
        self.confirm_password_entry.set_size(100,24)
        self.confirm_password_check=Label("image")
        account_tab.attach(self.confirm_password_label,0,1,3,4)
        account_tab.attach(self.confirm_password_entry,1,2,3,4)
        account_tab.attach(self.confirm_password_check,2,3,3,4)
        self.confirm_password_entry.entry.connect("press-return",self.on_confirm_password_return)
        self.confirm_password_entry.entry.connect("focus-in-event",self.on_confirm_password_focus_in)
        self.confirm_password_entry.entry.connect("focus-out-event",self.on_confirm_password_focus_out)
        self.confirm_password_entry.entry.connect("changed",self.on_confirm_password_changed)


        self.auto_login=RadioButton("自动登录")
        self.auth_login=RadioButton("登录时需要密码")
        account_tab.attach(self.auto_login,0,1,4,5)
        account_tab.attach(self.auth_login,1,3,4,5,xpadding=30)
        self.auto_login.set_active(True)

        self.account_tips=Label("         ")
        # self.account_tips.set_text("")
        # account_tab.attach(self.account_tips,1,2,5,6)
        
        error_frame=HorizontalFrame()
        error_frame.add(self.account_tips)
        error_frame.set(0.4,0.4,0,0)
        
        account_frame=HorizontalFrame()
        account_frame.set(0.4,0.4,0,0)
        account_frame.set_padding(50,20,50,50)
        account_frame.add(account_tab)
        # self.add(account_frame)
        self.pack_start(account_frame,False,False,4)
        self.pack_start(error_frame)
        # self.set_size_request(500,400)

    def display_error_msg(self,error_msg):
        '''display error msg tips for user'''
        if len(error_msg)!=0:
            self.account_tips.set_text(error_msg)
            error_msg=""
        else:
            self.account_tips.set_text("")

    def on_username_return(self,widget):
        '''confirm to input username'''
        account_util.assert_user_name()
        self.display_error_msg(account_util.username_error_msg)

    def on_username_focus_in(self,widget,event):
        '''username focus_in'''
        if self.username_entry.get_text()=="请输入用户名":
            self.username_entry.set_text("")

    def on_username_focus_out(self,widget,event):
        '''username_focus_out'''
        self.on_username_return(widget)

    def on_username_changed(self,widget,event):
        '''username_changed'''
        account_util.username=self.username_entry.get_text()
        account_util.forbid_create_exist_user()
        self.hostname_entry.set_text(account_util.generate_hostname())

    def on_hostname_return(self,widget):
        '''on_hostname_return'''
        account_util.assert_hostname()
        self.display_error_msg(account_util.hostname_error_msg)

    def on_hostname_focus_in(self,widget,event):
        '''on_hostname_focus_in'''
        if self.hostname_entry.get_text()=="请输入计算机名":
            self.hostname_entry.set_text("")

    def on_hostname_focus_out(self,widget,event):
        '''on_username_focus_out'''
        self.on_hostname_return(widget)

    def on_hostname_changed(self,widget,event):
        account_util.hostname=self.hostname_entry.get_text()

    def on_password_return(self,widget):
        '''on_password_return'''
        account_util.assert_user_password()
        self.display_error_msg(account_util.password_error_msg)

    def on_password_focus_in(self,widget,event):
        if self.password_entry.get_text()=="请输入密码":
            self.password_entry.set_text("")

    def on_password_focus_out(self,widget,event):
        self.on_password_return(widget)

    def on_password_changed(self,widget,event):
        account_util.password=self.password_entry.get_text()

    def on_confirm_password_return(self,widget):
        account_util.assert_user_confirm_password()
        self.display_error_msg(account_util.confirm_password_error_msg)

    def on_confirm_password_focus_in(self,widget,event):
        if self.confirm_password_entry.get_text()=="请输入确认密码":
            self.confirm_password_entry.set_text("")

    def on_confirm_password_focus_out(self,widget,event):
        self.on_confirm_password_return(widget)

    def on_confirm_password_changed(self,widget,event):
        account_util.confirm_password=self.confirm_password_entry.get_text()

if __name__=="__main__":
    pass
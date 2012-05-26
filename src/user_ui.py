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
## This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gtk

class UserUI(gtk.EventBox):
    
    def __init__(self):
        super(UserUI,self).__init__()

        self.user_vbox=gtk.VBox(False,0)

        self.account_form={}


        self.user_label=gtk.Label('帐户信息设置:')

        self.user_table=gtk.Table(2,4,False)

        self.user_name_label=gtk.Label("用户名:")
        self.user_pwd_label=gtk.Label("密码:")
        self.user_con_pwd_label=gtk.Label("确认密码：")
        self.user_hostname_label=gtk.Label("计算机名：")

        self.user_name_entry=gtk.Entry(10)
        self.user_pwd_entry=gtk.Entry(10)
        self.user_pwd_entry.set_visibility(False)
        self.user_con_pwd_entry=gtk.Entry(10)
        self.user_con_pwd_entry.set_visibility(False)
        self.user_hostname_entry=gtk.Entry(10)

        self.user_table.attach(self.user_name_label,0,1,0,1)
        self.user_table.attach(self.user_name_entry,1,2,0,1)
        self.user_table.attach(self.user_pwd_label,2,3,0,1)
        self.user_table.attach(self.user_pwd_entry,3,4,0,1)

        self.user_table.attach(self.user_hostname_label,0,1,1,2)
        self.user_table.attach(self.user_hostname_entry,1,2,1,2)
        self.user_table.attach(self.user_con_pwd_label,2,3,1,2)
        self.user_table.attach(self.user_con_pwd_entry,3,4,1,2)


        # self.user_vbox.pack_start(self.user_label,False,False,0)
        self.user_label_alignment=gtk.Alignment(0,0,0,0)
        self.user_label_alignment.add(self.user_label)
        self.user_vbox.pack_start(self.user_label_alignment,False,False,5)

        self.user_vbox.pack_start(self.user_table,False,False,3)


    def get_user_name(self):
        return self.user_name_entry.get_text()
            
    def get_user_pwd(self):
        return self.user_pwd_entry.get_text()

    def get_user_con_pwd(self):
        return self.user_con_pwd_entry.get_text()

    def get_user_hostname(self):
        return self.user_hostname_entry.get_text()

    def set_user_name(self,text):
        self.user_name_entry.set_text(text)
        self.account_form['username']=text

    def set_user_pwd(self,text):
        self.user_pwd_entry.set_text(text)
        self.account_form['password']=text

    def set_user_con_pwd(self,text):
        self.user_con_pwd_entry.set_text(text)
        self.account_form['confirm_password']=text

    def set_user_hostname(self,text):
        self.user_hostname_entry.set_text(text)
        self.account_form['hostname']=text

    def get_account_form(self):
        self.account_form['username']=self.get_user_name()
        self.account_form['password']=self.get_user_pwd()
        self.account_form['confirm_password']=self.get_user_con_pwd()
        self.account_form['hostname']=self.get_user_hostname()

        return self.account_form


        
     

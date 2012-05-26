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

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


import os
from basic_utils import run_os_command
from setupinfo_msg_ui import SetupInfoMsgUI
from user_ui import UserUI


class CreateAccount:
    '''create user account,home,etc'''
    def __init__(self):
        ''' get user account info from setupinfo_ui'''
        self.account_info=UserUI().get_account_form()
        self.username=self.account_info['username']
        self.hostname=self.account_info['hostname']
        self.password=self.account_info['password']
        self.confirm_password=self.account_info['confirm_password']
        

    def create_user(self):
        ''' use account info to create username ,password ,user group,etc,need to consider chroot '''
        cmd_addgroup="sudo groupadd"+" "+self.username
        run_os_command(cmd_addgroup)
        cmd_adduser="sudo useradd -g"+" "+self.username+" "+"-s /bin/bash -k /dev/null -m"+" "+self.username
        run_os_command(cmd_adduser)

        cmd_passwd="echo "+self.username+":"+self.passwd+" | chpasswd"
        run_os_command(cmd_passwd)

    def grant_sudo(self):
        '''add user to sudoers,need to consider chroot'''
        cmd_grantsudo="echo "+self.username+"        "+"ALL=(ALL:ALL) ALL"+">> /etc/sudoers"
        run_os_command(cmd_grantsudo)

    def assert_user_name(self):
        '''check user name length,validation,can't use root to build account'''
        length=self.username.length()

        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("用户名过")
            SetupInfoMsgUI.set_msg_label_show(True)

        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)

        if(self.username.startswith("root")):
            SetupInfoMsgUI.set_msg_label_text("请使用其它用户名")
            SetupInfoMsgUI.set_msg_label_show(True)
           

    def assert_user_password(self):
        '''check user password length,validation,etc'''
        length=self.password.length()

        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)

        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)
        
        if(self.password!=self.confirm_password):
            SetupInfoMsgUI.set_msg_label_text("密码与确认密码不一致")
            SetupInfoMsgUI.set_msg_label_show(True)


    def assert_hostname(self):
        '''check hostname length,validation,etc'''
        length=self.hostname.length()

        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)

        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)


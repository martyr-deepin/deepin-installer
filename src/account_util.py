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
from basic_utils import run_os_command,get_os_command_output
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
        self.system_user_list=[]
        self.login_type="password"#autologin,password
        self.encrypt_password=""

    def create_user(self):
        ''' use account info to create username ,password ,user group,etc,need to consider chroot '''
        cmd_addgroup="sudo groupadd"+" "+self.username
        run_os_command(cmd_addgroup)
        cmd_adduser="sudo useradd -g"+" "+self.username+" "+"-s /bin/bash -k /dev/null -m"+" "+self.username
        run_os_command(cmd_adduser)
        if len(self.encrypt_password)==0:
            self.set_user_password()
        else:    
            self.encrypt_home_dir()

    def set_user_password(self):
        '''set normal password,without encrypt_home_dir'''
        cmd_passwd="echo "+self.username+":"+self.passwd+" | chpasswd"
        run_os_command(cmd_passwd)

    def encrypt_home_dir(self):
        '''encrypt user home dir,use encrypted password'''
        cmd_encrypt_passwd="sudo usermod -p "+self.encrypt_passwd+" "+self.username
        run_os_command(cmd_encrypt_passwd)

    def get_system_user_list(self):
        '''return the list of system_user,forbidden to add user specified with the existed name'''
        system_user_command="cat /etc/passwd"
        userlist_data=get_os_command_output(system_user_command)
        for user in userlist_data:
            username=user.split(":")[0]
            self.system_user_list.append(username)
        # print self.system_user_list    
        return self.system_user_list    

    def forbid_create_exist_user(self):
        '''forbidden to use the name owened by system'''
        for username in self.get_system_user_list():
            if self.username.startswith(username):
                #here need add UI notify
                print "username confilct"
            else:
                continue

    def grant_sudo(self):
        '''add user to sudoers,need to consider chroot'''
        cmd_grantsudo="echo "+self.username+"        "+"ALL=(ALL:ALL) ALL"+">> /etc/sudoers"
        run_os_command(cmd_grantsudo)

    def assert_user_name(self):
        '''check user name length,validation,can't use root to build account'''
        length=self.username.length()
        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("用户名过短")
            SetupInfoMsgUI.set_msg_label_show(True)
        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("用户名过长")
            SetupInfoMsgUI.set_msg_label_show(True)
            
        self.forbid_create_exist_user()    
        # if(self.username.startswith("root")):
        #     SetupInfoMsgUI.set_msg_label_text("请使用其它用户名")
        #     SetupInfoMsgUI.set_msg_label_show(True)

    def assert_user_password(self):
        '''check user password length,validation,etc'''
        length=self.password.length()
        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("密码太短")
            SetupInfoMsgUI.set_msg_label_show(True)
        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("密码太长")
            SetupInfoMsgUI.set_msg_label_show(True)
        if(self.password!=self.confirm_password):
            SetupInfoMsgUI.set_msg_label_text("密码与确认密码不一致")
            SetupInfoMsgUI.set_msg_label_show(True)
        if (self.username==self.password):
            SetupInfoMsgUI.set_msg_label_text("用户名与密码不能相同")
            SetupInfoMsgUI.set_msg_label_show(True)

    def generate_hostname(self):
        '''generate hostname according to username'''
        self.hostname=self.username+"-deepin"
        #need set reaction to UI when user enter his username
        return self.hostname

    def assert_hostname(self):
        '''check hostname length,validation,etc'''
        length=self.hostname.length()
        if(length<5):
            SetupInfoMsgUI.set_msg_label_text("主机名过短")
            SetupInfoMsgUI.set_msg_label_show(True)
        elif(length>15):
            SetupInfoMsgUI.set_msg_label_text("主机名过长")
            SetupInfoMsgUI.set_msg_label_show(True)

    def set_host_resolve(self):
        '''set host resolve'''
        host_path="/target/etc/hosts"
        if not os.path.exists(host_path):
            run_os_command("mkdir -p "+os.path.dirname(host_path))
            run_os_command("touch "+host_path)
        host_conf_str="127.0.0.1 localhost \n"
        host_conf_str=host_conf_str+"127.0.0.1 "+self.hostname+" \n"
        host_conf_str=host_conf_str+"#The following lines are desire for IPv6 capable hosts\n"
        host_conf_str=host_conf_str+"::1     ip6-localhost ip6-loopback"
        host_conf_str=host_conf_str+"fe00::0 ip6-localnet"
        host_conf_str=host_conf_str+"ff00::0 ip6-mcastprefix"
        host_conf_str=host_conf_str+"ff02::1 ip6-allnodes"
        host_conf_str=host_conf_str+"ff02::2 ip6-allrouters"
        host_conf_str=host_conf_str+"ff02::3 ip6-allhosts"
        
        run_os_command(open(host_path,"w+").append(host_conf_str))

    def set_gdm_autologin(self):
        '''set gdm autologin'''
        gdm_path="/target/etc/gdm/custom.conf"
        if not os.path.exists(gdm_path):
            run_os_command('mkdir -p '+os.path.dirname(gdm_path))
            run_os_command("touch "+gdm_path)
        gdm_conf=open(gdm_path,'w+')
        gdm_conf_str="\n[daemon]\nTimedLoginEnable=true\nAutomaticLogin=%s\nTimedLoginDelay=10\n"\
            "AutomaticLoginEnable=true\nTimedLogin=%s\n\nDefaultSession=deepin\n" % (self.username,self.username)
        gdm_conf.write(gdm_conf_str)
        gdm_conf.close()

    def boot_with_logintype(self):
        '''autologin or need password'''
        if self.login_type=="autologin":
            self.set_gdm_autologin()
        elif self.login_type=="password":
            pass
        else:
            print "invalid login_type"


if __name__=="__main__":
    ca=CreateAccount()
    ca.get_system_user_list()

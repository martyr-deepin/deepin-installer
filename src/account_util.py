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

class CreateAccount:
    '''create user account,home,etc'''
    def __init__(self):
        ''' get user account info from setupinfo_ui'''
        self.username=""
        self.hostname=""
        self.password=""
        self.confirm_password=""
        self.system_user_list=[]
        self.login_type="password"#autologin,password
        self.encrypt_password=""
        self.username_error_msg=""
        self.password_error_msg=""
        self.confirm_password_error_msg=""
        self.hostname_error_msg=""

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
        return self.system_user_list    

    def forbid_create_exist_user(self):
        '''forbidden to use the name owened by system'''
        for username in self.get_system_user_list():
            # if self.username.startswith(username):
            #     print "username confilct"
            if self.username==username:
                self.username_error_msg="username confilct"
            else:
                continue

    def grant_sudo(self):
        '''add user to sudoers,need to consider chroot'''
        cmd_grantsudo="echo "+self.username+"        "+"ALL=(ALL:ALL) ALL"+">> /etc/sudoers"
        run_os_command(cmd_grantsudo)

    def assert_user_name(self):
        '''check user name length,validation,can't use root to build account'''
        length=len(self.username)
        if(length<5):
            self.username_error_msg="user name too short"
        elif(length>15):
            self.username_error_msg="user name too long"
        self.forbid_create_exist_user()    

    def assert_user_password(self):
        '''check user password length,validation,etc'''
        length=len(self.password)
        if(length<5):
            self.password_error_msg="password too short"
        elif(length>15):
            self.password_error_msg="password too long"
        if (self.username==self.password):
            self.password_error_msg="cann't set password same to username"

    def assert_user_confirm_password(self):
        '''assert confirm password equal to password'''
        if (self.password!=self.confirm_password):
            self.confirm_password_error_msg="password!=confirm_password"

    def generate_hostname(self):
        '''generate hostname according to username'''
        self.hostname=self.username+"-deepin"
        return self.hostname

    def assert_hostname(self):
        '''check hostname length,validation,etc'''
        length=len(self.hostname)
        if(length<5):
            self.hostname_error_msg="hostname too short"
        elif(length>15):
            self.hostname_error_msg="hostname too long"

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

account_util=CreateAccount()

if __name__=="__main__":
    ca=CreateAccount()
    ca.get_system_user_list()

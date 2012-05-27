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

import os

import syslog

import subprocess
import traceback



# def get_command_output(command):
#     '''Run commmand and return result:[] '''
#     result=[]
#     try:
#         subp=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
#         # result=subp.stdout.readlines().splitlines()
#         result=subp.communicate()[0].splitlines()
        
#     except:
#         for line in traceback.format_exc().split("\n"):
#             syslog.syslog(syslog.LOG_ERR,line)

#     return result        

def get_os_command_output(command):
    '''run command directly use os.system,return result:[]'''
    result=[]
    try:
        f=os.popen(command)
        data=f.readlines()
        
        for item in data:
            result.append(item[0:-1])
    except:
        for line in traceback.format_exc().split("\n"):
            syslog.syslog(syslog.LOG_ERR,line)
    finally:
        f.close()

    return result    


def get_os_command_oneput(command):
    '''run command directly use os.system,return a string item'''
    result=" "
    try:
        f=os.popen(command)
        result=f.readline()
    except:
        for line in traceback.format_exc().split("\n"):
            syslog.syslog(syslog.LOG_ERR,line)
    finally:
        f.close()

    return result    

#this also may have some problems
# def run_command(command):
#     '''just run command'''
#     try:
#         subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    
#     except:
#         for line in traceback.format_exc().split("\n"):
#             syslog.syslog(syslog.LOG_ERR,line)
        
def run_os_command(command):
    '''run command directly use os.sys'''
    try:
        os.system(command)
    except:
        for line in traceback.format_exc().split("\n"):
            syslog.syslog(syslog.LOG_ERR,line)


def mount_target_root(target,device):
    '''mount sys'''

    target_root="sudo mkdir "+target
    command_mountroot="sudo mount "+device+" "+target_root
  
    run_os_command(command_mountroot)


def mount_sys(target):
    '''mount sys'''

    command_mkdir="sudo mkdir -p "+target+"/sys"
    command_mountsys="sudo mount -o bind /sys "+target+"/sys"
    run_os_command(command_mkdir)
    run_os_command(command_mountsys)

def mount_proc(target):
    ''' mount proc'''

    command_mkdir="sudo mkdir -p "+target+"/proc"
    command_mountproc="sudo mount -t proc none "+target+"/proc"
    run_os_command(command_mkdir)
    run_os_command(command_mountproc)


def mount_dev(target):
    ''' mount dev'''

    command_mkdir="sudo mkdir -p "+target+"/dev"
    command_mountdev="sudo mount -o bind /dev "+target+"/dev"
    run_os_command(command_mkdir)
    run_os_command(command_mountdev)

def write_etc_fstab(target,fstab_info):
    '''write partition set to fstab'''

    fstab_information=fstab_info
    fstab_path=target+"/etc/fstab"
    pfstab=open(fstab_path,'a+')
    pfstab.write(fstab_information)
    pfstab.flush()

def do_chroot(target):
    '''chroot'''
    command="sudo chroot "+target
    run_os_command(command)

def do_grub_install(target):
    '''grub_install'''
    command="sudo grub-install "+target
    run_os_command(command)


def do_update_grub():
    '''update_grub'''
    command="sudo update-grub "
    run_os_command(command)

def do_reboot():
    command="sudo reboot "
    run_os_command(command)


if __name__=="__main__":
   print get_os_command_output("cat /etc/passwd")


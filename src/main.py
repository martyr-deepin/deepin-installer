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
import traceback
import syslog
        
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
    print "mount root success"

def mount_sys(target):
    '''mount sys'''

    command_mkdir="sudo mkdir -p "+target+"/sys"
    command_mountsys="sudo mount -o bind sys "+target+"/sys"
    run_os_command(command_mkdir)
    run_os_command(command_mountsys)
    print "mount sys success"

def mount_proc(target):
    ''' mount proc'''

    command_mkdir="sudo mkdir -p "+target+"/proc"
    command_mountproc="sudo mount -t proc none "+target+"/proc"
    run_os_command(command_mkdir)
    run_os_command(command_mountproc)
    print "mount proc success"

def mount_dev(target):
    ''' mount dev'''

    command_mkdir="sudo mkdir -p "+target+"/dev"
    command_mountdev="sudo mount -o bind dev "+target+"/dev"
    run_os_command(command_mkdir)
    run_os_command(command_mountdev)
    print "mount dev success"


def write_etc_fstab(target,fstab_info):
    '''write partition set to fstab'''

    pfstab=open(target,'a+')
    pfstab.write(fstab_info)
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



def parted_mklabel(device,label_type):
    '''label_type:mbr,gpt...,this leads to create a new partition'''

    command="sudo parted "+device+" "+label_type
    run_os_command(command)


def mkfs_mkfs(device,partition,fs_type):
    '''mkfs,support ext3,ext4...'''

    # partition=device+partition+" "
    partition="/dev/sda1"
    command="sudo mkfs -t "+fs_type+" "+partition
    run_os_command(command)

def parted_mkpart(device,part_type,start,end):
    '''part_type:primary,logic,extend...'''
    command="sudo parted "+device+" mkpart "+part_type+" "+start+" "+end+" "
    run_os_command(command)

def do_copy(src,destination):

   command="sudo cp -a "+src+" "+destination
   run_os_command(command)


def create_user():
    ''' use account info to create username ,password ,user group,etc'''
    cmd_addgroup="sudo groupadd"+" longwei "
    run_os_command(cmd_addgroup)
    cmd_adduser="sudo useradd -g"+" "+"longwei"+" "+"-s /bin/bash -k /dev/null -m"+" "+"longwei"
    run_os_command(cmd_adduser)
    ''' need to process interactive input password'''
    # cmd_passwd="sudo passwd"+" "+self.username
    # run_os_command(cmd_passwd)


parted_mklabel("/dev/sda","gpt")
parted_mkpart("/dev/sda","primary","0%","100%")
mkfs_mkfs("/dev/sda",1,"ext3")

mount_target_root("/target","/dev/sda1")

run_os_command("cd /")

mount_sys("/target")
mount_proc("/target")
mount_dev("/target")


# run_os_command("sudo mkdir /cdsrc")    
# run_os_command("sudo mount /media/cdrom/casper/filesystem.squashfs /cdsrc")
# do_copy("/cdsrc/*","/target")


# fstab_infomation="/dev/sda1 / ext3 defaults 0 1 "

# write_etc_fstab("/target/etc/fstab",fstab_infomation)


# do_chroot("/target")

# create_user()

# do_grub_install("/dev/sda")

# do_update_grub()

# do_reboot()


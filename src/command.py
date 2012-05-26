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


run_os_command("sudo parted /dev/sda mklabel msdos")
run_os_command("sudo parted /dev/sda mkpart primary 0% 100%")
run_os_command("sudo mkfs -t ext3 /dev/sda1")

run_os_command("cd /")

run_os_command("sudo mkdir /target")
run_os_command("sudo mount /dev/sda1 /target")

run_os_command("sudo mkdir -p /target/sys")
run_os_command("sudo mount -o bind /sys /target/sys")

run_os_command("sudo mkdir -p /target/proc")
run_os_command("sudo mount -t proc none /target/proc")

run_os_command("sudo mkdir -p /target/dev")
run_os_command("sudo mount -o bind /dev /target/dev")

run_os_command("sudo mkdir /cdsrc")
run_os_command("sudo mount /media/cdrom/casper/filesystem.squashfs /cdsrc")
run_os_command("sudo cp -a /cdsrc/* /target")


run_os_command("sudo chmod 666 /target/etc/fstab")
run_os_command("sudo echo '/dev/sda1 / ext3 defaults 0 1' >> /target/etc/fstab")

run_os_command("sudo chroot /target")

run_os_command("groupadd yilang")
run_os_command("useradd -g yilang -s /bin/bash -m yilang")
run_os_command("echo yilang:deepin | chpasswd")
run_os_command("echo 'yilang    ALL=(ALL:ALL) ALL' >> /etc/sudoers")
run_os_command("grub-install /dev/sda")
run_os_command("update-grub")

run_os_command("reboot")
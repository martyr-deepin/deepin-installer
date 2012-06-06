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

import gio

class MountUtil():
    '''Mount Util'''
    def __init__(self):
        self.vm=gio.VolumeMonitor()
        self.drives=self.vm.get_connected_drives()
        self.volumes=self.vm.get_volumes()
        self.drive_volumes={}
        for drive in self.drives:
            self.drive_volumes[drive]=drive.get_volumes()

    def mount_volume(self):
        pass

    def mount_file(self):
        pass

    def test(self):
        # for drive in self.drives:
            # print drive.get_name()
            # # print drive.enumerate_identifiers()
            # print drive.get_identifier("unix-device")
        for volume in self.volumes:
            print volume
            print volume.get_identifier("unix-device")
            print volume.get_identifier("uuid")
            print volume.get_mount()
            if volume.get_mount()!=None:

                print volume.get_mount().get_root()

        # for mount in self.vm.get_mounts():
        #     print mount.get_uuid()
        
        
if __name__=="__main__":
    mu=MountUtil()
    mu.test()


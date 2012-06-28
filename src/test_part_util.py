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

from part_util import global_part_util as pu

def test_disk_partition_info_tab():
    # for disk in pu.get_system_disks():
    #     print pu.disk_partition_info_tab[disk]
    disk=pu.get_disk_from_path("/dev/sdb")
    # print pu.disk_partition_info_tab[disk]
    print pu.disk_geom_info_tab[disk]
    pu.add_disk_partition_info_tab(disk,"primary",1024,(70,1024*1024*1024/disk.device.sectorSize,1024*1024*1024/disk.device.sectorSize),"ext4",None,None,"/home","start")
    # print pu.disk_partition_info_tab[disk]
    print pu.disk_geom_info_tab[disk]


def test_disk_part_display_path():
    for disk in pu.get_system_disks():
        print pu.disk_part_display_path[disk]


def test_disk_geom_info_tab():
    # for disk in pu.get_system_disks():
    #     print pu.disk_geom_info_tab[disk]
    disk=pu.get_disk_from_path("/dev/sdb")
    pu.add_disk_partition_info_tab(disk,"primary",1024,(100,None,16771855),"ext4",None,None,"/home","start")


    pu.add_disk_partition_info_tab(disk,"logical",1024,(3097148,None,8097148),"ext4",None,None,"/var","start")
    # print pu.get_disk_extend_list(disk)[0].geometry
    print pu.disk_partition_info_tab[disk]

    pu.add_disk_partition_info_tab(disk,"logical",1024,(8097148,None,13097148),"ext4",None,None,"/","start")
    # print_geom_info(disk)
    print pu.disk_partition_info_tab[disk]
    # print pu.get_disk_extend_list(disk)[0].geometry
    pu.add_custom_partition()

def test_reduce_extend_part():
    disk=pu.get_disk_from_path("/dev/sdb")
    print_geom_info(disk)
    # pu.add_disk_partition_info_tab(disk,"primary",1000,(100,None,2048100),"ext4",None,None,"/home","start")
    # pu.add_disk_partition_info_tab(disk,"extend",1024,(16771855,None,8097148),"ext4",None,None,"/home","start")

    pu.add_disk_partition_info_tab(disk,"logical",1000,(3097148,None,5197148),"ext4",None,None,"/var","start")
    print_geom_info(disk)

    # # print pu.get_disk_extend_list(disk)[0].geometry
    # print pu.disk_partition_info_tab[disk]


    pu.add_disk_partition_info_tab(disk,"logical",1000,(8097148,None,13097148),"ext4",None,None,"/","start")
    pu.add_disk_partition_info_tab(disk,"primary",1000,(12288000,None,14288000),"ext4",None,None,"/","start")


    print_geom_info(disk)
    # print pu.disk_partition_info_tab[disk]
    # print pu.get_disk_extend_list(disk)[0].geometry
    pu.add_custom_partition()

    
def print_geom_info(disk):
    print pu.disk_geom_info_tab[disk]
    for item in pu.disk_geom_info_tab[disk]:
        print item[0]
        print item[-1].start
        print item[-1].end

if __name__=="__main__":
    # test_disk_partition_info_tab()
    # test_disk_geom_info_tab()
    # test_disk_part_display_path()
    test_reduce_extend_part()
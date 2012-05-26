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

from part_util import *

class Part:
    '''do partition work:
    Infomation to a partition:device,partition,fstype;
    start,end,size,flag,label,state.
    '''
    
    def __init__(self):
        '''infos ready for write partitions to disk,can't modify them'''
        
        self.device=[]
        self.partition_info=[]
        self.partitions_info=[[]]
    
    def probe_device(self):
        '''a list for choose which to install deepin system'''
        
        self.device=get_install_device()

    def write_partition(self,partition_info):
        '''create one partition according to the given info'''

        pass

    def write_partitions(self,partitions_info):
        '''create partitions according to the given info'''
        
        index=0
        for part in partitions:
            partition_info=partitions_info[index]
            create_partition(partition_info)
            index=index+1

    def write_fstab(self,partitions_info):
        '''write to fstab'''
        
        pass


if __name__=="__main__":
    pass

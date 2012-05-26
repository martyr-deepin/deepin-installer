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


from constant import TARGET
from basic_utils import run_os_command

class Copy:
    '''Copy system from cdrom to destination device'''

    def __init__(self):
        self.src="/media/cdrom/casper/filesystem.squashfs"
        self.destination=TARGET

    def check_path(self,path):
        '''check src and destination'''
        
        pass

    def do_copy(self,src,destination):
        # self.check_path(src)
        # self.check_path(destination)
        command="sudo cp -a "+src+" "+destination
        # self.subp=subprocess.Popen(command,stdout=subprocess.PIPE)
        run_os_command(command)
    

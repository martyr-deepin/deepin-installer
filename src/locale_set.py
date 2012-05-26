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
from basic_utils.py import run_os_command

class Locale:
    '''set language,timezone,keyboard information,for install procesure and system both'''
    def __init__(self):
        self.language=""
        self.timezone=""
        self.keyboard=""

    def install_locale(self):
        #install english,chinese-simple,chinese-traditional 
        cmd_path="/user/share/locales/install-language-pack"
        cmd_install=cmd_path+self.language
        run_os_command(cmd_install)

    def set_language(self):
        self.set_install_language()
        self.set_system_language()

    def set_system_language(self):
        f=os.open("/etc/default/locale",'a')
        print f
        if self.language=="english":
            LANG="en_US.UTF-8"
            LANGUAGE="en_US:en"

        elif self.language=="chinese":
            pass
        else:
            pass
    def set_install_language(self):
        pass

    def set_timezone(self):
        pass

    def set_keyboard(self):
        pass

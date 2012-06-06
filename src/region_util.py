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
import locale
import time

class LocaleUtil():
    '''locale set util'''
    def __init__(self):
        self.locale_dict={}
        self.locale_list=[]
        self.languagelist=[]

    def set_language(self,language):
        pass

    def get_language_shortlist(self):
        '''need consider the location'''
        languagelist=open("/usr/share/localechooser/shortlists").readlines()

        for line in languagelist:
            if line.startswith("#"):
                continue
            self.languagelist.append(line.strip())

        return self.languagelist    

    def set_locale(self,category,value):
        locale.setlocale(category,value)

    def get_system_locales(self):
        localelist=open("/usr/share/i18n/SUPPORTED").readlines()
        for line in localelist:
            if line.startswith("#"):
                continue
            (locale_name,charset)=line.strip().split(None,1)
            self.locale_list.append(locale_name)
            self.locale_dict[locale_name]=charset

        return self.locale_list    

class TimezoneUtil():
    '''timezone set util'''
    def __init__(self):
        self.region_dict={}
        self.timezone=""
        self.string_time=""
        self.region=""
        self.timezone_list=[]

    def set_timezone(self):
        self.timezone=os.environ['TZ']
        if len(self.timezone)==0:
            os.environ.get('TZ',"GMT")
            self.timezone="GMT"
        time.tzset()
        self.string_time=time.ctime()

    def get_timezone_list(self):
        pass

    def get_selected_timezone(self,region):
        # self.timezone=self.regiona
        os.environ.get('TZ',self.timezone)
        self.set_timezone()

    def get_system_regions(self):
        regiondata=open("/usr/share/localechooser/regionmap").readlines()

        for line in regiondata:
            if line.startswith("#"):
                continue
            (country_city,island)=line.strip().split(None,1)
            self.region_dict[country_city]=island
            
        return self.region_dict    

class KeyboardUtil():
    '''keyboard set util'''
    def __init__(self):
        pass

    def set_keyboard_layout(self,layout):
        pass

if __name__=="__main__":
    # lu=LocaleUtil()
    # lu.get_system_locales()
    # print lu.locale_list
    # print lu.locale_dict
    tu=TimezoneUtil()
    tu.get_system_regions()
    print tu.region_dict

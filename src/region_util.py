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
        self.locale=""
        self.language=""
        self.init_locale_dict()

    def init_locale_dict(self):
        '''init locale data'''
        #to be implemented
        pass

    def get_system_locales(self):
        '''get system_locales'''
        localelist=open("/usr/share/i18n/SUPPORTED").readlines()
        for line in localelist:
            if line.startswith("#"):
                continue
            (locale_name,charset)=line.strip().split(None,1)
            self.locale_list.append(locale_name)
            self.locale_dict[locale_name]=charset

        return self.locale_list    

    def get_language_shortlist(self):
        '''need consider the location'''
        languagelist=open("/usr/share/localechooser/shortlists").readlines()

        for line in languagelist:
            if line.startswith("#"):
                continue
            self.languagelist.append(line.strip())

        return self.languagelist    

    def set_locale(self,category,value):
        '''set locale'''
        locale.setlocale(category,value)
        
    def get_locale(self):
        '''return custom locale'''
        return self.locale

    def set_language(self,language):
        '''set language'''
        pass
    
    def get_language(self):
        '''return custom language'''
        return self.language

    def set_default_locale(self):
        '''set default_locale for chinese people'''
        pass

    def set_default_language(self):
        '''set default_language for chinese people'''
        pass


class TimezoneUtil():
    '''timezone set util'''
    def __init__(self):
        self.timezone_dict={}
        self.timezone=""
        self.string_time=""
        self.region=""
        self.timezone_list=[]
        
        self.init_timezone_dict()

    def init_timezone_dict(self):
        '''fill timezone data into the dict'''
        #to be implemented
        pass

    def get_timezone_list(self):
        pass

    def get_system_regions(self):
        regiondata=open("/usr/share/localechooser/regionmap").readlines()

        for line in regiondata:
            if line.startswith("#"):
                continue
            (country_city,island)=line.strip().split(None,1)
            self.region_dict[country_city]=island
            
        return self.region_dict    


    def set_custom_timezone(self,region):
        '''set timezone according to user selected region'''
        # self.timezone=self.regiona
        os.environ.get('TZ',self.timezone)
        self.set_timezone()

    def set_timezone(self):
        '''atom function to set timezone'''
        self.timezone=os.environ['TZ']
        if len(self.timezone)==0:
            os.environ.get('TZ',"GMT")
            self.timezone="GMT"
        time.tzset()
        self.string_time=time.ctime()

    def get_timezone(self):
        '''return the timezone user had set'''
        return self.timezone

    def set_default_timezone(self):
        '''set timezone to china/shanghai,for chinese people'''
        pass

    def get_default_timezone(self):
        '''return default_timezone'''
        pass

class KeyboardUtil():
    '''keyboard set util'''
    def __init__(self):
        self.keyboard_layout=""
        self.country_list=[]
        self.country_keyboard_list=[]
        self.keyboard_dict={}

        self.init_keyboard_dict()

    def init_keyboard_dict(self):
        '''fill keyboard layout data into keyboard_dict'''
        #to be implemented
        return self.keyboard_dict

    def get_country_list(self):
        '''get country list'''
        return self.country_list

    def get_country_keyboard_list(self,country):
        '''get the keyboard layout list of the given country'''
        self.country_keyboard_list=self.keyboard_dict[country]
        return self.country_keyboard_list

    def set_default_keyboard_layout(self):
        '''set default_keyboard_layout convient for chinese'''
        pass

    def set_keyboard_layout(self,country,layout):
        '''set keyboard layout'''
        pass

    def get_keyboard_layout(self):
        '''return the custom layout'''
        return self.layout

if __name__=="__main__":
    # lu=LocaleUtil()
    # lu.get_system_locales()
    # print lu.locale_list
    # print lu.locale_dict
    tu=TimezoneUtil()
    tu.get_system_regions()
    print tu.region_dict

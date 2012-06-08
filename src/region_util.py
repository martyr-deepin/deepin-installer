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
from basic_utils import get_os_command_output

LOCALE_FILE="/usr/share/i18n/SUPPORTED"
LANGUAGE_FILE="/usr/share/localechooser/shotlists"
ISO3166_FILE="/usr/share/zoneinfo/iso3166.tab"
ZONE_FILE="/usr/share/zoneinfo/zone.tab"

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
        self.timezone="Asia/shanghai"
        self.iso3166_tab={}#{CN:CHINA}
        self.timezone_tab={}#{TZ:(code,coordinate)}

    def get_system_timezone_tab(self):
        '''get timezone_tab from zone.tab'''
        zone_tab_command="cat /usr/share/zoneinfo/zone.tab"
        timezone_data=get_os_command_output(zone_tab_command)
        for line in timezone_data:
            if line.startswith("#"):
                continue
            else:
                timezone_item=line.strip().split("\t")
                self.timezone_tab[timezone_item[2]]=(timezone_item[0],timezone_item[1])
        return self.timezone_tab        

    def get_iso3166_tab(self):
        '''get iso3166_tab from iso3166.tab'''
        iso3166_tab_command="cat /usr/share/zoneinfo/iso3166.tab"
        iso3166_data=get_os_command_output(iso3166_tab_command)
        for line in iso3166_data:
            if line.startswith("#"):
                continue
            else:
                iso3166_item=line.strip().split("\t")
                self.iso3166_tab[iso3166_item[0]]=iso3166_item[1]

        return self.iso3166_tab        

    def set_timezone(self,tz):
        '''atom function to set timezone'''
        if len(tz)==0:
            self.set_default_timezone()
        else:
            self.timezone=tz
            os.environ.get("TZ",tz)
            time.tzset()

    def get_timezone(self):
        '''return the timezone user had set'''
        return self.timezone

    def set_default_timezone(self):
        '''set timezone to china/shanghai,for chinese people'''
        self.timezone="Asia/shanghai"
        os.environ.get("TZ",self.timezone)
        time.tzset()

    def get_default_timezone(self):
        '''return default_timezone'''
        return "Asia/shanghai"

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
    # tu.get_system_regions()
    # print tu.region_dict
    print tu.get_system_timezone_tab()

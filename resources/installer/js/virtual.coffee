#Copyright (c) 2011 ~ 2013 Deepin, Inc.
#              2011 ~ 2013 yilang
#
#Author:      YuanChengLu <yuanchenglu001@gmail.com>
#Maintainer:  YuanChenglu <yuanchenglu001@gmail.com>
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, see <http://www.gnu.org/licenses/>.

class VirtualMachine extends Page
    constructor: (@id) ->
        super
        @title_txt = create_element("div", "VirtualTitle", @element)
        @title_txt.innerText = _("提示")
        @content = create_element("div", "VirtualContent", @element)
        @tips = create_element("div", "VirtualTips", @content)
        @tips.innerText = _("正在扫描本地磁盘....")
        @foot = create_element("div", "VirtualFoot", @element)
        @element.style.display = "block"

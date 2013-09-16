#Copyright (c) 2011 ~ 2013 Deepin, Inc.
#              2011 ~ 2013 yilang
#
#Author:      LongWei <yilang2007lw@gmail.com>
#Maintainer:  LongWei <yilang2007lw@gmail.com>
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

class Finish extends Page
    constructor: (@id)->
        super
        @title = create_element("div", "Title", @element)
        @title.innerHTML = "<span>完成Linux Deepin系统安装</span>"

        @sep = create_element("div", "Sep", @element)

        @finish_desc = create_element("p", "", @element)
        @finish_desc.innerText = "完成安装需要重启电脑"

        @restart_now = create_element("p", "", @element)
        @now_radio = create_element("input", "NowRadio", @restart_now)
        @now_radio.setAttribute("type", "radio")
        @now_radio.setAttribute("name", "restart")
        @now_radio.setAttribute("value", "now")
        @now_radio.setAttribute("checked", "true")
        @now_desc = create_element("span", "NowDesc", @restart_now)
        @now_desc.innerText = "立即重启电脑"

        @restart_later = create_element("p", "", @element)
        @later_radio = create_element("input", "LaterRadio", @restart_later)
        @later_radio.setAttribute("type", "radio")
        @later_radio.setAttribute("name", "restart")
        @later_radio.setAttribute("value", "later")
        @later_radio.setAttribute("checked", "true")
        @later_desc = create_element("span", "LaterDesc", @restart_later)
        @later_desc.innerText = "稍后重启电脑"

        @finish_install = create_element("div", "FinishBtn", @element)
        @finish_install.innerText = "完成"
        @finish_install.addEventListener("click", (e) =>
            echo "finish install"
            DCore.Installer.finish_install()
        )

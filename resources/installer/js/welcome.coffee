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

class Welcome extends Page
    constructor: (@id)->
        super
        @illegal_keys='\t\n\r~`!@#$%^&*()+}{|\\\':;<,>.?/'

        @title = create_element("div", "Title", @element)
        @title.innerHTML = "<span>欢迎安装使用 Linux Deepin <sup>12.12</sup></span>"

        @username = create_element("p", "Username", @element)
        @username_txt = create_element("span", "Txt", @username)
        @username_txt.innerText = "用户名 :"
        @username_input = create_element("input", "", @username)
        @username_input.setAttribute("placeholder", "2-14个字符，英文，数字，中文")
        @username_info = create_element("span", "Info", @username)
        @username_input.addEventListener("blur", (e) =>
            echo "username blur"
            if @is_username_valid()
                echo "valid"
                @username_info.innerText = ""
            else
                echo "invalid"
                @username_info.innerText = "请输入合法的用户名"
                echo @username_info.innerText
        )

        @hostname = create_element("p", "Hostname", @element)
        @hostname_txt = create_element("span","Txt", @hostname) 
        @hostname_txt.innerText = "计算机名 :"
        @hostname_input = create_element("input", "", @hostname)
        @hostname_input.setAttribute("placeholder", "请输入计算机名")
        @hostname_info = create_element("span", "Info", @hostname)
        @hostname_input.addEventListener("blur", (e) =>
            if @is_hostname_valid()
                @hostname_info.innerText = ""
            else
                @hostname_info.innerText = "请输入合法的计算机名"
        )
        
        @password = create_element("p", "Password", @element)
        @password_txt = create_element("span", "Txt", @password)
        @password_txt.innerText = "登录密码 :"
        @password_input = create_element("input", "", @password)
        @password_input.setAttribute("placeholder", "请输入密码")
        @password_info = create_element("span", "Info", @password)
        @password_input.addEventListener("blur", (e) =>
            if @is_password_valid()
                @password_info.innerText = ""
            else
                @password_info.innerText = "请输入合法的密码"
        )
        
        @confirm = create_element("p", "ConfirmPassword", @element)
        @confirm_txt = create_element("span", "Txt", @confirm)
        @confirm_txt.innerText = "确认密码 :"
        @confirm_input = create_element("input", "", @confirm)
        @confirm_input.setAttribute("placeholder", "请输入确认密码")
        @confirm_info = create_element("span", "Info", @confirm)
        @confirm_input.addEventListener("blur", (e) =>
            if @is_confirm_password_valid()
                @confirm_info.innerText = ""
            else
                @confirm_info.innerText = "请输入合法的确认密码"
        )

        @keyboard = create_element("p", "Keyboard", @element)
        @keyboard_txt = create_element("span", "Txt", @keyboard)
        @keyboard_txt.innerText = "键盘布局 :"
        @keyboard_select = create_element("select", "", @keyboard)
        @keyboard_info = create_element("span", "Info", @keyboard)
        @fill_keyboard()

        @timezone = create_element("p", "Timezone", @element)
        @timezone_txt = create_element("span", "Txt", @timezone)
        @timezone_txt.innerText = "时区 :"
        @timezone_select = create_element("select", "", @timezone)
        @timezone_info = create_element("span", "Info", @timezone)
        @fill_timezone()

        @start = create_element("div", "Start", @element)
        @start.innerText = "开始安装"

    is_username_valid: ->
        if not @username_input.value? or @username_input.value.length == 0
            return false
        if @username_input.value in DCore.Installer.get_system_users()
            return false
        for c in @username_input.value
            if c in @illegal_keys
                return false
        return true

    is_hostname_valid: ->
        if not @hostname_input.value? or @hostname_input.value.length == 0
            return false
        return true

    is_password_valid: ->
        if not @password_input.value? or @password_input.value.length == 0
            return false
        return true

    is_confirm_password_valid: ->
        if not @confirm_input.value?  or @confirm_input.value.length == 0
            return false
        if @confirm_input.value != @password_input.value
            return false
        return true

    fill_keyboard: ->
        echo "fill keyboard"

    fill_timezone: ->
        echo "fill timezone"
        for zone in DCore.Installer.get_timezone_list()
            zone_opt = create_element("option", "", @timezone_select)
            zone_opt.setAttribute("value", zone)
            zone_opt.innerText = zone


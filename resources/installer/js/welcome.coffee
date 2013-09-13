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
        @username.innerHTML = "<span>用户名 :</span>"
        @username_input = create_element("input", "", @username)
        @username_input.setAttribute("placeholder", "2-14个字符，英文，数字，中文")
        @username_input.addEventListener("blur", (e) =>
            if @is_username_valid()
                @error.innerText = ""
            else
                @error.innerText = "请输入合法的用户名"
        )

        @hostname = create_element("p", "Hostname", @element)
        @hostname.innerHTML = "<span>计算机名 :</span>"
        @hostname_input = create_element("input", "", @hostname)
        @hostname_input.setAttribute("placeholder", "请输入计算机名")
        @hostname_input.addEventListener("blur", (e) =>
            if @is_hostname_valid()
                @error.innerText = ""
            else
                @error.innerText = "请输入合法的计算机名"
        )
        
        @password = create_element("p", "Password", @element)
        @password.innerHTML = "<span>登录密码 :</span>"
        @password_input = create_element("input", "", @password)
        @password_input.setAttribute("placeholder", "请输入密码")
        @password_input.addEventListener("blur", (e) =>
            if @is_password_valid()
                @error.innerText = ""
            else
                @error.innerText = "请输入合法的密码"
        )
        
        @confirm = create_element("p", "ConfirmPassword", @element)
        @confirm.innerHTML = "<span>确认密码 :</span>"
        @confirm_input = create_element("input", "", @confirm)
        @confirm_input.setAttribute("placeholder", "请输入确认密码")
        @confirm_input.addEventListener("blur", (e) =>
            if @is_confirm_password_valid()
                @error.innerText = ""
            else
                @error.innerText = "请输入合法的确认密码"
        )

        @keyboard = create_element("p", "Keyboard", @element)
        @keyboard.innerHTML = "<span>键盘布局 :</sapn>"
        @keyboard_select = create_element("select", "", @keyboard)
        @fill_keyboard()

        @timezone = create_element("p", "Timezone", @element)
        @timezone.innerHTML = "<span>时区 :</span>"
        @timezone_select = create_element("select", "", @timezone)

        @error = create_element("p", "Error", @element)

        @start = create_element("p", "Start", @element)
        @start.innerHTML = "<i>开始安装</i>"

    is_username_valid: ->
        if not @username_input.value?
            return false
        if @username_input.value in DCore.Installer.get_system_users()
            return false
        for c in @username_input.value
            if c in @illegal_keys
                return false
        return true

    is_hostname_valid: ->
        if not @hostname_input.value?
            return false
        return true

    is_password_valid: ->
        if not @password_input.value?
            return false
        return true

    is_confirm_password_valid: ->
        if not @confirm_input.value? 
            return false
        if @confirm_input.value != @password_input.value
            return false
        return true

    fill_keyboard: ->
        echo "fill keyboard"

    fill_timezone: ->
        echo "fill timezone"


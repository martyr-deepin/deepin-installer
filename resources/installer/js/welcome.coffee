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

        @title_start = create_element("div", "", @title)
        @start_txt = create_element("p", "", @title_start)
        @start_txt.innerText = "安装向导"

        @title_set = create_element("div", "TitleSet", @title)
        @keyboard_set = create_element("span", "", @title_set)
        @timezone_set = create_element("span", "", @title_set)

        @form = create_element("div", "WelcomeForm", @element)

        @username = create_element("div", "WelcomeFormItem", @form)
        @username_txt = create_element("div", "", @username)
        @username_txt.innerText = "用户名 :"
        @username_input = create_element("input", "Username", @username)
        @username_input.setAttribute("placeholder", "2-14个字符，英文，数字，中文")
        @username_input.addEventListener("blur", (e) =>
            echo "username blur"
            if @is_username_valid()
                echo "valid"
                #@username_info.innerText = ""
            else
                echo "invalid"
                #@username_info.innerText = "请输入合法的用户名"
        )

        @hostname = create_element("div", "WelcomeFormItem", @form)
        @hostname_txt = create_element("div","", @hostname) 
        @hostname_txt.innerText = "计算机名 :"
        @hostname_input = create_element("input", "Hostname", @hostname)
        @hostname_input.setAttribute("placeholder", "请输入计算机名")
        @hostname_input.addEventListener("blur", (e) =>
            if @is_hostname_valid()
                echo "valid"
                #@hostname_info.innerText = ""
            else
                echo "invalid"
                #@hostname_info.innerText = "请输入合法的计算机名"
        )
        
        @password = create_element("div", "WelcomeFormItem", @form)
        @password_txt = create_element("div", "", @password)
        @password_txt.innerText = "登录密码 :"
        @password_input = create_element("input", "Password", @password)
        @password_input.setAttribute("placeholder", "请输入密码")
        @password_input.addEventListener("blur", (e) =>
            if @is_password_valid()
                echo "valid"
                #@password_info.innerText = ""
            else
                echo "invalid"
                #@password_info.innerText = "请输入合法的密码"
        )
        
        @confirm = create_element("div", "WelcomeFormItem", @form)
        @confirm_txt = create_element("div", "", @confirm)
        @confirm_txt.innerText = "确认密码 :"
        @confirm_input = create_element("input", "ConfirmPassword", @confirm)
        @confirm_input.setAttribute("placeholder", "请输入确认密码")
        @confirm_input.addEventListener("blur", (e) =>
            if @is_confirm_password_valid()
                echo "valid"
                #@confirm_info.innerText = ""
            else
                echo "invalid"
                #@confirm_info.innerText = "请输入合法的确认密码"
        )

        #@keyboard = create_element("p", "Keyboard", @element)
        #@keyboard_txt = create_element("span", "Txt", @keyboard)
        #@keyboard_txt.innerText = "键盘布局 :"
        #@keyboard_select = create_element("select", "", @keyboard)
        #@keyboard_info = create_element("span", "Info", @keyboard)
        #@fill_keyboard()

        #@timezone = create_element("p", "Timezone", @element)
        #@timezone_txt = create_element("span", "Txt", @timezone)
        #@timezone_txt.innerText = "时区 :"
        #@timezone_select = create_element("select", "", @timezone)
        #@timezone_info = create_element("span", "Info", @timezone)
        #@fill_timezone()

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


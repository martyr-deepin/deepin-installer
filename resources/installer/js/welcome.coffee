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

class Account extends Widget
    constructor: (@id)->
        super
        @title = create_element("div", "AccountTitle", @element)
        @title.innerText = "注册"
        @illegal_keys='\t\n\r~`!@#$%^&*()+}{|\\\':;<,>.?/'

        @username = create_element("div", "Username", @element)
        @username_img = create_img("UsernameImg", "images/user.png", @username)
        @username_input = create_element("input", "", @username)
        @username_input.setAttribute("placeholder", "请输入用户名")

        @username_input.addEventListener("blur", (e) =>
            if @is_username_valid()
                @error.innerText = ""
                @username_input.style.border = "2px solid blue"
            else
                @error.innerText = "请输入合法的用户名"
                @username_input.style.border = "2px solid red"
        )

        @hostname = create_element("div", "Hostname", @element)
        @hostname_img = create_img("HostnameImg", "images/user.png", @hostname)
        @hostname_input = create_element("input", "", @hostname)
        @hostname_input.setAttribute("placeholder", "请输入计算机名")

        @hostname_input.addEventListener("blur", (e) =>
            if @is_hostname_valid()
                @error.innerText = ""
                @hostname_input.style.border = "2px solid blue"
            else
                @error.innerText = "请输入合法的计算机名"
                @hostname_input.style.border = "2px solid red"
        )
        
        @password = create_element("div", "Password", @element)
        @password_img = create_img("PasswordImg", "images/user.png", @password)
        @password_input = create_element("input", "", @password)
        @password_input.setAttribute("placeholder", "请输入密码")

        @password_input.addEventListener("blur", (e) =>
            if @is_password_valid()
                @error.innerText = ""
                @password_input.style.border = "2px solid blue"
            else
                @error.innerText = "请输入合法的密码"
                @password_input.style.border = "2px solid red"
        )
        
        @confirm_password = create_element("div", "ConfirmPassword", @element)
        @confirm_img = create_img("ConfirmImg", "images/user.png", @confirm_password)
        @confirm_input = create_element("input", "", @confirm_password)
        @confirm_input.setAttribute("placeholder", "请输入确认密码")

        @confirm_input.addEventListener("blur", (e) =>
            if @is_confirm_password_valid()
                @error.innerText = ""
                @confirm_input.style.border = "2px solid blue"
            else
                @error.innerText = "请输入合法的确认密码"
                @confirm_input.style.border = "2px solid red"
        )

        @error = create_element("div", "Error", @element)

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

class InstallType extends Widget
    constructor: (@id)->
        super
        @title = create_element("div", "InstallTitle", @element)
        @title.innerText = "选择"

        @auto = create_element("div", "AutoInstall", @element)

        @auto_radio_container = create_element("div", "AutoRadioContainer", @auto)
        @auto_radio = create_element("input", "AutoRadio", @auto_radio_container)
        @auto_radio.setAttribute("type", "radio")
        @auto_radio.setAttribute("name", "install-type")
        @auto_radio.setAttribute("value", "auto")
        @auto_radio.setAttribute("checked", "true")

        @auto_desc = create_element("div", "AutoDesc", @auto)
        @auto_desc.innerText = "保留文档、图片和其它个人文件，您可以在多系统下选择你要进入的系统，系统将安装在第一块硬盘的第一个分区。"

        @custom = create_element("div", "CustomInstall", @element)
        
        @custom_radio_container = create_element("div", "CustomRadioContainer", @custom)
        @custom_radio = create_element("input", "CustomRadio", @custom_radio_container)
        @custom_radio.setAttribute("type", "radio")
        @custom_radio.setAttribute("name", "install-type")
        @custom_radio.setAttribute("value", "custom")

        @custom_desc = create_element("div", "CustomDesc", @custom)
        @custom_desc.innerText  = "您可以自己创建和调整分区, 或者为deepin选择多个分区。"

class StartInstall extends Widget
    constructor: (@id)->
        super
        @install_btn = create_element("button", "InstallBtn", @element)
        @install_btn.innerText = "开始安装"

class Welcome extends Page
    constructor: (@id)->
        super
        @title.innerText = "欢迎安装使用 Linux Deepin "
        
        @account_container = create_element("div", "AccountContainer", @content)
        @account = new Account("welcome")
        @account_container.appendChild(@account.element)

        @dividing = create_element("div", "Dividing", @content)
        #@dividing_img = create_img("DividingImg", "images/dividing.jpg", @dividing)
        @dividing.innerHTML = "<hr width=1 height=100% color=red/>"

        @install_container = create_element("div", "InstallContainer", @content)
        @install_type = new InstallType("welcome")
        @install_container.appendChild(@install_type.element)

        @start_install = new StartInstall("welcome")
        @footer.appendChild(@start_install.element)

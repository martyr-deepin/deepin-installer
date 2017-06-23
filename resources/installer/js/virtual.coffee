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
        lsb_release = DCore.Installer.read_text_file("/etc/lsb-release")
        release_reg = /DISTRIB_DESCRIPTION=([^\n]+)/
        build_version_match = release_reg.exec(lsb_release)
        if build_version_match
            build_version = build_version_match[1]
            @title_txt.innerText = _("安装方式 (" + build_version + ")")
        else
            @title_txt.innerText = _("安装方式")
        @content = create_element("div", "VirtualContent", @element)

        @auto_btn = new NextStep("VirtualMachineAutoBtn","全盘自动安装", @auto_cb)
        @content.appendChild(@auto_btn.element)
        @auto_btn.next_bt_enable()

        @manual_btn = new NextStep("VirtualMachineManualBtn","自定义安装", @manual_cb)
        @content.appendChild(@manual_btn.element)
        @manual_btn.next_bt_enable()

        @element.style.display = "block"

    auto_cb: =>
        console.debug("[virtual.coffee] VirtualMachine.auto_cb() auto_mode: #{pc.auto_mode}")
        progress_page = new Progress("progress") if not progress_page?
        pc.switch_page(progress_page)
        # DCore.Installer.start_install() is called in
        # DCore.Installer.auto_install()
        DCore.Installer.auto_part()

    manual_cb: =>
        console.debug("[virtual.coffee] VirtualMachine.manual_cb() auto_mode: #{pc.auto_mode}")
        if pc.auto_mode
            undo_part_table_info()
            part_page = new Part("part")
            pc.switch_page(part_page)
        else
            welcome_page = new Welcome("welcome") if not welcome_page?
            pc.switch_page(welcome_page)

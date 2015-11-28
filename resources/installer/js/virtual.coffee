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
        @title_txt.innerText = _("Tips")
        @content = create_element("div", "VirtualContent", @element)
        @tips = create_element("div", "VirtualTips", @content)
        @tips.innerText = "安装过程中会格式化整个磁盘，请提前备份重要数据!"
        @foot = create_element("div", "VirtualFoot", @element)
        @continue = new NextStep("VirtualMachineContinue","安装", @ok_cb)
        @foot.appendChild(@continue.element)
        @continue.next_bt_enable()
        @element.style.display = "block"

    ok_cb: =>
        console.debug("[virtual.coffee] VirtualMachine.ok_cb() auto_mode: #{pc.auto_mode}")
        if pc.auto_mode
            progress_page = new Progress("progress") if not progress_page?
            pc.switch_page(progress_page)
            DCore.Installer.start_install()
        else
            welcome_page = new Welcome("welcome") if not welcome_page?
            pc.switch_page(welcome_page)

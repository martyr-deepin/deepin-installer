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

pc = new PageContainer("pc")
document.body.appendChild(pc.element)

DCore.signal_connect("without_wm", ->
    console.log("[installer.coffee] connected to dbus signal `without_wm`")
    pc.without_wm = true
)

DCore.signal_connect("is_virtual_machine", ->
    console.log("[installer.coffee] connected to dbus signal `is_virtual_machine`")
    pc.is_virtual_machine = true
    virtual_page = new VirtualMachine("VirtualMachine") if not virtual_page?
    pc.switch_page(virtual_page)
)

DCore.signal_connect("auto_mode", ->
    console.log("[installer.coffee] connected to dbus signal `auto_mode`")
    pc.auto_mode = true
    if pc.is_virtual_machine then return
    progress_page = new Progress("progress") if not progress_page?
    pc.switch_page(progress_page)
    DCore.Installer.start_install()
)

welcome_page = new Welcome("welcome") if not welcome_page?
pc.switch_page(welcome_page)
DCore.Installer.emit_webview_ok()

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

DCore.signal_connect("auto_mode", (msg) ->
    auto_mode = true
    progress_page = new Progress("progress") if not progress_page?
    pc.switch_page(progress_page)
    DCore.Installer.start_install()
)
pc.switch_page(new Welcome("welcom"))
DCore.Installer.emit_webview_ok()


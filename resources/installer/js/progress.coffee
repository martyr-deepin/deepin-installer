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

__selected_target = null

DCore.signal_connect("extract", (msg) ->
    echo "extract"
    echo msg
    if parseInt(msg.step) == "finish"
        progress_install()
)

progress_install = ->
    try
        DCore.Installer.mount_procfs()
    catch error
        echo "mount procfs failed"
        return
    try
        DCore.Installer.chroot_target(__selected_target)
    catch error
        echo "chroot target failed"
        return
    try
        DCore.Installer.write_partition_mp()
    catch error
        echo "write partiton mp failed"
        return
    try
        DCore.Installer.set_timezone(__selected_timezone)
    catch error
        echo "set timezone failed"
        return
    try
        DCore.Installer.set_keyboard_layout_variant(__selected_layout, __selected_variant)
    catch error
        echo "set keyboard layout variant failed"
        return
    try
        DCore.Installer.create_user(__selected_username, __selected_hostname, __selected_password)
    catch error
        echo "create user failed"
        return
    try
        DCore.Installer.update_grub(__selected_grub)
    catch error
        echo "update grub failed"
        return

class Progress extends Page
    constructor: (@id)->
        super
        @progress_txt = create_element("p", "", @title)
        @progress_txt.innerText = "正在安装"

        @current_img = _ppt_list[0]

        @ppt = create_element("div", "Ppt", @element)
        @ppt_img = create_img("PptImg", @current_img, @ppt)
        @ppt_img.addEventListener("click", (e) =>
            if e.offsetX < 377
                @switch_ppt("prev")
            else 
                @switch_ppt("next")
        )

        @progress_container = create_element("div", "ProgressContainer", @element)
        @progressbar = create_element("div", "ProgressBar", @progress_container)
        #@progressdesc = create_element("div", "Progressdesc", @progress_container)
        #@progressdesc.innerText = "Progress Description"
        @progressbar.addEventListener("click", (e) =>
            pc.add_page(finish_page)
            pc.remove_page(progress_page)
        )

    switch_ppt: (direction)->
        if direction == "prev"
            index = _ppt_list.indexOf(@current_img)
            if index > 0
                @current_img = _ppt_list[index - 1]
        else if direction == "next"
            index = _ppt_list.indexOf(@current_img)
            if index < 4
                @current_img = _ppt_list[index + 1]
        else
            echo "invalid direction"

        @ppt_img.setAttribute("src", @current_img)

    update_progress: (progress) ->
        echo "update progress"

    start_extract: ->
        try
            DCore.Installer.mount_target(__selected_target)
        catch error
            echo "mount target failed"
            return
        try
            DCore.Installer.extract_squashfs()
        catch error
            echo "extract squashfs failed"
            return

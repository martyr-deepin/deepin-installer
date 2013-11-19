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

DCore.signal_connect("progress", (msg) ->
    if msg.stage == "extract"
        if msg.progress == "finish"
            progress_install()
        else
            progress_page.update_progress(msg.progress)
)

progress_install = ->
    try
        echo "mount procfs"
        DCore.Installer.mount_procfs()
    catch error
        echo "mount procfs failed"
        return
    try
        echo "__selected_target"
        echo __selected_target
        DCore.Installer.chroot_target(__selected_target)
    catch error
        echo "chroot target failed"
        return
    try
        echo "write partition mp"
        DCore.Installer.write_partition_mp()
    catch error
        echo "write partiton mp failed"
        return
    try
        echo "__selected_timezone"
        echo __selected_timezone
        DCore.Installer.set_timezone(__selected_timezone)
    catch error
        echo "set timezone failed"
        return
    try
        if __selected_layout.indexOf(",") != -1
            layout = __selected_layout.split(",")[0]
            variant = __selected_layout.split(",")[1]
        else
            layout = __selected_layout
            variant = null
        echo "layout"
        echo layout
        echo "variant"
        echo variant
        DCore.Installer.set_keyboard_layout_variant(layout,variant)
    catch error
        echo "set keyboard layout variant failed"
        return
    try
        echo "create user"
        echo __selected_username
        echo __selected_hostname
        echo __selected_password
        DCore.Installer.create_user(__selected_username, __selected_hostname, __selected_password)
    catch error
        echo "create user failed"
        return
    try
        echo "__selected_grub"
        echo __selected_grub
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
        @progressbar.style.width = progress

    start_extract: ->
        __selected_target = get_target_part()
        echo "start extrace selected target"
        echo __selected_target
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

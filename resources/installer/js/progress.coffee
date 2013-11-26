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

DCore.signal_connect("progress", (msg) ->
    if msg.stage == "extract" and __selected_stage == "extract"
        progress_page.handle_extract(msg.progress)
    else if msg.stage == "chroot" and __selected_stage == "chroot"
        progress_page.handle_chroot(msg.progress)
    else if msg.stage == "timezone" and __selected_stage == "timezone"
        progress_page.handle_set_timezone(msg.progress)
    else if msg.stage == "keyboard" and __selected_stage == "keyboard"
        progress_page.handle_set_keyboard(msg.progress)
    else if msg.stage == "user" and __selected_stage == "user"
        progress_page.handle_create_user(msg.progress)
    else if msg.stage == "grub" and __selected_stage == "grub"
        progress_page.handle_update_grub(msg.progress)
    else
        echo "other message or stage"
        echo msg.stage
        echo msg.progress
        echo __selected_stage
)

class ReportDialog extends Widget
    constructor: (@id) ->
        super(@id, @cb)
        @title = create_element("p", "DialogTitle", @element)
        @title_txt = create_element("div", "DialogTxt", @title)
        @title_txt.innerText = "错误报告"

        @title_close = create_element("div", "DialogClose", @title)
        @title_close.addEventListener("click", (e) =>
            @hide_dialog()
            @cb()
        )

        @content = create_element("div", "DialogContent", @element)
        @report_tips = create_element("p", "", @content)
        @report_tips.innerText = "安装失败，请把安装日志信息反馈Deepin社区"

        @foot = create_element("p", "DialogBtn", @element)
        @ok = create_element("span", "", @foot)
        @ok.innerText = "OK"
        @ok.addEventListener("click", (e) =>
            @hide_dialog()
            @cb()
        )

        @add_css_class("DialogCommon")
        @show_dialog()

    show_dialog: ->
        __in_model = true
        __board.setAttribute("style", "display:block")

    hide_dialog: ->
        __in_model = false
        @destroy()
        __board.setAttribute("style", "display:none")

    cb: ->
        DCore.Installer.finish_install()

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

    show_report: ->
        @report?.hide_dialog()
        @report =  new ReportDialog("report")
        document.body.appendChild(@report.element)

    handle_extract: (progress) ->
        if progress == "start"
            echo "start handle extract"
            __selected_target = get_target_part()
            try
                DCore.Installer.mount_target(__selected_target)
                DCore.Installer.extract_squashfs()
            catch error
                echo error
        else if progress == "finish"
            echo "extract finish"
            __selected_stage = "chroot"
            @handle_chroot("start")
        else if progress == "terminate"
            echo "extract terminate"
            @show_report()
        else
            @update_progress(progress)

    handle_chroot: (progress) ->
        if progress == "start"
            echo "start handle chroot"
            try
                DCore.Installer.mount_procfs()
                DCore.Installer.chroot_target()
            catch error
                echo error
        else if progress == "finish"
            echo "chroot finish"
            __selected_stage = "timezone"
            @handle_set_timezone("start")
        else if progress == "terminate"
            ehco "chroot terminate"
            @show_report()
        else
            echo "invalid progress for handle chroot"

    handle_set_timezone: (progress) ->
        if progress == "start"
            echo "start handle timezone"
            try
                write_fs_tab()
                DCore.Installer.set_timezone(__selected_timezone)
            catch error
                echo error
        else if progress == "finish"
            echo "timezone finish"
            __selected_stage = "keyboard"
            @handle_set_keyboard("start")
        else if progress == "terminate"
            echo "timezone terminate"
            @show_report()
        else
            echo "invalid progress for handle timezone"

    handle_set_keyboard: (progress) ->
        if progress == "start"
            echo "start handle keyboard"
            try
                if __selected_layout.indexOf(",") != -1
                    layout = __selected_layout.split(",")[0]
                    variant = __selected_layout.split(",")[1]
                else
                    layout = __selected_layout
                    variant = null
                DCore.Installer.set_keyboard_layout_variant(layout,variant)
            catch error
                echo error
        else if progress == "finish"
            echo "keyboard finish"
            __selected_stage = "user"
            @handle_create_user("start")
        else if progress == "terminate"
            echo "keyboard terminate"
            @show_report()
        else 
            echo "invalid progress for handle keyboard"

    handle_create_user: (progress) ->
        if progress == "start"
            echo "start handle user"
            try
                DCore.Installer.create_user(__selected_username, __selected_hostname, __selected_password)
            catch error
                echo error
        else if progress == "finish"
            echo "user finish"
            __selected_stage = "grub"
            @handle_update_grub("start")
        else if progress == "terminate"
            echo "user terminate"
            @show_report()
        else
            echo "invalid progress for handle user"
    
    handle_update_grub: (progress) ->
        if progress == "start"
            echo "start handle grub"
            #if __selected_grub.indexOf("part") != -1
            #    #only advance mode will install grub to partition
            #    disk = v_part_info[_selected_grub]["disk"]
            #    if v_disk_info[disk]["type"] == "gpt"
            #        try
            #            DCore.Installer.set_partition_flag(__selected_grub, "bios_grub", 1)
            #        catch error
            #            echo error
            try
                DCore.Installer.update_grub(__selected_grub)
            catch error
                echo error
        else if progress == "finish"
            echo "finish update grub"
            pc.add_page(finish_page)
            pc.remove_page(progress_page)
        else if progress == "terminate"
            echo "update grub terminate"
            @show_report()
        else
            echo "invalid progress for handle grub"

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
    else if msg.stage == "bootloader" and __selected_stage == "bootloader"
        progress_page.handle_update_bootloader(msg.progress)
    else
        echo "other message or stage"
        echo msg.stage
        echo msg.progress
        echo __selected_stage
)

class ReportDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Error report")
        @report_tips = create_element("p", "", @content)
        @report_tips.innerText = _("Installation failed. Please send the log to Linux Deepin Team.")

    cb: ->
        echo "report dialog cb"
        DCore.Installer.finish_install()

apply_progress_flash = (el, time)->
    apply_animation(el, "progressflash", "#{time}s", "cubic-bezier(0, 0, 0.35, -1)")

update_style_attr = (style, name, value) ->
    array = style.split(";")
    list = []
    found = false
    for item in array
        if item.indexOf(name + ":") != -1
            list.push(name + ":" + value)
            found = true
        else
            list.push(item)
    if not found
        list.push(name + ":" + value)
    return list.join(";")

delete_style_attr = (style, name) ->
    array = style.split(";")
    list = []
    for item in array
        if item.indexOf(name + ":") == -1
            list.push(item)
    return list.join(";")

update_el_attr = (el, name, value) ->
    origin = el.getAttribute("style") or ""
    style = update_style_attr(origin, name, value)
    el.setAttribute("style", style)

delete_el_attr = (el, name) ->
    origin = el.getAttribute("style") or ""
    style = delete_style_attr(origin, name)
    el.setAttribute("style", style)

__ppt_in_switch = false
__ppt_switch_id = -1

class PptItem extends Widget
    constructor: (@id, @src, @ppt) ->
        super
        @index = parseInt(@id[7..])
        @length = @ppt.images.length
        @img = create_img("", @src, @element)
        @init_position()

    init_position: ->
        if @index == 1 
            update_el_attr(@element, "z-index", 50)
        else if @index == @length
            update_el_attr(@element, "z-index", 50)
        else
            update_el_attr(@element, "z-index", 70)
        left =  (@index + 1 - @length) * 750
        update_el_attr(@element, "-webkit-transform", "translateX(0)")
        update_el_attr(@element, "left", left + "px")

    switch_prev: ->
        if @index == 1
            @index = @length
        else
            @index = @index - 1
        update_el_attr(@element, "-webkit-transform", "translateX(-750px)")
        update_el_attr(@element, "-webkit-transition", "all 0.5s ease-out")

    switch_next: ->
        if @index == @length
            @index = 1
        else
            @index = @index + 1
        update_el_attr(@element, "-webkit-transform", "translateX(750px)")
        update_el_attr(@element, "-webkit-transition", "all 0.5s ease-in")

class Ppt extends Widget
    constructor: (@id, @images)->
        super
        @container = create_element("div", "Container", @element)
        @itemwidth = 750
        @itemheight = 444
        @items = []
        i = 1
        for img in @images
            @create_item(i, img)
            i = i + 1

        @prev_btn = create_element("div", "PrevBtn", @element)
        @prev_btn.addEventListener("click", (e) =>
            if not __ppt_in_switch
                __ppt_in_switch = true
                @switch_prev()
                setTimeout(->
                    __ppt_in_switch = false
                , 1000)
        )

        @next_btn = create_element("div", "NextBtn", @element)
        @next_btn.addEventListener("click", (e) =>
            if not __ppt_in_switch
                __ppt_in_switch = true
                @switch_next()
                setTimeout(->
                    __ppt_in_switch = false
                , 1000)
        )
        __ppt_switch_id = setInterval(->
            Widget.look_up("pptslider")?.switch_next()
        ,5000)

    create_item: (index, img) ->
        item = new PptItem("pptitem" + index, img, @)
        @container.appendChild(item.element)
        @items.push(item)

    switch_prev: ->
        clearInterval(__ppt_switch_id)
        for item in @items
            item.switch_next()
            setTimeout(
                Widget.look_up(item.id)?.init_position()
            , 1000)
        __ppt_switch_id = setInterval(->
            Widget.look_up("pptslider")?.switch_next()
        ,5000)

    switch_next: ->
        clearInterval(__ppt_switch_id)
        for item in @items
            item.switch_prev()
            setTimeout(
                Widget.look_up(item.id)?.init_position()
            , 1000)
        __ppt_switch_id = setInterval(->
            Widget.look_up("pptslider")?.switch_next()
        ,5000)

class Progress extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_extract.png", @titleprogress)

        @progress_container = create_element("div", "ProgressContainer", @element)
        @progressbar = create_element("div", "ProgressBar", @progress_container)
        @light = create_element("div", "ProgressLight", @progress_container)
        @light.style.webkitAnimationName = "progressflash"
        @light.style.webkitAnimationDuration = "5s"
        @light.style.webkitAnimationIterationCount = 1000
        @light.style.webkitAnimationTimingFunction = "cubic-bezier(0, 0, 0.35, -1)"

        @ppt = new Ppt("pptslider", _ppt_list)
        @element.appendChild(@ppt.element)

        @ticker = 0

    update_progress: (progress) ->
        @progressbar.style.width = progress

    show_report: ->
        #@report?.hide_dialog()
        #@report =  new ReportDialog("report")
        #document.body.appendChild(@report.element)
        finish_page = new Finish("finish", false)
        pc.add_page(finish_page)
        pc.remove_page(progress_page)

    handle_extract: (progress) ->
        if progress == "start"
            echo "start handle extract"
            try
                if mount_custom_partitions()
                    @update_progress("5%")
                    #echo "mount custom partitions finish"
                    DCore.Installer.extract_intelligent()
            catch error
                echo error
        else if progress == "finish"
            echo "extract finish"
            try
                DCore.Installer.copy_whitelist()
            catch error
                echo error
            __selected_stage = "chroot"
            @handle_chroot("start")
        else if progress == "terminate"
            echo "extract terminate"
            @show_report()
        else
            @ticker = @ticker + 1
            pgr = 0.05 + @ticker/180*0.8
            if pgr > 0.84
                pgr = 0.84
            @update_progress(pgr*100.toFixed(2) + "%")

    handle_chroot: (progress) ->
        if progress == "start"
            echo "start handle chroot"
            @update_progress("85%")
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
            @update_progress("88%")
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
            @update_progress("91%")
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
            @update_progress("94%")
            try
                DCore.Installer.create_user(__selected_username, __selected_hostname, __selected_password)
            catch error
                echo error
        else if progress == "finish"
            echo "user finish"
            __selected_stage = "bootloader"
            @handle_update_bootloader("start")
        else if progress == "terminate"
            echo "user terminate"
            @show_report()
        else
            echo "invalid progress for handle user"
    
    handle_update_bootloader: (progress) ->
        if progress == "start"
            echo "start handle bootloader"
            @update_progress("95%")
            if __selected_grub != "uefi"
                try
                    DCore.Installer.update_bootloader(__selected_grub, false)
                catch error
                    echo error
            else
                boot = get_boot_part()
                if boot?
                    try
                        DCore.Installer.update_bootloader(boot, true)
                    catch error
                        echo error
                else
                    echo "in uefi but no boot part"
            @update_progress("98%")
        else if progress == "finish"
            echo "finish update bootloader"
            @update_progress("99%")
            finish_page = new Finish("finish", true)
            pc.remove_page(progress_page)
            pc.add_page(finish_page)
        else if progress == "terminate"
            echo "update bootloader terminate"
            @show_report()
        else
            echo "invalid progress for handle bootloader"

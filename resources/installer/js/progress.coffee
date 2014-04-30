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
    if __selected_stage == "terminate"
        return
    if msg.stage == "extract" and __selected_stage == "extract"
        progress_page.handle_extract(msg.progress)
    else if msg.stage == "chroot" and __selected_stage == "chroot"
        progress_page.handle_chroot(msg.progress)
    else if msg.stage == "timezone" and __selected_stage == "timezone"
        progress_page.handle_set_timezone(msg.progress)
    else if msg.stage == "keyboard" and __selected_stage == "keyboard"
        progress_page.handle_set_keyboard(msg.progress)
    else if msg.stage == "locale" and __selected_stage == "locale"
        progress_page.handle_set_locale(msg.progress)
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

__ppt_in_switch = false
__ppt_switch_id = -1
__install_failed = false

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
        if @length > 1
            left =  (@index + 1 - @length) * 750
        else
            left = 0
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

        if @images.length > 1
            @init_switch()

    init_switch: ->
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
        __ppt_switch_id = setInterval(=>
            @switch_next()
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
                item.init_position()
            , 1000)
        __ppt_switch_id = setInterval(=>
            @switch_next()
        ,5000)

    switch_next: ->
        clearInterval(__ppt_switch_id)
        for item in @items
            item.switch_prev()
            setTimeout(
                item.init_position()
            , 1000)
        __ppt_switch_id = setInterval(=>
            @switch_next()
        ,5000)

class Progress extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_part.png", @titleprogress)

        @loading = create_element("div", "Loading", @element)
        @loading_tips = create_element("div", "LoadingTxt", @loading)
        @loading_tips.innerText = _("Prepare for Installation")
        @rotate = create_element("div", "Rotate", @loading)
        @deg = 0
        setInterval(=>
            @update_rotate()
        , 30)

        @progress_container = create_element("div", "ProgressContainer", @element)
        @progress_container.style.display = "none"
        @progressbar = create_element("div", "ProgressBar", @progress_container)
        @light = create_element("div", "ProgressLight", @progress_container)
        @light.style.webkitAnimationName = "progressflash"
        @light.style.webkitAnimationDuration = "5s"
        @light.style.webkitAnimationIterationCount = 1000
        @light.style.webkitAnimationTimingFunction = "cubic-bezier(0, 0, 0.35, -1)"

        @ppt = new Ppt("pptslider", _ppt_list)
        @element.appendChild(@ppt.element)
        @ticker = 0
        @tu = 180
        @display_progress = false
        setTimeout(=>
            if @display_progress == false
                @display_progress = true
                @start_progress()
        , 2000)

    update_rotate: ->
        if @deg > 360
            @deg = 0
        @rotate.style.webkitTransform = "rotate(#{@deg}deg)"
        @deg += 12

    start_progress: ->
        @titleimg.setAttribute("src", "images/progress_extract.png")
        setTimeout(=>
            @loading.style.display = "none"
        ,1000)
        apply_animation(@loading, "loadingout", "1s", "linear")
        apply_animation(@progress_container, "pptin", "2s", "linear")
        apply_animation(@ppt.element, "pptin", "2s", "linear")
        @progress_container.style.display = "block"
        @ppt.element.style.display = "block"

    update_progress: (progress) ->
        @progressbar.style.width = progress

    show_report: ->
        #@report?.hide_dialog()
        #@report =  new ReportDialog("report")
        #document.body.appendChild(@report.element)
        __install_failed = true
        __selected_stage = "terminate"
        finish_page = new Finish("finish", false)
        pc.add_page(finish_page)
        pc.remove_page(progress_page)

    handle_extract: (progress) ->
        if progress == "start"
            echo "start handle extract"
            try
                if _is_use_wubi
                    mount_wubi_partitions()
                else
                    mount_custom_partitions()
                @update_progress("5%")
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
            if progress.length < 4 and progress.indexOf("%") != -1
                pgr = 0.05 + parseFloat(progress)/100 * 0.85
                if pgr > 0.90
                    pgr = 0.90
                @update_progress(pgr*100.toFixed(2) + "%")

    handle_chroot: (progress) ->
        if progress == "start"
            echo "start handle chroot"
            @update_progress("91%")
            try
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
            @update_progress("92%")
            try
                if _is_use_wubi()
                    write_wubi_fs_tab()
                else
                    write_fs_tab()
                if __selected_stage != "terminate"
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
            @update_progress("93%")
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
            __selected_stage = "locale"
            @handle_set_locale("start")
        else if progress == "terminate"
            echo "keyboard terminate"
            @show_report()
        else 
            echo "invalid progress for handle keyboard"

    handle_set_locale: (progress) ->
        if progress == "start"
            echo "start handle locale"
            try
                DCore.Installer.set_target_locale(__selected_locale)
            catch error
                echo error
        else if progress == "finish"
            echo "locale finish"
            __selected_stage = "user"
            @handle_create_user("start")
        else if progress == "terminate"
            echo "locale terminate"
            @show_report()
        else
            echo "invalid progress for handle locale"

    handle_create_user: (progress) ->
        if progress == "start"
            echo "start handle user"
            @update_progress("94%")
            try
                if not __selected_hostname?
                    echo "invalid hostname, use username instead"
                    __selected_hostname = __selected_username
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
            if _is_use_wubi
                update_wubi_bootloader()
            else
                if __selected_grub != "uefi"
                    try
                        DCore.Installer.update_bootloader(__selected_grub, false)
                    catch error
                        echo error
                else
                    boot = get_efi_boot_part()
                    if boot?
                        try
                            DCore.Installer.update_bootloader(boot, true)
                        catch error
                            echo error
                    else
                        echo "in uefi but no boot part"
        else if progress == "finish"
            echo "finish update bootloader"
            @update_progress("99%")
            if __install_failed != true
                finish_page = new Finish("finish", true)
                pc.remove_page(progress_page)
                pc.add_page(finish_page)
        else if progress == "terminate"
            echo "update bootloader terminate"
            @show_report()
        else
            @update_progress(progress)

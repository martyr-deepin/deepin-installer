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


class ReportDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Error Report")
        @report_tips = create_element("p", "", @content)
        @report_tips.innerText = _("System installation failed. Please send the log to Deepin Team.")

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
        @progress_container.style.display = "block"
        @progressbar = create_element("div", "ProgressBar", @progress_container)
        @light = create_element("div", "ProgressLight", @progress_container)
        @light.style.webkitAnimationName = "progressflash"
        @light.style.webkitAnimationDuration = "5s"
        @light.style.webkitAnimationIterationCount = 1000
        @light.style.webkitAnimationTimingFunction = "cubic-bezier(0, 0, 0.35, -1)"

        ppt_method = "iframe"
        switch ppt_method
            when "slider"
                @ppt = new Ppt("pptslider", _ppt_list)
                @element.appendChild(@ppt.element)
            when "iframe"
                @ppt = create_element("iframe","ppt_iframe",@element)
                if document.body.lang is "zh"
                    @ppt.src = "ppt/slideshow2014/index.html"
                else
                    @ppt.src = "ppt/slideshow2014/index_en.html"
            when "video"
                @ppt = create_element("video","ppt_video",@element)
                @ppt.autoplay = "autoplay"
                @ppt.loop = "loop"
                @ppt.width = "750"
                @ppt.width = "450"
                @ppt.src = "ppt/video_test.mp4"

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
            echo "@ppt show"
            @ppt.style.display = "block"
        ,1000)
        apply_animation(@loading, "loadingout", "1s", "linear")
        apply_animation(@progress_container, "pptin", "2s", "linear")
        apply_animation(@ppt, "pptin", "2s", "linear")
        @progress_container.style.display = "block"

    update_progress: (progress) ->
        @progressbar.style.width = progress

    show_report: ->
        __install_failed = true
        __selected_stage = "terminate"
        finish_page = new Finish("finish", false)
        pc.add_page(finish_page)
        pc.remove_page(progress_page)

DCore.signal_connect("install_progress", (per)->
    if per >= 100
        finish_page = new Finish("finish", true)
        pc.remove_page(progress_page)
        pc.add_page(finish_page)
    progress_page?.update_progress("#{per}%")
)

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
        DCore.Installer.finish_install()

class Progress extends Page
    constructor: (@id)->
        super
        @close.style.display = "none"
        @titleimg = create_img("", "images/progress_part.png", @titleprogress)

        @loading = create_element("div", "Loading", @element)
        @loading_tips = create_element("div", "LoadingTxt", @loading)
        @loading_tips.innerText = _("Preparing for Installation")
        @rotate = create_element("div", "Rotate", @loading)
        @deg = 0
        @tid_rotate = setInterval(=>
            @update_rotate()
        , 30)

        @progress_container = create_element("div", "ProgressContainer", @element)
        @progressbar = create_element("div", "ProgressBar", @progress_container)
        setTimeout(@ppt_prepare,500)

    update_rotate: ->
        if @deg > 360
            @deg = 0
        @rotate.style.webkitTransform = "rotate(#{@deg}deg)"
        @deg += 12

    update_progress: (progress) ->
        @progressbar.style.width = progress

    ppt_prepare: =>
        console.log("[progress.cofffee] Progress.ppt_prepare() start")
        @titleimg.setAttribute("src", "images/progress_extract.png")
        @progress_container.style.display = "block"
        @update_progress("1%")

progress_page = new Progress("progress")

DCore.signal_connect("install_progress", (per)->
    if per >= 100
        pc.switch_page(new Finish("finish", true, pc.without_wm))
        return
    progress_page.update_progress("#{per}%")
)
DCore.signal_connect("install_terminate", ->
        pc.switch_page(new Finish("finish", false, pc.without_wm))
)

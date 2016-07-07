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

class Finish extends Page
    constructor: (@id, @succeed, @withoutwm)->
        super
        if @succeed
            @titleimg = create_img("", "images/progress_succeed.png", @titleprogress)
        else
            @titleimg = create_img("", "images/progress_failed.png", @titleprogress)

        @info = create_element("div", "FinishInfo", @element)
        @image = create_img("Image", "", @info)
        @desc = create_element("div", "Desc", @info)
        @detail = create_element("div", "Detail", @info)

        @ops = create_element("div", "FinishOps", @element)
        @now = new NextStep("Now",_("Reboot Now"),@now_cb)
        @ops.appendChild(@now.element)
        @now.next_bt_enable()

        if @succeed
            @image.src = "images/succeed.png"
            @desc.innerText = _("Installation Succeed!")
            @detail.innerText = _("Tips: Please remove the installation media before reboot, so as to avoid reentering into the installation environment when you start the computer next time. ")
            @now.set_text( _("Reboot Now"))
        else
            @image.src = "images/failed.png"
            @desc.innerText = _("Installation Failed!")
            @detail.style.color = "rgba(255,255,255,1.0)"
            @detail.innerText = _("The installation log has been saved on the desktop for you.\n To help us improve the system better, please send the log to Deepin Team. Thank you! ")
            @now.set_text(_("Exit Installation"))
            @detail.addEventListener("click", (e) =>
                DCore.Installer.show_log()
            )
            @detail.addEventListener("mouseover", (e) =>
                @detail.style.color = "red"
            )
            @detail.addEventListener("mouseout", (e) =>
                @detail.style.color = "rgba(255,255,255,0.7)"
            )

        if @withoutwm and not @succeed
            @detail.style.display = "none"

    now_cb: =>
        try
            if DEBUG
                console.debug("[finish.coffee] Finish.now_cb() in DEBUG mode, do nothing")
            else
                DCore.Installer.finish_reboot()
        catch error
            console.error("[finish.coffee] Finish.now_cb() call finish_reboot() failed, error: ", error)

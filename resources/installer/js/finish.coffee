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
    constructor: (@id, @succeed,@without_wm = false)->
        super
        if @succeed
            @titleimg = create_img("", "images/progress_succeed.png", @titleprogress)
        else
            @titleimg = create_img("", "images/progress_failed.png", @titleprogress)

        @info = create_element("div", "FinishInfo", @element)
        @desc = create_element("div", "Desc", @info)
        @detail = create_element("div", "Detail", @info)
        @tips = create_element("div","Tips",@info)

        @ops = create_element("div", "FinishOps", @element)
        @later = create_element("div", "Later", @ops)
        @later_txt = create_element("div", "BtTxt", @later)

        if @succeed
            @desc.innerText = _("Installation Succeed!")
            @detail.innerText = _("You need to restart the computer to complete installation.")
            @tips.innerText = _("Tips: Please remove the installation media before reboot, so as to avoid reentering into the installation environment when you start the computer next time. ")
            @later.setAttribute("class", "Later")
            @later_txt.innerText = _("Reboot Later")
        else
            @desc.innerText = _("Installation Failed!")
            @detail.style.color = "rgba(255,255,255,1.0)"
            @detail.innerText = _("The installation log has been saved on the desktop for you.\n To help us improve the system better, please send the log to Deepin Team. Thank you! ")
            @detail.addEventListener("click", (e) =>
                DCore.Installer.show_log()
            )
            @detail.addEventListener("mouseover", (e) =>
                @detail.style.color = "red"
            )
            @detail.addEventListener("mouseout", (e) =>
                @detail.style.color = "rgba(255,255,255,0.7)"
            )
            @later.setAttribute("class", "Startover")
            @later_txt.innerText = _("Start Over")

        @later.addEventListener("click", (e) =>
            if @succeed
                try
                    DCore.Installer.finish_install()
                catch error
                    echo error
            else
                try
                    DCore.Installer.restart_installer()
                catch error
                    echo error
        )

        @now = create_element("div", "Now", @ops)
        @now_txt = create_element("div", "BtTxt", @now)
        @now_txt.innerText = _("Reboot Now")
        @now.addEventListener("click", (e) =>
            try
                DCore.Installer.finish_reboot()
            catch error
                echo error
        )

        #just hide later/now when failed
        if not @succeed
            @later.style.display = "none"
            @now.style.display = "none"

        #just hide close/later/tips in without_wm
        if @without_wm
            @later.style.display = "none"
            @tips.style.display = "none"
            @now.style.display = "block"
            @ops.setAttribute("class","FinishOpsForAutoMode")
            @now.setAttribute("class", "NowForAutoMode")
            @now_txt.setAttribute("class", "BtTxtForAutoMode")
            if not @succeed
                @detail.style.display = "none"
                @now_txt.innerText = _("Exit Installation")

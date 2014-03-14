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
    constructor: (@id, @succeed)->
        super
        if @succeed
            @titleimg = create_img("", "images/progress_succeed.png", @titleprogress)
        else
            @titleimg = create_img("", "images/progress_failed.png", @titleprogress)

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        @info = create_element("div", "FinishInfo", @element)
        @desc = create_element("div", "Desc", @info)
        @detail = create_element("div", "Detail", @info)

        @ops = create_element("div", "FinishOps", @element)

        @later = create_element("div", "", @ops)
        @later_txt = create_element("div", "Txt", @later)
        if @succeed
            @desc.innerText = _("Congratulations!")
            @detail.innerText = _("You need to restart the computer to complete installation.")
            @later.setAttribute("class", "Later")
            @later_txt.innerText = _("Reboot later")
        else
            @desc.innerText = _("Installation failed!")
            @detail.innerText = _("Please send the log to Linux Deepin Team.")
            @later.setAttribute("class", "Startover")
            @later_txt.innerText = _("Start over")

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
        @now_txt = create_element("div", "Txt", @now)
        @now_txt.innerText = _("Reboot now")
        @now.addEventListener("click", (e) =>
            try
                DCore.Installer.finish_reboot()
            catch error
                echo error
        )

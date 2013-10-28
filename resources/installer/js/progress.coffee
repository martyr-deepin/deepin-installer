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

class Progress extends Page
    constructor: (@id)->
        super
        @title.innerHTML = "<span>欢迎安装使用 Linux Deepin <sup>12.12</sup></span>"

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
        echo "update progress"

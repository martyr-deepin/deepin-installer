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
        @title.innerText = "欢迎安装Linux Deepin"

        @ppt = create_element("div", "PPT", @content)
        @ppt_img = create_img("PptImg", "images/deepin.jpg", @ppt)
        @progress_container = create_element("div", "ProgressContainer", @footer)
        @progressbar = create_element("div", "ProgressBar", @progress_container)

    update_progress: (progress) ->
        echo "pass"

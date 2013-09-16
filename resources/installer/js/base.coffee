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

__current_page = null

_ppt_list = ["images/deepin.jpg", "images/deepin1.jpg", "images/deepin2.jpg", 
             "images/deepin3.jpg", "images/deepin4.jpg"]

_color_list = ["#89AFD0","#6D91BC","#E8948A","#DE5F4E","#C2A02D",
               "#94A92C","#876763","#6AA9B4","#B0823E","#8C3F4D",
               "#84CfD3","#6E56A1","#9C76A1","#BF92D9","#DFBEEB",
               "#B59479","#91C9ED","#F06693","#DE4EA3","#3EB0A4",
               "#A25EE1","#9DD089","#D05793","#D5426C"]

class Page extends Widget
    constructor: (@id)->
        super

class PageContainer extends Widget
    constructor: (@id)->
        super

    add_page: (page_id) ->
        echo "add page"
        try
            @element.appendChild(page_id.element)
        catch error
            echo error

    remove_page: (page_id) ->
        echo "remove page"
        try
            @element.removeChild(page_id.element)
        catch error
            echo error

    switch_page: (old_page, new_page) ->
        echo "switch page"

    current_page:
        echo "current_page"

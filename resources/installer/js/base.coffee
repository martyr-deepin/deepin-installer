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

class Page extends Widget
    constructor: (@id)->
        super
        @title = create_element("div", "Title", @element)
        @content = create_element("div", "Content", @element)
        @footer = create_element("div", "Footer", @element)

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

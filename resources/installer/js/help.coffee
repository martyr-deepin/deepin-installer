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

class Help extends Widget
    constructor: (@id) ->
        super
        @title = create_element("div", "Title", @element)
        @text = create_element("p", "", @title)
        @text.innerText = "安装帮助"

        @link = create_element("div", "Link", @title)
        @add_link = create_element("span", "", @link)
        @add_link.innerHTML = "<a href=#add>" + "新建分区" + "</a>"
        @link_sep = create_element("span", "", @link)
        @link_sep.innerText = " | "
        @delete_link = create_element("span", "", @link)
        @delete_link.innerHTML = "<a href=#delete>" + "删除分区" + "</a>"

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_help()
        )

        @content = create_element("div", "Content", @element)
        @fill_content()

    exit_help: ->
        DCore.InstallerHelp.exit()

    fill_content: ->
        @new = create_element("p", "", @content)
        @new_txt = create_element("a", "", @new)
        @new_txt.innerText = "新建分区"
        @new_txt.setAttribute("name", "add")
        @new_img = create_img("", "images/new.png", @new)

        @delete = create_element("p", "", @content)
        @delete_txt = create_element("a", "", @delete)
        @delete_txt.innerText = "删除分区"
        @delete_txt.setAttribute("name", "delete")
        @delete_img = create_img("", "images/delete.png", @delete)

        @confirm_delete = create_element("p", "", @content)
        @confirm_delete_txt = create_element("div", "", @confirm_delete)
        @confirm_delete_txt.innerText = "确认删除"
        @confirm_delete_img = create_img("", "images/confirm_delete.png", @confirm_delete)

help = new Help()
document.body.appendChild(help.element)

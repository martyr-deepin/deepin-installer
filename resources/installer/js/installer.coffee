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
    
pc = new PageContainer("pc")
document.body.appendChild(pc.element)

#welcome_page = new Welcome("welcome")
#pc.add_page(welcome_page)

#part_page = new Part("part")
#pc.add_page(part_page)
#welcome_page.start.addEventListener("click", (e) =>
#    pc.add_page(part_page)
#    pc.remove_page(welcome_page)
#    __selected_item?.focus()
#)

progress_page = new Progress("progress")
pc.add_page(progress_page)
#part_page.next_btn.addEventListener("click", (e) =>
#    do_partition()
#    pc.add_page(progress_page)
#    pc.remove_page(part_page)
#)
#
#finish_page = new Finish("finish")
#progress_page.progressbar.addEventListener("click", (e) =>
#    pc.add_page(finish_page)
#    pc.remove_page(progress_page)
#)

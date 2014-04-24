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

determine_target_id = (target) ->
    if target.indexOf("/dev/") == -1
        throw "invalid target to install"
    for disk in disks
        if m_disk_info[disk]["path"] == target
            __selected_target = disk
            return disk
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["path"] == target
                __selected_target = part
                return part
    return null

determine_target_home_id = (target) ->
    if target.indexOf("/dev/") == -1
        return null
    if target == __selected_target
        return null
    for disk in disks
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["path"] == target and m_part_info[part]["type"] in ["normal", "logical"]
                __selected_home = part

fetch_install_info = ->
    try
        info = DCore.Installer.get_installation_info()
        if info["username"] != "deepin" and info["username"] in DCore.Installer.get_system_users().indexOf(info["username"])
            throw "invalid username"
        for c in info["username"]
            if c in __illegal_keys
                throw "invalid username"
        __selected_username = info["username"]
        __selected_password = info["password"]
        if info["hostname"]? and info["hostname"].length > 0
            __selected_hostname = info["hostname"]
        else
            __selected_hostname = __selected_username
        if info["timezone"]? and info["timezone"] not in DCore.Installer.get_timezone_list().indexOf(info["timezone"])
            __selected_timezone = info["timezone"]
        else
            __selected_timezone = "Asia/Shanghai"
        if info["layout"]? and info["layout"] in DCore.Installer.get_keyboard_layouts()
            __selected_layout = info["layout"]
            if info["variant"]? and info["variant"] in DCore.Installer.get_layout_variants(info["layout"])
                __selected_layout = info["layout"] + "," + info["variant"]
        else
            __selected_layout = "us"
        if info["locale"]? and info["locale"].length > 0
            __selected_locale = info["locale"]
        else
            __selected_locale = "zh_CN.UTF-8"

        determine_target_id(info["target"])
        if info["home"]
            determine_target_home_id(info["home"])

        if info["grub"]?
            if info["grub"] == "uefi"
                __selected_grub = "uefi"
            else if info["grub"] in disks
                __selected_grub = info["grub"]
            else 
                for disk in disks
                    for part in v_disk_info[disk]["partitions"]
                        if info["grub"] == v_part_info[part]["path"] and v_part_info[part]["type"] in ["normal", "logical"]
                            __selected_grub = info["grub"]
        if not __selected_grub?
            if __selected_target.indexOf("part") != -1
                __selected_grub = v_part_info[__selected_target]["disk"]
            else
                __selected_grub = __selected_target
    catch error
        throw error

handle_automatic_install = ->
    if __init_parted_finish
        try
            undo_part_table_info()
            fetch_install_info()
            if not __selected_target?
                throw "invalid __selected_target"
            if __selected_target.indexOf("disk") != -1
                do_simple_partition(__selected_target, "disk")
            else if __selected_target.indexOf("part") != -1
                do_simple_partition(__selected_target, "part")
            else
                throw "invalid __selected_target"
            progress_page = new Progress("progress")
            pc.add_page(progress_page)
        catch error
            echo error
            welcome_page = new Welcome("welcome")
            pc.add_page(welcome_page)
    else
        setTimeout(=>
            handle_automatic_install()
        , 1000)

if DCore.Installer.is_installation_auto()
    handle_automatic_install()
else
    welcome_page = new Welcome("welcome")
    pc.add_page(welcome_page)

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


#there hasn't the concept of disk sector_size!
#all the size unit is Byte, use these constant convert units which you want.
KB = 1024.0
MB = 1024 * KB
GB = 1024 * MB

BOOT_SIZE_MIN = 300 * MB
minimum_disk_size_required = 8 * GB
# An additional trace or MEBIBYTE is required for the Master Boot Record or
# Extended Boot Record. Or else it is unable to install grub into filesystems
# like btrfs.
MINIMUM_PRECEDING_SIZE = 1 * MB

DCore.signal_connect("used", (msg) ->
    v_part_info[msg.part]["used"] = msg.free
    m_part_info[msg.part]["used"] = msg.free
    Widget.look_up(msg.part)?.update_part_used()
)

random_list = []
get_random_color = ->
    if random_list.length == 0
        for item in _color_list
            random_list.push(item)
    random_list.splice(0,1)

#
#Model
#Model: for origin disk partition table
#disks = DCore.Installer.list_disks()
disks = null

is_match_install_require = ->
    for disk in DCore.Installer.list_disks()
        if DCore.Installer.get_disk_size(disk) > minimum_disk_size_required
            return true
    return false

m_disk_info = {}
#never change the partitions list
init_m_disk_info = ->
    m_disk_info = {}
    for disk in disks
        m_disk_info[disk] = {}
        m_disk_info[disk]["change"] = false
        m_disk_info[disk]["path"] = v_disk_info[disk]["path"]
        m_disk_info[disk]["partitions"] = []
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["type"] in ["normal", "extended", "logical", "freespace"]
                m_disk_info[disk]["partitions"].push(part)

m_part_info = {}
#may had part not in m_disk_info partitions list
init_m_part_info = ->
    m_part_info = {}
    for disk in disks
        for part in m_disk_info[disk]["partitions"]
            m_part_info[part] = {}
            m_part_info[part]["disk"] = disk
            m_part_info[part]["used"] = v_part_info[part]["used"]
            m_part_info[part]["type"] = v_part_info[part]["type"]
            m_part_info[part]["start"] = v_part_info[part]["start"]
            m_part_info[part]["length"] = v_part_info[part]["length"]
            m_part_info[part]["end"] = v_part_info[part]["end"]
            m_part_info[part]["fs"] = v_part_info[part]["fs"]
            m_part_info[part]["format"] = false
            m_part_info[part]["mp"] = v_part_info[part]["mp"]
            m_part_info[part]["path"] = v_part_info[part]["path"]
            m_part_info[part]["color"] = v_part_info[part]["color"]
            m_part_info[part]["width"] = v_part_info[part]["width"]
            m_part_info[part]["os"] = v_part_info[part]["os"]
            m_part_info[part]["label"] = v_part_info[part]["label"]
            m_part_info[part]["lvm"] = v_part_info[part]["lvm"]
            m_part_info[part]["op"] = "keep"
            m_part_info[part]["path"] = v_part_info[part]["path"]

#sort part op flags as below:
#1)first delete, then update and add
#2)delete logical first than delete extended
#3)add extended first then add logical
#4)update extended first then logcial when size grown
#5)update logical first then exnteded when size reduce
_sort_part_op =  (part_a, part_b) ->
    if m_part_info[part_a]["op"] == "delete"
        if m_part_info[part_a]["type"] == "extended"
            return 1
        else
            return -1
    else if m_part_info[part_a]["op"] == "update"
        if m_part_info[part_a]["type"] == "extended"
            if m_part_info[part_a]["length"] < v_part_info[part_a]["length"]
                return -1
            else
                return 1
        else if m_part_info[part_b]["type"] == "extended"
            if m_part_info[part_b]["length"] < v_part_info[part_b]["length"]
                return 1
            else
                return -1
        else
            return -1
    else if m_part_info[part_a]["op"] == "add"
        if m_part_info[part_a]["type"] == "extended"
            return -1
        else
            return 1
    else
        console.error("[part_util.coffee] _sort_part_op() error, invalid op: ", m_part_info[part_a]["op"])
        return -1

_sort_disk = (disk_a, disk_b) ->
    a_path = DCore.Installer.get_disk_path(disk_a)
    b_path = DCore.Installer.get_disk_path(disk_b)
    if a_path < b_path
        return -1
    else
        return 1

_filter_modeled_delete = (part, index) ->
    return m_part_info[part]["op"] == "delete"

_filter_modeled_update = (part, index) ->
    return m_part_info[part]["op"] == "update"

_filter_modeled_add = (part, index) ->
    return m_part_info[part]["op"] == "add"

#include original to delete part and virtual new part
get_modeled_partitions = (disk) ->
    console.log("[part_util.coffee] get_modeled_partitons: #{disk}")
    partitions = []
    for part of m_part_info
        if m_part_info[part]["disk"] == disk and m_part_info[part]["op"] != "keep"
            partitions.push(part)

    delete_partitions = partitions.filter(_filter_modeled_delete).sort(_sort_part_op)
    update_partitions = partitions.filter(_filter_modeled_update).sort(_sort_part_op)
    add_partitions = partitions.filter(_filter_modeled_add).sort(_sort_part_op)

    modeled = []
    for part in delete_partitions
        modeled.push(part)
    for part in update_partitions
        modeled.push(part)
    for part in add_partitions
        modeled.push(part)

    return modeled

#mark update for original part
mark_update = (part) ->
    console.log("[part_util.coffee] mark_update() part: #{part}, #{m_part_info[part]}")
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        m_disk_info[v_part_info[part]["disk"]]["change"] = true
        if m_part_info[part]["op"] not in ["keep", "update"]
            console.error("[part_util.coffee] mark_update(), should not reach for mark update, op: " + m_part_info[part]["op"])
        m_part_info[part]["op"] = "update"
    else
        console.warn("[part_util.coffee] mark_update(), skip mark update for new partition, " + m_part_info[part])

#mark add flag for new partition
mark_add = (part) ->
    console.log("[part_util.coffee] mark_add() part: #{part}, #{m_part_info[part]}")
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        console.error("[part_util.coffee] mark_add() error in mark add, part originally exists, #{part}, #{m_disk_info[v_part_info][part]}")
    else
        m_disk_info[v_part_info[part]["disk"]]["change"] = true
        m_part_info[part] = {}
        m_part_info[part]["op"] = "add"
        m_part_info[part]["disk"] = v_part_info[part]["disk"]
        m_part_info[part]["type"] = v_part_info[part]["type"]
        m_part_info[part]["fs"] = v_part_info[part]["fs"]
        m_part_info[part]["start"] = v_part_info[part]["start"]
        m_part_info[part]["length"] = v_part_info[part]["length"]
        m_part_info[part]["end"] = v_part_info[part]["end"]
        m_part_info[part]["mp"] = v_part_info[part]["mp"]

#mark delete for original part, delete from flags for new partition
mark_delete = (part) ->
    console.log("[part_util.coffee] mark_delete() part: #{part}")
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        if m_part_info[part]["op"] in ["keep", "update"]
            m_disk_info[v_part_info[part]["disk"]]["change"] = true
            m_part_info[part]["op"] = "delete"
        else
            console.error("[part_util.coffee] mark_delete() invalid flag for orig partition to mark delete, #{part}, #{m_part_info[part]}")
    else
        if m_part_info[part]["op"] == "add"
            m_disk_info[v_part_info[part]["disk"]]["change"] = true
            delete m_part_info[part]
        else
            console.error("[part_util.coffee] mark_delete() invalid flag for new part to mark delete, #{part}")

__update_fs_json = (part, fs) ->
    return '{"op":"update_fs","part":"#{part}","fs":"#{fs}"}'

#do real add/delete/update partition operation
do_partition = ->
    console.log("[part_util.coffee] do_partition()")
    for disk in disks
        if m_disk_info[disk]["change"] != true
            continue
        for part in get_modeled_partitions(disk)
            switch m_part_info[part]["op"]
                when "delete"
                    try
                        DCore.Installer.delete_disk_partition(part)
                    catch error
                        console.error("[part_util.coffee] call delete_disk_partition() failed, part: #{part}")
                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        console.error("[part_util.coffee] call write_disk() failed, disk: #{disk}")

                when "update"
                    if m_part_info[part]["start"] != v_part_info[part]["start"] or m_part_info[part]["length"] != v_part_info[part]["length"]
                        try
                            DCore.Installer.update_partition_geometry(part, v_part_info[part]["start"], v_part_info[part]["length"])
                        catch error
                            console.error("[part_util.coffee] do_partition() call update_partition_geometry() failed: #{error}")
                        try
                            DCore.Installer.write_disk(disk)
                        catch error
                            console.error("[part_util.coffee] do_partition() call write_disk() failed: #{error}")

                    if m_part_info[part]["fs"] != v_part_info[part]["fs"] or v_part_info[part]["format"]
                        if v_part_info[part]["fs"] not in ["", "unused"]
                            try
                                ret = DCore.Installer.update_partition_fs(part, v_part_info[part]["fs"])
                            catch error
                                console.error("[part_util.coffee] do_partition() call update_partition_fs() failed: #{error}")
                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        console.error("[part_util.coffee] do_partition() call write_disk() error: #{error}, disk: #{disk}")

                when "add"
                    try
                        DCore.Installer.new_disk_partition(part, disk, m_part_info[part]["type"], m_part_info[part]["fs"], m_part_info[part]["start"], m_part_info[part]["end"])
                    catch error
                        console.error("[part_util.coffee] do_partition() call new_disk_partition() error: #{error}, part: #{part}, disk: #{disk}")
                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        console.error("[part_util.coffee] do_partition() call write_disk() error: #{error}, disk: #{disk}")

                    if v_part_info[part]["type"] != "extended"
                        try
                            DCore.Installer.update_partition_fs(part, v_part_info[part]["fs"])
                        catch error
                            console.error("[part_util.coffee] do_partition() call update_partition_fs() error: #{error}, part: #{part}")
                        try
                            DCore.Installer.write_disk(disk)
                        catch error
                            console.error("[part_util.coffee] do_partition() call write_disk() error: #{error}, disk: #{disk}")

#auto partition for simple mode
auto_simple_partition = (device, type) ->
    console.log("[part_util.coffee] auto_simple_partition(), device: #{device}, type: #{type}")
    if type == "part"
        #create a new part when install to freespace
        if m_part_info[device]["type"] == "freespace"
            partid = device
            if is_in_extended(partid)
                type = "logical"
            else
                type = "normal"
            size = v_part_info[partid]["length"]
            align = "start"
            fs = "ext4"
            mp = "/"
            add_part(partid, type, size, align, fs, mp)
        #just update the part fs and mp to install
        else if m_part_info[device]["type"] in ["normal", "logical"]
            update_part_fs(device,"ext4")
            update_part_format(device, true)
            update_part_mp(device,"/")
        else
            console.error("[part_util.coffee] auto_simple_partition(), invalid as extended doesn't show in simple view")
        if __selected_home?
            update_part_mp(__selected_home, "/home")
    else
        console.error("[part_util.coffee] auto_simple_partition(), invalid type to do simple partition")

#get recommand part to install target
get_recommand_target = ->
    recommand = null
    for disk in disks
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["type"] != "extended"
                if recommand?
                    if m_part_info[part]["length"] > m_part_info[recommand]["length"]
                        recommand = part
                else
                    recommand = part
    return recommand

#get target part that mount root
get_target_part = ->
    console.log("[part_util.coffee] get_target_part()")
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["mp"] == "/"
                return part
    return null

#check whether had a part that mount root
check_target_part = ->
    if get_target_part()?
        return true
    else
        return false

#when had partition mount, should attention him unmount first before part operation
check_has_mount = (disk)->
    for part in m_disk_info[disk]["partitions"]
        if DCore.Installer.is_partition_busy(part)
            return true
    return false

#when only swap partition mount, should not show dialog of attention him unmount first before part operation
check_only_swap_mount = (disk)->
    for part in m_disk_info[disk]["partitions"]
       if DCore.Installer.is_partition_busy(part)
            if v_part_info[part]["type"] != "extended"
                if v_part_info[part]["fs"] != "swap"
                    return false
    return true

get_efi_boot_part = ->
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["fs"] == "efi"
                return part
    return null


#Model end
#Model
#

#
#View
#View: for data display in UI
v_disk_info = {}
init_v_disk_info = ->
    disks = DCore.Installer.list_disks()
    disks.sort(_sort_disk)
    v_disk_info = {}
    for disk in disks
        v_disk_info[disk] = {}
        v_disk_info[disk]["length"] = DCore.Installer.get_disk_size(disk)
        v_disk_info[disk]["model"] = DCore.Installer.get_disk_model(disk)
        v_disk_info[disk]["max_primary"] = DCore.Installer.get_disk_max_primary_count(disk)
        v_disk_info[disk]["path"] = DCore.Installer.get_disk_path(disk)
        v_disk_info[disk]["type"] = DCore.Installer.get_disk_type(disk)
        v_disk_info[disk]["partitions"] = []
        for part in DCore.Installer.get_disk_partitions(disk)
            type = DCore.Installer.get_partition_type(part)
            if type == "freespace"
                v_disk_info[disk]["partitions"].push(part)
            else if type in ["normal", "extended", "logical"]
                v_disk_info[disk]["partitions"].push(part)

v_part_info = {}
init_v_part_info = ->
    v_part_info = {}
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            v_part_info[part] = {}
            v_part_info[part]["type"] = DCore.Installer.get_partition_type(part)
            v_part_info[part]["disk"] = disk
            v_part_info[part]["used"] = "unknown"
            v_part_info[part]["start"] = DCore.Installer.get_partition_start(part)
            v_part_info[part]["length"] = DCore.Installer.get_partition_size(part)
            v_part_info[part]["end"] = DCore.Installer.get_partition_end(part)
            v_part_info[part]["mp"] = "unused"
            v_part_info[part]["path"] = DCore.Installer.get_partition_path(part)
            v_part_info[part]["color"] = get_random_color()
            v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
            v_part_info[part]["format"] = false
            if v_part_info[part]["type"] != "freespace"
                DCore.Installer.get_partition_free (part)
                v_part_info[part]["fs"] = DCore.Installer.get_partition_fs(part)
                if not v_part_info[part]["fs"]?
                    v_part_info[part]["fs"] = "unused"
                else if v_part_info[part]["fs"] == "fat32"
                    if DCore.Installer.get_partition_flag(part, "boot") == true
                        if v_disk_info[disk]["type"] == "gpt"
                            v_part_info[part]["fs"] = "efi"
                v_part_info[part]["os"] = DCore.Installer.get_partition_os(part)
                v_part_info[part]["label"] = DCore.Installer.get_partition_label(part)
                v_part_info[part]["lvm"] = DCore.Installer.get_partition_flag(part, "lvm")
            else
                v_part_info[part]["fs"] = "unused"
                v_part_info[part]["os"] = null
                v_part_info[part]["label"] = null
                v_part_info[part]["lvm"] = null

get_disk_fake_length = (disk) ->
    sort_v_disk_info(disk)
    length = 0
    for part in v_disk_info[disk]["partitions"]
        if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
            length += v_part_info[part]["length"]
    v_disk_info[disk]["length"] = length

sync_part_os = ->
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["type"] != "freespace"
                v_part_info[part]["os"] = DCore.Installer.get_partition_os(part)
        for part in m_disk_info[disk]["partitions"]
            m_part_info[part]["os"] = DCore.Installer.get_partition_os(part)
        if Widget.look_up("part_table")?
            for item in Widget.look_up("part_table").partitems
                item.update_device_os()

_sort_part_geom = (part_a, part_b) ->
    if v_part_info[part_a]["start"] == v_part_info[part_b]["start"]
        if v_part_info[part_a]["type"] == "extended"
            return -1
        else if v_part_info[part_b]["type"] == "extended"
            return 1
        else
            console.error("[part_util.coffee] _sort_part_geom() error, part type is invalid: #{v_part_info[part_a]}, #{v_part_info[part_b]}")
            return v_part_info[part_a]["end"] - v_part_info[part_b]["end"]
    else
        return v_part_info[part_a]["start"] - v_part_info[part_b]["start"]

sort_v_disk_info = (disk) ->
    v_disk_info[disk]["partitions"].sort(_sort_part_geom)

#below get methods shoud gurantee the v_part_info dict fresh(please set data first)
_filter_extended = (part, index) ->
    return v_part_info[part]["type"] == "extended"

get_extended_partition = (disk) ->
    extended =  v_disk_info[disk]["partitions"].filter(_filter_extended)
    if extended? and extended.length > 0
        return extended[0]

is_in_extended = (part) ->
    if v_part_info[part]["type"] in ["normal", "extended"]
        return false
    else if v_part_info[part]["type"] == "logical"
        return true
    else if v_part_info[part]["type"] == "freespace"
        extended = get_extended_partition(v_part_info[part]["disk"])
        if extended?
            if v_part_info[part]["start"] >= v_part_info[extended]["start"] and v_part_info[part]["end"] <= v_part_info[extended]["end"]
                return true
            else
                return false
        else
            return false
    else
        console.error("[part_util.coffee] is_in_extended() error, unknown part type: #{v_part_info[part]["type"]}")
        return false

_filter_in_extended = (part, index) ->
    return is_in_extended(part)

_filter_not_in_extended = (part, index) ->
    return not is_in_extended(part)

_filter_normal = (part, index) ->
    return v_part_info[part]["type"] == "normal"

_filter_primary = (part, index) ->
    return v_part_info[part]["type"] in ["normal", "extended"]

_filter_logical = (part, index) ->
    return v_part_info[part]["type"] == "logical"

_filter_freespace = (part, index) ->
    return v_part_info[part]["type"] == "freespace"

#list of normal/freespace/extended partitions
get_main_blocks = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_not_in_extended)

#list of logical/freespace partitions
get_secondary_blocks = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_in_extended)

get_normal_partitions = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_normal)

get_primary_partitions = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_primary)

get_logical_partitions = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_logical)

get_freespace_partitions = (disk) ->
    return v_disk_info[disk]["partitions"].filter(_filter_freespace)

get_part_num = (part) ->
    if v_part_info[part]["type"] not in ["normal", "logical", "extended"]
        console.error("[part_util.coffee] get_part_num() invalid part type: #{v_part_info[part]["type"]}, part: #{part}")
        return -1
    if v_part_info[part]["path"]?
        return parseInt(v_part_info[part]["path"].replace(/\D/g, ''))
    else
        console.error("[part_util.coffee] get_part_num() invalid part path: #{v_part_info[part]["path"]}, part: #{part}")
        return -1

is_in_same_block = (part_a, part_b) ->
    disk = v_part_info[part_a]["disk"]
    main_blocks = get_main_blocks(disk)
    if part_a in main_blocks
        return part_b in main_blocks
    else
        return part_b not in main_blocks

#get prev part:include freespace block
get_prev_part = (part) ->
    disk = v_part_info[part]["disk"]
    sort_v_disk_info(disk)
    main_blocks = get_main_blocks(disk)
    secondary_blocks = get_secondary_blocks(disk)

    if part in main_blocks
        index = main_blocks.indexOf(part)
        if index > 0
            return main_blocks[index-1]

    else if part in secondary_blocks
        index = secondary_blocks.indexOf(part)
        if index > 0
            return secondary_blocks[index-1]

    else
        console.error("[part_util.coffee] get_prev_part() error: part #{part} is neither in main_blocks nor in secondary_blocks")

#get next part:include freespace block
get_next_part = (part) ->
    disk = v_part_info[part]["disk"]
    sort_v_disk_info(disk)
    main_blocks = get_main_blocks(disk)
    secondary_blocks = get_secondary_blocks(disk)

    if part in main_blocks
        index = main_blocks.indexOf(part)
        if index < main_blocks.length-1
            return main_blocks[index+1]

    else if part in secondary_blocks
        index = secondary_blocks.indexOf(part)
        if index < secondary_blocks.length - 1
            return secondary_blocks[index+1]

    else
        console.error("[part_util.coffee] get_next_part() error: part #{part} is neither in main_blocks nor in secondary_blocks")

#whether can add a normal partition in the freespace
can_add_normal = (part) ->
    if v_part_info[part]["type"] != "freespace"
        return false

    disk = v_part_info[part]["disk"]
    if get_primary_partitions(disk).length >  v_disk_info[disk]["max_primary"] - 1
        return false
    #whether the part between two logical partititons
    if is_in_extended(part)
        logical_before = false
        logical_after = false

        for logical in get_logical_partitions(disk)
            if v_part_info[logical]["end"] < v_part_info[part]["start"]
                logical_before = true

            if v_part_info[logical]["start"] > v_part_info[part]["end"]
                logical_after = true

        if logical_before and logical_after
            return false
    return true

#whether can add a logical partition in the freespace
can_add_logical = (part) ->
    if v_part_info[part]["type"] != "freespace"
        return false

    disk = v_part_info[part]["disk"]
    if v_disk_info[disk]["type"] == "gpt"
        return false

    extended = get_extended_partition(disk)
    if extended?
    #whether has a normal between the free and extended part
        if v_part_info[part]["end"] < v_part_info[extended]["start"]
            for normal in get_normal_partitions(disk)
                if v_part_info[normal]["start"] > v_part_info[part]["end"] and v_part_info[normal]["end"] < v_part_info[extended]["start"]
                    return false

        if v_part_info[part]["start"] > v_part_info[extended]["end"]
            for normal in get_normal_partitions(disk)
                if v_part_info[normal]["start"] > v_part_info[extended]["end"] and v_part_info[normal]["end"] < v_part_info[part]["start"]
                    return false
    else
    #whether normal partitions occupied all primary count
        if get_normal_partitions(disk).length == v_disk_info[disk]["max_primary"]
            return false

    return true
#View end
#View
#

#
#Control
#Control: communicate with model and view
update_part_fs = (part, fs) ->
    console.log("[part_util.coffee] update_part_fs(), part: #{part}, fs: #{fs}, path: #{v_part_info[part]["path"]}")
    v_part_info[part]["fs"] = fs
    if fs in ["unused", "fat16", "fat32", "ntfs", "swap", "efi"]
        v_part_info[part]["mp"] = "unused"
        Widget.look_up(part)?.fill_mount()
    Widget.look_up(part)?.fill_format()
    mark_update(part)

update_part_format = (part, format) ->
    console.log("[part_util.coffee] update_part_format(), part: #{part}, format: #{format}, path: #{v_part_info[part]["path"]}")
    v_part_info[part]["format"] = format
    mark_update(part)

update_part_mp = (part, mp) ->
    console.log("[part_util.coffee] update_part_mp(), part: #{part}, mp: #{mp}")
    v_part_info[part]["mp"] = mp
    mark_update(part)


#when add part, set its path to max plus 1, after mark the part type
#when delete part, set others part minus 1, before mark the part type
#--------------------
#TODO: above logic is wrong!!!!
#TODO: Rewrite above code, don't try mock the logic, let Parted do it.
#next partition number is not equal to the last parition number + 1.
#and the partition path is not equal disk_path + partition num
update_part_display_path = (part, op) ->
    disk = v_part_info[part]["disk"]
    disk_path = v_disk_info[disk]["path"]
    if op == "add"
        maxnum = 0
        if v_part_info[part]["type"] == "logical"
            maxnum = 4 #logical partition num is start at 5
            for item in get_logical_partitions(disk)
                part_num = get_part_num(item)
                if part_num > maxnum
                    maxnum = part_num
            v_part_info[part]["path"] = disk_path + (maxnum + 1)
        else if v_part_info[part]["type"] in ["normal", "extended"]
            path_list = get_primary_partitions(disk).map(get_part_num).sort()
            part_num = 1
            while part_num in path_list
                part_num++
            v_part_info[part]["path"] = disk_path + part_num
        else
            console.error("[part_util.coffee] update_part_display_path(), invalid part type: #{v_part_info[part]["type"]}, part: #{part}, op: #{op}")

    else if op == "delete"
        current_num = get_part_num(part)
        if v_part_info[part]["type"] == "logical"
            for item in get_logical_partitions(disk)
                part_num = get_part_num(item)
                if part_num > current_num
                    v_part_info[item]["path"] = disk_path + (part_num - 1)
            v_part_info[part]["path"] = ""
        else if v_part_info[part]["type"] in ["normal", "extended"]
            v_part_info[part]["path"] = ""
        else
            console.error("[part_util.coffee] update_part_display_path(), invalid part type: #{v_part_info[part]["type"]}, part: #{part}, op: #{op}")

    else
        console.error("[part_util.coffee] update_part_display_path(), invalid op type: #{op}, part: #{part}")

get_selected_mp = ->
    mp_list = []
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["mp"]? and v_part_info[part]["mp"] != "unused"
                mp_list.push(v_part_info[part]["mp"])
    return mp_list

get_mp_partition = (mp) ->
    for disk in disks
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["mp"]? and v_part_info[part]["mp"] == mp
                return part
    return null

#delete normal partition, merge slibing freespace
_delete_normal = (disk, part) ->
    console.log("[part_util.coffee] _delete_normal(), disk: #{disk}, part: #{part}")
    prev = get_prev_part(part)
    if prev?
        if v_part_info[prev]["type"] == "freespace"
            v_part_info[part]["start"] = v_part_info[prev]["start"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            prev_index = v_disk_info[disk]["partitions"].indexOf(prev)
            v_disk_info[disk]["partitions"].splice(prev_index, 1)
            delete v_part_info[prev]

        else if v_part_info[prev]["type"] == "extended"
            secondary_blocks = get_secondary_blocks(disk)
            if secondary_blocks.length > 0
                extended_last = secondary_blocks[-1]

            if extended_last? and v_part_info[extended_last]["type"] == "freespace"
                v_part_info[part]["start"] = v_part_info[extended_last]["start"]
                v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

                #always reduce the extended size ,if need delete it, will handle this later
                mark_update(prev)
                v_part_info[prev]["end"] = v_part_info[extended_last]["start"] - 1
                v_part_info[prev]["length"] = v_part_info[prev]["end"] - v_part_info[prev]["start"] + 1
                v_part_info[prev]["width"] = Math.floor((v_part_info[prev]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

                extended_last_index = v_disk_info[disk]["partitions"].indexOf(extended_last)
                v_disk_info[disk]["partitions"].splice(extended_last, 1)
                delete v_part_info[extended_last]

    next = get_next_part(part)
    if next?
        if v_part_info[next]["type"] == "freespace"
            v_part_info[part]["end"] = v_part_info[next]["end"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            next_index = v_disk_info[disk]["partitions"].indexOf(next)
            v_disk_info[disk]["partitions"].splice(next_index, 1)
            delete v_part_info[next]

        else if v_part_info[next]["type"] == "extended"
            secondary_blocks = get_secondary_blocks(disk)
            if secondary_blocks.length > 0
                extended_first = secondary_blocks[0]

            if extended_first? and v_part_info[extended_first]["type"] == "freespace"
                v_part_info[part]["end"] = v_part_info[extended_first]["end"]
                v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

                #always reduce the extended size ,if need delete it, will handle this later
                mark_update(next)
                v_part_info[next]["start"] = v_part_info[extended_first]["end"] + 1
                v_part_info[next]["length"] = v_part_info[next]["end"] - v_part_info[next]["start"] + 1
                v_part_info[next]["width"] = Math.floor((v_part_info[next]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

                extended_first_index = v_disk_info[disk]["partitions"].indexOf(extended_first)
                v_disk_info[disk]["partitions"].splice(extended_first_index, 1)
                delete v_part_info[extended_first]

#delete logical partition, merge slibing freespace
_delete_logical = (disk, part) ->
    console.log("[part_util.coffee] _delete_logical() disk: #{disk}, part: #{part}")
    prev = get_prev_part(part)
    if prev?
        if v_part_info[prev]["type"] == "freespace"
            v_part_info[part]["start"] = v_part_info[prev]["start"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            prev_index = v_disk_info[disk]["partitions"].indexOf(prev)
            v_disk_info[disk]["partitions"].splice(prev_index, 1)
            delete v_part_info[prev]
    else
        extended = get_extended_partition(disk)
        if extended?
            before_extended = get_prev_part(extended)
        else
            console.error("[part_util.coffee] _delete_logical() handle prev part error, should already have an extended part, disk: #{disk}, part: #{part}")
        if before_extended? and v_part_info[before_extended]["type"] == "freespace"
            v_part_info[part]["start"] = v_part_info[before_extended]["start"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            before_extended_index = v_disk_info[disk]["partitions"].indexOf(before_extended)
            v_disk_info[disk]["partitions"].splice(before_extended_index, 1)
            delete v_part_info[before_extended]

            #always reduce the extended size, if need delete it, will handle this later
            mark_update(extended)
            v_part_info[extended]["start"] = v_part_info[part]["end"] + 1
            v_part_info[extended]["length"] = v_part_info[extended]["end"] - v_part_info[extended]["start"] + 1
            v_part_info[extended]["width"] = Math.floor((v_part_info[extended]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

    next = get_next_part(part)
    if next?
        if v_part_info[next]["type"] == "freespace"
            v_part_info[part]["end"] = v_part_info[next]["end"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            next_index = v_disk_info[disk]["partitions"].indexOf(next)
            v_disk_info[disk]["partitions"].splice(next_index, 1)
            delete v_part_info[next]
    else
        extended = get_extended_partition(disk)
        if extended?
            after_extended = get_next_part(extended)
        else
            console.error("[part_util.coffee] _delete_logical() handle next part error, should already have an extended part, disk: #{disk}, part: #{part}")
        if after_extended? and v_part_info[after_extended]["type"] == "freespace"
            v_part_info[part]["end"] = v_part_info[after_extended]["end"]
            v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

            after_extended_index = v_disk_info[disk]["partitions"].indexOf(after_extended)
            v_disk_info[disk]["partitions"].splice(after_extended_index, 1)
            delete v_part_info[after_extended]

            #always reduce the extended size ,if need delete it, will handle this later
            mark_update(extended)
            v_part_info[extended]["end"] = v_part_info[part]["start"] - 1
            v_part_info[extended]["length"] = v_part_info[extended]["end"] - v_part_info[extended]["start"] + 1
            v_part_info[extended]["width"] = Math.floor((v_part_info[extended]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

_delete_extended = (disk, part) ->
    console.log("[part_util.coffee] _delete_extended(), disk: #{disk}, part: #{part}")
    mark_delete(part)
    secondarys = get_secondary_blocks(disk)
    #fix me, need test delete in for loop
    for block in secondarys
        part_index = v_disk_info[disk]["partitions"].indexOf(block)
        v_disk_info[disk]["partitions"].splice(part_index, 1)
        delete v_part_info[block]

    prev = get_prev_part(part)
    if prev? and v_part_info[prev]["type"] == "freespace"
        v_part_info[part]["start"] = v_part_info[prev]["start"]
        v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

        prev_index = v_disk_info[disk]["partitions"].indexOf(prev)
        v_disk_info[disk]["partitions"].splice(prev_index, 1)
        delete v_part_info[prev]

    next = get_next_part(part)
    if next? and v_part_info[next]["type"] == "freespace"
        v_part_info[part]["end"] = v_part_info[next]["end"]
        v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

        next_index = v_disk_info[disk]["partitions"].indexOf(next)
        v_disk_info[disk]["partitions"].splice(next_index, 1)
        delete v_part_info[next]

    update_part_display_path(part, "delete")
    v_part_info[part]["type"] = "freespace"
    v_part_info[part]["fs"] = ""
    v_part_info[part]["os"] = ""
    v_part_info[part]["label"] = ""
    v_part_info[part]["mp"] = "unused"
    v_part_info[part]["width"] = Math.round((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

delete_part = (part) ->
    console.log("[part_util.coffee] delete_part(), part: #{part}")
    remain_part = part
    mark_delete(part)
    disk = v_part_info[part]["disk"]
    #update part geometry
    if v_part_info[part]["type"] == "normal"
        _delete_normal(disk, part)
    else if v_part_info[part]["type"] == "logical"
        _delete_logical(disk, part)
    else
        console.error("[part_util.coffee] delete_part(), invalid parttion type: #{v_part_info[part]["type"]}, part: #{part}")

    #need update part type first to decide whether need delete extended
    update_part_display_path(part, "delete")
    v_part_info[part]["type"] = "freespace"
    v_part_info[part]["fs"] = ""
    v_part_info[part]["os"] = ""
    v_part_info[part]["label"] = ""
    v_part_info[part]["mp"] = "unused"
    v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
    v_part_info[part]["used"] = v_part_info[part]["length"]

    extended = get_extended_partition(disk)
    if extended? and get_logical_partitions(disk).length == 0
        _delete_extended(disk, extended)
        remain_part = extended

    sort_v_disk_info(disk)
    return remain_part

#add normal partition
_add_normal = (disk, free_part) ->
#adapt extended space when free_part has intersection with extended, usually reduce extended size
    console.log("[part_util.coffee] _add_normal(), disk: #{disk}, free_part: #{free_part}")
    extended = get_extended_partition(disk)
    if extended?
        #has left intersection
        if v_part_info[extended]["start"] > v_part_info[free_part]["start"] and v_part_info[extended]["start"] < v_part_info[free_part]["end"]
            if v_part_info[free_part]["end"] > v_part_info[extended]["end"]
                console.error("[part_util.coffee] _add_normal() invalid extended partition, #{extended}, disk: #{disk}")
            v_part_info[extended]["start"] = v_part_info[free_part]["end"] + 1
            mark_update(extended)

        #has right intersection
        if v_part_info[extended]["end"] > v_part_info[free_part]["start"] and v_part_info[extended]["end"] < v_part_info[free_part]["end"]
            if v_part_info[free_part]["start"] < v_part_info[extended]["start"]
                console.error("[part_util.coffee] _add_normal() invalid extended partition, #{extended}, disk: #{disk}")
            v_part_info[extended]["end"] = v_part_info[free_part]["start"] - 1
            mark_update(extended)

        #freespace is in extended
        if v_part_info[extended]["start"] <= v_part_info[free_part]["start"] and v_part_info[extended]["end"] >= v_part_info[free_part]["end"]
            sort_v_disk_info(disk)
            secondary_blocks = get_secondary_blocks(disk)
            if secondary_blocks?
                if secondary_blocks.indexOf(free_part) == 0
                    v_part_info[extended]["start"] = v_part_info[free_part]["end"] + 1
                    mark_update(extended)
                else if secondary_blocks.indexOf(free_part) == secondary_blocks.length - 1
                    v_part_info[extended]["end"] = v_part_info[free_part]["start"] - 1
                    mark_update(extended)
                else
                    console.error("[part_util.coffee] _add_normal() error, cannot add a normal partion inside an extended partition, disk: #{disk}")
            else
                console.error("[part_util.coffee] _add_normal() error, invalid secondary blocks, disk: #{disk}")

        v_part_info[extended]["length"] = v_part_info[extended]["end"] - v_part_info[extended]["start"] + 1
        v_part_info[extended]["width"] = Math.floor((v_part_info[extended]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

#add logical partition adaption
_add_logical = (disk, free_part) ->
#automaticly create an extended partition or expand extended size as needed
    console.log("[part_util.coffee] _add_logical(), disk: #{disk}, free_part: #{free_part}")
    if not get_extended_partition(disk)?
        extended = DCore.Installer.rand_uuid("part")
        v_part_info[extended] = {}
        v_part_info[extended]["disk"] = disk
        v_part_info[extended]["start"] = v_part_info[free_part]["start"]
        v_part_info[extended]["length"] = v_part_info[free_part]["length"]
        v_part_info[extended]["end"] = v_part_info[free_part]["end"]
        v_part_info[extended]["type"] = "extended"
        update_part_display_path(extended, "add")
        v_disk_info[disk]["partitions"].push(extended)
        mark_add(extended)
    else
        extended = get_extended_partition(disk)
        if v_part_info[extended]["start"] > v_part_info[free_part]["start"]
            v_part_info[extended]["start"] = v_part_info[free_part]["start"]
            mark_update(extended)
        if v_part_info[extended]["end"] < v_part_info[free_part]["end"]
            v_part_info[extended]["end"] = v_part_info[free_part]["end"]
            mark_update(extended)

        v_part_info[extended]["length"] = v_part_info[extended]["end"] - v_part_info[extended]["start"] + 1
        v_part_info[extended]["width"] = Math.floor((v_part_info[extended]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

#add partition
add_part = (free_part, type, size, align, fs, mp) ->
    console.log("[part_util.coffee] add_part(), free_part: #{free_part}, type: #{type}, size: #{size}, align: #{align}, fs: #{fs}, mp: #{mp}")
    new_part = DCore.Installer.rand_uuid("part")
    disk = v_part_info[free_part]["disk"]
    v_part_info[new_part] = {}
    v_part_info[new_part]["disk"] = disk
    v_part_info[new_part]["color"] = get_random_color()
    v_part_info[new_part]["format"] = true
    v_part_info[new_part]["type"] = type
    v_part_info[new_part]["align"] = align
    v_part_info[new_part]["fs"] = fs
    v_part_info[new_part]["mp"] = mp
    v_part_info[new_part]["lvm"] = false

    if type == "normal"
        _add_normal(disk, free_part)
    else if type == "logical"
        _add_logical(disk, free_part)
    else
        console.error("[part_util.coffee] add_part() error, invalid partition type: #{type}")
    #handle new part geometry
    if Math.abs(size - v_part_info[free_part]["length"]) < 10 * MB
        v_part_info[new_part]["start"] = v_part_info[free_part]["start"]
        v_part_info[new_part]["length"] = v_part_info[free_part]["length"]
        v_part_info[new_part]["end"] = v_part_info[free_part]["end"]

        free_index = v_disk_info[disk]["partitions"].indexOf(free_part)
        v_disk_info[disk]["partitions"].splice(free_index, 1)
        delete v_part_info[free_part]
    else
        if align == "start"
            v_part_info[new_part]["start"] = v_part_info[free_part]["start"]
            v_part_info[new_part]["length"] = size
            v_part_info[new_part]["end"] = v_part_info[new_part]["start"] + v_part_info[new_part]["length"] - 1

            v_part_info[free_part]["start"] = v_part_info[new_part]["end"] + 1
            v_part_info[free_part]["length"] = v_part_info[free_part]["end"] - v_part_info[free_part]["start"] + 1
        else
            v_part_info[new_part]["length"] = size
            v_part_info[new_part]["end"] = v_part_info[free_part]["end"]
            v_part_info[new_part]["start"] = v_part_info[new_part]["end"] - v_part_info[new_part]["length"] + 1

            v_part_info[free_part]["end"] = v_part_info[new_part]["start"] - 1
            v_part_info[free_part]["length"] = v_part_info[free_part]["end"] - v_part_info[free_part]["start"] + 1
        v_part_info[free_part]["width"] = Math.floor((v_part_info[free_part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

    # If |new_part| is at the beginning of disk device, add some preceding
    # space reserved for MBR.
    console.log("[part_util.coffee] add_part() start of new_part:", v_part_info[new_part]["start"])
    if v_part_info[new_part]["start"] < MINIMUM_PRECEDING_SIZE
        v_part_info[new_part]["start"] = MINIMUM_PRECEDING_SIZE
        v_part_info[new_part]["length"] = v_part_info[new_part]["end"] - v_part_info[new_part]["start"]

    update_part_display_path(new_part, "add")
    v_part_info[new_part]["width"] = Math.floor((v_part_info[new_part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
    v_part_info[new_part]["used"] = v_part_info[new_part]["length"]
    v_disk_info[disk]["partitions"].push(new_part)
    sort_v_disk_info(disk)
    mark_add(new_part)
    return new_part

undo_part_table_info = ->
    if __init_parted_finish
        init_v_disk_info()
        init_v_part_info()
        init_m_disk_info()
        init_m_part_info()
    else
        setTimeout(=>
            undo_part_table_info()
        ,1000)

#Control end

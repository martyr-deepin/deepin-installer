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

get_random_color = ->
    return '#'+(Math.random()*0xffffff<<0).toString(16)
#
#Model
#Model: for origin disk partition table 
disks = DCore.Installer.list_disks()

m_disk_info = {}
#never change the partitions list
for disk in disks
    m_disk_info[disk] = {}
    m_disk_info[disk]["change"] = false
    m_disk_info[disk]["partitions"] = []
    for part in DCore.Installer.get_disk_partitions(disk)
        if DCore.Installer.get_partition_type(part) in ["normal", "extended", "logical"]
            m_disk_info[disk]["partitions"].push(part)

m_part_info = {}
#may had part not in m_disk_info partitions list
for disk in disks
    for part in m_disk_info[disk]["partitions"]
        m_part_info[part] = {}
        m_part_info[part]["disk"] = disk
        m_part_info[part]["type"] = DCore.Installer.get_partition_type(part)
        m_part_info[part]["start"] = DCore.Installer.get_partition_start(part)
        m_part_info[part]["length"] = DCore.Installer.get_partition_length(part)
        m_part_info[part]["end"] = DCore.Installer.get_partition_end(part)
        if m_part_info[part]["type"] == "extended"
            m_part_info[part]["fs"] = "extended"
        else if m_part_info[part]["type"] in ["normal", "logical"]
            m_part_info[part]["fs"] = DCore.Installer.get_partition_fs(part)
        else
            m_part_info[part]["fs"] = ""
        m_part_info[part]["format"] = false
        try
            m_part_info[part]["mp"] = DCore.Installer.get_partition_mp(part)
        catch error
            m_part_info[part]["mp"] = "unused"
        m_part_info[part]["path"] = DCore.Installer.get_partition_path(part)
        m_part_info[part]["op"] = "keep"
    
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
        echo "error in sort part op"
        return -1

_filter_modeled_delete = (part, index) ->
    return m_part_info[part]["op"] == "delete"

_filter_modeled_update = (part, index) ->
    return m_part_info[part]["op"] == "update"

_filter_modeled_add = (part, index) ->
    return m_part_info[part]["op"] == "add"

#include original to delete part and virtual new part
get_modeled_partitions = (disk) ->
    echo "get modeled partitons"
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
    echo "mark update"
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        m_disk_info[v_part_info[part]["disk"]]["change"] = true
        if m_part_info[part]["op"] not in ["keep", "update"]
            echo "should not reach for mark update"
        m_part_info[part]["op"] = "update"
    else
        echo "should not mark update for new partition"

#mark add flag for new partition
mark_add = (part) ->
    echo "mark add"
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        echo "error in mark add, part originally exists"
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
    echo "mark delete"
    if part in m_disk_info[v_part_info[part]["disk"]]["partitions"]
        if m_part_info[part]["op"] in ["keep", "update"]
            m_disk_info[v_part_info[part]["disk"]]["change"] = true
            m_part_info[part]["op"] = "delete"
        else
            echo "invalid flag for orig partition to mark delete"
    else
        if m_part_info[part]["op"] == "add"
            m_disk_info[v_part_info[part]["disk"]]["change"] = true
            delete m_part_info[part]
        else
            echo "invalid flag for new part to mark delete"

#do real add/delete/update partition operation
do_partition = ->
    echo "do partition"
    for disk in disks
        if m_disk_info[disk]["change"] == true
            for part in get_modeled_partitions(disk)
                if m_part_info[part]["op"] == "delete"
                    try
                        DCore.Installer.delete_disk_partition(disk, part)
                    catch error
                        echo error
                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        echo error

                else if m_part_info[part]["op"] == "update"
                    if m_part_info[part]["fs"] != v_part_info[part]["fs"]
                        try
                            DCore.Installer.update_partition_fs(part, v_part_info[part]["fs"])
                        catch error
                            echo error

                    if m_part_info[part]["mp"] != v_part_info[part]["mp"]
                        echo "mp changed"

                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        echo error

                else if m_part_info[part]["op"] == "add"
                    try
                        DCore.Installer.new_disk_partition(part, disk, m_part_info[part]["type"], m_part_info[part]["fs"], m_part_info[part]["start"], m_part_info[part]["end"])
                    catch error
                        echo error

                    try
                        DCore.Installer.write_disk(disk)
                    catch error
                        echo error

                    if v_part_info[part]["type"] != "extended"
                        try
                            DCore.Installer.update_partition_fs(part, v_part_info[part]["fs"])
                        catch error
                            echo error

                        try
                            DCore.Installer.write_disk(disk)
                        catch error
                            echo error

                else
                    echo "just keep part"

        else
            echo "just keep disk"

#get target part that mount root
get_target_part = ->
    echo "get target part"
    for disk in disks
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["op"] != "delete" and m_part_info[part]["mp"] == "/"
                return part
    return null

#check whether had a part that mount root
check_target_part = ->
    if get_target_part()?
        return true
    else
        return false

#write /etc/fstab
write_fs_tab = ->
    echo "write fs tab"
    target = get_target_part()
    if not target?
        echo "get target failed"
        return 

    for disk in disks
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["op"] != "delete"
                if m_part_info[part]["mp"]? and m_part_info[part]["mp"].length > 0
                    try
                        DCore.Installer.write_fs_tab(part, m_part_info[part]["mp"])
                    catch error
                        echo error

#Model end
#Model
#

#
#View
#View: for data display in UI
v_disk_info = {}
for disk in disks
    v_disk_info[disk] = {}
    v_disk_info[disk]["length"] = DCore.Installer.get_disk_length(disk)
    v_disk_info[disk]["model"] = DCore.Installer.get_disk_model(disk)
    v_disk_info[disk]["max_primary"] = DCore.Installer.get_disk_max_primary_count(disk)
    v_disk_info[disk]["path"] = DCore.Installer.get_disk_path(disk)
    v_disk_info[disk]["partitions"] = []
    for part in DCore.Installer.get_disk_partitions(disk)
        if DCore.Installer.get_partition_type(part) == "freespace"
            if DCore.Installer.get_partition_length(part) > 4096
                v_disk_info[disk]["partitions"].push(part)
        else if DCore.Installer.get_partition_type(part) in ["normal", "extended", "logical"]
            if DCore.Installer.get_partition_path(part).indexOf("/dev/mapper") == -1
                v_disk_info[disk]["partitions"].push(part)

v_part_info = {}
for disk in disks
    for part in v_disk_info[disk]["partitions"]
        v_part_info[part] = {}
        v_part_info[part]["disk"] = disk
        v_part_info[part]["type"] = DCore.Installer.get_partition_type(part)
        #please fix the return null string
        #v_part_info[part]["name"] = DCore.Installer.get_partition_name(part)
        v_part_info[part]["start"] = DCore.Installer.get_partition_start(part)
        v_part_info[part]["length"] = DCore.Installer.get_partition_length(part)
        v_part_info[part]["end"] = DCore.Installer.get_partition_end(part)
        if v_part_info[part]["type"] == "extended"
            v_part_info[part]["fs"] = "extended"
        else if v_part_info[part]["type"] in ["normal", "logical"]
            v_part_info[part]["fs"] = DCore.Installer.get_partition_fs(part)
        else
            v_part_info[part]["fs"] = ""
        try
            v_part_info[part]["mp"] = DCore.Installer.get_partition_mp(part)
        catch error
            v_part_info[part]["mp"] = "unused"
        v_part_info[part]["path"] = DCore.Installer.get_partition_path(part)
        v_part_info[part]["color"] = get_random_color() 
        v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
        try
            v_part_info[part]["used"] = DCore.Installer.get_partition_used(part)
            #v_part_info[part]["used"] = "80G"
        catch error
            v_part_info[part]["used"] = "unknown"

_sort_part_geom = (part_a, part_b) ->
    if v_part_info[part_a]["start"] == v_part_info[part_b]["start"]
        if v_part_info[part_a]["type"] == "extended"
            return -1
        else if v_part_info[part_b]["type"] == "extended"
            return 1
        else
            echo "error in sort part geom"
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
        echo "invalid part type in v_part_info"
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
        echo "error in get prev part"

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
        echo "error in get prev part"

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
    v_part_info[part]["fs"] = fs
    mark_update(part)

update_part_format = (part, format) ->
    v_part_info[part]["format"] = format
    mark_update(part)

update_part_mp = (part, mp) ->
    v_part_info[part]["mp"] = mp
    mark_update(part)

compute_display_path = (disk) ->
    disk_path = v_disk_info[disk]["path"]
    sort_v_disk_info(disk)

    i = 1
    for part in get_primary_partitions(disk)
        if i > v_disk_info[disk]["max_primary"]
            echo "error, path exceeded max primary partition count"
        v_part_info[part]["path"] = disk_path+i
        i = i+1

    j = v_disk_info[disk]["max_primary"]+1 
    for part in get_logical_partitions(disk)
        v_part_info[part]["path"] = disk_path+j
        j=j+1

    for part in get_freespace_partitions(disk)
        v_part_info[part]["path"] = ""

#delete normal partition, merge slibing freespace
_delete_normal = (disk, part) ->
    echo "delete normal"
    prev = get_prev_part(part)
    if prev? 
        if v_part_info[prev]["type"] == "freespace" 
            echo "directly merge prev freespace"
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
                echo "merge freespace block before last of extended"
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
            echo "directly merge next freespace"
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
                echo "merge freespace block which first in extended"
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
    echo "delete logical"
    prev = get_prev_part(part)
    if prev?
        if v_part_info[prev]["type"] == "freespace"
            echo "directly merge prev freespace"
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
            echo "error in delete logical, should already has an extended contains it"
        if before_extended? and v_part_info[before_extended]["type"] == "freespace"
            echo "merge freespace block just before the extended partition"
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
           echo "directly merge next freespace"
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
           echo "error in delete logical, should already has an extended contains it"
       if after_extended? and v_part_info[after_extended]["type"] == "freespace"
           echo "merge freespace block just after the extended partition"
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
    echo "delete extended"
    mark_delete(part)
    secondarys = get_secondary_blocks(disk)
    #fix me, need test delete in for loop
    for block in secondarys
        part_index = v_disk_info[disk]["partitions"].indexOf(block)
        v_disk_info[disk]["partitions"].splice(part_index, 1)
        delete v_part_info[block]

    echo "delete parts in extended finish"

    prev = get_prev_part(part)
    if prev? and v_part_info[prev]["type"] == "freespace" 
        echo "directly merge prev freespace"
        v_part_info[part]["start"] = v_part_info[prev]["start"]
        v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

        prev_index = v_disk_info[disk]["partitions"].indexOf(prev)
        v_disk_info[disk]["partitions"].splice(prev_index, 1)
        delete v_part_info[prev]

    next = get_next_part(part)
    if next? and v_part_info[next]["type"] == "freespace"
        echo "directly merge next freespace"
        v_part_info[part]["end"] = v_part_info[next]["end"]
        v_part_info[part]["length"] = v_part_info[part]["end"] - v_part_info[part]["start"] + 1

        next_index = v_disk_info[disk]["partitions"].indexOf(next)
        v_disk_info[disk]["partitions"].splice(next_index, 1)
        delete v_part_info[next]

    v_part_info[part]["type"] = "freespace"
    v_part_info[part]["fs"] = ""
    v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

delete_part = (part) ->
    echo "delete part"
    mark_delete(part)
    disk = v_part_info[part]["disk"]
    #update part geometry
    if v_part_info[part]["type"] == "normal"
        _delete_normal(disk, part)
    else if v_part_info[part]["type"] == "logical"
        _delete_logical(disk, part)
    else
        echo "error in delete part, invalid partition type"

    #need update part type first to decide whether need delete extended
    v_part_info[part]["type"] = "freespace"
    v_part_info[part]["fs"] = ""
    v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

    extended = get_extended_partition(disk)
    if extended? and get_logical_partitions(disk).length == 0
        _delete_extended(disk, extended)

    compute_display_path(disk)

#add normal partition
_add_normal = (disk, free_part) ->
#adapt extended space when free_part has intersection with extended, usually reduce extended size
    echo "add normal"
    extended = get_extended_partition(disk)
    if extended?
        #has left intersection
        if v_part_info[extended]["start"] > v_part_info[free_part]["start"] and v_part_info[extended]["start"] < v_part_info[free_part]["end"]
            if v_part_info[free_part]["end"] > v_part_info[extended]["end"]
                echo "error in _add_normal, invalid extended"
            v_part_info[extended]["start"] = v_part_info[free_part]["end"] + 1
            mark_update(extended)

        #has right intersection
        if v_part_info[extended]["end"] > v_part_info[free_part]["start"] and v_part_info[extended]["end"] < v_part_info[free_part]["end"]
            if v_part_info[free_part]["start"] < v_part_info[extended]["start"]
                echo "error in _add_normal, invalid extended"
            v_part_info[extended]["end"] = v_part_info[free_part]["start"] - 1
            mark_update(extended)

        #freespace is in extended
        if v_part_info[extended]["start"] <= v_part_info[free_part]["start"] and v_part_info[extended]["end"] >= v_part_info[free_part]["end"]
            sort_v_disk_info(disk)
            secondary_blocks = get_secondary_blocks(disk)
            if secondary_blocks?
                if secondary_blocks[0] == free_part
                    v_part_info[extended]["start"] = v_part_info[free_part]["end"] + 1
                    mark_update(extended)
                else if secondary_blocks[-1] == free_part
                    v_part_info[extended]["end"] = v_part_info[free_part]["start"] - 1
                    mark_update(extended)
                else
                    echo "error, can't add an normal deep inner an extended partition"
            else
                echo "error in _add_normal, invalid secondary blocks"
            
        v_part_info[extended]["length"] = v_part_info[extended]["end"] - v_part_info[extended]["start"] + 1
        v_part_info[extended]["width"] = Math.floor((v_part_info[extended]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

#add logical partition adaption
_add_logical = (disk, free_part) ->
#automaticly create an extended partition or expand extended size as needed 
    echo "add logical"
    if not get_extended_partition(disk)?
        extended = "part" + DCore.Installer.rand_uuid()
        v_part_info[extended] = {}
        v_part_info[extended]["disk"] = disk
        v_part_info[extended]["start"] = v_part_info[free_part]["start"]
        v_part_info[extended]["length"] = v_part_info[free_part]["length"]
        v_part_info[extended]["end"] = v_part_info[free_part]["end"]
        v_part_info[extended]["type"] = "extended"
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
    new_part = "part"+DCore.Installer.rand_uuid()
    disk = v_part_info[free_part]["disk"]
    v_part_info[new_part] = {}
    v_part_info[new_part]["disk"] = disk

    if type == "normal"
        _add_normal(disk, free_part)
    else if type == "logical"
        _add_logical(disk, free_part)
    else
        echo "error in add_part, invalid partition type"

    #handle new part geometry
    if size == v_part_info[free_part]["length"]
        echo "whole freespace to new part"
        v_part_info[new_part]["start"] = v_part_info[free_part]["start"]
        v_part_info[new_part]["length"] = v_part_info[free_part]["length"]
        v_part_info[new_part]["end"] = v_part_info[free_part]["end"]

        free_index = v_disk_info[disk]["partitions"].indexOf(free_part)
        v_disk_info[disk]["partitions"].splice(free_index, 1)
        delete v_part_info[free_part]

    else
        echo "partial freespace to new part"
        if align == "start"
            v_part_info[new_part]["start"] = v_part_info[free_part]["start"]
            v_part_info[new_part]["length"] = size
            v_part_info[new_part]["end"] = v_part_info[new_part]["start"] + v_part_info[new_part]["length"] - 1

            v_part_info[free_part]["start"] = v_part_info[new_part]["end"] + 1
            v_part_info[free_part]["length"] = v_part_info[free_part]["end"] - v_part_info[free_part]["start"] + 1
        else
            v_part_info[new_part]["length"] = size 
            v_part_info[new_part]["end"] = v_part_info[free_part]["end"]
            v_part_info[new_part]["start"] = v_part_info[free_part]["start"]

            v_part_info[free_part]["end"] = v_part_info[new_part]["start"] - 1
            v_part_info[free_part]["length"] = v_part_info[free_part]["end"] - v_part_info[free_part]["start"] + 1
            v_part_info[free_part]["width"] = Math.floor((v_part_info[free_part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"

    v_part_info[new_part]["type"] = type
    v_part_info[new_part]["align"] = align
    v_part_info[new_part]["fs"] = fs
    v_part_info[new_part]["mp"] = mp
    v_part_info[new_part]["width"] = Math.floor((v_part_info[new_part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
    v_disk_info[disk]["partitions"].push(new_part)
    compute_display_path(disk)
    mark_add(new_part)

undo_table = (disk) ->
    #restore view
    v_disk_info[disk] = {}
    v_disk_info[disk]["length"] = DCore.Installer.get_disk_length(disk)
    v_disk_info[disk]["model"] = DCore.Installer.get_disk_model(disk)
    v_disk_info[disk]["max_primary"] = DCore.Installer.get_disk_max_primary_count(disk)
    v_disk_info[disk]["path"] = DCore.Installer.get_disk_path(disk)
    v_disk_info[disk]["partitions"] = []
    for part in DCore.Installer.get_disk_partitions(disk)
        if DCore.Installer.get_partition_type(part) == "freespace"
            if DCore.Installer.get_partition_length(part) > 4096
                v_disk_info[disk]["partitions"].push(part)
        else if DCore.Installer.get_partition_type(part) in ["normal", "extended", "logical"]
            if DCore.Installer.get_partition_path(part).indexOf("/dev/mapper") == -1
                v_disk_info[disk]["partitions"].push(part)

    for part in v_disk_info[disk]["partitions"]
        v_part_info[part] = {}
        v_part_info[part]["disk"] = disk
        v_part_info[part]["type"] = DCore.Installer.get_partition_type(part)
        #please fix the return null string
        #v_part_info[part]["name"] = DCore.Installer.get_partition_name(part)
        v_part_info[part]["start"] = DCore.Installer.get_partition_start(part)
        v_part_info[part]["length"] = DCore.Installer.get_partition_length(part)
        v_part_info[part]["end"] = DCore.Installer.get_partition_end(part)
        if v_part_info[part]["type"] == "extended"
            v_part_info[part]["fs"] = "extended"
        else if v_part_info[part]["type"] in ["normal", "logical"]
            v_part_info[part]["fs"] = DCore.Installer.get_partition_fs(part)
        else
            v_part_info[part]["fs"] = ""
        try
            v_part_info[part]["mp"] = DCore.Installer.get_partition_mp(part)
        catch error
            v_part_info[part]["mp"] = "unused"
        v_part_info[part]["path"] = DCore.Installer.get_partition_path(part)
        v_part_info[part]["color"] = get_random_color() 
        v_part_info[part]["width"] = Math.floor((v_part_info[part]["length"] / v_disk_info[disk]["length"]) * 100) + "%"
        try
            v_part_info[part]["used"] = DCore.Installer.get_partition_used(part)
            #v_part_info[part]["used"] = "80G"
        catch error
            v_part_info[part]["used"] = "unknown"
#Control end
#Control
#

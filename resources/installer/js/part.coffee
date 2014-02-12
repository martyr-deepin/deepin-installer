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

__selected_target = null
__selected_grub = null

__selected_disk_item = null
__selected_disk = null
__selected_item = null
__selected_line = null
__selected_mode = "simple"
__selected_stage = null

class AddPartDialog extends Dialog
    constructor: (@id, @partid) ->
        super(@id, true, @add_part_cb)
        @add_css_class("DialogCommon")
        @element.style.top = "85px"
        @title_txt.innerText = _("Add partition")
        @fill_type()
        @fill_size()
        @fill_align()
        @fill_fs()
        @fill_mount()
        @fill_tips()
        
    add_part_cb: ->
        @gather_info()
        new_part = add_part(@partid, @n_type, @n_size, @n_align, @n_fs, @n_mp)
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(new_part)?.focus()
        Widget.look_up("part")?.fill_bootloader()

    fill_type: ->
        @type = create_element("div", "", @content)
        @type_desc = create_element("span", "AddDesc", @type)
        @type_desc.innerText = _("Type:")
        @type_value = create_element("span", "AddValue", @type)

        @primary_span = create_element("span", "AddValueItem", @type_value)
        @type_primary = create_element("span", "", @primary_span)
        @primary_desc = create_element("span", "", @primary_span)
        @primary_desc.innerText = _("Primary")

        @logical_span = create_element("span", "AddValueItem", @type_value)
        @type_logical = create_element("span", "", @logical_span)
        @logical_desc = create_element("span", "", @logical_span)
        @logical_desc.innerText = _("Logical")

        @type_radio = "primary"
        if not can_add_normal(@partid)
            @primary_span.style.display = "none"
            @type_radio = "logical"
            @type_primary.setAttribute("class", "RadioUnChecked")
            @type_logical.setAttribute("class", "RadioChecked")
        else
            @type_radio = "primary"
            @type_primary.setAttribute("class", "RadioChecked")
            @type_logical.setAttribute("class", "RadioUnchecked")

        if not can_add_logical(@partid)
            @logical_span.style.display = "none"

        @type_primary.addEventListener("click", (e) =>
            @type_radio = "primary"
            @type_primary.setAttribute("class", "RadioChecked")
            @type_logical.setAttribute("class", "RadioUnchecked")
        )
        @type_logical.addEventListener("click", (e) =>
            @type_radio = "logical"
            @type_primary.setAttribute("class", "RadioUnChecked")
            @type_logical.setAttribute("class", "RadioChecked")
        )

    fill_size: ->
        @size = create_element("div", "", @content)
        @size_desc = create_element("span", "AddDesc", @size)
        @size_desc.innerText = _("Size:")
        @max_size_mb = sector_to_mb(v_part_info[@partid]["length"], 512)

        @size_value = create_element("span", "AddValue", @size)
        @size_wrap = create_element("div", "SizeWrap", @size_value)
        @size_input = create_element("input", "", @size_wrap)
        #@size_input.setAttribute("placeholder", @max_size_mb)
        @size_input.setAttribute("value", @max_size_mb)
        @size_input.addEventListener("blur", (e) =>
            parse = parseInt(@size_input.value)
            if isNaN(parse)
                @size_input.value = @max_size_mb 
            else
                if parse < 0
                    @size_input.value = 0 
                else if parse > @max_size_mb
                    @size_input.value = @max_size_mb 
                else
                    @size_input.value = parse 
        )
        @minus_img = create_element("div", "SizeMinus", @size_wrap)
        @minus_img.addEventListener("click", (e) =>
            parse = parseInt(@size_input.value)
            if isNaN(parse)
                @size_input.value = @max_size_mb 
            else
                if parse >= 1
                    @size_input.value = parse - 1 
        )
        @add_img = create_element("div", "SizeAdd", @size_wrap)
        @add_img.addEventListener("click", (e) =>
            parse = parseInt(@size_input.value)
            if isNaN(parse)
                @size_input.value = @max_size_mb 
            else
                if parse <= @max_size_mb - 1 
                    @size_input.value = parse + 1 
        )
        @dw = create_element("div", "SizeDw", @size_wrap)
        @dw.innerText = "MB"
        
    fill_align: ->
        @align = create_element("div", "", @content)
        @align_desc = create_element("span", "AddDesc", @align)
        @align_desc.innerText = _("Align:")
        @align_value = create_element("span", "AddValue", @align)

        @start_span = create_element("span", "AddValueItem", @align_value)
        @align_start = create_element("span", "", @start_span)
        @start_desc = create_element("span", "", @start_span)
        @start_desc.innerText = _("Begin")

        @end_span = create_element("span", "AddValueItem", @align_value)
        @align_end = create_element("span", "", @end_span)
        @end_desc = create_element("span", "", @end_span)
        @end_desc.innerText = _("End")

        @align_radio = "start"
        @align_start.setAttribute("class", "RadioChecked")
        @align_end.setAttribute("class", "RadioUnchecked")

        @align_start.addEventListener("click", (e) =>
            @align_radio = "start"
            @align_start.setAttribute("class", "RadioChecked")
            @align_end.setAttribute("class", "RadioUnchecked")
        )
        @align_end.addEventListener("click", (e) =>
            @align_radio = "end"
            @align_start.setAttribute("class", "RadioUnChecked")
            @align_end.setAttribute("class", "RadioChecked")
        )

    fill_fs: ->
        @fs = create_element("div", "", @content)
        @fs_desc = create_element("span", "AddDesc", @fs)
        @fs_desc.innerText = _("Filesystem:")
        @fs_value = create_element("span", "AddValue", @fs)
        @fs_select = new DropDown("dd_fs_" + @partid, @fs_change_cb)
        @fs_value.appendChild(@fs_select.element)
        if DCore.Installer.disk_support_efi(v_part_info[@partid]["disk"])
            @fs_select.set_drop_items(__fs_efi_keys, __fs_efi_values)
        else
            @fs_select.set_drop_items(__fs_keys, __fs_values)
        @fs_select.set_drop_size(130,22)
        @fs_select.show_drop()
        @fs_select.set_selected("ext4")

    fs_change_cb: (part, fs) ->
        if fs in ["efi", "swap", "unused", "fat16", "fat32", "ntfs"]
            Widget.look_up("AddModel").mp.style.display = "none"
        else
            Widget.look_up("AddModel").mp.style.display = "block"

    fill_mount: ->
        @mp = create_element("div", "", @content)
        @mp_desc = create_element("span", "AddDesc", @mp)
        @mp_desc.innerText = _("Mount:")
        @mount_value = create_element("span", "AddValue", @mp)
        @mount_select = new DropDown("dd_mp_" + @partid, null)
        @mount_value.appendChild(@mount_select.element)
        @mount_select.set_drop_items(__mp_keys, __mp_values)
        @mount_select.set_drop_size(130,22)
        @mount_select.show_drop()
        @mount_select.set_selected("unused")

    fill_tips: ->
        @tips = create_element("div", "", @content)

    gather_info: ->
        if @type_radio == "primary"
            @n_type = "normal"
        else 
            @n_type = "logical"
        if parseInt(@size_input.value) == @max_size_mb
            @n_size = v_part_info[@partid]["length"]
        else
            @n_size = mb_to_sector(parseInt(@size_input.value), 512)
        if not @n_size?
            @tips.innerText = _("Please enter a valid partition size.")
        @n_align = @align_radio
        @n_fs = @fs_select.get_selected()
        @n_mp = @mount_select.get_selected()

class DeletePartDialog extends Dialog
    constructor: (@id,@partid) ->
        super(@id, true, @delete_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Delete partition")
        @delete_tips = create_element("div", "", @content)
        @delete_tips.innerText = _("Are you sure you want to delete this partition?")

    delete_part_cb: ->
        remain_part = delete_part(@partid)
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(remain_part)?.focus()
        Widget.look_up("part")?.fill_bootloader()

class UnmountDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @unmount_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Unmount partition")
        @unmount_tips = create_element("div", "", @content)
        @unmount_tips.innerText = _("Partition is already mounted. Do you want to unmount it?")

    unmount_cb: ->
        echo "unmount all partitions"
        for disk in disks
            for part in m_disk_info[disk]["partitions"]
                try
                    if DCore.Installer.get_partition_mp(part) not in ["/", "/cdrom"]
                        DCore.Installer.unmount_partition(part)
                catch error
                    echo error
        for item in Widget.look_up("part_table")?.partitems
            item.check_busy()

class FormatDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @format_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Format partition")
        @format_tips = create_element("div", "", @content)
        @format_tips.innerText = _("Are you sure you want to format this partition?")

    format_cb: ->
        echo "format to do install"

class RootDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @need_root_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Install tips")
        @root_tips = create_element("div", "", @content)
        @root_tips.innerText = _("A root partition is required.")

    need_root_cb: ->
        echo "need mount root to do install"

class UefiDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @uefi_require_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Install tips")
        @root_tips = create_element("div", "", @content)
        @root_tips.innerText = _("Uefi needs mount a fat32 part to /boot whose size greater than 100M.")

    uefi_require_cb: ->
        echo "uefi require cb"

class UefiBootDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @uefi_boot_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Install tips")
        @root_tips = create_element("div", "", @content)
        @root_tips.innerText = _("In uefi mode, no needs  to mount a part to /boot manually")

    uefi_boot_cb: ->
        echo "uefi boot cb"

DCore.signal_connect("part_operation", (msg) ->
    if progress_page? and progress_page.display_progress == false
        progress_page.display_progress = true
        progress_page.start_progress()
    progress_page?.update_progress("2%")
    __selected_stage = "extract"
    progress_page?.handle_extract("start")
)

class InstallDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @confirm_install_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Proceed with installation")
        @root_tips = create_element("div", "", @content)
        @fill_install_info()

    confirm_install_cb: ->
        echo "confirm install"
        progress_page = new Progress("progress")
        pc.remove_page(part_page)
        pc.add_page(progress_page)
        setTimeout(->
            if __selected_mode == "simple"
                do_simple_partition(__selected_item.id, "part")
            else if __selected_mode == "advance"
                echo "do advance partition"
                do_partition()
        , 300)

    fill_install_info: ->
        if __selected_mode == "advance"
            target = get_target_part()
        else
            target = __selected_item.id
        path = v_part_info[target]["path"]
        @root_tips.innerText = "Linux Deepin will be installer to " + path

class PartLineItem extends Widget
    constructor: (@id) ->
        super
        @part = @id[4...16]
        @init_line_item()

    init_line_item: ->
        @color = v_part_info[@part]["color"]
        @element.style.background = @color
        @element.style.width = v_part_info[@part]["width"]
        disk = v_part_info[@part]["disk"]
        @element.style.left = v_part_info[@part]["start"]/v_disk_info[disk]["length"] * 700
        @mask = create_element("div", "Mask", @element)

    focus: ->
        @passive_focus()
        Widget.look_up(@part)?.passive_focus()
    
    passive_focus: ->
        __selected_line?.blur()
        __selected_line = @
        @element.setAttribute("class", "PartLineItemActive")

    blur: ->
        @element.setAttribute("class", "PartLineItem")

    do_click: (e)->
        if __selected_line != @ 
            @focus()

class PartLineMaps extends Widget
    constructor: (@id)->
        super
        @fill_linemap()

    fill_linemap: ->
        @element.innerHTML = ""
        @disk_line = create_element("div", "Line", @element)
        for part in v_disk_info[__selected_disk]["partitions"]
            if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                item = new PartLineItem("line"+part)
                @disk_line.appendChild(item.element)

class PartTableItem extends Widget
    constructor: (@id)->
        super
        @lineid = "line" + @id
        @active = false
        @device = create_element("div", "Fat", @element)
        @size = create_element("div", "Thin Size", @element)
        @used = create_element("div", "Thin", @element)
        @fs = create_element("div", "Thin Fs", @element)
        @mount = create_element("div", "Thin Mount", @element)
        @format = create_element("div", "Thin", @element)
        @fill_device()
        @fill_size()
        @fill_used()
        @fill_fs()
        @fill_mount()
        @fill_format()

    fill_device: ->
        @device.innerHTML = ""
        @lock = create_element("span", "Lock", @device)
        @os = create_element("span", "Os", @device)
        @color = create_element("span", "Color", @device)
        @lp = create_element("span", "LabelPath", @device)
        @label = create_element("div", "Label", @lp)
        @path = create_element("div", "Path", @lp)
        @label.addEventListener("mouseover", (e) =>
            @show_detail_label()
        )
        @label.addEventListener("mouseout", (e) =>
            @hide_detail_label()
        )

        if __selected_mode == "advance"
            if v_part_info[@id]["type"] != "freespace"
                @path.innerText = v_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
            if v_part_info[@id]["label"]? and v_part_info[@id]["label"].length > 0
                if v_part_info[@id]["label"].length > 12
                    @label.innerText = v_part_info[@id]["label"].substring(0,12) + "..."
                else
                    @label.innerText = v_part_info[@id]["label"]
            else
                @label.style.display = "none"
                @path.setAttribute("style", "margin:10px 0;")
            color_value = v_part_info[@id]["color"]
            @color.style.background = color_value
            @color.style.display = "block"
        else if __selected_mode == "simple"
            if m_part_info[@id]["type"] != "freespace"
                @path.innerText = m_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
            if m_part_info[@id]["label"]? and m_part_info[@id]["label"].length > 0
                if m_part_info[@id]["label"].length > 12
                    @label.innerText = m_part_info[@id]["label"].substring(0,12) + "..."
                else
                    @label.innerText = m_part_info[@id]["label"]
            else
                @label.style.display = "none"
                @path.setAttribute("style", "margin:10px 0;")
            @color.style.display = "none"
        @update_device_os()

    show_detail_label: ->
        if not @label_detail?
            @label_detail = create_element("div", "LabelDetail", @label)
            if __selected_mode == "advance"
                @label_detail.innerText = v_part_info[@id]["label"] 
            else
                @label_detail.innerText = m_part_info[@id]["label"]
        @label_detail.style.display = "block"

    hide_detail_label: ->
        if @label_detail?
            @label_detail.style.display = "none"

    update_device_os: ->
        @os.innerHTML = ""
        if __selected_mode == "simple"
            os = m_part_info[@id]["os"]
        else
            os = v_part_info[@id]["os"]
        if os? and os.length > 2
            if os.toLowerCase().indexOf("deepin") != -1
                os_img = "images/deepin.png"
            else if os.toLowerCase().indexOf("linux") != -1
                os_img = "images/linux.png"
            else if os.toLowerCase().indexOf("windows") != -1
                os_img = "images/windows.png"
            else if os.toLowerCase().indexOf("mac") != -1
                os_img = "images/apple.png"
            else
                echo "--------upate device os--------"
                echo os
                os_img = "images/linux.png"
            create_img("", os_img, @os)

    fill_size: ->
        @size.innerHTML = ""
        if __selected_mode == "advance"
            @size.innerText += sector_to_gb(v_part_info[@id]["length"], 512).toFixed(1) + "G"
        else
            @size.innerText += sector_to_gb(m_part_info[@id]["length"], 512).toFixed(1) + "G"

    fill_used: ->
        @used.innerHTML = ""
        if __selected_mode == "advance" and v_part_info[@id]["type"] != "freespace"
            if isNaN(v_part_info[@id]["used"])
                @used.innerText = _("Unknown")
            else
                @used.innerText = (v_part_info[@id]["used"]/1000).toFixed(1) + "G"
        else if __selected_mode == "simple" and m_part_info[@id]["type"] != "freespace"
            if isNaN(m_part_info[@id]["used"])
                @used.innerText = _("Unknown")
            else
                @used.innerText = (m_part_info[@id]["used"]/1000).toFixed(1) + "G"

    update_part_used: ->
        @fill_used()

    fill_format: ->
        @format.innerHTML = ""
        if __selected_mode == "advance"
            if v_part_info[@id]? and v_part_info[@id]["type"] != "freespace"
                @format_img = create_img("Format", "images/check-01.png", @format)
                if not @active
                    if v_part_info[@id]["format"]
                        @format_img.setAttribute("src", "images/check-02.png")
                    else
                        @format_img.setAttribute("src", "images/check-01.png")
                else
                    if v_part_info[@id]["format"]
                        @format_img.setAttribute("src", "images/check-04.png")
                    else
                        @format_img.setAttribute("src", "images/check-03.png")
                if @is_busy()
                    @format_img.setAttribute("src", "images/check-05.png")
                else if @is_format_mandatory()
                    @format_img.setAttribute("src", "images/check-06.png")
                @format_img.addEventListener("click", (e) =>
                    if @is_busy()
                        @format_img.setAttribute("src", "images/check-05.png")
                    else if @is_format_mandatory()
                        update_part_format(@id, true)
                        @format_img.setAttribute("src", "images/check-06.png")
                    else
                        if v_part_info[@id]["format"]
                            update_part_format(@id, false)
                            @format_img.setAttribute("src", "images/check-03.png")
                        else
                            update_part_format(@id, true)
                            @format_img.setAttribute("src", "images/check-04.png")
                )
            @format.style.display = "block"
        else
            @format.style.display = "none"

    is_format_mandatory: ->
        if v_part_info[@id]["type"] == "freespace"
            return false
        if @id not in m_disk_info[v_part_info[@id]["disk"]]["partitions"] or m_part_info[@id]["op"] == "add"
            return true
        if m_part_info[@id]["fs"] != v_part_info[@id]["fs"]
            return true
        return false

    fill_fs: ->
        @fs.innerHTML = ""
        if __selected_mode == "simple" 
            if m_part_info[@id]? and m_part_info[@id]["type"] != "freespace"
                @fs_txt = create_element("div", "", @fs)
                if m_part_info[@id]["fs"] != "unused"
                    @fs_txt.innerText = m_part_info[@id]["fs"]
                else
                    @fs_txt.innerText = ""
        else if __selected_mode == "advance"
            if v_part_info[@id]? and v_part_info[@id]["type"] != "freespace"
                if @active
                    @fs_select = new DropDown("dd_fs_" + @id, @fs_change_cb)
                    @fs.appendChild(@fs_select.element)
                    if DCore.Installer.disk_support_efi(v_part_info[@id]["disk"])
                        @fs_select.set_drop_items(__fs_efi_keys, __fs_efi_values)
                    else
                        @fs_select.set_drop_items(__fs_keys, __fs_values)
                    @fs_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
                    @fs_select.show_drop()
                    @fs_select.set_selected(v_part_info[@id]["fs"])
                else
                    @fs_txt = create_element("div", "", @fs)
                    if v_part_info[@id]["fs"] != "unused"
                        @fs_txt.innerText = v_part_info[@id]["fs"]
                    else
                        @fs_txt.innerText = ""

    fs_change_cb: (part, fs) ->
        if fs in ["efi", "swap", "unused", "fat16", "fat32", "ntfs"]
            Widget.look_up("dd_mp_" + part)?.hide_drop()
        else
            Widget.look_up("dd_mp_"+ part)?.set_drop_items(__mp_keys, __mp_values)
            Widget.look_up("dd_mp_"+ part)?.show_drop()
        update_part_fs(part, fs)

    fill_mount: ->
        @mount.innerHTML = ""
        if __selected_mode != "advance" 
            return
        if not v_part_info[@id]? or v_part_info[@id]["type"] == "freespace"
            return
        if @active 
                @mount_select = new DropDown("dd_mp_" + @id, @mp_change_cb)
                @mount.appendChild(@mount_select.element)
                if v_part_info[@id]["fs"]? 
                    @mount_select.set_drop_items(__mp_keys, __mp_values)
                @mount_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
                @mount_select.show_drop()
                @mount_select.set_selected(v_part_info[@id]["mp"])
                if v_part_info[@id]["fs"] in ["efi", "swap", "unused", "fat16", "fat32", "ntfs"]
                    @mount_select.hide_drop()
                return
        else
            if v_part_info[@id]["fs"] not in ["efi", "swap", "unused", "fat16", "fat32", "ntfs"]
                @mount_txt = create_element("div", "", @mount)
                if v_part_info[@id]["mp"] == "unused"
                    @mount_txt.innerText = ""
                else
                    @mount_txt.innerText = v_part_info[@id]["mp"]

    mp_change_cb: (partid, mp) ->
        if mp in get_selected_mp()
            part = get_mp_partition(mp)
            if part?
                v_part_info[part]["mp"] = "unused"
                Widget.look_up(part)?.fill_mount()
            else
                echo "error in get mp partition"
        update_part_mp(partid, mp)

    set_btn_status: ->
        if __selected_mode != "advance"
            return 
        type = v_part_info[@id]["type"]

        add_btn = document.getElementById("part_add")
        delete_btn = document.getElementById("part_delete")

        if type == "freespace"
            if not can_add_normal(@id) and not can_add_logical(@id)
                add_btn.setAttribute("class", "PartBtn")
            else
                add_btn.setAttribute("class", "PartBtnActive")
            delete_btn.setAttribute("class", "PartBtn")
        else if type in ["normal", "logical"]
            add_btn.setAttribute("class", "PartBtn")
            delete_btn.setAttribute("class", "PartBtnActive")
        else
            add_btn.setAttribute("class", "PartBtn")
            delete_btn.setAttribute("class", "PartBtn")

        if type != "disk" and v_part_info[@id]["lvm"]
            add_btn.setAttribute("class", "PartBtn")
            delete_btn.setAttribute("class", "PartBtn")

    focus: ->
        @passive_focus()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(@lineid)?.passive_focus()

    passive_focus: ->
        __selected_item?.blur()
        __selected_item = @
        @element.scrollIntoView()
        @active = true
        @fill_fs()
        @fill_mount()
        @set_btn_status()
        @check_busy()
        @fill_format()
        style = "background:rgba(246,178,82,0.5);"
        style += "font-style:bold;"
        style += "text-shadow:0 1px 2px rgba(0,0,0,0.7);"
        @element.setAttribute("style", style)

    blur: ->
        @active = false
        @fill_fs()
        @fill_mount()
        @fill_format()
        @lock.innerHTML = ""
        @element.setAttribute("style", "")

    is_busy: ->
        if __selected_mode == "advance"
            if v_part_info[@id]["lvm"]? and v_part_info[@id]["lvm"] == true
                return true
        else
            if @id in m_disk_info[v_part_info[@id]["disk"]]["partitions"]
                if m_part_info[@id]["lvm"]? and m_part_info[@id]["lvm"] == true
                    return true
        if @id in m_disk_info[__selected_disk]["partitions"]
            if DCore.Installer.get_partition_busy(@id)
                return true
        return false

    do_click: (e)->
        if __selected_item != @ 
            @focus()

    lock_busy: ->
        @lock.innerHTML = ""
        if __selected_mode == "advance"
            create_img("", "images/lock.png", @lock)
            delete_btn = document.getElementById("part_delete")
            delete_btn.setAttribute("class", "PartBtn")
            @fs_select?.set_list_enable(false)
            @mount_select?.set_list_enable(false)

    unbusy: ->
        @lock.innerHTML = ""
        if __selected_mode == "advance"
            @fs_select?.set_list_enable(true)
            @mount_select?.set_list_enable(true)

    check_busy: ->
        if @is_busy()
            @lock_busy()
        else
            @unbusy()

class DiskTabItem extends Widget
    constructor: (@id, @disk)->
        super
        @focuspart = null
        @element.innerText = v_disk_info[@disk]["path"]
        if __selected_disk == @disk
            @focus()

    do_click: (e) ->
        if __selected_disk_item != @
            @focus()

    blur: ->
        __selected_disk_item = null
        @element.setAttribute("style", "")

    focus: ->
        __selected_disk_item?.blur()
        __selected_disk_item = @
        __selected_disk = @disk
        @element.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(rgba(255,255,255,0.5)), to(rgba(255,255,255, 0.2)));")
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up(__selected_item?.id)?.focus()

class PartTable extends Widget
    constructor: (@id)->
        super
        @disktab = create_element("div", "PartTab", @element)

        @disk_content = create_element("div", "PartContent", @element)
        @header = create_element("div", "PartTableHeader", @disk_content)
        @device_header = create_element("div", "Fat", @header)
        @device_header.innerText = _("Device")
        @size_header = create_element("div", "Size", @header)
        @size_header.innerText = _("Size")
        @used_header = create_element("div", "Thin", @header)
        @used_header.innerText = _("Free Space")
        @fs_header = create_element("div", "Fs", @header)
        @fs_header.innerText = _("Filesystem")
        @info_header = create_element("div", "Info", @header)
        @info_header.innerText = _("Information")
        @mount_header = create_element("div", "Mount", @header)
        @mount_header.innerText = _("Mount point")
        @format_header = create_element("div", "Thin", @header)
        @format_header.innerText = _("Format")
        if __selected_mode == "advance"
            @info_header.style.display = "none"
            @mount_header.style.display = "block"
            @format_header.style.display = "block"
        else
            @info_header.style.display = "block"
            @mount_header.style.display = "none"
            @format_header.style.display = "none"

        @items = create_element("div", "PartTableItems", @disk_content)
        @fill_disk_tab()
        @fill_items()

    fill_disk_tab: ->
        @disktabs = []
        for disk in disks
            item = new DiskTabItem("disk_tab_" + disk, disk)
            @disktab.appendChild(item.element)
            @disktabs.push(item)

    fill_items: ->
        @items.innerHTML = ""
        @partitems = []
        disk = __selected_disk
        if __selected_mode == "advance"
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
                    @partitems.push(item)
        else
            for part in m_disk_info[disk]["partitions"]
                if m_part_info[part]["type"] in ["normal", "logical"] and m_part_info[part]["op"] != "add"
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
                    @partitems.push(item)
        if @partitems.length > 0 and @items.scrollHeight > @items.clientHeight
            @partitems[@partitems.length - 1].element.setAttribute("class", "PartTableItem PartTableItemLast")
            
    update_mode: (mode) ->
        if __selected_item?
            id = __selected_item.id
            __selected_item = null
        else 
            id = null
        if mode == "advance"
            @info_header.style.display = "none"
            @mount_header.style.display = "block"
            @format_header.style.display = "block"
            @items.setAttribute("style", "height:180px")
            @element.setAttribute("style", "top:140px;height:230px;")
        else
            @format_header.style.display = "none"
            @mount_header.style.display = "none"
            @info_header.style.display = "block"
            @items.setAttribute("style", "")
            @element.setAttribute("style", "")
        @fill_items()
        if id?
            __selected_item = Widget.look_up(id)
            __selected_item?.focus()

class Help extends Widget
    constructor: (@id)->
        super
        @displayed = false
        @title = create_element("div", "HelpTitle", @element)
        @title.innerText = _("Installation Help")

        @content = create_element("div", "HelpContent", @element)

        @step1 = create_element("p", "HelpStep", @content)
        @step1_desc = create_element("div", "StepDesc", @step1)
        @step1_desc.innerText = _("1. Please select partitions for installation, specify a root partition and partition types.") 

        @step1_img = create_img("", "images/help1.png", @step1)

        @step1_detail = create_element("div", "StepDetail", @step1)
        @step1_detail.innerHTML = "<p>" + _("Note: to install Linux Deepin, you'll have to set up a root partition. An Ext4 filesystem is recommended.") + "</p>"
        @step1_detail.innerHTML += "<p>" +  _("Setting up separate partitions for /home, /var, or /boot is optional.") + "</p>"
        #@step1_detail.innerHTML += "<p>" +  _("If there are no existing partitions on your disk, please refer to Setting up a new partition.") + "</p>"

        @step2 = create_element("p", "HelpStep", @content)
        @step2_desc = create_element("div", "StepDesc", @step2)
        @step2_desc.innerText = _("2. Select where GRUB will be installed.")

        @step2_img = create_img("", "images/help2.png", @step2)

        @step2_detail = create_element("div", "StepDetail", @step2)
        @step2_detail.innerHTML = "<p>" +  _("Note: You can simply ignore this step. The default location is recommended.") + "</p>"

        @step3 = create_element("p", "HelpStep", @content)
        @step3_desc = create_element("div", "StepDesc", @step3)
        @step3_desc.innerText = _("3. Click \"install\" to proceed with installation, and your system will be ready to use after that.") 

        @step3_detail = create_element("div", "StepDetail", @step3)

        @hide()

    show: ->
        @displayed = true
        @element.style.display = "block"

    hide: ->
        @displayed = false
        @element.style.display = "none"

class Part extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_part.png", @titleprogress)

        @help = create_element("div", "PartTitleSet", @title)
        @t_help = create_element("span", "PartTitleHelp", @help)
        @t_help.innerText = _("Help")
        @t_help.addEventListener("click", (e) =>
            if @help.displayed
                @hide_help()
            else
                @show_help()
        )
        @t_help.style.display = "none"

        @t_mode = create_element("span", "PartTitleMode", @help)
        @t_mode.innerText = _("Expert mode")
        @t_mode.addEventListener("click", (e) =>
            @switch_mode()
        )

        @help = new Help("help")
        @element.appendChild(@help.element)

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        @wrap = create_element("div", "", @element)

        @next_btn = create_element("div", "NextStep", @wrap)
        @next_btn.setAttribute("id", "mynextstep")
        @next_input = create_element("input", "InputBtn", @next_btn)
        @next_input.setAttribute("type", "submit")
        next = _("Install")
        @next_input.setAttribute("value", next)
        @next_btn.addEventListener("mousedown", (e) =>
            @next_input.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(#F8AD4B), to(#FFC040));color:rgba(0,0,0,1);")
        )
        @next_btn.addEventListener("click", (e) =>
            if __selected_mode == "advance"
                target = get_target_part()
                if not target?
                    @root_model = new RootDialog("RootModel")
                    document.body.appendChild(@root_model.element)
                    return
                efi_boot = get_efi_boot_part()
                if efi_boot?
                    __selected_grub = "uefi"
                    if v_part_info[efi_boot]["length"] <= mb_to_sector(100, 512)
                        @uefi_model = new UefiDialog("UefiModel")
                        document.body.appendChild(@uefi_model.element)
                        return
                    legacy_boot = get_legacy_boot_part()
                    if legacy_boot?
                        @uefi_boot_model = new UefiBootDialog("UefiBootModel")
                        document.body.appendChild(@uefi_boot_model.element)
                        return
                else
                    __selected_grub = @grubdropdown?.get_selected()
                if not __selected_grub
                    __selected_grub = v_part_info[target]["disk"]
            else
                __selected_grub = __selected_disk

            @install_model = new InstallDialog("InstallModel")
            document.body.appendChild(@install_model.element)
        )
        @init_part_page()

    init_part_page: ->
        if __selected_mode == null
            __selected_mode = "simple"
        if __selected_disk == null
            __selected_disk = disks[0]
        @linemap = new PartLineMaps("part_line_maps")
        @wrap.appendChild(@linemap.element)
        @table = new PartTable("part_table")
        @wrap.appendChild(@table.element)
        @fill_advance_op()
        if __selected_mode == "advance"
            @show_advance_mode()
        else
            @hide_advance_mode()
        recommand = get_recommand_target()
        if recommand?
            Widget.look_up("disk_tab_"+ m_part_info[recommand]["disk"])?.focus()
            Widget.look_up(recommand)?.focus()
        else
            Widget.look_up("disk_tab" + __selected_disk)?.focus()
        if  __selected_item?
            @next_btn.setAttribute("style", "pointer-events:auto")
            @next_input.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(#F4C688), to(#FFBE57));color:rgba(0,0,0,1);")

    switch_mode: ->
        if __selected_mode != "advance"
            __selected_mode = "advance"
            if check_has_mount()
                @unmount_model = new UnmountDialog("UnmountModel")
                document.body.appendChild(@unmount_model.element)
            @show_advance_mode()
            @table.update_mode(__selected_mode)
            @t_mode.innerText = _("Simple mode")
        else
            __selected_mode = "simple"
            @hide_help()
            @add_model?.hide_dialog()
            @delete_model?.hide_dialog()
            @unmount_model?.hide_dialog()
            @hide_advance_mode()
            @table.update_mode(__selected_mode)
            @t_mode.innerText = _("Expert mode") 

    show_help: ->
        @help.show()
        @wrap.style.display = "none"

    hide_help: ->
        @help.hide()
        @wrap.style.display = "block"

    fill_advance_op: ->
        @op = create_element("div", "PartOp", @wrap)
        @part_delete = create_element("div", "PartBtn", @op)
        @part_delete.setAttribute("id", "part_delete")
        @delete_input = create_element("input", "InputBtn", @part_delete)
        @delete_input.setAttribute("type", "submit")
        delete_value = _("Delete partition")
        @delete_input.setAttribute("value", delete_value)
        @part_delete.addEventListener("click", (e)=>
            echo "handle delete"
            if __in_model
                echo "already had delete part mode dialog"
                return 
            @del_model = new DeletePartDialog("DeleteModel", __selected_item.id)
            document.body.appendChild(@del_model.element)
        )

        @part_add = create_element("div", "PartBtn", @op)
        @part_add.setAttribute("id", "part_add")
        @add_input = create_element("input", "InputBtn", @part_add)
        @add_input.setAttribute("type", "submit")
        new_value = _("New partition")
        @add_input.setAttribute("value", new_value)
        @part_add.addEventListener("click", (e)=>
            echo "handle add"
            if __in_model
                echo "already had add part mode dialog"
                return 
            @add_model = new AddPartDialog("AddModel", __selected_item.id)
            document.body.appendChild(@add_model.element)
        )

        @part_loader = create_element("div", "PartBtn", @op)
        @part_loader.setAttribute("id", "part_loader")
        @loader_input = create_element("input", "InputBtn", @part_loader)
        @loader_input.setAttribute("type", "submit")
        loader_value = _("Setup loader")
        @loader_input.setAttribute("value", loader_value)
        @part_loader.addEventListener("click", (e) =>
            efi_boot = get_efi_boot_part()
            if efi_boot?
                return
            if @part_grub_display
                @hide_part_grub()
            else
                @show_part_grub()
        )

        @part_grub = create_element("div", "PartGrub", @wrap)
        @hide_part_grub()
        @grub_loader = create_element("div", "PartGrubLoader", @part_grub)
        @grub_loader.innerText = _("Boot loader")
        @grub_select = create_element("div", "PartGrubSelect", @part_grub)
        @fill_bootloader()

    hide_part_grub: ->
        @part_grub.style.display = "none"
        @part_grub_display = false

    show_part_grub: ->
        @part_grub.style.display = "block"
        @part_grub_display = true

    fill_bootloader: ->
        keys = []
        values = []
        for disk in disks
            text = v_disk_info[disk]["path"] + "\t" + v_disk_info[disk]["model"] + "\t" + sector_to_gb(v_disk_info[disk]["length"], 512) + "GB"
            keys.push(disk)
            values.push(text)
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical"]
                    keys.push(part)
                    values.push(v_part_info[part]["path"])
        if @grub_dropdown?
            @grub_dropdown?.destroy()
            @grub_dropdown = null
        @grub_dropdown = new DropDown("dd_grub", null)
        @grub_select.appendChild(@grub_dropdown.element)
        @grub_dropdown.set_drop_items(keys, values)
        @grub_dropdown.set_drop_size(560, 20)
        @grub_dropdown.show_drop()
        #@grub_dropdown.set_list_background("url(\"images/dropdown.png\");")
        @grub_dropdown.set_list_scroll_height(true, 180)

    show_advance_mode: ->
        @t_help.style.display = "block"
        @linemap.element.setAttribute("style", "display:block")
        @op.setAttribute("style", "display:block")
        #@part_grub.setAttribute("style", "display:block")

    hide_advance_mode: ->
        @t_help.style.display = "none"
        @linemap.element.setAttribute("style", "display:none")
        @op.setAttribute("style", "display:none")
        @part_grub.setAttribute("style", "display:none")

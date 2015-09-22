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
__selected_home = null
__selected_bootloader = null
__selected_disk = null
__selected_item = null
__selected_line = null
__selected_mode = "simple"
__selected_stage = null

class PartLineItem extends Widget
    constructor: (@id) ->
        super
        @part = @id[4...16]
        @init_line_item()

    init_line_item: ->
        disk = v_part_info[@part]["disk"]
        @color = v_part_info[@part]["color"]
        @element.style.background = @color
        @element.style.width = Math.round(v_part_info[@part]["length"] / v_disk_info[disk]["length"] * 700)
        @element.style.left = Math.round(v_part_info[@part]["start"] / v_disk_info[disk]["length"] * 700)
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
        get_disk_fake_length(__selected_disk)
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
        @mount = create_element("div", "Thin SimpleMount", @element)
        @format = create_element("div", "Thin", @element)
        @fill_device()
        @fill_size()
        @update_part_used()
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
            @fill_device_advance()
        else if __selected_mode == "simple"
            @fill_device_simple()
        txt = @path.innerText
        @path.title = @path.innerText
        @path.addEventListener("mouseover", (e) =>
            if os? and os.length > 2
                @path.innerText = DCore.Installer.get_partition_os_desc(@id).split("(")[0].trim()
                @path.title = @path.innerText
        )
        @path.addEventListener("mouseout", (e) =>
            @path.innerText = txt
            @path.title = @path.innerText
        )

    fill_device_advance: ->
        if v_part_info[@id]["type"] != "freespace"
            @path.innerText = v_part_info[@id]["path"]
        else
            @path.innerText = _("Freespace")
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
        os = v_part_info[@id]["os"]
        @update_device_os(os)

    fill_device_simple: ->
        if m_part_info[@id]["type"] != "freespace"
            @path.innerText = m_part_info[@id]["path"]
        else
            @path.innerText = _("Freespace")
        if m_part_info[@id]["label"]? and m_part_info[@id]["label"].length > 0
            if m_part_info[@id]["label"].length > 12
                @label.innerText = m_part_info[@id]["label"].substring(0,12) + "..."
            else
                @label.innerText = m_part_info[@id]["label"]
        else
            @label.style.display = "none"
            @path.setAttribute("style", "margin:10px 0;")
        @color.style.display = "none"
        os = m_part_info[@id]["os"]
        @update_device_os(os)

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

    update_device_os: (os) ->
        console.log("[part.coffee] PartTableItem.update_device_os, os: #{os}")
        if os? and os.length > 2
            if os.toLowerCase().indexOf("deepin") != -1
                os_img = "images/deepin.png"
            else if os.toLowerCase().indexOf("linux") != -1
                os_img = "images/linux.png"
            else if os.toLowerCase().indexOf("windows") != -1
                os_img = "images/windows.png"
            else if os.toLowerCase().indexOf("mac") != -1
                os_img = "images/mac.png"
            else
                os_img = "images/linux.png"
            create_img("", os_img, @os)

    fill_size: ->
        @size.innerHTML = ""
        if __selected_mode == "advance"
            @size.innerText += (v_part_info[@id]["length"] / GB).toFixed(1) + "G"
        else
            @size.innerText += (m_part_info[@id]["length"] / GB).toFixed(1) + "G"

    update_part_used: ->
        @used.innerHTML = ""
        if __selected_mode == "advance" and v_part_info[@id]["type"] != "freespace"
            if isNaN(v_part_info[@id]["used"])
                @used.innerText = _("Unknown")
            else
                @used.innerText = (v_part_info[@id]["used"] / GB).toFixed(1) + "G"
        else if __selected_mode == "simple" and m_part_info[@id]["type"] != "freespace"
            if isNaN(m_part_info[@id]["used"])
                @used.innerText = _("Unknown")
            else
                @used.innerText = (m_part_info[@id]["used"] / GB).toFixed(1) + "G"

    fill_format: ->
        @format.innerHTML = ""
        if __selected_mode == "advance"
            @fill_format_advance()
        else
            @format.style.display = "none"

    fill_format_advance: ->
        if not v_part_info[@id]? or v_part_info[@id]["type"] == "freespace"
            return
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
            @fill_fs_simple()
        else if __selected_mode == "advance"
            @fill_fs_advance()
        else
            assert(false, "not_reached")

    fill_fs_advance: ->
        if not v_part_info[@id]? or v_part_info[@id]["type"] == "freespace"
            return
        if @active
            @fs_select = new DropDown("dd_fs_" + @id, false, (data)=> @fs_change_cb(@id, data))
            @fs.appendChild(@fs_select.element)
            if __selected_use_uefi
                @fs_select.set_drop_items(__fs_efi_keys, __fs_efi_values)
            else
                @fs_select.set_drop_items(__fs_keys, __fs_values)
            @fs_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
            @fs_select.set_selected(v_part_info[@id]["fs"])
            @fs_select.show_drop()
        else
            @fs_txt = create_element("div", "", @fs)
            if v_part_info[@id]["fs"] != "unused"
                @fs_txt.innerText = v_part_info[@id]["fs"]
            else
                @fs_txt.innerText = ""

    fill_fs_simple: ->
        if m_part_info[@id]? and m_part_info[@id]["type"] != "freespace"
            @fs_txt = create_element("div", "", @fs)
            if m_part_info[@id]["fs"] != "unused"
                @fs_txt.innerText = m_part_info[@id]["fs"]
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
            @mount.setAttribute("class", "Thin SimpleMount")
            if @active
                @mount.innerText = _("Install Here")
            return
        else
            @mount.setAttribute("class", "Thin AdvancedMount")
            @fill_mount_advance()

    fill_mount_advance: ->
        if not v_part_info[@id]? or v_part_info[@id]["type"] == "freespace"
            return
        if @active
            @mount_select = new DropDown("dd_mp_" + @id, true, (data)=>@mp_change_cb(@id, data))
            @mount.appendChild(@mount_select.element)
            if v_part_info[@id]["fs"]?
                @mount_select.set_drop_items(__mp_keys, __mp_values)
            @mount_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
            if v_part_info[@id]["mp"].substring(0,1) != "/"
                v_part_info[@id]["mp"] = "unused"
            @mount_select.set_selected(v_part_info[@id]["mp"])
            @mount_select.show_drop()
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
        console.log("[part.coffee] PartTableItem.mp_change_cb(), partid: #{partid}, mp: #{mp}")
        if mp.substring(0,1) != "/"
            mp = "unused"

        if mp is "/boot" and v_part_info[partid]["length"] < BOOT_SIZE_MIN
            new MessageDialog(
                _("Warning"),
                _("You  may not boot due to too small space of /boot mount point, please improve the space to 300M at least.")
            ).show_at(document.body)
            return false

        if mp in get_selected_mp()
            part = get_mp_partition(mp)
            if part? and part != partid
                v_part_info[part]["mp"] = "unused"
                Widget.look_up(part)?.fill_mount()
            else
                console.error("[part.coffee] PartTableItem.mp_change_cb() error, invalid part: #{part}, partid: #{partid}")
        update_part_mp(partid, mp)

    set_btn_status: ->
        if __selected_mode != "advance"
            return
        type = v_part_info[@id]["type"]
        add_btn = document.getElementById("part_add")
        delete_btn = document.getElementById("part_delete")

        if type == "freespace"
            if not can_add_normal(@id) and not can_add_logical(@id)
                add_btn.setAttribute("class", "PartAddBtn")
            else
                add_btn.setAttribute("class", "PartAddBtnActive")
            delete_btn.setAttribute("class", "PartDeleteBtn")
        else if type in ["normal", "logical"]
            add_btn.setAttribute("class", "PartAddBtn")
            delete_btn.setAttribute("class", "PartDeleteBtnActive")
        else
            add_btn.setAttribute("class", "PartAddBtn")
            delete_btn.setAttribute("class", "PartDeleteBtn")

        if type != "disk" and v_part_info[@id]["lvm"]
            add_btn.setAttribute("class", "PartAddBtn")
            delete_btn.setAttribute("class", "PartDeleteBtn")

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
        style = "background:rgba(255,255,3,0.3);"
        style += "font-style:bold;"
        style += "text-shadow:0 1px 2px rgba(0,0,0,0.7);"
        @element.setAttribute("style", style)
        @onfocus = true

    blur: ->
        @active = false
        @fill_fs()
        @fill_mount()
        @fill_format()
        @lock.innerHTML = ""
        @element.setAttribute("style", "")
        @onfocus = false

    is_busy: ->
        if __selected_mode == "advance"
            if v_part_info[@id]["lvm"]? and v_part_info[@id]["lvm"] == true
                return true
        else
            if @id in m_disk_info[v_part_info[@id]["disk"]]["partitions"]
                if m_part_info[@id]["lvm"]? and m_part_info[@id]["lvm"] == true
                    return true
        if @id in m_disk_info[__selected_disk]["partitions"]
            if DCore.Installer.is_partition_busy(@id)
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
            delete_btn.setAttribute("class", "PartDeleteBtn")
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

class DiskTab extends Widget
    constructor: (@id) ->
        super
        @prev = create_element("div", "Prev", @element)
        enable_tab(@prev)
        @prev.addEventListener("click", (e) =>
            @prev.style.background = "images/arrow_left_press.png"
            @switch_prev()
        )
        @content = create_element("div", "Content", @element)
        @next = create_element("div", "Next", @element)
        enable_tab(@next)
        @next.addEventListener("click", (e) =>
            @next.style.background = "images/arrow_right_press.png"
            @switch_next()
        )
        if disks.length < 2
            @element.style.display = "none"

    focus_disk: (disk) ->
        index = disks.indexOf(disk) + 1
        if index > 0
            __selected_disk = disk
            size = (v_disk_info[disk]["length"] / GB).toFixed(0)
            @content.innerText = _("Disk") + index  + "  (" + +  size + "GB) "
            Widget.look_up("part_line_maps")?.fill_linemap()
            Widget.look_up("part_table")?.fill_items()
            Widget.look_up(__selected_item?.id)?.focus()

    switch_prev: ->
        index = disks.indexOf(__selected_disk)
        if index > 0
            @focus_disk(disks[index-1])

    switch_next: ->
        index = disks.indexOf(__selected_disk)
        if index < disks.length
            @focus_disk(disks[index+1])

class PartTable extends Widget
    constructor: (@id)->
        super
        @header = create_element("div", "PartTableHeader", @element)
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
        @mount_header.innerText = _("Mount Point")
        @format_header = create_element("div", "Thin", @header)
        @format_header.innerText = _("Formatting")

        @items = create_element("div", "PartTableItems", @element)

        @op = create_element("div", "PartOp", @element)
        @part_delete = create_element("div", "PartDeleteBtn", @op)
        @part_delete.setAttribute("id", "part_delete")
        @part_delete.innerText = _("Delete Partition")
        @part_delete.addEventListener("click", (e)=>
            console.log("[part.coffee] PartTable.constructor() handle click(delete) event")
            if __in_model
                console.warn("[part.coffee] PartTable.constructor() handle click(delete) event, delete-part-mode-dialog exists, ignored!")
                return
            new DeletePartDialog("DeleteModel", __selected_item.id).show_at(document.body)
        )

        @part_add = create_element("div", "PartAddBtn", @op)
        @part_add.setAttribute("id", "part_add")
        @part_add.innerText = _("New Partition")
        @part_add.addEventListener("click", (e)=>
            console.log("[part.coffee] PartTable.constructor() handle click(add) event")
            if __in_model
                console.warn("[part.coffee] PartTable.constructor() handle click(add) event, add-part-mode-dialog exists, ignored!")
                return
            @add_model = new AddPartDialog("AddModel", __selected_item.id)
            @add_model.show_at(document.body)
        )

        @update_mode(__selected_mode)

    fill_items: ->
        @items.innerHTML = ""
        @partitems = []
        if __selected_mode == "advance"
            @fill_items_advance()
        else
            @fill_items_simple()

    key_select: (enter_cb) =>
        enable_tab(@element)
        @element.addEventListener("keyup",(e)=>
            e.stopPropagation()
            currentIndex = i for item,i in @partitems when item.onfocus
            switch e.keyCode
                when KEYCODE.UP_ARROW
                    currentIndex--
                when KEYCODE.DOWN_ARROW
                    currentIndex++
                when KEYCODE.ENTER
                    console.log("[part.coffee] PartTable.key_select() ENTER key pressed")
                    enter_cb?()
                else
                    return
            if currentIndex < 0
                currentIndex = @partitems.length - 1
            else if currentIndex > @partitems.length - 1
                currentIndex = 0
            console.debug("[part.coffee] PartTable.key_select() currentIndex: #{currentIndex}")
            select = @partitems[currentIndex]
            if select
                select.focus()
            return
        )


    fill_items_advance: ->
        @info_header.style.display = "none"
        @mount_header.style.display = "block"
        @format_header.style.display = "block"
        @op.style.display = "block"
        disk = __selected_disk
        for part in v_disk_info[disk]["partitions"]
            if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                item = new PartTableItem(part)
                @items.appendChild(item.element)
                @partitems.push(item)
        @items.setAttribute("style","height:222px")
        @element.setAttribute("style", "height:280px")

    fill_items_simple: ->
        @format_header.style.display = "none"
        @mount_header.style.display = "none"
        @op.style.display = "none"
        @info_header.style.display = "block"
        disk = __selected_disk
        for part in m_disk_info[disk]["partitions"]
            if m_part_info[part]["type"] in ["normal", "logical", "freespace"] and m_part_info[part]["op"] != "add"
                item = new PartTableItem(part)
                @items.appendChild(item.element)
                @partitems.push(item)
        @items.setAttribute("style", "")
        @element.setAttribute("style", "")

    update_mode: (mode) ->
        if __selected_item?
            id = __selected_item.id
            __selected_item = null
        else
            id = null
        @fill_items()
        if id?
            __selected_item = Widget.look_up(id)
            __selected_item?.focus()

class Part extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_part.png", @titleprogress)

        @helpset = create_element("div", "TitleSet", @title)

        @t_mode = create_element("div", "PartTitleMode", @helpset)
        enable_tab(@t_mode)
        @t_mode.innerText = _("Expert Mode")
        @t_mode.addEventListener("click", (e) =>
            @t_mode.setAttribute("class", "PartTitleMode TitlesetActive")
            @switch_mode()
        )

        @wrap = create_element("div", "part_wrap", @element)

        @init_part_page()

        @next_step = new NextStep("mynextstep",_("Install"),@start_install_cb)
        @wrap.appendChild(@next_step.element)
        @next_step.set_pos("absolute","285px","20px")
        @next_step.next_bt_disable()

        @switch_mode_simple()
        recommand = get_recommand_target()
        if recommand?
            @disktab.focus_disk(m_part_info[recommand]["disk"])
            Widget.look_up(recommand)?.focus()
        else
            @disktab.focus_disk(__selected_disk)
        if  __selected_item? then @next_step.next_bt_enable()

        # Global instance of confirm dialog.
        @confirm_installation_dialog = null

    start_install_cb: =>
        if __selected_mode == "advance"
            @handle_install_advance()
        else
            @handle_install_simple()


    handle_install_advance: ->
        __selected_use_uefi = @uefi_radio.checked
        target = get_target_part()
        if not target?
            new MessageDialog(
                _("Installation Tips"),
                _("A root partition (/) is required.")
            ).show_at(document.body)
            return

        if __selected_use_uefi
            if not DCore.Installer.disk_is_gpt(__selected_disk)
                new MessageDialog(
                    _("Installation Tips"),
                    _("UEFI-native installation only supports GPT-formatted disk.")
                ).show_at(document.body)
                return

            esp_uuid = get_efi_boot_part()
            if not esp_uuid or v_part_info[esp_uuid]["length"] < 100 * MB
                new MessageDialog(
                    _("Installation Tips"),
                    _("ESP’s support is needed in UEFI mode and please create a partition of 100MB or more in efi format.")
                ).show_at(document.body)
                return

            __selected_bootloader = esp_uuid
        else
            __selected_bootloader = @grub_dropdown?.get_selected()
            if not __selected_bootloader
                __selected_bootloader = v_part_info[target]["disk"]

        if @confirm_installation_dialog
            @confirm_installation_dialog.hide_dialog()
        @confirm_installation_dialog = new InstallDialog("InstallModel")
        callback = ->
            document.querySelector(".PartTable").focus()
        @confirm_installation_dialog.cancel_cb = callback
        @confirm_installation_dialog.title_close.addEventListener("click", (e)=>
            callback()
        )
        @confirm_installation_dialog.show_at(document.body)

    handle_install_simple: ->
        if not __selected_item
            return

        if m_part_info[__selected_item.id]["type"] == "freespace"
            if not can_add_normal(__selected_item.id) and not can_add_logical(__selected_item.id)
                new MessageDialog(
                    _("Add Partition"),
                    _("Can't create a partition here")
                ).show_at(document.body)
                return

        if DCore.Installer.system_support_efi() and not DCore.Installer.disk_is_gpt(__selected_disk)
            __selected_use_uefi = false
        __selected_bootloader = __selected_disk

        if @confirm_installation_dialog
            @confirm_installation_dialog.hide_dialog()
        @confirm_installation_dialog = new InstallDialog("InstallModel")
        callback = ->
            document.querySelector(".PartTable").focus()
        @confirm_installation_dialog.cancel_cb = callback
        @confirm_installation_dialog.title_close.addEventListener("click", (e)=>
            callback()
        )
        @confirm_installation_dialog.show_at(document.body)

    init_part_page: ->
        if __selected_mode == null
            __selected_mode = "simple"
        if __selected_disk == null
            __selected_disk = disks[0]

        @disktab = new DiskTab("disk_tab")
        @wrap.appendChild(@disktab.element)

        @linemap = new PartLineMaps("part_line_maps")
        @wrap.appendChild(@linemap.element)

        @table = new PartTable("part_table")
        @table.key_select(@start_install_cb)
        @wrap.appendChild(@table.element)

        @part_uefi = create_element("div","PartUefi",@wrap)
        enable_tab(@part_uefi)
        @uefi_radio = create_element("input","grub_radio",@part_uefi)
        @uefi_radio.setAttribute("type","radio")
        @uefi_radio.setAttribute("id", "uefi_radio_id")
        @uefi_radio.defaultChecked = false
        uefi_label = create_element("label", "uefi_label", @part_uefi)
        uefi_label.setAttribute("for", "uefi_radio_id")
        @uefi_txt = create_element("div","uefi_txt",@part_uefi)
        @uefi_txt.innerText = _("UEFI")

        @part_grub = create_element("div", "PartGrub", @wrap)
        enable_tab(@part_grub)
        @grub_radio = create_element("input", "grub_radio", @part_grub)
        @grub_radio.setAttribute("type","radio")
        @grub_radio.setAttribute("id", "grub_radio_id")
        @grub_radio.defaultChecked = true
        grub_label = create_element("label", "grub_label", @part_grub)
        grub_label.setAttribute("for", "grub_radio_id")
        @grub_loader = create_element("div", "PartGrubLoader", @part_grub)
        @grub_loader.innerText = _("Boot Loader")
        @grub_select = create_element("div", "PartGrubSelect", @part_grub)
        @fill_bootloader()


    fill_bootloader: ->
        keys = []
        values = []
        for disk in disks
            text = v_disk_info[disk]["path"] + "\t" + v_disk_info[disk]["model"] + "\t" + (v_disk_info[disk]["length"] / GB).toFixed(0) + "GB"
            keys.push(disk)
            values.push(text)
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical"]
                    keys.push(part)
                    values.push(v_part_info[part]["path"])
        if @grub_dropdown?
            @grub_dropdown?.destroy()
            @grub_dropdown = null
        @grub_dropdown = new DropDown("dd_grub", false, null)
        @grub_select.appendChild(@grub_dropdown.element)
        @grub_dropdown.set_drop_items(keys, values)
        width = 500
        @grub_dropdown.set_drop_size(width, 20)
        @grub_dropdown.show_drop()

    switch_mode: ->
        if __selected_mode != "advance"
            @switch_mode_advance()
        else
            @switch_mode_simple()

    unmount_part: =>
        for disk in disks
            for part in m_disk_info[disk]["partitions"]
                try
                    if DCore.Installer.get_partition_mp(part) not in ["/", "/cdrom"]
                        DCore.Installer.unmount_partition(part)
                catch error
                    console.error("[part.coffee] Part.umount_part() call umount_part() error: #{error}, part: #{part}")
        for item in Widget.look_up("part_table")?.partitems
            item.check_busy()

    switch_mode_advance: ->
        __selected_mode = "advance"
        if check_has_mount(__selected_disk)
            if !check_only_swap_mount(__selected_disk)
                @unmount_model = new PromptDialog(
                    _("Unmount Partition"),
                    _("Partition is detected to have been mounted.\nAre you sure you want to unmount it?"),
                    @unmount_part
                )
                @unmount_model.show_at(document.body)
            else @unmount_part()
        @linemap.element.setAttribute("style", "display:block")

        if DCore.Installer.system_support_efi()
            @part_uefi.style.display = ""
            width = 500
            @grub_dropdown.set_drop_size(width, 20)
            @grub_dropdown.show_drop()

            click_cb = =>
                @grub_radio?.checked = false
                @uefi_radio?.checked = true

            ok_cb = =>
                @grub_select?.style.display = "none"
                @grub_radio?.checked = false
                @uefi_radio?.checked = true

            cancel_cb = =>
                @grub_select?.style.display = "block"
                @grub_radio?.checked = true
                @uefi_radio?.checked = false

            if DCore.Installer.disk_is_gpt(__selected_disk)
                ok_cb()
            else
                cancel_cb()
            @uefi_radio.addEventListener("click",(e)=>
                e.stopPropagation()
                if __selected_disk is null then return
                click_cb()
                if DCore.Installer.disk_is_gpt(__selected_disk)
                    ok_cb()
                else
                    new PromptDialog(
                        _("Warning"),
                        _("UEFI-native installation only supports GPT-formatted disk. You will lose all disk data if you insist on installing."),
                        ok_cb,
                        cancel_cb
                    ).show_at(document.body)
            )
            @grub_radio.addEventListener("click",cancel_cb)
        else
            @part_uefi.style.display = "none"
            @grub_radio.style.display = "none"

        @part_grub.style.display = ""

        @table.update_mode(__selected_mode)
        @t_mode.innerText = _("Simple Mode")

    switch_mode_simple: ->
        __selected_mode = "simple"
        @add_model?.hide_dialog()
        @delete_model?.hide_dialog()
        @unmount_model?.hide_dialog()
        @linemap.element.setAttribute("style", "display:none")
        @part_uefi.style.display = "none"
        @part_grub.style.display = "none"
        @table.update_mode(__selected_mode)
        @t_mode.innerText = _("Expert Mode")

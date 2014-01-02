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
        @fs_desc.innerText = _("Format:")
        @fs_value = create_element("span", "AddValue", @fs)
        @fs_select = new DropDown("dd_fs_" + @partid, null)
        @fs_value.appendChild(@fs_select.element)
        @fs_select.set_drop_items(__fs_keys, __fs_values)
        @fs_select.set_drop_size(130,22)
        @fs_select.show_drop()
        @fs_select.set_selected("ext4")

    fill_mount: ->
        @mp = create_element("div", "", @content)
        @mp_desc = create_element("span", "AddDec", @mp)
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
            @tips.innerText = _("Please enter invalid partition size")
        @n_align = @align_radio
        @n_fs = @fs_select.get_selected()
        @n_mp = @mount_select.get_selected()

class DeletePartDialog extends Dialog
    constructor: (@id,@partid) ->
        super(@id, true, @delete_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Delete partition")
        @delete_tips = create_element("div", "", @content)
        @delete_tips.innerText = _("Confirm to delete?")

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
        @unmount_tips.innerText = _("Detected partitions mounted, confirm to unmount them?")

    unmount_cb: ->
        echo "unmount all partitions"
        for disk in disks
            for part in m_disk_info[disk]["partitions"]
                try
                    DCore.Installer.unmount_partition(part)
                catch error
                    echo error

class FormatDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @format_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Format partition")
        @format_tips = create_element("div", "", @content)
        @format_tips.innerText = _("Confirm to format the partition?")

    format_cb: ->
        echo "format to do install"

class RootDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @need_root_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Install tips")
        @root_tips = create_element("div", "", @content)
        @root_tips.innerText = _("Need mount a partition as root")

    need_root_cb: ->
        echo "need mount root to do install"

class InstallDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @confirm_install_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Confirm Install")
        @root_tips = create_element("div", "", @content)
        @fill_install_info()

    confirm_install_cb: ->
        echo "confirm install"
        DCore.Installer.hide_help()
        progress_page = new Progress("progress")
        progress_page.update_progress("0%")

        if __selected_mode == "simple"
            __selected_grub = __selected_disk
            pc.add_page(progress_page)
            pc.remove_page(part_page)
            do_simple_partition(__selected_item.id, __selected_item.device_type)

        else if __selected_mode == "advance"
            grub = Widget.look_up("part")?.grub_select
            if grub?
                __selected_grub = grub.options[grub.selectedIndex].value
            else
                __selected_grub = __selected_disk
            pc.add_page(progress_page)
            pc.remove_page(part_page)
            do_partition()
        progress_page.update_progress("2%")
        __selected_stage = "extract"
        progress_page.handle_extract("start")

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
        if __selected_line == @ 
            echo "part line item already selected"
        else
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
                if __selected_item?.id == __selected_disk
                    item.element.setAttribute("class", "PartLineItemActive")

class PartTableItem extends Widget
    constructor: (@id)->
        super
        @lineid = "line" + @id
        @active = false
        @device = create_element("span", "", @element)
        @size = create_element("div", "", @element)
        @used = create_element("div", "", @element)
        @fs = create_element("div", "Fs", @element)
        @mount = create_element("span", "Mount", @element)
        @fill_device()
        @fill_size()
        @fill_used()
        @fill_fs()
        @fill_mount()

    fill_device: ->
        @device.innerHTML = ""
        @os = create_element("span", "Os", @device)
        @color = create_element("span", "Color", @device)
        color_value = Widget.look_up(@lineid)?.color or get_random_color()
        @color.style.background = color_value
        @lp = create_element("span", "LabelPath", @device)
        @label = create_element("div", "Label", @lp)
        @path = create_element("div", "Path", @lp)
        @label.addEventListener("hover", (e) =>
            @show_detail_label()
        )

        if __selected_mode == "advance"
            if v_part_info[@id]["type"] != "freespace"
                @path.innerText = v_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
            if v_part_info[@id]["label"]? and v_part_info[@id]["label"].length > 0
                @label.innerText = v_part_info[@id]["label"]
            else
                @label.style.display = "none"
                @path.setAttribute("style", "margin:10px 0;")
        else if __selected_mode == "simple"
            if m_part_info[@id]["type"] != "freespace"
                @path.innerText = m_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
            if m_part_info[@id]["label"]? and m_part_info[@id]["label"].length > 0
                @label.innerText = m_part_info[@id]["label"]
            else
                @label.style.display = "none"
                @path.setAttribute("style", "margin:10px 0;")
        @update_device_os()

    show_detail_label: ->
        text = @label.innerText
        if not text or text.length < 5
            return
        else
            echo "show detail label"

    update_device_os: ->
        @os.innerHTML = ""
        os = DCore.Installer.get_partition_os(@id)
        if os? and os.length > 2
            if os.toLowerCase().indexOf("deepin") != -1
                os_img = "images/deepin.png"
            else if os.toLowerCase().indexOf("linux") != -1
                os_img = "images/linux.png"
            else if os.toLowerCase().indexOf("windows") != -1
                os_img = "images/windows.png"
            else if os.toLowerCase().indexOf("mac") != -1
                os_img = "images/apple.png"
            create_img("osimg", os_img, @os)

    fill_size: ->
        @size.innerHTML = ""
        if __selected_mode == "advance"
            @size.innerText += sector_to_gb(v_part_info[@id]["length"], 512).toFixed(3) + "GB"
        else
            @size.innerText += sector_to_gb(m_part_info[@id]["length"], 512).toFixed(3) + "GB"

    fill_used: ->
        @used.innerHTML = ""
        if __selected_mode == "advance" and v_part_info[@id]["type"] != "freespace"
            @used.innerText = (v_part_info[@id]["used"]/1000).toFixed(3) + "GB"
        else if __selected_mode == "simple" and m_part_info[@id]["type"] != "freespace"
            @used.innerText = (m_part_info[@id]["used"]/1000).toFixed(3) + "GB"

    update_part_used: ->
        if __selected_mode == "advance"
            @used.innerText = (v_part_info[@id]["used"]/1000).toFixed(3) + "GB"
        else
            @used.innerText = (m_part_info[@id]["used"]/1000).toFixed(3) + "GB"

    fill_fs: ->
        @fs.innerHTML = ""
        if __selected_mode == "simple" 
            if m_part_info[@id]? and m_part_info[@id]["type"] != "freespace"
                @fs_txt = create_element("div", "", @fs)
                @fs_txt.innerText = m_part_info[@id]["fs"]
        else if __selected_mode == "advance"
            if v_part_info[@id]? and v_part_info[@id]["type"] != "freespace"
                if @active
                    @fs_select = new DropDown("dd_fs_" + @id, update_part_fs)
                    @fs.appendChild(@fs_select.element)
                    if v_part_info[@id]["mp"]? and v_part_info[@id]["mp"] != "unused"
                        @fs_select.set_drop_items(__filter_fs_keys, __filter_fs_values)
                    else
                        @fs_select.set_drop_items(__fs_keys, __fs_values)
                    @fs_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
                    @fs_select.show_drop()
                    @fs_select.set_selected(v_part_info[@id]["fs"])
                else
                    @fs_txt = create_element("div", "", @fs)
                    @fs_txt.innerText = v_part_info[@id]["fs"]

    fill_mount: ->
        @mount.innerHTML = ""
        if __selected_mode != "advance" 
            return
        if not v_part_info[@id]? or v_part_info[@id]["type"] == "freespace"
            return
        if @active 
                @mount_select = new DropDown("dd_mp_" + @id, update_part_mp)
                @mount.appendChild(@mount_select.element)
                if v_part_info[@id]["fs"]? and v_part_info[@id]["fs"] not in ["fat16", "fat32", "ntfs", "swap"]
                    @mount_select.set_drop_items(__mp_keys, __mp_values)
                else 
                    @mount_select.set_drop_items(__filter_mp_keys, __filter_mp_values)
                @mount_select.set_base_background("-webkit-gradient(linear, left top, left bottom, from(rgba(133,133,133,0.6)), color-stop(0.1, rgba(255,255,255,0.6)), to(rgba(255,255,255,0.6)));")
                @mount_select.show_drop()
                @mount_select.set_selected(v_part_info[@id]["mp"])
                return
        else
            @mount_txt = create_element("div", "", @mount)
            @mount_txt.innerText = v_part_info[@id]["mp"]

    set_btn_status: ->
        if __selected_mode != "advance"
            return 
        type = v_part_info[@id]["type"]

        add_btn = document.getElementById("part_add")
        delete_btn = document.getElementById("part_delete")

        if type == "freespace"
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
        #@element.scrollIntoView()
        @active = true
        @fill_fs()
        @fill_mount()
        @set_btn_status()
        style = "background:rgba(246,178,82,0.5);"
        style += "font-style:bold;"
        style += "text-shadow:0 1px 2px rgba(0,0,0,0.7);"
        @element.setAttribute("style", style)

    blur: ->
        @active = false
        @fill_fs()
        @fill_mount()
        @element.setAttribute("style", "")

    do_click: (e)->
        if __selected_item == @ 
            echo "part table item already selected"
        else
            @focus()

class DiskTabItem extends Widget
    constructor: (@id, @disk)->
        super
        @element.innerText = v_disk_info[@disk]["path"]
        if __selected_disk == @disk
            @focus()

    do_click: (e) ->
        __selected_disk = @disk
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up("part_table")?.fill_items()
        for item in Widget.look_up("part_table")?.disktabs
            if item != @
                item.blur()
            else
                item.focus()

    blur: ->
        @element.setAttribute("style", "")

    focus: ->
        @element.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(rgba(255,255,255,0.5)), to(rgba(255,255,255, 0.2)));")

class PartTable extends Widget
    constructor: (@id)->
        super
        @disktab = create_element("div", "PartTab", @element)

        @disk_content = create_element("div", "PartContent", @element)
        @header = create_element("div", "PartTableHeader", @disk_content)
        @device_header = create_element("span", "", @header)
        @device_header.innerText = _("Device")
        @size_header = create_element("div", "", @header)
        @size_header.innerText = _("Size")
        @used_header = create_element("div", "", @header)
        @used_header.innerText = _("Freespace")
        @fs_header = create_element("div", "", @header)
        @fs_header.innerText = _("Filesystem")
        @mount_header = create_element("span", "", @header)
        if __selected_mode == "advance"
            @mount_header.innerText = _("Mount point")
        else
            @mount_header.innerText = _("Info")

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
        disk = __selected_disk
        if __selected_mode == "advance"
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
        else
            for part in m_disk_info[disk]["partitions"]
                if m_part_info[part]["type"] in ["normal", "logical"] and m_part_info[part]["op"] != "add"
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
            
    update_mode: (mode) ->
        if __selected_item?
            id = __selected_item.id
            __selected_item = null
        else 
            id = null
        if mode == "advance"
            @mount_header.innerText = _("Mount point")
            @items.setAttribute("style", "height:180px")
            @element.setAttribute("style", "top:140px;height:230px;")
        else
            @mount_header.innerText = ""
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
        @step1_desc.innerText = _("1.Set up partition")
        @step1_detail = create_element("div", "StepDetail", @step1)
        @step1_detail.innerText = _("step 1 content") 

        @step2 = create_element("p", "HelpStep", @content)
        @step2_desc = create_element("div", "StepDesc", @step2)
        @step2_desc.innerText = _("2.Set up grub")
        @step2_detail = create_element("div", "StepDetail", @step2)
        @step2_detail.innerText = _("step 2 content") 

        @step3 = create_element("p", "HelpStep", @content)
        @step3_desc = create_element("div", "StepDesc", @step3)
        @step3_desc.innerText = _("3.Click to install")
        @step3_detail = create_element("div", "StepDetail", @step3)
        @step3_detail.innerText = _("step 3 content") 

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
        @next_btn.addEventListener("click", (e) =>
            if __selected_mode == "advance" and not check_target_part()
                @root_model = new RootDialog("RootModel")
                document.body.appendChild(@root_model.element)
            else
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
        __selected_item = Widget.look_up(recommand)
        __selected_item?.focus()

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

        @part_grub = create_element("div", "PartGrub", @wrap)
        @grub_loader = create_element("div", "PartGrubLoader", @part_grub)
        @grub_loader.innerText = _("Boot loader")
        @grub_select = create_element("div", "PartGrubSelect", @part_grub)
        @fill_bootloader()

    fill_bootloader: ->
        keys = []
        values = []
        for disk in disks
            text = v_disk_info[disk]["path"] + "\t" + v_disk_info[disk]["model"] + "\t" +sector_to_gb(v_disk_info[disk]["length"], 512) + "GB"
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
        @grub_dropdown.set_list_background("url(\"images/dropdown.png\");")
        @grub_dropdown.set_list_scroll_height(true, 200)

    show_advance_mode: ->
        @t_help.style.display = "block"
        @linemap.element.setAttribute("style", "display:block")
        @op.setAttribute("style", "display:block")
        @part_grub.setAttribute("style", "display:block")

    hide_advance_mode: ->
        @t_help.style.display = "none"
        @linemap.element.setAttribute("style", "display:none")
        @op.setAttribute("style", "display:none")
        @part_grub.setAttribute("style", "display:none")

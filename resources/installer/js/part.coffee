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

create_option = (select, value, text) ->
    option = create_element("option", "", select)
    option.setAttribute("value", value)
    option.innerText = text

create_mp_option = (select, value, text) ->
    option = create_element("option", "", select)
    option.setAttribute("value", value)
    option.innerText = text
    select.addEventListener("focus", (e) =>
        if value in get_selected_mp()
            option.setAttribute("disabled", "disabled")
        else
            option.removeAttribute("disabled")
    )

fill_mp_option = (select) ->
    for opt in ["unused", "/","/boot","/home","/tmp","/usr", "/var","/srv", "/local"]
        create_mp_option(select, opt, opt)

__fs_keys = ["unused", "ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap"]

__fs_values = ["unused", "ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap"]

__mp_keys = ["unused", "/","/boot","/home","/tmp","/usr", "/var","/srv", "/local"]

__mp_values = ["unused", "/","/boot","/home","/tmp","/usr", "/var","/srv", "/local"]

class GrubDropDown extends DropDown
    constructor: (@id, @keys, @values, @on_change_cb) ->
        super(@id, @keys, @values, @on_change_cb)

class CommonDropDown extends DropDown
    constructor: (@id, @keys, @values, @on_change_cb) ->
        super(@id, @keys, @values, @on_change_cb)

class AddPartDialog extends Dialog
    constructor: (@id, @partid) ->
        super(@id, true, @add_part_cb)
        @add_css_class("DialogCommon")
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
        Widget.look_up("part")?.fill_grub()

    fill_type: ->
        @type = create_element("p", "", @content)
        @type_desc = create_element("span", "AddDesc", @type)
        @type_desc.innerText = _("type:")
        @type_value = create_element("span", "AddValue", @type)
        @type_primary = create_element("input", "", @type_value)
        @type_primary.setAttribute("type", "radio")
        @type_primary.setAttribute("name", "type")
        @primary_desc = create_element("span", "", @type_value)
        @primary_desc.innerText = _("Primary")

        @type_logical = create_element("input", "", @type_value)
        @type_logical.setAttribute("type", "radio")
        @type_logical.setAttribute("name", "type")
        @logical_desc = create_element("span", "", @type_value)
        @logical_desc.innerText = _("Logical")

        if not can_add_normal(@partid)
            @type_primary.style.display = "none"
            @primary_desc.style.display = "none"
            @type_logical.setAttribute("checked", "true")
        else
            @type_primary.setAttribute("checked", "true")

        if not can_add_logical(@partid)
            @type_logical.style.display = "none"
            @logical_desc.style.display = "none"

    fill_size: ->
        @size = create_element("p", "", @content)
        @size_desc = create_element("span", "AddDesc", @size)
        @size_desc.innerText = _("Size:")
        @max_size_mb = sector_to_mb(v_part_info[@partid]["length"], 512)
        @size_value = create_element("span", "AddValue", @size)
        @size_input = create_element("input", "", @size_value)
        @size_input.setAttribute("type", "number")
        @size_input.setAttribute("min", 1)
        @size_input.setAttribute("max", @max_size_mb)
        @size_input.setAttribute("step", 1)
        @size_input.setAttribute("value", @max_size_mb)
        @size_input.addEventListener("blur", (e) =>
            if isNaN(parseInt(@size_input.value))
                @size_input.value = @max_size_mb
            else
                if parseInt(@size_input.value) < 0
                    @size_input.value = 0
                else if parseInt(@size_input.value) > @max_size_mb
                    @size_input.value = @max_size_mb
        )
        @size_limit = create_element("span", "", @size_value)
        @size_limit.innerText = "Limited size:" + @max_size_mb
        
    fill_align: ->
        @align = create_element("p", "", @content)
        @align_desc = create_element("span", "AddDesc", @align)
        @align_desc.innerText = _("Align:")
        @align_value = create_element("span", "AddValue", @align)
        @align_start = create_element("input", "", @align_value)
        @align_start.setAttribute("type", "radio")
        @align_start.setAttribute("name", "align")
        @align_start.setAttribute("checked", "true")
        start_desc = create_element("span", "", @align_value)
        start_desc.innerText = _("Begin")
        @align_end = create_element("input", "", @align_value)
        @align_end.setAttribute("type", "radio")
        @align_end.setAttribute("name", "align")
        end_desc = create_element("span", "", @align_value)
        end_desc.innerText = _("End")

    fill_fs: ->
        @fs = create_element("p", "", @content)
        @fs_desc = create_element("span", "AddDesc", @fs)
        @fs_desc.innerText = _("Format:")
        @fs_value = create_element("span", "AddValue", @fs)
        @fs_select = create_element("select", "", @fs_value)
        for opt in ["ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap","encrypt","unused"]
            create_option(@fs_select, opt, opt)

    fill_mount: ->
        @mp = create_element("p", "", @content)
        @mp_desc = create_element("span", "AddDec", @mp)
        @mp_desc.innerText = _("Mount point:")
        @mount_value = create_element("span", "AddValue", @mp)
        @mount_select = create_element("select", "", @mount_value)
        fill_mp_option(@mount_select)

    fill_tips: ->
        @tips = create_element("p", "", @content)

    gather_info: ->
        if @type_primary.checked
            @n_type = "normal"
        else if @type_logical.checked
            @n_type = "logical"
        if parseInt(@size_input.value) == @max_size_mb
            @n_size = v_part_info[@partid]["length"]
        else
            @n_size = mb_to_sector(parseInt(@size_input.value), 512)
        if not @n_size?
            @tips.innerText = _("Please enter invalid partition size")
        if @align_start.getAttribute("checked")
            @n_align = "start"
        else
            @n_align = "end"
        @n_fs = @fs_select.options[@fs_select.selectedIndex].value
        @n_mp = @mount_select.options[@mount_select.selectedIndex].value

class DeletePartDialog extends Dialog
    constructor: (@id,@partid) ->
        super(@id, true, @delete_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Delete partition")
        @delete_tips = create_element("p", "", @content)
        @delete_tips.innerText = _("Confirm to delete?")

    delete_part_cb: ->
        remain_part = delete_part(@partid)
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(remain_part)?.focus()
        Widget.look_up("part")?.fill_grub()

class UnmountDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @unmount_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Unmount partition")
        @unmount_tips = create_element("p", "", @content)
        @unmount_tips.innerText = _("Detected partitions mounted, confirm to unmount them?")
        @add_css_class("DialogCommon")

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
        @format_tips = create_element("p", "", @content)
        @format_tips.innerText = _("Confirm to format the partition?")

    format_cb: ->
        echo "format to do install"

class RootDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @need_root_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Install tips")
        @root_tips = create_element("p", "", @content)
        @root_tips.innerText = _("Need mount a partition as root")

    need_root_cb: ->
        echo "need mount root to do install"

class InstallDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @confirm_install_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Confirm Install")
        @root_tips = create_element("p", "", @content)
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
        @disk_line = create_element("div", "", @element)
        for part in v_disk_info[__selected_disk]["partitions"]
            if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                item = new PartLineItem("line"+part)
                @disk_line.appendChild(item.element)
                if __selected_item?.id == __selected_disk
                    item.element.setAttribute("class", "PartLineItemActive")

class PartTableItem extends Widget
    constructor: (@id)->
        super
        #__selected_item = @
        @lineid = "line" + @id
        @product_part_item()

    product_part_item: ->
        @device = create_element("span", "", @element)
        @size = create_element("div", "", @element)
        @used = create_element("div", "", @element)
        @fs = create_element("div", "", @element)
        @mount = create_element("span", "", @element)
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
                if v_part_info[@id]["label"]?
                    @label.innerText = v_part_info[@id]["label"]
                @path.innerText = v_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
        else if __selected_mode == "simple"
            if m_part_info[@id]["type"] != "freespace"
                if m_part_info[@id]["label"]?
                    @label.innerText = m_part_info[@id]["label"]
                @path.innerText = m_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
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
            if os.toLowerCase().indexOf("linux") != -1
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

    #fill_fs: ->
    #    @fs.innerHTML = ""
    #    if __selected_mode == "simple" 
    #        if m_part_info[@id]["type"] != "freespace"
    #            @fs_txt = create_element("div", "", @fs)
    #            @fs_txt.innerText = m_part_info[@id]["fs"]
    #    else if __selected_mode == "advance"
    #        if v_part_info[@id]["type"] != "freespace"
    #            @fs_select = create_element("select", "", @fs)
    #            for opt in ["unused", "ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap","encrypt"]
    #                create_option(@fs_select, opt, opt)
    #            for opt, i in @fs_select
    #                if opt.value == v_part_info[@id]["fs"]
    #                    @fs_select.selectedIndex = i
    #            @fs_select.addEventListener("focus", (e) =>
    #                if __selected_item != @
    #                    @focus()
    #            )
    #            @fs_select.addEventListener("change", (e) =>
    #                update_part_fs(@id, @fs_select.options[@fs_select.selectedIndex].value)
    #            )
    #    else
    #        echo "fill fs:invalid mode"
    fill_fs: ->
        @fs.innerHTML = ""
        if __selected_mode == "simple" 
            if m_part_info[@id]["type"] != "freespace"
                @fs_txt = create_element("div", "", @fs)
                @fs_txt.innerText = m_part_info[@id]["fs"]
        else if __selected_mode == "advance"
            if v_part_info[@id]["type"] != "freespace"
                @fs_select = new CommonDropDown("dd_fs_" + @id, __fs_keys, __fs_values, update_part_fs)
                @fs.appendChild(@fs_select.element)
                @fs_select.set_selected(v_part_info[@id]["fs"])

    #fill_mount: ->
    #    @mount.innerHTML = ""
    #    if __selected_mode != "advance" 
    #        return
    #    if v_part_info[@id]["type"] == "freespace"
    #        return
    #    @mount_select = create_element("select", "", @mount)
    #    fill_mp_option(@mount_select)
    #    for opt, i in @mount_select
    #        if opt.value == v_part_info[@id]["mp"]
    #            @mount_select.selectedIndex = i
    #    @mount_select.addEventListener("focus", (e) =>
    #        if __selected_item != @
    #            @focus()
    #    )
    #    @mount_select.addEventListener("change", (e) =>
    #        update_part_mp(@id, @mount_select.options[@mount_select.selectedIndex].value)
    #    )
    fill_mount: ->
        @mount.innerHTML = ""
        if __selected_mode != "advance" 
            return
        if v_part_info[@id]["type"] == "freespace"
            return
        @mount_select = new CommonDropDown("dd_mp_" + @id, __mp_keys, __mp_values, update_part_mp)
        @mount.appendChild(@mount_select.element)
        @mount_select.set_selected(v_part_info[@id]["mp"])

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
        @element.scrollIntoView()
        @set_btn_status()
        @element.setAttribute("style", "background:#27BEFF")
        #@update_install_btn()

    blur: ->
        @element.setAttribute("style", "")

    update_install_btn: ->
        if __selected_mode == "advance"
            if v_part_info[@id]["label"]? and v_part_info[@id]["label"].length > 0
                txt = v_part_info[@id]["label"]
            else
                txt = v_part_info[@id]["path"]
            install_txt = "Install to " + txt
            Widget.look_up("part")?.update_next_btn(install_txt)
        else
            if m_part_info[@id]["label"]? and m_part_info[@id]["label"].length > 0
                txt = m_part_info[@id]["label"]
            else
                txt = m_part_info[@id]["path"]
            install_txt = "Install to " + txt
            Widget.look_up("part")?.update_next_btn(install_txt)

    do_click: (e)->
        if __selected_item == @ 
            echo "part table item already selected"
        else
            @focus()

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
        for disk in disks
            @fill_disk_tab_item(disk)
        @fill_items()

    fill_disk_tab_item: (disk) ->
        disktab = create_element("div", "", @disktab)
        disktab.innerText = v_disk_info[disk]["path"]
        disktab.addEventListener("click", (e) =>
            __selected_disk = disk
            Widget.look_up("part_line_maps")?.fill_linemap()
            @fill_items()
        )

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
                if m_part_info[part]["type"] in ["normal", "logical", "freespace"] and m_part_info[part]["op"] != "add"
                    if m_part_info[part]["type"] == "freespace"
                        echo "create freespace item"
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
            
    update_mode: (mode) ->
        if __selected_item?
            id = __selected_item.id
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

class Part extends Page
    constructor: (@id)->
        super
        @part_txt = create_element("p", "", @title)
        @part_txt.innerText = _("Choose partition")

        @help = create_element("div", "TitleSet", @title)
        @t_mode = create_element("span", "", @help)
        @t_mode.innerText = _("Expert mode")
        @t_mode.addEventListener("click", (e) =>
            @switch_mode()
        )
        @t_help = create_element("span", "", @help)
        @t_help.innerText = _("Help")
        @t_help.addEventListener("click", (e) =>
            @toggle_show_help()
        )

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        @next_btn = create_element("div", "NextStep", @element)
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
        @element.appendChild(@linemap.element)
        @table = new PartTable("part_table")
        @element.appendChild(@table.element)
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
            @add_model?.hide_dialog()
            @delete_model?.hide_dialog()
            @unmount_model?.hide_dialog()
            @hide_advance_mode()
            @table.update_mode(__selected_mode)
            @t_mode.innerText = _("Expert mode") 

    update_next_btn: (txt) ->
        @next_btn.innerText = txt
        left = "left:" + (755 - document.getElementById("mynextstep")?.offsetWidth) / 2
        @next_btn.setAttribute("style", left)

    fill_advance_op: ->
        #part op buttons
        @op = create_element("div", "PartOp", @element)
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

        @part_grub = create_element("div", "PartGrub", @element)
        @grub_loader = create_element("div", "PartGrubLoader", @part_grub)
        @grub_loader.innerText = _("Boot loader")
        #@part_grub.innerHTML = "<span>" + _("Boot loader") + "</span>"
        #@grub_select = create_element("select", "", @part_grub)
        @grub_select = create_element("div", "PartGrubSelect", @part_grub)
        @fill_bootloader()
        #@fill_grub()

    fill_grub: ->
        @grub_select.innerHTML = ""
        for disk in disks
            text = v_disk_info[disk]["path"] + "\t" + v_disk_info[disk]["model"] + "\t" +sector_to_gb(v_disk_info[disk]["length"], 512) + "GB"
            create_option(@grub_select, disk, text)
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical"]
                    create_option(@grub_select, part, v_part_info[part]["path"])

    fill_bootloader: ->
        echo "fill bootloader"
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

        @grub_dropdown = new GrubDropDown("dd_bt_", keys, values, null)
        @grub_select.appendChild(@grub_dropdown.element)
        @grub_dropdown.set_selected(__selected_disk)

    show_advance_mode: ->
        @linemap.element.setAttribute("style", "display:block")
        @op.setAttribute("style", "display:block")
        @part_grub.setAttribute("style", "display:block")

    hide_advance_mode: ->
        @linemap.element.setAttribute("style", "display:none")
        @op.setAttribute("style", "display:none")
        @part_grub.setAttribute("style", "display:none")

    toggle_show_help: ->
        if DCore.Installer.is_help_running()
            DCore.Installer.hide_help()
        else
            DCore.Installer.show_help()

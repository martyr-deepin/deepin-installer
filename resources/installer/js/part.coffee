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

__board = create_element("div", "Board", "")
__board.setAttribute("id", "board")
document.body.appendChild(__board)

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

class Dialog extends Widget
    constructor: (@id, @cb) ->
        super
        @title = create_element("p", "DialogTitle", @element)
        @title_txt = create_element("div", "DialogTxt", @title)
        @title_close = create_element("div", "DialogClose", @title)
        @title_close.addEventListener("click", (e) =>
            @hide_dialog()
        )

        @content = create_element("div", "DialogContent", @element)
        @foot = create_element("p", "DialogBtn", @element)
        @ok = create_element("span", "", @foot)
        @ok.innerText = "OK"
        @ok.addEventListener("click", (e) =>
            @cb()
            @hide_dialog()
        )
        @cancel = create_element("span", "", @foot)
        @cancel.innerText = "Cancel"
        @cancel.addEventListener("click", (e) =>
            @hide_dialog()
        )
        @show_dialog()
        #@element.setAttribute("draggable", "true")

    show_dialog: ->
        __in_model = true
        __board.setAttribute("style", "display:block")

    hide_dialog: ->
        __in_model = false
        @destroy()
        __board.setAttribute("style", "display:none")

        #do_dragstart: (event) ->
        #    echo "do dragstart in dialog"
        #    event.dataTransfer.setData("Text", event.target.id)

        #do_drop: (event) ->
        #    echo "do drop in dialog"
        #    event.preventDefault()
        #    data = event.dataTransfer.getData("Text")
        #    style = "top:"+ event.y + "px;" + "left:" + event.x + "px"
        #    event.target.setAttribute("style", style)

        #do_dragover: (event) ->
        #    echo "do dragover in dialog"
        #    event.preventDefault()

class AddPartDialog extends Dialog
    constructor: (@id, @partid) ->
        super(@id, @add_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = "新建分区"
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
        @type_desc.innerText = "类型:"
        @type_value = create_element("span", "AddValue", @type)
        @type_primary = create_element("input", "", @type_value)
        @type_primary.setAttribute("type", "radio")
        @type_primary.setAttribute("name", "type")
        primary_desc = create_element("span", "", @type_value)
        primary_desc.innerText = "Primary"

        @type_logical = create_element("input", "", @type_value)
        @type_logical.setAttribute("type", "radio")
        @type_logical.setAttribute("name", "type")
        logical_desc = create_element("span", "", @type_value)
        logical_desc.innerText = "Logical"

        if not can_add_logical(@partid)
            @type_logical.setAttribute("disabled", "disabled")
        else
            @type_logical.setAttribute("checked", "true")

        if not can_add_normal(@partid)
            @type_primary.setAttribute("disabled", "disabled")
        else
            @type_primary.setAttribute("checked", "true")

    fill_size: ->
        @size = create_element("p", "", @content)
        @size_desc = create_element("span", "AddDesc", @size)
        @size_desc.innerText = "大小:"
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
        @align_desc.innerText = "位置:"
        @align_value = create_element("span", "AddValue", @align)
        @align_start = create_element("input", "", @align_value)
        @align_start.setAttribute("type", "radio")
        @align_start.setAttribute("name", "align")
        @align_start.setAttribute("checked", "true")
        start_desc = create_element("span", "", @align_value)
        start_desc.innerText = "Start"
        @align_end = create_element("input", "", @align_value)
        @align_end.setAttribute("type", "radio")
        @align_end.setAttribute("name", "align")
        end_desc = create_element("span", "", @align_value)
        end_desc.innerText = "End"

    fill_fs: ->
        @fs = create_element("p", "", @content)
        @fs_desc = create_element("span", "AddDesc", @fs)
        @fs_desc.innerText = "格式:"
        @fs_value = create_element("span", "AddValue", @fs)
        @fs_select = create_element("select", "", @fs_value)
        for opt in ["ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap","encrypt","unused"]
            create_option(@fs_select, opt, opt)

    fill_mount: ->
        @mp = create_element("p", "", @content)
        @mp_desc = create_element("span", "AddDec", @mp)
        @mp_desc.innerText = "挂载:"
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
            @tips.innerText = "请输入合法的分区大小"
        if @align_start.getAttribute("checked")
            @n_align = "start"
        else
            @n_align = "end"
        @n_fs = @fs_select.options[@fs_select.selectedIndex].value
        @n_mp = @mount_select.options[@mount_select.selectedIndex].value

class DeletePartDialog extends Dialog
    constructor: (@id, @partid) ->
        super(@id, @delete_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = "删除分区"
        @delete_tips = create_element("p", "", @content)
        @delete_tips.innerText = "确定删除分区吗?"

    delete_part_cb: ->
        delete_part(@partid)
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(@partid)?.focus()
        Widget.look_up("part")?.fill_grub()

class UnmountDialog extends Dialog
    constructor: (@id) ->
        super(@id, @unmount_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = "卸载分区"
        @unmount_tips = create_element("p", "", @content)
        @unmount_tips.innerText = " 检测到有磁盘已经挂载，操作对应分区需要卸载,确定卸载吗?"
        @add_css_class("DialogCommon")

    unmount_cb: ->
        echo "unmount partitions"

class FormatDialog extends Dialog
    constructor: (@id) ->
        super(@id, @format_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = "安装执行"
        @format_tips = create_element("p", "", @content)
        @format_tips.innerText = "确定执行分区操作并安装LinuxDeepin吗?"

    format_cb: ->
        echo "format to do install"

class RootDialog extends Dialog
    constructor: (@id) ->
        super(@id, @need_root_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = "信息提示"
        @root_tips = create_element("p", "", @content)
        @root_tips.innerText = "安装执行需要挂载/分区"

    need_root_cb: ->
        echo "need mount root to do install"

class PartLineItem extends Widget
    constructor: (@id) ->
        super
        @part = @id[4...16]
        @init_line_item()

    init_line_item: ->
        if __selected_mode == "advance"
            @color = v_part_info[@part]["color"]
            @element.style.background = @color
            @element.style.width = v_part_info[@part]["width"]
        else
            @color = m_part_info[@part]["color"]
            @element.style.background = @color
            @element.style.width = m_part_info[@part]["width"]

    focus: ->
        __selected_line?.blur()
        __selected_line = @
        try
            if __selected_item == null or __selected_line.id != __selected_item?.lineid?
                Widget.look_up(@part)?.passive_focus()
        catch error
            echo error
        @add_css_class("PartLineItemActive")
    
    passive_focus: ->
        __selected_line?.blur()
        __selected_line = @
        @add_css_class("PartLineItemActive")

    blur: ->
        @element.setAttribute("class", "PartLineItem")

    do_click: (e)->
        if __selected_line == @ 
            echo "already selected"
        else
            @focus()

class PartLineMaps extends Widget
    constructor: (@id)->
        super
        @fill_linemap()

    fill_linemap: ->
        @element.innerHTML = ""
        @disk_line = create_element("div", "", @element)
        if __selected_mode == "advance"
            @fill_advance_linemap()
        else
            @fill_simple_linemap()

    fill_advance_linemap: ->
        for part in v_disk_info[__selected_disk]["partitions"]
            if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                item = new PartLineItem("line"+part)
                @disk_line.appendChild(item.element)
                if __selected_item?.id == __selected_disk
                    item.element.setAttribute("class", "PartLineItemActive")

    fill_simple_linemap: ->
        for part in m_disk_info[__selected_disk]["partitions"]
            if m_part_info[part]["type"] in ["normal", "logical", "freespace"] and m_part_info[part]["op"] != "add"
                item = new PartLineItem("line"+part)
                @disk_line.appendChild(item.element)
                if __selected_item?.id == __selected_disk
                    item.element.setAttribute("class", "PartLineItemActive")

class PartTableItem extends Widget
    constructor: (@id, @device_type)->
        super
        __selected_item = @
        if @device_type == "part"
            @lineid = "line" + @id
            @product_part_item()
        else if @device_type == "disk"
            @product_disk_item()
        else
            echo "invalid device type"

    product_part_item: ->
        @device = create_element("span", "", @element)
        @size = create_element("div", "", @element)
        @used = create_element("div", "", @element)
        @fs = create_element("div", "", @element)
        @mount = create_element("div", "", @element)
        @fill_device()
        @fill_size()
        @fill_used()
        @fill_fs()
        @fill_mount()

    product_disk_item: ->
        @device = create_element("div","", @element)
        @fill_disk_item()

    fill_disk_item: ->
        @device.innerHTML = ""
        if __selected_mode == "advance"
            @device.innerText = v_disk_info[@id]["path"]
        else
            @device.innerText = m_disk_info[@id]["path"]

    fill_device: ->
        @device.innerHTML = ""
        @os = create_element("span", "Os", @device)
        @color = create_element("span", "Color", @device)
        color_value = Widget.look_up(@lineid)?.color or get_random_color()
        @color.style.background = color_value
        @lp = create_element("span", "LabelPath", @device)
        @label = create_element("div", "Label", @lp)
        @path = create_element("div", "Path", @lp)

        if __selected_mode == "advance"
            if v_part_info[@id]["type"] != "freespace"
                if v_part_info[@id]["os"]? 
                    if v_part_info[@id]["os"].toLowerCase().indexOf("linux") != -1
                        os_img = "images/linux.png"
                    else if v_part_info[@id]["os"].toLowerCase().indexOf("windows") != -1
                        os_img = "images/windows.png"
                    else if v_part_info[@id]["os"].toLowerCase().indexOf("mac") != -1
                        os_img = "images/apple.png"
                    create_img("osimg", os_img, @os)
                if v_part_info[@id]["label"]?
                    @label.innerText = v_part_info[@id]["label"]
                @path.innerText = v_part_info[@id]["path"]
            else
                @path.innerText = "freespace"
        else if __selected_mode == "simple"
            if m_part_info[@id]["type"] != "freespace"
                if m_part_info[@id]["os"]? 
                    if m_part_info[@id]["os"].toLowerCase().indexOf("linux") != -1
                        os_img = "images/linux.png"
                    else if m_part_info[@id]["os"].toLowerCase().indexOf("windows") != -1
                        os_img = "images/windows.png"
                    else if m_part_info[@id]["os"].toLowerCase().indexOf("mac") != -1
                        os_img = "images/apple.png"
                    create_img("osimg", os_img, @os)
                if m_part_info[@id]["label"]?
                    @label.innerText = m_part_info[@id]["label"]
                @path.innerText = m_part_info[@id]["path"]
            else
                @path.innerText = "freespace"

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
            if m_part_info[@id]["type"] != "freespace"
                @fs_txt = create_element("div", "", @fs)
                @fs_txt.innerText = m_part_info[@id]["fs"]
        else if __selected_mode == "advance"
            if v_part_info[@id]["type"] != "freespace"
                @fs_select = create_element("select", "", @fs)
                for opt in ["unused", "ext4","ext3","ext2","reiserfs","btrfs","jfs","xfs","fat16","fat32","ntfs","swap","encrypt"]
                    create_option(@fs_select, opt, opt)
                for opt, i in @fs_select
                    if opt.value == v_part_info[@id]["fs"]
                        @fs_select.selectedIndex = i
                @fs_select.addEventListener("focus", (e) =>
                    if __selected_item != @
                        @focus()
                )
                @fs_select.addEventListener("change", (e) =>
                    update_part_fs(@id, @fs_select.options[@fs_select.selectedIndex].value)
                )
        else
            echo "fill fs:invalid mode"

    fill_mount: ->
        @mount.innerHTML = ""
        if __selected_mode != "advance" 
            return
        if v_part_info[@id]["type"] == "freespace"
            return
        @mount_select = create_element("select", "", @mount)
        fill_mp_option(@mount_select)
        for opt, i in @mount_select
            if opt.value == v_part_info[@id]["mp"]
                @mount_select.selectedIndex = i
        @mount_select.addEventListener("focus", (e) =>
            if __selected_item != @
                @focus()
        )
        @mount_select.addEventListener("change", (e) =>
            update_part_mp(@id, @mount_select.options[@mount_select.selectedIndex].value)
        )

    set_btn_status: ->
        if __selected_mode != "advance"
            return 
        if @device_type == "part"
            type = v_part_info[@id]["type"]
        else
            type = "disk"

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
        __selected_item?.blur()
        __selected_item = @

        if @device_type == "disk"
            __selected_disk = @id
        else
            if __selected_mode == "advance"
                __selected_disk = v_part_info[@id]["disk"]
            else
                __selected_disk = m_part_info[@id]["disk"]
        try
            Widget.look_up("part_line_maps")?.fill_linemap()
            Widget.look_up(@lineid)?.passive_focus()
        catch error
            echo error

        @set_btn_status()
        @element.setAttribute("style", "background:#27BEFF")
        @update_install_btn()

    passive_focus: ->
        __selected_item?.blur()
        __selected_item = @

        @set_btn_status()
        @element.setAttribute("style", "background:#27BEFF")
        @update_install_btn()

    blur: ->
        @element.setAttribute("style", "")

    update_install_btn: ->
        if __selected_mode == "advance"
            if @device_type == "disk"
                txt = v_disk_info[@id]["path"]
            else
                if v_part_info[@id]["label"]?
                    txt = v_part_info[@id]["label"]
                else
                    txt = v_part_info[@id]["path"]
            install_txt = "Install to " + txt
            Widget.look_up("part")?.update_next_btn(install_txt)
        else
            if @device_type == "disk"
                txt = m_disk_info[@id]["path"]
            else
                if m_part_info[@id]["label"]?
                    txt = m_part_info[@id]["label"]
                else
                    txt = m_part_info[@id]["path"]
            install_txt = "Install to " + txt
            Widget.look_up("part")?.update_next_btn(install_txt)

    do_click: (e)->
        if __selected_item == @ 
            echo "already selected"
        else
            @focus()

class PartTable extends Widget
    constructor: (@id)->
        super
        @header = create_element("div", "PartTableHeader", @element)
        @device_header = create_element("span", "", @header)
        @device_header.innerText = "Device"
        @size_header = create_element("div", "", @header)
        @size_header.innerText = "Size"
        @used_header = create_element("div", "", @header)
        @used_header.innerText = "Free"
        @fs_header = create_element("div", "", @header)
        @fs_header.innerText = "FileSystem"
        @mount_header = create_element("div", "", @header)
        if __selected_mode == "advance"
            @mount_header.innerText = "MountPoint"
        else
            @mount_header.innerText = ""
        @items = create_element("div", "PartTableItems", @element)
        @fill_items()

    fill_items: ->
        @items.innerHTML = ""
        for disk in disks
            item = new PartTableItem(disk, "disk")
            @items.appendChild(item.element)
            if __selected_mode == "advance"
                for part in v_disk_info[disk]["partitions"]
                    if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                        item = new PartTableItem(part, "part")
                        @items.appendChild(item.element)
            else
                for part in m_disk_info[disk]["partitions"]
                    if m_part_info[part]["type"] in ["normal", "logical", "freespace"] and m_part_info[part]["op"] != "add"
                        if m_part_info[part]["type"] == "freespace"
                            echo "create freespace item"
                        item = new PartTableItem(part, "part")
                        @items.appendChild(item.element)
            
    update_mode: (mode) ->
        if mode == "advance"
            @mount_header.innerText = "MountPoint"
            @items.setAttribute("style", "height:210px")
        else
            @mount_header.innerText = ""
            @items.setAttribute("style", "")
        @fill_items()

class Part extends Page
    constructor: (@id)->
        super
        if __selected_mode == null
            __selected_mode = "simple"

        if __selected_disk == null
            __selected_disk = disks[0]

        @part_txt = create_element("p", "", @title)
        @part_txt.innerText = "选择磁盘"

        @help = create_element("div", "PartHelp", @title)
        @t_mode = create_element("span", "", @help)
        @t_mode.innerText = "高级模式"
        @t_mode.addEventListener("click", (e) =>
            if __selected_mode != "advance"
                __selected_mode = "advance"
                if check_has_mount()
                    @unmount_model = new UnmountDialog("UnmountModel")
                    document.body.appendChild(@unmount_model.element)
                @show_advance_op()
                @table.update_mode(__selected_mode)
                @t_mode.innerText = "精简模式"
            else
                __selected_mode = "simple"
                @add_model?.hide_dialog()
                @delete_model?.hide_dialog()
                @unmount_model?.hide_dialog()
                @hide_advance_op()
                @table.update_mode(__selected_mode)
                @t_mode.innerText ="高级模式" 
        )
        @t_sep = create_element("span", "", @help)
        @t_sep.innerText = "  |  "
        @t_help = create_element("span", "", @help)
        @t_help.innerText = "查看帮助"
        @t_help.addEventListener("click", (e) =>
            @toggle_show_help()
        )

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        #linemap
        @linemap = new PartLineMaps("part_line_maps")
        @element.appendChild(@linemap.element)

        #part table
        @table = new PartTable("part_table")
        @element.appendChild(@table.element)

        if __selected_item == null
            __selected_item = Widget.look_up(@table.items_part?[0])?

        @fill_advance_op()
        if __selected_mode == "advance"
            @show_advance_op()
        else
            @hide_advance_op()

        @next_btn = create_element("div", "NextStep", @element)
        @next_btn.setAttribute("id", "mynextstep")
        @next_btn.innerText = "开始安装"
        @next_btn.addEventListener("click", (e) =>
            if __selected_mode == "simple"
                @handle_simple_install()
            else if __selected_mode == "advance"
                @handle_advance_install()
        )

    update_next_btn: (txt) ->
        @next_btn.innerText = txt
        left = "left:" + (755 - document.getElementById("mynextstep")?.offsetWidth) / 2
        @next_btn.setAttribute("style", left)

    handle_simple_install: ->
        do_simple_partition(__selected_item.id, __selected_item.device_type)
        DCore.Installer.hide_help()
        __selected_grub = __selected_disk
        pc.add_page(progress_page)
        pc.remove_page(part_page)
        progress_page.handle_extract("start")
        __selected_stage = "extract"

    handle_advance_install: ->
        if check_target_part()
            do_partition()
            DCore.Installer.hide_help()
            __selected_grub = @grub_select.options[@grub_select.selectedIndex].value
            pc.add_page(progress_page)
            pc.remove_page(part_page)
            progress_page.handle_extract("start")
            __selected_stage = "extract"
        else
            @root_model = new RootDialog("RootModel")
            document.body.appendChild(@root_model.element)

    fill_advance_op: ->
        #part op buttons
        @op = create_element("p", "PartOp", @element)
        @part_add = create_element("div", "PartBtn", @op)
        @part_add.setAttribute("id", "part_add")
        @part_add.innerText = "新建分区"
        @part_add.addEventListener("click", (e)=>
            echo "handle add"
            if __in_model
                echo "already had add part mode dialog"
                return 
            @add_model = new AddPartDialog("AddModel", __selected_item.id)
            document.body.appendChild(@add_model.element)
        )

        @part_delete = create_element("div", "PartBtn", @op)
        @part_delete.setAttribute("id", "part_delete")
        @part_delete.innerText = "删除分区"
        @part_delete.addEventListener("click", (e)=>
            echo "handle delete"
            if __in_model
                echo "already had delete part mode dialog"
                return 
            @del_model = new DeletePartDialog("DeleteModel", __selected_item.id)
            document.body.appendChild(@del_model.element)
        )

        @part_grub = create_element("p", "PartGrub", @element)
        @part_grub.innerHTML = "<span>安装启动引导器的设备：</span>"
        @grub_select = create_element("select", "", @part_grub)
        @fill_grub()

    fill_grub: ->
        @grub_select.innerHTML = ""
        for disk in disks
            text = v_disk_info[disk]["path"] + "\t" + v_disk_info[disk]["model"] + "\t" +sector_to_gb(v_disk_info[disk]["length"], 512) + "GB"
            create_option(@grub_select, disk, text)
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical"]
                    create_option(@grub_select, part, v_part_info[part]["path"])

    show_advance_op: ->
        @op.setAttribute("style", "display:block")
        @part_grub.setAttribute("style", "display:block")

    hide_advance_op: ->
        @op.setAttribute("style", "display:none")
        @part_grub.setAttribute("style", "display:none")

    toggle_show_help: ->
        if DCore.Installer.is_help_running()
            DCore.Installer.hide_help()
        else
            DCore.Installer.show_help()

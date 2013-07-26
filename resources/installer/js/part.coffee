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

__selected_disk = null
__selected_item = null
__selected_line = null

class ModelDialog extends Widget
    constructor: (@id, @partid)->
        super
        @wrap = create_element("div", "ModelDialogWrap", @element)
        @close = create_element("span", "Close", @wrap)
        @close.innerText = "X" 
        @close.addEventListener("click", (e) =>
            @hide_dialog()
        )
        @desc = create_element("p", "", @wrap)
        @desc.innerText = "新建分区"
        @fill_type()
        @fill_size()
        @fill_align()
        @fill_fs()
        @fill_mount()
        @fill_tips()
        @fill_btn()

    fill_type: ->
        @type = create_element("p", "", @wrap)
        @type_desc = create_element("span", "", @type)
        @type.innerText = "请选择新建分区类型"
        @type_primary = create_element("input", "", @type)
        @type_primary.setAttribute("type", "radio")
        @type_primary.setAttribute("name", "type")
        primary_desc = create_element("span", "", @type)
        primary_desc.innerText = "Primary"

        @type_logical = create_element("input", "", @type)
        @type_logical.setAttribute("type", "radio")
        @type_logical.setAttribute("name", "type")
        logical_desc = create_element("span", "", @type)
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
        @size = create_element("p", "", @wrap)
        @size_desc = create_element("span", "", @size)
        @size_desc.innerText = "新建分区大小"
        @size_input = create_element("input", "", @size)
        @size_input.setAttribute("type", "number")
        @size_input.setAttribute("min", 1)
        @size_input.setAttribute("max", v_part_info[@partid]["length"])
        @size_input.setAttribute("step", 1)
        @size_input.setAttribute("value", v_part_info[@partid]["length"])
        @size_input.addEventListener("blur", (e) =>
            if isNaN(parseInt(@size_input.value))
                @size_input.value = v_part_info[@partid]["length"]
            else
                if parseInt(@size_input.value) < 0
                    @size_input.value = 0
                else if parseInt(@size_input.value) > v_part_info[@partid]["length"]
                    @size_input.value = v_part_info[@partid]["length"]
        )
        @size_limit = create_element("span", "", @size)
        @size_limit.innerText = "Limited size:" + v_part_info[@partid]["length"]

    fill_align: ->
        @align = create_element("p", "", @wrap)
        @align_desc = create_element("span", "", @align)
        @align_desc.innerText = "请选择分区对齐方式"
        @align_start = create_element("input", "", @align)
        @align_start.setAttribute("type", "radio")
        @align_start.setAttribute("name", "align")
        @align_start.setAttribute("checked", "true")
        start_desc = create_element("span", "", @align)
        start_desc.innerText = "Start"
        @align_end = create_element("input", "", @align)
        @align_end.setAttribute("type", "radio")
        @align_end.setAttribute("name", "align")
        end_desc = create_element("span", "", @align)
        end_desc.innerText = "End"

    fill_fs: ->
        @fs = create_element("p", "", @wrap)
        @fs_desc = create_element("span", "", @fs)
        @fs_desc.innerText = "请选择分区文件系统"
        @fs_select = create_element("select", "", @fs)
        @ext4_option = create_element("option", "", @fs_select)
        @ext4_option.setAttribute("value", "ext4")
        @ext4_option.setAttribute("selected", "selected")
        @ext4_option.innerText = "ext4"
        @ext3_option = create_element("option", "", @fs_select)
        @ext3_option.setAttribute("value", "ext3")
        @ext3_option.innerText = "ext3"
        @ext2_option = create_element("option", "", @fs_select)
        @ext2_option.setAttribute("value", "ext2")
        @ext2_option.innerText = "ext2"
        @reiserfs_option = create_element("option", "", @fs_select)
        @reiserfs_option.setAttribute("value", "reiserfs")
        @reiserfs_option.innerText = "reiserfs"
        @btrfs_option = create_element("option", "", @fs_select)
        @btrfs_option.setAttribute("value", "btrfs")
        @btrfs_option.innerText = "btrfs"
        @jfs_option = create_element("option", "", @fs_select)
        @jfs_option.setAttribute("value", "jfs")
        @jfs_option.innerText = "jfs"
        @xfs_option = create_element("option", "", @fs_select)
        @xfs_option.setAttribute("value", "xfs")
        @xfs_option.innerText = "xfs"
        @fat16_option = create_element("option", "", @fs_select)
        @fat16_option.setAttribute("value", "fat16")
        @fat16_option.innerText = "fat16"
        @fat32_option = create_element("option", "", @fs_select)
        @fat32_option.setAttribute("value", "fat32")
        @fat32_option.innerText = "fat32"
        @swap_option = create_element("option", "", @fs_select)
        @swap_option.setAttribute("value", "swap")
        @swap_option.innerText = "swap"
        @encrypt_option = create_element("option", "", @fs_select)
        @encrypt_option.setAttribute("value", "encrypt")
        @encrypt_option.innerText = "encrypt"
        @unused_option = create_element("option", "", @fs_select)
        @unused_option.setAttribute("value", "unused")
        @unused_option.innerText = "unused"

    fill_mount: ->
        @mp = create_element("p", "", @wrap)
        @mp_desc = create_element("span", "", @mp)
        @mp_desc.innerText = "请选择分区挂载点"
        @mount_select = create_element("select", "", @mp)
        @root_option = create_element("option", "", @mount_select)
        @root_option.setAttribute("value", "/")
        @root_option.setAttribute("selected", "selected")
        @root_option.innerText = "/"
        @boot_option = create_element("option", "", @mount_select)
        @boot_option.setAttribute("value", "/boot")
        @boot_option.innerText = "/boot"
        @home_option = create_element("option", "", @mount_select)
        @home_option.setAttribute("value", "/home")
        @home_option.innerText = "/home"
        @tmp_option = create_element("option", "", @mount_select)
        @tmp_option.setAttribute("value", "/tmp")
        @tmp_option.innerText = "/tmp"
        @usr_option = create_element("option", "", @mount_select)
        @usr_option.setAttribute("value", "/usr")
        @usr_option.innerText = "/usr"
        @var_option = create_element("option", "", @mount_select)
        @var_option.setAttribute("value", "/var")
        @var_option.innerText = "/var"
        @srv_option = create_element("option", "", @mount_select)
        @srv_option.setAttribute("value", "/srv")
        @srv_option.innerText = "/srv"
        @opt_option = create_element("option", "", @mount_select)
        @opt_option.setAttribute("value", "/opt")
        @opt_option.innerText = "/opt"
        @local_option = create_element("option", "", @mount_select)
        @local_option.setAttribute("value", "/local")
        @local_option.innerText = "/local"

    fill_tips: ->
        @tips = create_element("p", "", @wrap)

    fill_btn: ->
        @op_btn = create_element("p", "", @wrap)
        @ok = create_element("input", "", @op_btn)
        @ok.setAttribute("type", "button")
        @ok.setAttribute("value", "Ok")

        @cancel = create_element("input", "", @op_btn)
        @cancel.setAttribute("type", "button")
        @cancel.setAttribute("value", "Cancel")
        @cancel.addEventListener("click", (e) =>
            echo "cancel add partition"
            @hide_dialog()
        )

    show_dialog: ->
        echo "show dialog"
        @add_css_class("ModelDialogShow")

    gather_info: ->
        if @type_primary.checked
            @n_type = "normal"
        else if @type_logical.checked
            @n_type = "logical"

        @n_size = parseInt(@size_input.value)
        if not @n_size?
            @tips.innerText = "请输入合法的分区大小"

        if @align_start.getAttribute("checked")
            @n_align = "start"
        else
            @n_align = "end"

        @n_fs = @fs_select.options[@fs_select.selectedIndex].value

        @n_mp = @mount_select.options[@mount_select.selectedIndex].value

    hide_dialog: ->
        echo "hide dialog"
        @element.setAttribute("class", "ModelDialog")
        @destroy()

class PartLineItem extends Widget
    constructor: (@id) ->
        super
        @part = @id[4...16]
        @update_item()

    focus: ->
        __selected_line?.blur()
        __selected_line = @
        try
            if __selected_item == null or __selected_line.id != __selected_item?.lineid?
                Widget.look_up(@part)?.passive_focus()
        catch error
            echo error
        @add_css_class("PartLineItemSelected")
    
    passive_focus: ->
        __selected_line?.blur()
        __selected_line = @
        @add_css_class("PartLineItemSelected")

    blur: ->
        @element.setAttribute("class", "PartLineItem")

    do_click: (e)->
        if __selected_line == @ 
            echo "already selected"
        else
            @focus()

    update_item: ->
        @color = v_part_info[@part]["color"]
        @element.style.background = @color
        @element.innerText = v_part_info[@part]["path"]
        @element.innerHTML += "</br>"
        @element.innerText += v_part_info[@part]["type"]
        @element.style.width = v_part_info[@part]["width"]
        if v_part_info[@part]["disk"] != __selected_disk
            @element.style.display = "none"
        else
            @element.style.display = "inline"

class PartLineMaps extends Widget
    constructor: (@id)->
        super
        @items_part =[]
        @fill_linemap()

    fill_linemap: ->
        for part in @items_part
            Widget.look_up("line"+part)?.destroy()

        @items_part = []
        for disk in disks
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                    item = new PartLineItem("line"+part)
                    @element.appendChild(item.element)
                    @items_part.push(part)

class PartTableItem extends Widget
    constructor: (@id)->
        super
        @mode = "advance"
        @lineid = "line"+@id
        __selected_item = @

        @device = create_element("div", "PartTableItemCell", @element)
        @color = create_element("div", "Color", @device)
        @path = create_element("div", "Path", @device)

        @fs = create_element("div", "PartTableItemCell", @element)
        @fs_txt = create_element("div", "ItemCellTxt", @fs)
        @fs_select = create_element("select", "", @fs)
        @fs_select.addEventListener("focus", (e) =>
            if __selected_item != @
                @focus()
        )

        @mount = create_element("div", "PartTableItemCell", @element)
        @mount_txt = create_element("div", "ItemCellTxt", @mount)
        @mount_select = create_element("select", "", @mount)
        @mount_select.addEventListener("focus", (e) =>
            if __selected_item != @
                @focus()
        )
        
        @size = create_element("div", "PartTableItemCell", @element)

        @used = create_element("div", "PartTableItemCell", @element)

        @fill_item()

    fill_device: ->
        @type = v_part_info[@id]["type"]
        @color_value = Widget.look_up(@lineid)?.color or get_random_color()
        @color.style.background = @color_value
        @path.innerText = v_part_info[@id]["path"]
        @path.innerHTML += "</br>"
        @path.innerText += @type
        @path.innerHTML += "</br>"
        @path.innerText += @id

    fill_fs: ->
        @fs_txt.innerText = v_part_info[@id]["fs"]
        @ext4_option = create_element("option", "", @fs_select)
        @ext4_option.setAttribute("value", "ext4")
        @ext4_option.innerText = "ext4"
        @ext3_option = create_element("option", "", @fs_select)
        @ext3_option.setAttribute("value", "ext3")
        @ext3_option.innerText = "ext3"
        @ext2_option = create_element("option", "", @fs_select)
        @ext2_option.setAttribute("value", "ext2")
        @ext2_option.innerText = "ext2"
        @reiserfs_option = create_element("option", "", @fs_select)
        @reiserfs_option.setAttribute("value", "reiserfs")
        @reiserfs_option.innerText = "reiserfs"
        @btrfs_option = create_element("option", "", @fs_select)
        @btrfs_option.setAttribute("value", "btrfs")
        @btrfs_option.innerText = "btrfs"
        @jfs_option = create_element("option", "", @fs_select)
        @jfs_option.setAttribute("value", "jfs")
        @jfs_option.innerText = "jfs"
        @xfs_option = create_element("option", "", @fs_select)
        @xfs_option.setAttribute("value", "xfs")
        @xfs_option.innerText = "xfs"
        @fat16_option = create_element("option", "", @fs_select)
        @fat16_option.setAttribute("value", "fat16")
        @fat16_option.innerText = "fat16"
        @fat32_option = create_element("option", "", @fs_select)
        @fat32_option.setAttribute("value", "fat32")
        @fat32_option.innerText = "fat32"
        @swap_option = create_element("option", "", @fs_select)
        @swap_option.setAttribute("value", "swap")
        @swap_option.innerText = "swap"
        @encrypt_option = create_element("option", "", @fs_select)
        @encrypt_option.setAttribute("value", "encrypt")
        @encrypt_option.innerText = "encrypt"
        @unused_option = create_element("option", "", @fs_select)
        @unused_option.setAttribute("value", "unused")
        @unused_option.innerText = "unused"
        @unused_option.setAttribute("selected", "selected")
        for opt, i in @fs_select
            if opt.value == v_part_info[@id]["fs"]
                @fs_select.selectedIndex = i

        @fs_select.addEventListener("change", (e) =>
            @fs_txt.innerText = @fs_select.options[@fs_select.selectedIndex].value
            update_part_fs(@id, @fs_select.options[@fs_select.selectedIndex].value)
        )

    fill_mount: ->
        @mount_txt.innerText = v_part_info[@id]["mp"]
        @root_option = create_element("option", "", @mount_select)
        @root_option.setAttribute("value", "/")
        @root_option.innerText = "/"
        @boot_option = create_element("option", "", @mount_select)
        @boot_option.setAttribute("value", "/boot")
        @boot_option.innerText = "/boot"
        @home_option = create_element("option", "", @mount_select)
        @home_option.setAttribute("value", "/home")
        @home_option.innerText = "/home"
        @tmp_option = create_element("option", "", @mount_select)
        @tmp_option.setAttribute("value", "/tmp")
        @tmp_option.innerText = "/tmp"
        @usr_option = create_element("option", "", @mount_select)
        @usr_option.setAttribute("value", "/usr")
        @usr_option.innerText = "/usr"
        @var_option = create_element("option", "", @mount_select)
        @var_option.setAttribute("value", "/var")
        @var_option.innerText = "/var"
        @srv_option = create_element("option", "", @mount_select)
        @srv_option.setAttribute("value", "/srv")
        @srv_option.innerText = "/srv"
        @opt_option = create_element("option", "", @mount_select)
        @opt_option.setAttribute("value", "/opt")
        @opt_option.innerText = "/opt"
        @local_option = create_element("option", "", @mount_select)
        @local_option.setAttribute("value", "/local")
        @local_option.innerText = "/local"
        @unused_option = create_element("option", "", @mount_select)
        @unused_option.setAttribute("value", "unused")
        @unused_option.innerText = "unused"
        @unused_option.setAttribute("selected", "selected")
        for opt, i in @mount_select
            if opt.value == v_part_info[@id]["mp"]
                @mount_select.selectedIndex = i

        @mount_select.addEventListener("change", (e) =>
            @mount_txt.innerText = @mount_select.options[@mount_select.selectedIndex].value
            update_part_mp(@id, @mount_select.options[@mount_select.selectedIndex].value)
        )

    fill_size: ->
        @size.innerText = v_part_info[@id]["start"]
        @size.innerText += "-------->" + v_part_info[@id]["end"]
        @size.innerHTML +="</br>"
        @size.innerText += v_part_info[@id]["length"]

    fill_used: ->
        @used.innerText = v_part_info[@id]["used"]
        #@used.innerText = "80G"

    fill_item: ->
        @fill_device()
        @fill_fs()
        @fill_mount()
        @fill_size()
        @fill_used()
        @update_mode(@mode)
        @element.style.display = "block"

    update_mode: (mode)->
        if mode == "simple"
            @mount.style.display = "none"
            for child in [@device, @fs, @mount, @size, @used]
                child.setAttribute("class","PartTableItemSimple")
        else if mode == "advance"
            @mount.style.display = "inline"
            for child in [@device, @fs, @mount, @size, @used]
                child.setAttribute("class", "PartTableItemCell")

    set_btn_status: ->
        @type = v_part_info[@id]["type"]
        #table_btn = document.getElementById("table_btn")
        add_btn = document.getElementById("add_btn")
        delete_btn = document.getElementById("delete_btn")
        query_btn = document.getElementById("query_btn")
        mode_btn = document.getElementById("mode_btn")
        undo_btn = document.getElementById("undo_btn")

        if @type == "freespace"
            #table_btn.setAttribute("class", "PartOpBtnDisabled")
            add_btn.setAttribute("class", "PartOpBtn")
            delete_btn.setAttribute("class", "PartOpBtnDisabled")
            query_btn.setAttribute("class", "PartOpBtnDisabled")
            mode_btn.setAttribute("class", "PartOpBtn")
            undo_btn.setAttribute("class", "PartOpBtn")

        else if @type == "normal"
            #table_btn.setAttribute("class", "PartOpBtnDisabled")
            add_btn.setAttribute("class", "PartOpBtnDisabled")
            delete_btn.setAttribute("class", "PartOpBtn")
            query_btn.setAttribute("class", "PartOpBtn")
            mode_btn.setAttribute("class", "PartOpBtn")
            undo_btn.setAttribute("class", "PartOpBtn")

        else if @type == "logical"
            #table_btn.setAttribute("class", "PartOpBtnDisabled")
            add_btn.setAttribute("class", "PartOpBtnDisabled")
            delete_btn.setAttribute("class", "PartOpBtn")
            query_btn.setAttribute("class", "PartOpBtn")
            mode_btn.setAttribute("class", "PartOpBtn")
            undo_btn.setAttribute("class", "PartOpBtn")

        else if @type == "extended"
            #table_btn.setAttribute("class", "PartOpBtnDisabled")
            add_btn.setAttribute("class", "PartOpBtnDisabled")
            delete_btn.setAttribute("class", "PartOpBtnDisabled")
            query_btn.setAttribute("class", "PartOpBtnDisabled")
            mode_btn.setAttribute("class", "PartOpBtnDisabled")
            undo_btn.setAttribute("class", "PartOpBtnDisabled")

        else
            #table_btn.setAttribute("class", "PartOpBtnDisabled")
            add_btn.setAttribute("class", "PartOpBtnDisabled")
            delete_btn.setAttribute("class", "PartOpBtnDisabled")
            query_btn.setAttribute("class", "PartOpBtnDisabled")
            mode_btn.setAttribute("class", "PartOpBtn")
            undo_btn.setAttribute("class", "PartOpBtn")

    focus: ->
        __selected_item?.blur()
        __selected_item = @

        if __selected_disk == null or __selected_disk != v_part_info[@id]["disk"]
            __selected_disk = v_part_info[@id]["disk"]

        try
            if __selected_line == null or __selected_item.id != __selected_line?.partid?
                Widget.look_up("part_line_maps")?.fill_linemap()
                Widget.look_up(@lineid)?.passive_focus()
        catch error
            echo error

        @set_btn_status()
        @add_css_class("PartTableItemSelected")

    passive_focus: ->
        __selected_item?.blur()
        __selected_item = @

        @set_btn_status()
        @add_css_class("PartTableItemSelected")

    blur: ->
        @element.setAttribute("class", "PartTableItem")

    do_click: (e)->
        if __selected_item == @ 
            echo "already selected"
        else
            @focus()

class PartTable extends Widget
    constructor: (@id)->
        super
        @header = create_element("div", "PartTableHeader", @element)
        @device_header = create_element("div", "PartTableHeaderCell", @header)
        @device_header.innerText = "Device"
        @fs_header = create_element("div", "PartTableHeaderCell", @header)
        @fs_header.innerText = "FileSystem"
        @mount_header = create_element("div", "PartTableHeaderCell", @header)
        @mount_header.innerText = "MountPoint"
        @size_header = create_element("div", "PartTableHeaderCell", @header)
        @size_header.innerText = "Size"
        @used_header = create_element("div", "PartTableHeaderCell", @header)
        @used_header.innerText = "Used"

        @items_part = []
        @items = create_element("div", "PartTableItems", @element)
        @fill_items()

    fill_items: ->
        echo "update part table items"
        for part in @items_part
            Widget.look_up(part)?.destroy()
            
        @items_part = []
        for disk in disks
            for part in v_disk_info[disk]["partitions"]
                #if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                #display extended for debug use
                if v_part_info[part]["type"] in ["normal", "logical", "freespace", "extended"]
                    item = new PartTableItem(part)
                    @items.appendChild(item.element)
                    @items_part.push(part)

    update_mode: (mode) ->
        if mode == "simple"
            @mount_header.style.display = "none"
            for child in [@device_header, @fs_header, @mount_header, @size_header, @used_header]
                child.setAttribute("class", "PartTableHeaderSimple")
        else if mode == "advance"
            @mount_header.style.display = "inline"
            for child in [@device_header, @fs_header, @mount_header, @size_header, @used_header]
                child.setAttribute("class", "PartTableHeaderCell")
        for disk in disks
            for part in v_disk_info[disk]["partitions"]
                Widget.look_up(part)?.update_mode(mode)?

class Part extends Page
    constructor: (@id)->
        super
        @title.innerText = "请选择要安装Deepin的磁盘"
        @mode = "advance"

        if __selected_disk == null
            __selected_disk = disks[0]

        @linemap = new PartLineMaps("part_line_maps")
        @content.appendChild(@linemap.element)

        @table = new PartTable("part_table")
        @content.appendChild(@table.element)

        if __selected_item == null
            __selected_item = Widget.look_up(@table.items_part?[0])?
        #part op buttons
        @op = create_element("div", "PartOp", @content)
        @btn_div = create_element("div", "BtnContainer", @op)
        @part_grub = create_element("div", "PartGrub", @op)

        ##@part_table =create_element("div", "PartOpBtn", @btn_div)
        #@part_table = create_element("input", "PartOpBtn", @btn_div)
        #@part_table.setAttribute("type", "button")
        #@part_table.setAttribute("id", "table_btn")
        ##@part_table.innerText = "新建分区表"
        #@part_table.setAttribute("value", "新建分区表")
        #@part_table.addEventListener("click", (e)=>
        #    echo "handle table"
        #)

        #@part_add = create_element("div", "PartOpBtn", @btn_div)
        @part_add = create_element("input", "PartOpBtn", @btn_div)
        @part_add.setAttribute("type", "button")
        @part_add.setAttribute("id", "add_btn")
        #@part_add.innerText = "新建分区"
        @part_add.setAttribute("value", "新建分区")
        @part_add.addEventListener("click", (e)=>
            echo "handle add"
            @add_model = new ModelDialog("AddModel", __selected_item.id)
            @btn_div.appendChild(@add_model.element)
            @add_model.show_dialog()

            @add_model.ok.addEventListener("click", (e)=>
                echo "confirm add partition"
                @add_model.gather_info()
                add_part(__selected_item.id, @add_model.n_type, @add_model.n_size, @add_model.n_align, @add_model.n_fs, @add_model.n_mp)
                @add_model.hide_dialog()
                Widget.look_up("part_table")?.fill_items()
                Widget.look_up("part_line_maps")?.fill_linemap()?
            )
        )

        #@part_delete = create_element("div", "PartOpBtn", @btn_div)
        @part_delete = create_element("input", "PartOpBtn", @btn_div)
        @part_delete.setAttribute("type", "button")
        @part_delete.setAttribute("id", "delete_btn")
        #@part_delete.innerText = "删除分区"
        @part_delete.setAttribute("value", "删除分区")
        @part_delete.addEventListener("click", (e)=>
            echo "handle delete"
            delete_part(__selected_item.id)
            Widget.look_up("part_table")?.fill_items()
            Widget.look_up("part_line_maps")?.fill_linemap()?
        )

        #@part_query = create_element("div", "PartOpBtn", @btn_div)
        @part_query = create_element("input", "PartOpBtn", @btn_div)
        @part_query.setAttribute("type", "button")
        @part_query.setAttribute("id", "query_btn")
        #@part_query.innerText = "查看分区"
        @part_query.setAttribute("value", "查看分区")
        @part_query.addEventListener("click", (e)=>
            echo "handle query"
        )

        #@part_mode = create_element("div", "PartOpBtn", @btn_div)
        @part_mode = create_element("input", "PartOpBtn", @btn_div)
        @part_mode.setAttribute("type", "button")
        @part_mode.setAttribute("id", "mode_btn")
        #@part_mode.innerText = "精简模式"
        @part_mode.setAttribute("value", "精简模式")
        @part_mode.addEventListener("click", (e)=>
            echo "handle mode"
            if @mode == "advance"
                @mode = "simple"
                @part_mode.setAttribute("value", "高级模式")
            else if @mode == "simple"
                @mode = "advance"
                @part_mode.setAttribute("value", "精简模式")
            else
                echo "invalid mode"
            Widget.look_up("part_table")?.update_mode(@mode)?
        )

        @part_undo = create_element("input", "PartOpBtn", @btn_div)
        @part_undo.setAttribute("type", "button")
        @part_undo.setAttribute("id", "undo_btn")
        @part_undo.setAttribute("value", "撤消操作")
        @part_undo.addEventListener("click", (e)=>
            echo "handle undo"
            for disk in disks
                undo_table(disk)
            Widget.look_up("part_table")?.fill_items()
            Widget.look_up("part_line_maps")?.fill_linemap()?
        )

        @grub_desc = create_element("div", "GrubDesc", @part_grub)
        @grub_desc.innerText = "安装启动引导器的设备："
        @grub_select_div = create_element("div", "GrubSelect", @part_grub)
        @grub_select = create_element("select", "", @grub_select_div)
        for disk in disks
            path = v_disk_info[disk]["path"]
            select_opt = create_element("option", "", @grub_select)
            select_opt.setAttribute("value", path)
            select_opt.innerText = path
            select_opt.innerText += v_disk_info[disk]["model"]

        @error = create_element("div", "Error", @footer)
        @error.innerText = "磁盘空间已满，无法安装。请整理出30G空白磁盘"

        @next_step = create_element("div", "NextStep", @footer)
        @next_btn = create_element("button", "", @next_step)
        @next_btn.innerText = "Next"

        @debug_btn = create_element("button", "", @next_step)
        @debug_btn.innerText = "debug"
        @debug_btn.addEventListener("click", (e) =>
            @debug_part()
        )

    debug_part: ->
        @debug_mark()

    debug_mark: ->
        echo "debug mark"
        @error.innerText = ""
        @error.innerText += "original partitions"
        @error.innerHTML += "</br>"
        for disk in disks
            @error.innerText += disk 
            for part in DCore.Installer.get_disk_partitions(disk)
                if DCore.Installer.get_partition_type(part) in ["normal", "extended", "logical"]
                    @error.innerText += part 
            @error.innerHTML += "</br>"

            if m_disk_info[disk]["change"] == true
                @error.innerText += "disk changed"
                @error.innerText += disk
                @error.innerHTML += "</br>"

                for part in get_modeled_partitions(disk)
                    if m_part_info[part]["op"] == "delete"
                        @error.innerText += "delete part"
                        @error.innerText += part
                        @error.innerHTML += "</br>"

                for part in get_modeled_partitions(disk)
                    if m_part_info[part]["op"] == "update"
                        @error.innerText += "update part"
                        @error.innerText += part
                        @error.innerHTML += "</br>"

                for part in get_modeled_partitions(disk)
                    if m_part_info[part]["op"] == "add"
                        @error.innerText += "add part"
                        @error.innerText += part
                        @error.innerHTML += "</br>"
            else
                @error.innerText += "disk not changed"
                @error.innerText += disk
                @error.innerHTML += "</br>"

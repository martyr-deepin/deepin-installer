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
__selected_mode = null

class AddPartDialog extends Widget
    constructor: (@id, @partid) ->
        super
        @title = create_element("p", "DialogTitle", @element)
        @title.innerText = "新建分区"

        @content = create_element("div", "DialogContent", @element)

        @foot = create_element("p", "DialogBtn", @element)
        @ok = create_element("span", "", @foot)
        @ok.innerText = "OK"
        @ok.addEventListener("click", (e)=>
            echo "confirm add partition"
            @gather_info()
            add_part(__selected_item.id, @n_type, @n_size, @n_align, @n_fs, @n_mp)
            @hide_dialog()
            Widget.look_up("part_table")?.fill_items()
            Widget.look_up("part_line_maps")?.fill_linemap()?
        )
        @cancel = create_element("span", "", @foot)
        @cancel.innerText = "Cancel"
        @cancel.addEventListener("click", (e) =>
            @hide_dialog()
        )
        @fill_type()
        @fill_size()
        @fill_align()
        @fill_fs()
        @fill_mount()
        @fill_tips()
        @show_dialog()

    show_dialog: ->
        __in_model = true

    hide_dialog: ->
        __in_model = false
        @destroy()

    fill_type: ->
        @type = create_element("p", "", @content)
        @type_desc = create_element("span", "", @type)
        @type.innerText = "类型:"
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
        @size = create_element("p", "", @content)
        @size_desc = create_element("span", "", @size)
        @size_desc.innerText = "大小:"
        @max_size_mb = sector_to_mb(v_part_info[@partid]["length"], 512)
        @size_input = create_element("input", "", @size)
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
        @size_limit = create_element("span", "", @size)
        @size_limit.innerText = "Limited size:" + @max_size_mb
        
    fill_align: ->
        @align = create_element("p", "", @content)
        @align_desc = create_element("span", "", @align)
        @align_desc.innerText = "位置:"
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
        @fs = create_element("p", "", @content)
        @fs_desc = create_element("span", "", @fs)
        @fs_desc.innerText = "格式:"
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
        @ntfs_option = create_element("option", "", @fs_select)
        @ntfs_option.setAttribute("value", "ntfs")
        @ntfs_option.innerText = "ntfs"
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
        @mp = create_element("p", "", @content)
        @mp_desc = create_element("span", "", @mp)
        @mp_desc.innerText = "挂载:"
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

class DeletePartDialog extends Widget
    constructor: (@id, @partid) ->
        super
        @title = create_element("p", "DialogTitle", @element)
        @title.innerText = "删除分区"

        @content = create_element("div", "DialogContent", @element)
        @content.innerText = "确定删除分区吗?"

        @foot = create_element("p", "DialogBtn", @element)
        @ok = create_element("span", "", @foot)
        @ok.innerText = "OK"
        @ok.addEventListener("click", (e) =>
            delete_part(__selected_item.id)
            @hide_dialog()
            Widget.look_up("part_table")?.fill_items()
            Widget.look_up("part_line_maps")?.fill_linemap()?
        )

        @cancel = create_element("span", "", @foot)
        @cancel.innerText = "Cancel"
        @cancel.addEventListener("click", (e) =>
            @hide_dialog()
        )
        @show_dialog()

    show_dialog: ->
        __in_model = true

    hide_dialog: ->
        __in_model = false
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

    update_item: ->
        @color = v_part_info[@part]["color"]
        @element.style.background = @color
        @element.style.width = v_part_info[@part]["width"]
        if v_part_info[@part]["disk"] != __selected_disk
            @element.style.display = "none"
        else
            @element.style.display = "inline"

class PartLineMaps extends Widget
    constructor: (@id)->
        super
        @fill_linemap()

    fill_linemap: ->
        @element.innerHTML = ""

        for disk in disks
            line = create_element("div", "", @element)
            line.setAttribute("id", "line"+disk)
            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                    item = new PartLineItem("line"+part)
                    line.appendChild(item.element)

            if disk == __selected_disk
                line.style.display = "inline"
            else
                line.style.display = "none"
                
    disk_active: ->
        line = document.getElementById("line"+__selected_disk)
        line.setAttribute("class", "PartLineDiskActive")

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
        @fill_device()

        @size = create_element("div", "", @element)
        @size.innerText += sector_to_mb(v_part_info[@id]["length"], 512)

        @used = create_element("div", "", @element)
        @used.innerText = v_part_info[@id]["used"]

        @fs = create_element("div", "", @element)
        @fill_fs()
        
        @mount = create_element("div", "", @element)
        @fill_mount()

    product_disk_item: ->
        @device = create_element("div","", @element)
        @device.innerText = v_disk_info[@id]["path"]

    fill_device: ->
        @os = create_element("span", "Os", @device)
        if v_part_info[@id]["os"]?
            #@os.innerText = v_part_info[@id]["os"]
            if v_part_info[@id]["os"].toLowerCase().indexOf("linux") != -1
                os_img = "images/linux.png"
            else if v_part_info[@id]["os"].toLowerCase().indexOf("windows") != -1
                os_img = "images/windows.png"
            else if v_part_info[@id]["os"].toLowerCase().indexOf("mac") != -1
                os_img = "images/apple.png"
            create_img("osimg", os_img, @os)

        @color = create_element("span", "Color", @device)
        color_value = Widget.look_up(@lineid)?.color or get_random_color()
        @color.style.background = color_value

        @lp = create_element("span", "LabelPath", @device)
        @label = create_element("div", "Label", @lp)
        if v_part_info[@id]["label"]?
            @label.innerText = v_part_info[@id]["label"]

        @path = create_element("div", "Path", @lp)
        @path.innerText = v_part_info[@id]["path"]

    fill_fs: ->
        @fs.innerHTML = ""
        if __selected_mode == "simple"
            @fs_txt = create_element("div", "", @fs)
            @fs_txt.innerText = v_part_info[@id]["fs"]
        else if __selected_mode == "advance"
            @fill_fs_select()
        else
            echo "fill fs:invalid mode"

    fill_fs_select: ->
        @fs_select = create_element("select", "", @fs)
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
        @ntfs_option = create_element("option", "", @fs_select)
        @ntfs_option.setAttribute("value", "ntfs")
        @ntfs_option.innerText = "ntfs"
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

        @fs_select.addEventListener("focus", (e) =>
            if __selected_item != @
                @focus()
        )

        @fs_select.addEventListener("change", (e) =>
            update_part_fs(@id, @fs_select.options[@fs_select.selectedIndex].value)
        )

    fill_mount: ->
        @mount.innerHTML = ""
        if __selected_mode == "advance"
            @fill_mount_select()

    fill_mount_select: ->
        @mount_select = create_element("select", "", @mount)
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
            Widget.look_up("part_line_maps")?.disk_active()
        else
            __selected_disk = v_part_info[@id]["disk"]
            try
                if __selected_line == null or __selected_item.id != __selected_line?.partid?
                    Widget.look_up("part_line_maps")?.fill_linemap()
                    Widget.look_up(@lineid)?.passive_focus()
            catch error
                echo error

        @set_btn_status()
        @add_css_class("PartTableItemActive")

    passive_focus: ->
        __selected_item?.blur()
        __selected_item = @

        @set_btn_status()
        @add_css_class("PartTableItemActive")

    blur: ->
        @element.setAttribute("class", "PartTableItem")

    do_click: (e)->
        if __selected_item == @ 
            echo "already selected"
        else
            @focus()

    update_mode: (mode) ->
        @fill_fs()
        @fill_mount()

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

        @items_device = []
        @items = create_element("div", "PartTableItems", @element)
        @fill_items()

    fill_items: ->
        echo "update part table items"
        for device in @items_device
            Widget.look_up(device)?.destroy()
            
        @items_device = []
        for disk in disks
            item = new PartTableItem(disk, "disk")
            @items.appendChild(item.element)
            @items_device.push(disk)

            for part in v_disk_info[disk]["partitions"]
                if v_part_info[part]["type"] in ["normal", "logical", "freespace"]
                    item = new PartTableItem(part, "part")
                    @items.appendChild(item.element)
                    @items_device.push(part)

    update_mode: (mode) ->
        if mode == "advance"
            @mount_header.innerText = "MountPoint"
        for disk in disks
            for part in v_disk_info[disk]["partitions"]
                Widget.look_up(part)?.update_mode(mode)?

class Part extends Page
    constructor: (@id)->
        super
        if __selected_mode == null
            __selected_mode = "simple"

        if __selected_disk == null
            __selected_disk = disks[0]

        @title = create_element("div", "PartTitle", @element)
        @t_desc = create_element("i", "", @title)
        @t_desc.innerText = "请选择要安装Deepin的磁盘"
        @t_help = create_element("span", "", @title)
        @t_help.innerText = "查看帮助"
        @t_sep = create_element("span", "", @title)
        @t_sep.innerText = " | "
        @t_mode = create_element("span", "", @title)
        @t_mode.innerText = "精简模式"

        #linemap
        @linemap = new PartLineMaps("part_line_maps")
        @element.appendChild(@linemap.element)

        #part table
        @table = new PartTable("part_table")
        @element.appendChild(@table.element)

        if __selected_item == null
            __selected_item = Widget.look_up(@table.items_part?[0])?

        if __selected_mode == "advance"
            @fill_advance_op()

        @next_step = create_element("p", "NextStep", @element)
        @next_btn = create_element("span", "", @next_step)
        @next_btn.innerText = "开始安装"

        @t_mode.addEventListener("click", (e) =>
            if __selected_mode != "advance"
                __selected_mode = "advance"
                @fill_advance_op()
                @table.update_mode(__selected_mode)
        )

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
        @part_grub.innerHTML = "<span>安装启动引导器：</span>"
        @grub_select = create_element("select", "", @part_grub)
        for disk in disks
            path = v_disk_info[disk]["path"]
            select_opt = create_element("option", "", @grub_select)
            select_opt.setAttribute("value", path)
            select_opt.innerText = path
            select_opt.innerText += v_disk_info[disk]["model"]

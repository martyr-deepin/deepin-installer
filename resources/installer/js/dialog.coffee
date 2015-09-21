class Dialog extends Widget
    constructor: (@id, @with_cancel, @ok_cb,@cancel_cb) ->
        super
        @title = create_element("div", "DialogTitle", @element)
        @title_txt = create_element("div", "DialogTxt", @title)
        @title_close = create_element("div", "DialogClose", @title)
        @title_close.addEventListener("click", (e) =>
            @hide_dialog()
        )

        @content = create_element("div", "DialogContent", @element)
        @foot = create_element("div", "DialogBtn", @element)
        @ok = create_element("div", "DialogOKButton", @foot)
        @ok.addEventListener("click", (e) =>
            @hide_dialog()
            @ok_cb?()
        )
        if @with_cancel
            @cancel = create_element("div", "DialogCancelButton", @foot)
            @cancel.addEventListener("click", (e) =>
                @hide_dialog()
                @cancel_cb?()
            )
        else
            @ok.setAttribute("style", "margin:31px 145px 0 0")
        @set_button_text()
        @show_dialog()

        if @with_cancel
            @foot.addEventListener('keyup', (e) =>
                code = e.keyCode
                if code == KEYCODE.LEFT_ARROW
                    @cancel.focus()
                else if code == KEYCODE.RIGHT_ARROW
                    @ok.focus()
            )

    show_at: (parent) ->
        parent.appendChild(@element)

    show_dialog: ->
        __in_model = true
        __board.setAttribute("style", "display:block")
        for el in tabEnableElements
            el.setAttribute("tabindex","-1")
        enable_tab(@title_close)
        enable_tab(@ok)
        enable_tab(@cancel) if @cancel
        callback = ->
            cancelButton = document.querySelector(".DialogCancelButton")
            if cancelButton
                cancelButton.focus()
            else
                okButton = document.querySelector(".DialogOKButton")
                if okButton
                    okButton.focus()
        setTimeout callback, 50

    hide_dialog: ->
        __in_model = false
        @destroy()
        for el in tabEnableElements
            el.setAttribute("tabindex","0")
        __board.setAttribute("style", "display:none")

    set_button_text:(@ok_text = _("OK"), @cancel_text = _("Cancel")) ->
        @ok?.innerText = @ok_text
        @cancel?.innerText = @cancel_text

class ExitDialog extends Dialog
    constructor: (exit_installer_cb) ->
        super("ExitDialog", true, exit_installer_cb)
        @add_css_class("DialogCommon")
        @exit_tips = create_element("div", "dialog_tips", @content)
        @exit_tips.innerText = _("Are you sure to exit from installation?")


class PromptDialog extends Dialog
    constructor: (title, content, ok_cb, cancel_cb) ->
        super("PromptDialog", true, ok_cb,cancel_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = title
        @root_tips = create_element("div", "dialog_tips", @content)
        @root_tips.innerText = content

class MessageDialog extends Dialog
    constructor: (title, content, cancel_cb) ->
        super("MessageDialog", false, cancel_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = title
        @root_tips = create_element("div", "dialog_tips", @content)
        @root_tips.innerText = content

class DeletePartDialog extends Dialog
    constructor: (@id,@partid) ->
        super(@id, true, @delete_part_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Delete Partition")
        @delete_tips = create_element("div", "dialog_tips", @content)
        @delete_tips.innerText = _("Are you sure you want to delete this partition?")

    delete_part_cb: ->
        remain_part = delete_part(@partid)
        Widget.look_up("part_table")?.fill_items()
        Widget.look_up("part_line_maps")?.fill_linemap()
        Widget.look_up(remain_part)?.focus()
        Widget.look_up("part")?.fill_bootloader()

class FormatDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @format_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Formatting Partition")
        @format_tips = create_element("div", "dialog_tips", @content)
        @format_tips.innerText = _("Are you sure you want to format this partition?")

    format_cb: ->
        console.log("[dialog.coffee] FormatDialog.format_cb()")

class InstallDialog extends Dialog
    constructor: (@id) ->
        super(@id, true, @confirm_install_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Proceed with installation")
        @root_tips = create_element("div", "dialog_tips", @content)

        if __selected_mode == "advance"
            target = get_target_part()
        else
            target = __selected_item.id
        path = v_part_info[target]["path"]
        if v_part_info[target]["type"] == "freespace"
            @root_tips.innerText = _("Deepin OS will be installed to freespace.")
        else
            @root_tips.innerText = _("Deepin OS will be installed to ") + path + _("\nThe partition will be formatted if you continue the installation. Please make a backup for the important data. \n") + _("Do you want to continue the installation ?")

    confirm_install_cb: ->
        try_removed_start_install()


class AddPartDialog extends Dialog
    constructor: (@id, @partid) ->
        super(@id, true, @add_part_cb)
        @add_css_class("DialogCommon")
        @element.style.top = "85px"
        @title_txt.innerText = _("Add Partition")
        @fill_type()
        @fill_size()
        @fill_align()
        @fill_fs()
        @fill_mount()
        @fill_tips()

    add_part_cb: ->
        @gather_info()
        new_part = add_part(@partid, @n_type, @n_size, @n_align, @n_fs, @n_mp)
        v_part_info[new_part]["mp"] = @n_mp
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
        @max_size_mb = (v_part_info[@partid]["length"] / MB).toFixed(0)

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
        @fs_select = new DropDown("dd_fs_" + @partid, false, @fs_change_cb)
        @fs_value.appendChild(@fs_select.element)

        #TODO: update fs
        if __selected_use_uefi
            @fs_select.set_drop_items(__fs_efi_keys, __fs_efi_values)
        else
            @fs_select.set_drop_items(__fs_keys, __fs_values)

        @fs_select.set_drop_size(130,22)
        @fs_select.set_selected("ext4")
        @fs_select.show_drop()

    fs_change_cb: (fs) ->
        if fs in ["efi", "swap", "unused", "fat16", "fat32", "ntfs"]
            Widget.look_up("AddModel").mp.style.display = "none"
        else
            Widget.look_up("AddModel").mp.style.display = "block"

    fill_mount: ->
        @mp = create_element("div", "", @content)
        @mp_desc = create_element("span", "AddDesc", @mp)
        @mp_desc.innerText = _("Mount:")
        @mount_value = create_element("span", "AddValue", @mp)
        @mount_select = new DropDown("dd_mp_" + @partid, true, (data) => @mp_change_cb(@partid, data))
        @mount_value.appendChild(@mount_select.element)
        @mount_select.set_drop_items(__mp_keys, __mp_values)
        @mount_select.set_drop_size(130,22)
        @mount_select.set_selected("unused")
        @mount_select.show_drop()

    mp_change_cb: (partid, mp) ->
        if mp.substring(0,1) != "/"
            mp = "unused"
        if mp in get_selected_mp()
            part = get_mp_partition(mp)
            if part? and part != partid
                v_part_info[part]["mp"] = "unused"
                Widget.look_up(part)?.fill_mount()
            else
                console.error("[dialog.coffee] AddPartDialog.mp_change_cb(), failed to get mp_partition in add_dialog, partid:", partid, ", mp:", mp)

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
            @n_size = parseInt(@size_input.value) * MB
        if not @n_size?
            @tips.innerText = _("Please enter a valid partition size.")
        @n_align = @align_radio
        @n_fs = @fs_select.get_selected()
        @n_mp = @mount_select.get_selected()

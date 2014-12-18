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

__focused_layout_item = null

__legal_keys='abcdefghijklmnopqrstuvwxyz' + '0123456789' + '-_'
illegalErrCode =
    empty:1
    invalid:2
    firstUpper:3
    exist:4
    systemUsed:5
    different:6

__selected_zone_index = 8

__database = JSON.parse(timezone_json)

_sort_layout = (layout_a, layout_b) ->
    a_desc = DCore.Installer.get_layout_description(layout_a)
    b_desc = DCore.Installer.get_layout_description(layout_b)
    return a_desc.localeCompare(b_desc)

get_matched_items = (key, list) ->
    matched = []
    for item in list
        if item.toLowerCase().indexOf(key.toLowerCase()) != -1
            matched.push(item)
    return matched

is_ancestor = (ancestor, el) ->
    while el?
        if el == ancestor
            return true
        el = el.parentNode
    return false

__keyboard_widget = null
__timezone_widget = null
__account_widget = null

keyboardSet_div = null
timezoneSet_div = null

tooltipOffsetX = (document.body.clientWidth - document.body.offsetWidth) / 2
tooltipOffsetY = (document.body.clientHeight - document.body.offsetHeight) / 2

update_keyboard_text = (tri) ->
    current_layout = DCore.Installer.get_layout_description(__selected_layout)
    echo "current_layout:#{current_layout}"
    keyboardSet_div?.innerText = current_layout
    keyboardSet_div?.title = current_layout

    x = keyboardSet_div?.offsetLeft - 15
    tri?.style.left = x if x > 0

    DCore.Installer.set_layout(__selected_layout)

update_timezone_text = (tri) ->
    console.debug "[update_timezone_text]: __selected_timezone:#{__selected_timezone}"

    utc = DCore.Installer.get_timezone_utc(__selected_timezone)
    echo "current_timezone_utc:#{utc}"
    timezoneSet_div?.title = utc

    city = __selected_timezone.split("/")[1]
    right = DCore.dgettext("tzdata", city)
    if not right? then right = city
    timezoneSet_div?.innerText = right

    x = timezoneSet_div?.offsetLeft - 15
    tri?.style.left = x if x > 0

class LayoutItem extends Widget
    constructor: (@id, @layout, @keyboard)->
        super
        @element.innerText = DCore.Installer.get_layout_description(@layout)
        if @layout == __selected_layout.split(",")[0]
            @focus()

    focus: ->
        __focused_layout_item?.blur()
        __focused_layout_item = @
        @keyboard.fill_variants(@layout)
        if __focused_layout_item == __selected_layout_item
            @element.setAttribute("class", "LayoutItem KeyboardActive")
        else
            @element.setAttribute("class", "LayoutItem KeyboardFocus")
        @element.scrollIntoView()

    blur: ->
        if __focused_layout_item == __selected_layout_item
            @element.setAttribute("class", "LayoutItem KeyboardSelect")
        else
            @element.setAttribute("class", "LayoutItem")

    active: ->
        __selected_layout_item?.unactive()
        __selected_layout_item = @
        @element.setAttribute("class", "LayoutItem KeyboardActive")

    unactive: ->
        @element.setAttribute("class", "LayoutItem")

    do_click: (e) ->
        if __focused_layout_item != @
            @focus()

    do_mouseover: (e) ->
        if __focused_layout_item == @
            return
        if __selected_layout_item == @
            @element.setAttribute("class", "LayoutItem KeyboardSelectHover")
            return
        @element.setAttribute("class", "LayoutItem KeyboardHover")

    do_mouseout: (e) ->
        if __focused_layout_item == @
            return
        if __selected_layout_item == @
            @element.setAttribute("class", "LayoutItem KeyboardSelect")
            return
        @element.setAttribute("class", "LayoutItem")

class VariantItem extends Widget
    constructor: (@id, @variant, @keyboard)->
        super
        @element.innerText = DCore.Installer.get_layout_description(@variant)
        if @variant == __selected_layout
            @focus()

    focus: ->
        __selected_variant_item?.blur()
        __selected_variant_item = @
        @element.setAttribute("class", "VariantItem KeyboardActive")
        __selected_layout = @variant
        __focused_layout_item?.active()
        @keyboard.update_layout(@variant)

    blur: ->
        @element.setAttribute("class", "VariantItem")

    do_click: (e) ->
        if __selected_variant_item != @
            @focus()

    do_dblclick: =>
        if __selected_variant_item != @
            @focus()
        __account_widget.show()

    do_mouseover: (e) ->
        if __selected_variant_item != @
            @element.setAttribute("class", "VariantItem KeyboardHover")

    do_mouseout: (e) ->
        if __selected_variant_item != @
            @element.setAttribute("class", "VariantItem")

class KeyboardDetectDialog extends  Widget
    constructor: (@id, @type, @keyboard) ->
        super
        @add_css_class("DialogCommon")
        @element.style.top = "200px"
        @title = create_element("div", "", @element)
        @title_close = create_element("div", "DialogClose", @title)
        @title_close.addEventListener("click", (e) =>
            @hide_dialog()
        )

        @content = create_element("div", "DialogContent", @element)
        @desc = create_element("div", "Desc", @content)
        @detail = create_element("div", "Detail", @content)

        @foot = create_element("div", "DialogBtn", @element)
        @ok = create_element("div", "", @foot)
        @ok.innerText = _("Yes")
        @ok.addEventListener("click", (e) =>
            @process_have_key()
        )
        @cancel = create_element("div", "", @foot)
        @cancel.innerText = _("No")
        @cancel.addEventListener("click", (e) =>
            @process_no_have_key()
        )

    do_keypress: (e) =>
        if @type == "press"
            @process_key_press(e)

    process_have_key: ->
        #echo "detect dialog process have key"
        step = null
        r = null
        try
            step = DCore.Installer.keyboard_detect_get_present()
        catch error
            echo error
        if step?
            try
                r = DCore.Installer.keyboard_detect_read_step(step)
            catch error
                echo error
        if r?
            @keyboard.process_keyboard_detect(r)
        else
            @hide_dialog()

    process_no_have_key: ->
        #echo "detect dialog process no have key"
        step = null
        r = null
        try
            step = DCore.Installer.keyboard_detect_get_not_present()
        catch error
            echo error
        if step?
            try
                r = DCore.Installer.keyboard_detect_read_step(step)
            catch error
                echo error
        if r?
            @keyboard.process_keyboard_detect(r)
        else
            @hide_dialog()

    process_key_press: (e) =>
        #echo "detect dialog process key press"
        keycodes = null
        step = null
        r = null
        code = DCore.Installer.get_keycode_from_keysym(e.which) - 8
        if code > 255 or code < 0
            echo "invalid code"
            return
        try
            keycodes = DCore.Installer.keyboard_detect_get_keycodes()
        catch error
            echo error
        if keycodes?
            try
                step = keycodes[code]
            catch error
                echo error
            if step?
                try
                    r = DCore.Installer.keyboard_detect_read_step(step)
                catch error
                    echo error
                if r?
                    @keyboard.process_keyboard_detect(r)
                else
                    @hide_dialog()
            else
                @hide_dialog()
        else
            @hide_dialog()

    fill_detail: (symbols) ->
        @detail.innerHTML = ""
        @detail_ul = create_element("ul", "", @detail)
        for item in symbols
            @fill_detail_item(item)

    fill_detail_item: (s) ->
        item = create_element("li", "", @detail_ul)
        item.innerText = s

    update_type: (type) ->
        try
            symbols = DCore.Installer.keyboard_detect_get_symbols()
        catch error
            echo error
        if type == "press"
            @type = type
            @foot.style.display = "none"
            @desc.innerText = _("Please press one of the following keys.")
            @fill_detail(symbols)
            @show_dialog()
        else if type == "have"
            @type = type
            @foot.style.display = "block"
            @desc.innerText = _("Please confirm you have the following keys.")
            @fill_detail(symbols)
            @show_dialog()
        else if type == "result"
            @hide_dialog()
        else
            echo "invalid type"
            @hide_dialog()

    #TODO: Should this directly inherit from Dialog ?
    show_at: (parent)->
        parent.appendChild(@element)

    show_dialog: ->
        __in_model = true
        __board.setAttribute("style", "display:block")
        @element.style.display = "block"
        @element.setAttribute("tabindex", 100)
        @element.focus()

    hide_dialog: ->
        __in_model = false
        __board.setAttribute("style", "display:none")
        @element.style.display = "none"

class Keyboard extends Widget
    constructor: (@id)->
        super
        @init_layouts()

        @tri = create_element("div", "KeyboardTri", @element)

        @query = create_element("div", "Query", @element)
        @query_ul = create_element("ul", "", @query)
        @init_query_ul()

        @content = create_element("div", "KeyBoardContent", @element)
        @list = create_element("div", "KeyBoardList", @content)
        @layout_list = create_element("div", "LayoutList", @list)
        @variant_list = create_element("div", "VariantList", @list)

        @op = create_element("div","KeyBoardOp", @content)
        @detect_btn = create_element("div", "Detect", @op)
        @detect_btn.innerText = _("Detect Layout")
        @detect_btn.addEventListener("click", (e) =>
            @detect_keyboard_cb(e)
        )

        @active_btn = create_element("div", "Default", @op)
        @active_btn.innerText = _("View Current")
        @active_btn.addEventListener("click", (e) =>
            if __selected_layout?
                @set_to_layout(__selected_layout)
        )

        @fill_layouts(@layouts)
        @hide()

        update_keyboard_text(@tri)

        @searcher = new Trie()
        @layouts_des_pinyin = {}
        for item in @layouts
            des = DCore.Installer.get_layout_description(item)
            des_pinyin = codefans_net_CC2PY(des)
            @layouts_des_pinyin[des_pinyin] = des
            @searcher.insert(des_pinyin)

    show: ->
        __timezone_widget?.hide()
        __account_widget?.hide()
        @displayed = true
        @element.style.display = "block"
        __selected_layout_item?.focus()

    hide: ->
        @displayed = false
        @element.style.display = "none"

    init_query_ul: ->
        @selected_letter = null
        for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            @create_query_li(c)

    create_query_li:(c) ->
        li = create_element("li", "", @query_ul)
        li.innerText = c
        li.addEventListener("click", (e) =>
            @selected_letter?.setAttribute("class", "")
            @selected_letter = li
            @selected_letter.setAttribute("class","LetterActive")
            @execute_letter_query(c)
        )

    init_layouts: ->
        lay_var = DCore.Installer.get_current_layout_variant()
        lay = lay_var["layouts"]
        if lay? and lay.length > 0
            mylay = lay[0]
        variant = lay_var["variants"]
        if variant? and variant.length > 0
            myvar = variant[0]
        if myvar? and myvar.length > 0
            current = mylay + "," + myvar
        else
            current = mylay
        if current?
            __selected_layout = current

        @layouts = []
        @variants = {}
        for layout in DCore.Installer.get_keyboard_layouts()
            @layouts.push(layout)
            @variants[layout] = []
            @variants[layout].push(layout)
            for variant in DCore.Installer.get_layout_variants(layout)
                @variants[layout].push(layout + "," + variant)
        @layouts.sort(_sort_layout)

    fill_layouts: (layouts) ->
        @layout_list.innerHTML = ""
        for layout in layouts
            item = new LayoutItem("layoutitem_" + layout, layout, @)
            @layout_list.appendChild(item.element)

    fill_variants: (layout) ->
        @variant_list.innerHTML = ""
        for variant in @variants[layout]
            item = new VariantItem("variantitem_" + variant, variant, @)
            @variant_list.appendChild(item.element)

    update_layout: (layout) ->
        __selected_layout = layout
        update_keyboard_text(@tri)

    execute_letter_query: (letter) ->
        matched_pinyin = @searcher.autoComplete(letter)
        matched = []
        for match in matched_pinyin
            matched.push(@layouts_des_pinyin[match])
        echo matched
        @layout_list.innerHTML = ""
        @variant_list.innerHTML = ""

        matched_layouts = []
        for key in @layouts
            desc = DCore.Installer.get_layout_description(key)
            if desc in matched
                matched_layouts.push(key)

        @fill_layouts(matched_layouts)
        if matched_layouts.length > 0
            Widget.look_up("layoutitem_" + matched_layouts[0])?.focus()

    set_to_layout: (layout) ->
        a = layout.split(",")
        if a.length > 1
            lay_var = a[0]
        else
            lay_var = layout
        if lay_var in @layouts
            @fill_layouts(lay_var.split())
            Widget.look_up("layoutitem_" + lay_var)?.focus()
            Widget.look_up("variantitem_" + layout)?.focus()
        else
            echo "set_to_layout invalid layout"
            echo layout
            echo lay_var

    detect_keyboard_cb: (e)=>
        #echo "detect keyboard cb"
        r = DCore.Installer.keyboard_detect_read_step("0")
        if @detect_dialog?
            @detect_dialog.destroy()
            @detect_dialog = null
        @detect_dialog = new KeyboardDetectDialog("detect_dialog", null, @)
        @detect_dialog.show_at(document.body)
        @detect_dialog.hide_dialog()
        @process_keyboard_detect(r)

    process_keyboard_detect: (r) ->
        if r == 1
            @detect_dialog.update_type("press")
        else if r == 2 or r == 3
            @detect_dialog.update_type("have")
        else if r == 4
            @detect_dialog.update_type("result")
            @handle_detect_result()
        else
            echo "Invalid Detect Type"
            echo r

    handle_detect_result: ->
        echo "handle detect result"
        detect_result = DCore.Installer.keyboard_detect_get_result()
        layout = detect_result.split(":").join(",")
        echo layout
        @set_to_layout(layout)

class Timezone extends Widget
    constructor: (@id) ->
        super
        @tri = create_element("div", "TimezoneTri", @element)

        @query = create_element("div", "Query", @element)
        @query_div = create_element("div", "Left", @query)
        @query_wrap = create_element("div", "QueryWrap", @query_div)
        @query_input = create_element("input", "", @query_wrap)
        @query_input.setAttribute("placeholder", _("Please select or search your location."))
        @query_input.addEventListener("keyup", (e) =>
            if e.which == 13
                @execute_query()
            else if e.which == 27
                @query_complete?.style.display = "none"
        )
        @query_input.addEventListener("input", (e) =>
            @show_query_complete()
        )
        @query_img = create_element("div", "QueryImg", @query_wrap)
        @query_img.addEventListener("click", (e) =>
            @execute_query()
        )

        @picker_wrap = create_element("div","TmezoneWrap", @element)
        @picker = create_element("div", "TimezonePicker", @picker_wrap)
        @picker.addEventListener("mousemove", (e) =>
            px = e.x - 25
            mouse_zone = Math.round(px * 24 / 700 - 12).toString()
            if mouse_zone != __selected_zone_index
                __selected_zone_index = mouse_zone
                @destroy_canvas("picker")
                for area in @get_offset_areas(__selected_zone_index)
                    @draw_canvas(area)
        )
        @canvas = create_element("canvas", "TimezoneCanvas", @picker)
        @canvas.setAttribute("width", 700)
        @canvas.setAttribute("height", 370)
        @img = create_img("TimezoneMap", "images/zonemap.png", @picker)
        @img.setAttribute("usemap", "#ImageMap")
        @construct_map()
        @hide()
        update_timezone_text(@tri)

    init_search_list: ->
        @search_list = []
        @zone_dict = {}
        for zone in DCore.Installer.get_timezone_list()
            @search_list.push(zone)
            @zone_dict[zone] = zone
            try
                left = DCore.dgettext("tzdata", zone.split("/")[0])
                right = DCore.dgettext("tzdata", zone.split("/")[1])
                if left? and right?
                    l = left + "/" + right
                    if l not in @search_list
                        @search_list.push(l)
                        @zone_dict[l] = zone
            catch error
                pass

    show: ->
        __keyboard_widget?.hide()
        __account_widget?.hide()
        @displayed = true
        @element.style.display = "block"
        ArrowToolTip.container?.style.display = "block"

    hide: ->
        @displayed = false
        @element.style.display = "none"
        ArrowToolTip.container?.style.display = "none"

    construct_map: ->
        @imagemap = create_element("map", "", @element)
        @imagemap.setAttribute("name", "ImageMap")
        @imagemap.setAttribute("id", "ImageMap")
        for key, val of __database
            @construct_area(key)

    construct_area: (key) ->
        area = create_element("area", "TimezoneArea", @imagemap)
        area.setAttribute("data-timezone", key)
        area.setAttribute("data-country", __database[key]["country"])
        area.setAttribute("data-pin", __database[key]["pin"])
        area.setAttribute("data-offset", __database[key]["offset"])
        area.setAttribute("href", "#")
        if __database[key]["polys"].length != 0
            area.setAttribute("shape", "poly")
            area.setAttribute("coords", __database[key]["polys"])
        else if __database[key]["rects"].length != 0
            area.setAttribute("shape", "rect")
            area.setAttribute("coords", __database[key]["rects"])
        area.addEventListener("click", (e) =>
            @show_pin(area)
            __selected_timezone = area.getAttribute("data-timezone")
            update_timezone_text(@tri)
        )
        area.addEventListener("mousemove", (e) =>
            @destroy_canvas(area)
            @draw_timezone(area)
        )
        area.addEventListener("mouseout", (e) =>
            @destroy_canvas(area)
        )

    show_pin: (area) ->
        if not @pin?
            @pin = create_element("div", "Pin", @picker_wrap)
            @pin_img = create_img("", "images/pin.png", @pin)
            @pin.addEventListener("dblclick",=>
                __account_widget.show()
            )
        pin = area.getAttribute("data-pin").split(",")
        x = parseInt(pin[0]) - 6
        y = parseInt(pin[1]) + 50 - 6
        style = "left:" + x + "px;" + "top:" + y + "px"
        @pin.setAttribute("style", style)

        text = area.getAttribute("data-timezone").split("/")[1]
        console.debug "#{text}-->#{_(text, "tzdata")},(#{pin[0]},#{pin[1]})-->#{style}"

        if not @tooltip?
            @tooltip = new ArrowToolTip(@pin, _(text,"tzdata"))
        @tooltip.set_text(_(text,"tzdata"))
        @tooltip.setPointerEvents('none')
        @tooltip.show()
        pos = @tooltip.get_xy()
        ArrowToolTip.move_to(@tooltip, pos.x - tooltipOffsetX, pos.y - tooltipOffsetY)

        if @circle?
            @circle.parentElement.removeChild(@circle)
        @circle = create_element("div", "Circle", @pin)
        @circle.setAttribute("id", "pin_circle")
        @circle_inner = create_element("div", "Inner", @circle)
        @circle_outer = create_element("div", "Outer", @circle)
        apply_animation(@circle_outer, "pinflash", "1.5s", "linear")
        apply_animation(@circle_inner, "flashpin", "1.5s", "linear")
        setTimeout(->
            document.getElementById("pin_circle").style.display = "none"
        , 1500)

    draw_timezone: (myarea) ->
        offset = myarea.getAttribute("data-offset")
        for area in @get_offset_areas(offset)
            @draw_canvas(area)

    get_offset_areas: (offset) ->
        areas = []
        for area in @imagemap.children
            if area.getAttribute("data-offset") == offset
                areas.push(area)
        return areas

    get_area: (timezone) ->
        for area in @imagemap.children
            if area.getAttribute("data-timezone") == timezone
                return area

    set_timezone: (timezone) ->
        area = @get_area(timezone)
        if area?
            @draw_timezone(area)
            @show_pin(area)
            __selected_timezone = timezone
            update_timezone_text(@tri)

    draw_canvas: (area) ->
        ctx = @canvas.getContext("2d")
        poly = area.getAttribute("coords").split(",")
        ctx.beginPath()
        ctx.moveTo(poly[0],poly[1])
        i = 2
        while i < poly.length - 1
            ctx.lineTo(poly[i], poly[i+1])
            i = i + 2
        ctx.closePath()
        ctx.fillStyle = "rgba(255,255,255,1)"
        ctx.fill()

    destroy_canvas: (area) ->
        ctx = @canvas.getContext("2d")
        ctx.clearRect(0,0,700,370)

    show_query_complete: ->
        if @query_complete?
            @query_div.removeChild(@query_complete)
            @query_complete = null
        @query_complete = create_element("div", "QueryComplete", @query_div)
        if not @search_list?
            @init_search_list()
        key = @query_input.value
        matched = get_matched_items(key, @search_list)
        if matched.length > 0
            @query_complete.style.display = "block"
            for item in matched
                @create_complete_item(item)
        else
            @query_complete.style.display = "none"

    create_complete_item: (txt) ->
        item = create_element("div", "QueryCompleteItem", @query_complete)
        item.innerText = txt
        item.addEventListener("click", (e) =>
            @set_timezone(@zone_dict[txt])
            @query_complete.style.display = "none"
        )

    execute_query: ->
        if not @search_list?
            @init_search_list()
        key = @query_input.value
        matched = get_matched_items(key, @search_list)
        if matched.length == 1
            @set_timezone(@zone_dict[matched[0]])

class WelcomeFormItem extends Widget
    ACCOUNTS = "com.deepin.daemon.Accounts"
    constructor: (@id)->
        super
        @tooltip = null
        @valid = {}
        @account_dbus = null
        @value_origin = null
        @change = false
        @msg_tid = null

        @input = create_element("input", "input_#{@id}", @element)
        @fill_widget()
        @input.addEventListener("focus", (e) =>
            @check_capslock()
        )
        @input.addEventListener("blur", (e) =>
            @check_capslock()
            @input.setAttribute("style", "")
            if @check_valid()
                @destroy_tooltip()
            else
                setTimeout(=>
                    msg = @valid.msg
                    if @id == "confirmpassword"
                        if Widget.look_up("password").input.value.length != 0
                            msg = _("The two passwords do not match.")
                        else msg = ""
                    @set_tooltip(msg)
                ,500)
        )
        @input.addEventListener("input", (e) =>
            switch @id
                when "username"
                    @input.value = @input.value.toLowerCase()
                    if @check_valid()
                        @destroy_tooltip()
                    else
                        if @valid.code in [illegalErrCode.invalid,illegalErrCode.firstUpper]
                            @input.value = @value_origin
                            @set_tooltip(@valid.msg)
                        else if @valid.code in [illegalErrCode.systemUsed,illegalErrCode.exist]
                            @set_tooltip(@valid.msg)
                    if Widget.look_up("account")?.hostname.change == false
                        Widget.look_up("account")?.hostname.input.value = @input.value + "-pc"
                when "password"
                    @destroy_tooltip()
                    confirm = Widget.look_up("confirmpassword")
                    if @input.value != confirm.input.value and confirm.change
                        msg = _("The two passwords do not match.")
                        confirm.set_tooltip(msg)
                    else
                        confirm.destroy_tooltip()
                when "confirmpassword"
                    clearTimeout(@msg_tid)
                    if not @check_valid() and @valid.code in [illegalErrCode.different]
                        @set_tooltip(@valid.msg)
                    else
                        @destroy_tooltip()

            @value_origin = @input.value
            Widget.look_up("account")?.check_start_ready()
        )
        @input.addEventListener("change", (e) =>
            @change = true
        )
        @input.addEventListener("keydown", (e) =>
            @check_capslock()
            if @id == "confirmpassword" and e.keyCode == 13
                Widget.look_up("account")?.start_install_cb()
        )

    set_tooltip: (text) ->
        if text is null or text is "" then return
        @input.setAttribute("style", "border:2px solid #F79C3B;border-radius:4px;background-position:-2px -2px;")
        if @tooltip == null
            @tooltip = new ArrowToolTip(@input, text)
            @input.removeEventListener('mouseover', @tooltip.on_mouseover)
            @input.removeEventListener('mouseout', @tooltip.hide)
            @tooltip.set_delay_time(0)
        @tooltip.set_text(text)
        @tooltip.show()
        pos = @tooltip.get_xy()
        ArrowToolTip.move_to(@tooltip, pos.x - tooltipOffsetX, pos.y - tooltipOffsetY - 10)

    destroy_tooltip:->
        @input.setAttribute("style", "border:2px solid rgba(255,255,255,0.6);border-radius:4px;background-position:-2px -2px;")
        @tooltip?.hide()
        @tooltip?.destroy()
        @tooltip = null

    check_valid: ->
        @valid = @get_valid_msg_code()
        return @valid.valid

    get_valid_msg_code: ->
        console.debug "[get_valid_msg_code]:value:===#{@input.value}---"
        if not @account_dbus? then @account_dbus = DCore.DBus.sys(ACCOUNTS)
        switch @id
            when "username"
                val = @account_dbus.IsUsernameValid_sync(@input.value)
                #TODO:this is only a temporary solution
                #fixed the deepin username in pxe is invalid
                if not val[0]
                    user_path = @account_dbus.FindUserByName_sync(@input.value)
                    if user_path != ""
                        return {valid:true,msg:"",code:val[2]}
                return {valid:val[0],msg:DCore.dgettext("dde-daemon",val[1]),code:val[2]}
            when "hostname"
                #TODO:The Account dbus must provide a function to check hostname
                if not @input.value? or @input.value.length == 0
                    return {valid:false,msg:_("Computer name can not be empty."),code:illegalErrCode.empty}
                else
                    return {valid:true,msg:"",code:-1}
            when "password"
                #TODO:The Account dbus must provide a function to check password
                if not @input.value? or @input.value.length == 0
                    return {valid:false,msg:_("Password can not be empty."),code:illegalErrCode.empty}
                else if not @account_dbus.IsPasswordValid_sync(@input.value)
                    return {valid:false,msg:_("Invalid password."),code:illegalErrCode.invalid}
                else
                    return {valid:true,msg:"",code:-1}
            when "confirmpassword"
                pwd = Widget.look_up("password")?.input.value
                if 0 < @input.value.length <= pwd.length and pwd.indexOf(@input.value) == 0
                    if @input.value != pwd
                        return {valid:false,msg:"",code:-1}
                    else
                        return {valid:true,msg:"",code:-1}
                else
                    if @input.value.length == 0
                        return {valid:false,msg:"",code:-1}
                    else
                        return {valid:false,msg:_("The two passwords do not match."),code:illegalErrCode.different}
            else
                return {valid:true,msg:"",code:-1}

    get_input_value: ->
        return @input.value

    check_capslock: ->
        if @id == "password" or @id == "confirmpassword"
            if DCore.Installer.detect_capslock()
                @warn.style.display = "block"
            else
                @warn.style.display = "none"

    fill_widget: ->
        if @id == "username"
            username_holder = _("Username")
            @input.setAttribute("placeholder", username_holder)
        else if @id == "hostname"
            hostname_holder = _("Computer Name")
            @input.setAttribute("placeholder", hostname_holder)
        else if @id == "password"
            password_holder = _("Password")
            @input.setAttribute("placeholder", password_holder)
            @input.classList.add("PasswordStyle")
            @warn = create_element("div", "CapsWarning", @element)
        else if @id == "confirmpassword"
            confirm_holder = _("Retype Password")
            @input.setAttribute("placeholder", confirm_holder)
            @input.classList.add("PasswordStyle")
            @warn = create_element("div", "CapsWarning", @element)

    input_focus: ->
        @input.setAttribute("autofocus","autofocus")
        @input.focus()


class Account extends Widget
    constructor: (@id) ->
        super
        @form = create_element("div", "WelcomeForm", @element)

        @username = new WelcomeFormItem("username")
        @form.appendChild(@username.element)

        @hostname = new WelcomeFormItem("hostname")
        @form.appendChild(@hostname.element)

        @password = new WelcomeFormItem("password")
        @form.appendChild(@password.element)

        @confirmpassword = new WelcomeFormItem("confirmpassword")
        @form.appendChild(@confirmpassword.element)

        @next_step = new NextStep("start",_("Next"),@start_install_cb)
        @element.appendChild(@next_step.element)
        @next_step.set_pos("absolute","260px","60px")
        @next_step.next_bt_disable()

        @username.input_focus()

    show: ->
        __timezone_widget?.hide()
        __keyboard_widget?.hide()
        @element.style.display = "block"

    hide: ->
        @element.style.display = "none"

    check_start_ready: ->
        if !__init_parted_finish
            return false
        if @username.check_valid() and @hostname.check_valid() and @password.check_valid() and @confirmpassword.check_valid()
            @next_step?.next_bt_enable()
            return true
        else
            @next_step?.next_bt_disable()
            return false

    fill_item_data: ->
        __selected_username = @username.get_input_value()
        __selected_hostname = @hostname.get_input_value()
        __selected_password = @password.get_input_value()
        echo "#{__selected_username};#{__selected_hostname};"

    start_install_cb: =>
        @fill_item_data()
        if @check_start_ready()
            undo_part_table_info()
            @username.destroy_tooltip()
            @hostname.destroy_tooltip()
            @password.destroy_tooltip()
            @confirmpassword.destroy_tooltip()
            pc.switch_page(new Part("Part"))
            __selected_item?.focus()
        else
            if __init_parted_finish
                @username.check_valid()
                @hostname.check_valid()
                @password.check_valid()
                @confirmpassword.check_valid()

class Welcome extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_account.png", @titleprogress)

        @title_set = create_element("div", "TitleSet", @title)

        @timezone_set = create_element("div", "TimezoneSet", @title_set)
        @timezone_set.setAttribute("id","TimezoneSet")
        @timezone_set.innerText = _("Time Zone", "INSTALLER")
        timezoneSet_div = @timezone_set

        @keyboard_set = create_element("div", "KeyboardSet", @title_set)
        @keyboard_set.setAttribute("id","KeyboardSet")
        @keyboard_set.innerText = _("Keyboard")
        keyboardSet_div = @keyboard_set

        @wrap = create_element("div", "WelcomeWrap", @element)

        @keyboard = new Keyboard("keyboard")
        @wrap.appendChild(@keyboard.element)
        __keyboard_widget = @keyboard

        @timezone = new Timezone("timezone")
        @wrap.appendChild(@timezone.element)
        __timezone_widget = @timezone

        @account = new Account("account")
        @wrap.appendChild(@account.element)
        @account.show()
        __account_widget = @account

    do_click: (e) ->
        if is_ancestor(@keyboard_set, e.target)
            if @keyboard.displayed
                @account.show()
            else
                @keyboard.show()
        else if is_ancestor(@timezone_set, e.target)
            if @timezone.displayed
                @account.show()
            else
                @timezone.show()
        else
            if @keyboard.displayed
                if not is_ancestor(@keyboard.element, e.target)
                    @account.show()
            if @timezone.displayed
                if not is_ancestor(@timezone.element, e.target)
                    @account.show()

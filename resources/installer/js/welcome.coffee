#Copyright (c) 2011 ~ 2013 Deepin, Inc.
#              2011 ~ 2013 yilang
#
#Author:      LongWei <yilang2007lw@gmail.com>
#Maintainer:  LongWei <yilang2007lw@gmail.com>
#             Xu Shaohua <xushaohua@linuxdeepin.com>
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

# TODO: remove this variable
__legal_keys='abcdefghijklmnopqrstuvwxyz' + '0123456789' + '-_'

ErrorCode =
    EMPTY:       1
    INVALID:     2
    FIRST_UPPER: 3
    EXIST:       4
    SYSTEM_USED: 5
    NOT_MATCH:   6

__selected_zone_index = 8

# TODO: rename it to `timezone`
__database = JSON.parse(timezone_json)

_sort_layout = (layout_a, layout_b) ->
    a_desc = DCore.Installer.get_layout_description(layout_a)
    b_desc = DCore.Installer.get_layout_description(layout_b)
    return a_desc.localeCompare(b_desc)

# TODO: move this function to `utils` module
get_matched_items = (key, list) ->
    matched = []
    for item in list
        if item.toLowerCase().indexOf(key.toLowerCase()) != -1
            matched.push(item)
    return matched

# TODO: move this function to `utils` module
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
    console.log("[welcome.coffee] update_keyboard_text() current layout: #{current_layout}")
    keyboardSet_div?.innerText = current_layout
    keyboardSet_div?.title = current_layout

    x = keyboardSet_div?.offsetLeft - 15
    tri?.style.left = x if x > 0

    DCore.Installer.set_layout(__selected_layout)

update_timezone_text = (tri) ->
    utc = DCore.Installer.get_timezone_utc(__selected_timezone)
    console.log("[welcome.coffee] update_timezone_text(), current timezone: #{utc}")
    timezoneSet_div?.title = utc

    try
        city = __selected_timezone.split("/").pop()
    catch
        city = __selected_timezone
    right = DCore.dgettext("tzdata", city)
    if not right? then right = city
    timezoneSet_div?.innerText = right

    x = timezoneSet_div?.offsetLeft - 15
    tri?.style.left = x if x > 0

# Keyboard Layout wrapper
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

# Keyboard Variant
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
        console.log("[welcome.coffee] KeyboardDetectDialog.process_have_key(), detect dialog process have key")
        step = null
        r = null
        try
            step = DCore.Installer.keyboard_detect_get_present()
        catch error
            console.error("[welcome.coffee] call keyboard_detect_get_present() error: #{error}")
        if step?
            try
                r = DCore.Installer.keyboard_detect_read_step(step)
            catch error
                console.error("[welcome.coffee] call keyboard_detect_read_step() error: #{error}")
        if r?
            @keyboard.process_keyboard_detect(r)
        else
            @hide_dialog()

    process_no_have_key: ->
        console.log("[welcome.coffee] KeyboardDetectDialog.process_no_have_key() detect dialog process no have key")
        step = null
        r = null
        try
            step = DCore.Installer.keyboard_detect_get_not_present()
        catch error
            console.error("[welcome.coffee] call keyboard_detect_get_not_present() error: #{error}")
        if step?
            try
                r = DCore.Installer.keyboard_detect_read_step(step)
            catch error
                console.error("[welcome.coffee] call keyboard_detect_read_step() error: #{error}")
        if r?
            @keyboard.process_keyboard_detect(r)
        else
            @hide_dialog()

    process_key_press: (e) =>
        console.log("[welcome.coffee] KeyboardDetectDialog.process_key_process() detect dialog process key press")
        keycodes = null
        step = null
        r = null
        code = DCore.Installer.get_keycode_from_keysym(e.which) - 8
        if code > 255 or code < 0
            console.eror("[welcome.coffee] call get_keycode_from_keysym() error: got invalid code #{code}")
            return
        try
            keycodes = DCore.Installer.keyboard_detect_get_keycodes()
        catch error
            console.error("[welcome.coffee] call keyboard_detect_get_keycodes() error: #{error}")
        if keycodes?
            try
                step = keycodes[code]
            catch error
                console.error("[welcome.coffee] KeyboardDetectDialog.process_key_press() error, failed to get `step`: #{error}")
            if step?
                try
                    r = DCore.Installer.keyboard_detect_read_step(step)
                catch error
                    console.error("[welcome.coffee] call keyboard_detect_read_step() error: #{error}")
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
            console.error("[welcome.coffee] call keyboard_detect_get_symbols() error: #{error}")
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
            console.warn("[welcome.coffee] KeyboardDetectDialog.update_type() invalid type: #{type}")
            @hide_dialog()

    #TODO: Should this directly inherit from Dialog ?
    show_at: (parent)->
        parent.appendChild(@element)

    show_dialog: ->
        __in_model = true
        __board.setAttribute("style", "display:block")
        @element.style.display = "block"
        enable_tab(@element)
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
        ArrowToolTip.container?.style.display = "none"

    hide: ->
        @displayed = false
        @element.style.display = "none"
        #ArrowToolTip.container?.style.display = "none"

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
            console.warn("[welcome.coffee] Keyboard.set_to_layout() invalid layout, layout: #{layout}, lay_var: #{lay_var}")

    detect_keyboard_cb: (e)=>
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
            console.warn("[welcome.coffee] Keyboard.process_keyboard_detect() invalid keyboard type: #{r}")

    handle_detect_result: ->
        detect_result = DCore.Installer.keyboard_detect_get_result()
        layout = detect_result.split(":").join(",")
        console.log("[welcome.coffee] Keyboard.handle_detect_result() layout: #{layout}")
        @set_to_layout(layout)


class Timezone extends Widget
    constructor: (@id) ->
        super
        @tri = create_element("div", "TimezoneTri", @element)

        @query = create_element("div", "Query", @element)
        @query_div = create_element("div", "Left", @query)
        @query_wrap = create_element("div", "QueryWrap", @query_div)
        @query_input = create_element("input", "", @query_wrap)
        @query_input.setAttribute("placeholder",
                                  _("Please select or search your location."))
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
        @img.addEventListener("click", (e) =>
            @hide_query_complete()
        )
        @construct_map()
        @hide()

        update_timezone_text(@tri)
        # Mark default timezone on timezone map.
        @set_timezone(__selected_timezone, false)

        # Set this flag to true if timezone is changed by user
        @timezone_changed = false

        @guessTimeZone((timezone) =>
            console.log("[welcome.coffee] Timezone guessTimezone returns: #{timezone}")
            if not @timezone_changed
                console.log("[welcome.coffee] Timezone change current timezone based on IP address: #{timezone}")
                @set_timezone(timezone, false)
        )

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
                console.error("[welcome.coffee] call dgettext() error: #{error}")

    show: ->
        __keyboard_widget?.hide()
        __account_widget?.hide()
        @displayed = true
        @element.style.display = "block"
        enable_tab(@query_input)
        ArrowToolTip.container?.style.display = "none"

    hide: ->
        @displayed = false
        @element.style.display = "none"
        disable_tab(@query_input)
        ArrowToolTip.container?.style.display = "none"

    construct_map: ->
        @imagemap = create_element("map", "", @element)
        @imagemap.setAttribute("name", "ImageMap")
        @imagemap.setAttribute("id", "ImageMap")
        for key, val of __database
            @construct_area(key)

    construct_area: (key) ->
        area = create_element("area", "TimezoneArea", @imagemap)
        disable_tab(area)
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

    show_pin: (area, show_tooltip=true) ->
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

        text = area.getAttribute("data-timezone").split("/").pop()

        if show_tooltip
            if not @tooltip?
                @tooltip = new ArrowToolTip(@pin, _(text,"tzdata"), false)
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

    set_timezone: (timezone, show_tooltip=true) ->
        area = @get_area(timezone)
        console.log("[welcome.coffee] Timezone.set_timezone() timezone: #{timezone}")
        if area?
            @draw_timezone(area)
            @show_pin(area, show_tooltip)
            __selected_timezone = timezone
            @timezone_changed = true
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
        console.log("[welcome.coffee.Timezone] show_query_complete()")
        if @query_complete?
            @query_div.removeChild(@query_complete)
            @query_complete = null
        @query_complete = create_element("div", "QueryComplete", @query_div)
        if not @search_list?
            @init_search_list()
        key = @query_input.value
        matched = get_matched_items(key, @search_list)
        if key.length > 0 and matched.length > 0
            @query_complete.style.display = "block"
            for item in matched
                @create_complete_item(item)
        else
            @query_complete.style.display = "none"

    hide_query_complete: ->
        console.log("[welcome.coffee.Timezone] hide_query_complete()")
        @query_complete?.style.display = "none"

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

    guessTimeZone: (callback) ->
        url = "http://freegeoip.net/json/"
        ajax(url, true, (xhr) =>
            info = JSON.parse(xhr.response)
            console.log("[welcome.coffee] Timezone guessTimezone() ip info: #{xhr.response}")
            if info and info["time_zone"]
                callback(info["time_zone"])
        )

class WelcomeFormItem extends Widget
    ACCOUNTS = "com.deepin.daemon.Accounts"
    constructor: (@id)->
        super
        # Global tooltip object
        @tooltip = null

        # To mark content validation status
        @valid = {ok: false, msg: _("Empty"), code: ErrorCode.EMPTY}

        @account_dbus = DCore.DBus.sys(ACCOUNTS)

        # Content of input form before user-changed
        @value_origin = null

        # To mark content of <input> is setup or not
        @changed = false
        @msg_tid = null
        @status = {}

        @input = create_element("input", "input_#{@id}", @element)

        @fill_widget()

        @input.addEventListener("focus", (e) =>
            @check_capslock()
        )
        @input.addEventListener("blur", (e) =>
            @check_capslock()

            switch @id
                when "username"
                    @validateUsername(@input.value)
                when "hostname"
                    @validateHostname(@input.value, false)
                when "password"
                    @validatePassword(@input.value, false)
                when "confirmpassword"
                    @validateConfirmPassword(@input.value, false)

            if @valid.ok
                @destroy_tooltip()
            else
                @next_step.next_bt_disable()
                @set_tooltip(@valid.msg)
        )

        @input.addEventListener('input', (e) =>
            switch @id
                when "username"
                    @input.value = @input.value.toLowerCase()
                    @validateUsername(@input.value)
                    if @valid.ok
                        @destroy_tooltip()
                        # Setup hostname based on username
                        hostnameItem = Widget.look_up("hostname")
                        if hostnameItem.changed == false
                            hostnameItem.input.value = @input.value + "-pc"
                            # Reset validation status of hostname element.
                            hostnameItem.valid = {ok: true, msg: "", code: 0}
                    else
                        if @valid.code in [ErrorCode.INVALID, ErrorCode.FIRST_UPPER]
                            @input.value = @value_origin
                        if @input.value.length == 0
                            hostnameItem = Widget.look_up("hostname")
                            if hostnameItem.changed == false
                                hostnameItem.input.value = ""
                        @set_tooltip(@valid.msg)

                when "hostname"
                    @validateHostname(@input.value, true)
                    if @valid.ok
                        @destroy_tooltip()
                    else
                        @set_tooltip(@valid.msg)

                when "password"
                    @validatePassword(@input.value, true)
                    if @valid.ok
                        @destroy_tooltip()
                    else if @valid.code == ErrorCode.PARTICAL_MATCH
                        @destroy_tooltip()
                    else
                        @set_tooltip(@valid.msg)
                    confirm = Widget.look_up("confirmpassword")
                    if @input.value != confirm.input.value and confirm.changed
                        confirm.valid =
                            ok: false
                            msg: _("The two passwords do not match.")
                            code: ErrorCode.NOT_MATCH
                        confirm.set_tooltip(confirm.valid.msg)
                    else
                        confirm.destroy_tooltip()

                when "confirmpassword"
                    @validateConfirmPassword(@input.value, true)
                    if @valid.ok
                        @destroy_tooltip()
                    # If `confirmPassword` matches first half of `password`
                    # ignore this error.
                    else if @valid.code == ErrorCode.PARTICAL_MATCH
                        @destroy_tooltip()
                    else
                        @set_tooltip(@valid.msg)

            @value_origin = @input.value
            Widget.look_up("account")?.check_start_ready()
        )
        @input.addEventListener("change", (e) =>
            @changed = true
        )
        @input.addEventListener('keydown', (e) =>
            @check_capslock()
            if @id == "confirmpassword" and e.keyCode == 13
                Widget.look_up("account")?.start_install_cb()
        )

    validateUsername: (username) =>
        val = @account_dbus.IsUsernameValid_sync(username)
        if val[0]
            @valid = {ok: true, msg: "", code: 0}
            return
        user_val = []
        try
            user_val = @account_dbus.FindUserByName_sync(username)
            if user_val[0] != ""
                @valid = {ok: true, msg: "", code: 0}
                return
        catch error
            console.error("[welcome.coffee] call FindUserByName_sync() error: #{error}")
        @valid =
            ok: false
            msg: DCore.dgettext("dde-daemon", val[1])
            code: val[2]

    #TODO: move these two validate functions to dde-api
    doValidateHostnameTmp: (hostname) =>
        regexp = /^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9\-\.])*$/
        return regexp.test(hostname)

    doValidateHostname: (hostname) =>
        regexp = /^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$/
        return regexp.test(hostname)

    validateHostname: (hostname, temp) =>
        if hostname.length > 0 and ((temp and @doValidateHostnameTmp(hostname)) or @doValidateHostname(hostname))
            @valid = {ok: true, msg: "", code: 0}
        else
            @valid =
                ok: false
                msg: _("Computer name is invald.")
                code: ErrorCode.INVALID

    validatePassword: (password, temp) =>
        # Clear validation status of confirmPassword item.
        confirmPasswordItem = Widget.look_up("confirmpassword")
        # Make sure that confirmPassword is changed by user explicitly.
        if confirmPasswordItem.changed
            confirmPasswordItem.valid = {ok: true, msg:"", code: 0}
        confirmPassword = confirmPasswordItem.input.value
        if not password? or password.length == 0
            @valid =
                ok: false
                msg: _("Password can not be empty.")
                code: ErrorCode.EMPTY
        else if password == confirmPassword
            @valid = {ok: true, msg: "", code: 0}
        # Check if user input password in `confirmpassword` element, then input
        # password again in `password` element.
        else if temp and confirmPassword.substring(0, password.length) == password
            @valid =
                ok: false
                msg: "Partial match"
                code: ErrorCode.PARTICAL_MATCH
        else
            @valid = {ok: true, msg: "", code: 0}

    validateConfirmPassword: (confirmPassword, temp) =>
        # Clear validation status of passwordItem
        passwordItem = Widget.look_up("password")
        passwordItem.valid = {ok: true, msg: "", code: 0}
        password = passwordItem.input.value
        if password == confirmPassword
            @valid = {ok: true, msg: "", code: 0}
        # Check `confirmPassword` matches first half of `password`.
        else if temp and password.substring(0, confirmPassword.length) == confirmPassword
            @valid =
                ok: false
                msg: "Partial match"
                code: ErrorCode.PARTICAL_MATCH
        else
            @valid =
                ok: false
                msg: _("The two passwords do not match.")
                code: ErrorCode.NOT_MATCH

    set_tooltip: (text) ->
        if text is null or text is "" then return
        if @tooltip == null
            @tooltip = new ArrowToolTip(@input, text, true)
            @input.removeEventListener('mouseover', @tooltip.on_mouseover)
            @input.removeEventListener('mouseout', @tooltip.hide)
            @tooltip.set_delay_time(0)
        @tooltip.set_text(text)
        @tooltip.show()
        pos = @tooltip.get_xy()
        ArrowToolTip.move_to(@tooltip, 228, pos.y - tooltipOffsetY - 95)

    destroy_tooltip:->
        # TODO: do not destroy tooltip object manually
        @tooltip?.hide()
        @tooltip?.destroy()
        @tooltip = null

    get_input_value: ->
        return @input.value

    check_capslock: ->
        if @id == "password" or @id == "confirmpassword"
            if DCore.Installer.detect_capslock()
                @warn.style.display = "block"
            else
                @warn.style.display = "none"

    fill_widget: ->
        # Setup placeholder text of <input> elements
        switch @id
            when "username"
                username_holder = _("Username")
                @input.setAttribute("placeholder", username_holder)
            when "hostname"
                hostname_holder = _("Computer Name")
                @input.setAttribute("placeholder", hostname_holder)
            when "password"
                password_holder = _("Password")
                @input.setAttribute("placeholder", password_holder)
                @input.classList.add("PasswordStyle")
                @warn = create_element("div", "CapsWarning", @element)
            when "confirmpassword"
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

        # Username <input>
        @username = new WelcomeFormItem("username")
        @username.input.setAttribute("maxLength",32)
        @username.valid =
            ok: false
            msg: _("Username can not be empty.")
            code: ErrorCode.EMPTY
        @form.appendChild(@username.element)

        # Hostname <input>
        @hostname = new WelcomeFormItem("hostname")
        @hostname.valid =
            ok: false
            msg: _("Computer name can not be empty."),
            code: ErrorCode.EMPTY
        @form.appendChild(@hostname.element)

        # Password <input>
        @password = new WelcomeFormItem("password")
        @password.valid =
            ok: false
            msg: _("Password can not be empty.")
            code: ErrorCode.EMPTY
        @form.appendChild(@password.element)


        # ConfirmPassword <input>
        @confirmpassword = new WelcomeFormItem("confirmpassword")
        @confirmpassword.valid =
            ok: false
            msg: _("Password cannot be empty.")
            code: ErrorCode.EMPTY
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
        if DEBUG then __init_parted_finish = true
        if !__init_parted_finish
            return false
        console.log("[welcome.coffee] Account check_start_ready():", @username.valid,
            @hostname.valid, @password.valid, @confirmpassword.valid)
        if @username.valid.ok and @hostname.valid.ok and
                @password.valid.ok and @confirmpassword.valid.ok
            @next_step?.next_bt_enable()
            return true
        else
            @next_step?.next_bt_disable()
            return false

    fill_item_data: ->
        __selected_username = @username.get_input_value()
        __selected_hostname = @hostname.get_input_value()
        __selected_password = @password.get_input_value()
        console.log("[welcome.coffee] Account.fill_item_data() selected_username: #{__selected_username}, selected_hostname: #{__selected_hostname}")

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

class Welcome extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_account.png",
                               @titleprogress)

        @title_set = create_element("div", "TitleSet", @title)

        @timezone_set = create_element("div", "TimezoneSet", @title_set)
        @timezone_set.setAttribute("id","TimezoneSet")
        enable_tab(@timezone_set)
        @timezone_set.innerText = _("Time Zone", "INSTALLER")
        timezoneSet_div = @timezone_set

        @keyboard_set = create_element("div", "KeyboardSet", @title_set)
        @keyboard_set.setAttribute("id","KeyboardSet")
        enable_tab(@keyboard_set)
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

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

__selected_layout = "cn"
__selected_layout_item = null
__selected_variant_item = null
__focused_layout_item = null

__selected_timezone = "Asia/Shanghai"
__selected_username = null
__selected_hostname = null
__selected_password = null
__illegal_keys='\t\n\r~`!@#$%^&*()+}{|\\\':;<,>.?/'

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

class RequireMatchDialog extends Dialog
    constructor: (@id) ->
        super(@id, false, @require_match_cb)
        @add_css_class("DialogCommon")
        @title_txt.innerText = _("Installation requirements")
        @format_tips = create_element("p", "", @content)
        @format_tips.innerText = _("To install Linux Deepin, you need to have at least 15GB disk space.")

    require_match_cb: ->
        echo "require match cb"
        DCore.Installer.finish_install()

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
        @title = create_element("div", "DialogTitle", @element)
        @title_txt = create_element("div", "DialogTxt", @title)
        @title_close = create_element("div", "DialogClose", @title)
        @title_close.addEventListener("click", (e) =>
            @hide_dialog()
        )
        @title_txt.innerText = _("Detect keyboard layout")

        @content = create_element("div", "DialogContent", @element)
        @desc = create_element("div", "Desc", @content)
        @detail = create_element("div", "Detail", @content)

        @foot = create_element("div", "DialogBtn", @element)
        @ok = create_element("div", "", @foot)
        @ok_input = create_element("input", "InputBtn", @ok)
        @ok_input.setAttribute("type", "submit")
        value = _("YES")
        @ok_input.setAttribute("value", value)
        @ok.addEventListener("click", (e) =>
            @process_have_key()
        )
        @cancel = create_element("div", "", @foot)
        @cancel_input = create_element("input", "InputBtn", @cancel)
        @cancel_input.setAttribute("type", "submit")
        value = _("NO")
        @cancel_input.setAttribute("value", value)
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
            @desc.innerText = _("Please press the following key")
            @fill_detail(symbols)
            @show_dialog()
        else if type == "have"
            @type = type
            @foot.style.display = "block"
            @desc.innerText = _("Confirm you have the following key")
            @fill_detail(symbols)
            @show_dialog()
        else if type == "result"
            @hide_dialog()
        else
            echo "invalid type"
            @hide_dialog()

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
        @layout_list = create_element("div", "LayoutList", @content)
        @variant_list = create_element("div", "VariantList", @content)

        @op = create_element("div","KeyBoardOp", @element)
        @active_btn = create_element("div", "PartBtnActive", @op)
        @active_btn.setAttribute("id", "active_btn")
        @active_input = create_element("input", "InputBtn", @active_btn)
        @active_input.setAttribute("type", "submit")
        @active_input.setAttribute("value", _("Display Current"))
        @active_btn.addEventListener("click", (e) =>
            if __selected_layout?
                @set_to_layout(__selected_layout)
        )

        @detect_btn = create_element("div", "PartBtnActive", @op)
        @detect_input = create_element("input", "InputBtn", @detect_btn)
        @detect_input.setAttribute("type", "submit")
        @detect_input.setAttribute("value", _("Detect Layout"))
        @detect_btn.addEventListener("click", (e) =>
            @detect_keyboard_cb(e)
        )
        #@current = create_element("div", "Current", @element)
        #@current.innerText = DCore.Installer.get_layout_description(__selected_layout)

        @fill_layouts(@layouts)
        @hide()
        @init_dbus_query()

    show: ->
        #echo "keyboard show"
        __timezone_widget?.hide()
        __account_widget?.hide()
        @displayed = true
        update_el_attr(@element, "-webkit-transform", "translateX(-725px)")
        #update_el_attr(@element, "-webkit-transition", "all 0.3s cubic-bezier(0, 0, 0.35, -1)")
        __selected_layout_item?.focus()

    hide: ->
        #echo "keyboard hide"
        @displayed = false
        update_el_attr(@element, "-webkit-transform", "translateX(0px)")

    init_dbus_query: ->
        try
            @search_bus = DCore.DBus.session_object("com.deepin.api.Search", "/com/deepin/api/Search", "com.deepin.api.Search")
            dict = {}
            for item in @layouts
                dict[item] = DCore.Installer.get_layout_description(item)
            @search_handle = @search_bus.NewTrieWithString_sync(dict, "installer")
        catch
            echo "init dbus query failed"

    init_query_ul: ->
        for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            @create_query_li(c)

    create_query_li:(c) ->
        li = create_element("li", "", @query_ul)
        a = create_element("a", "", li)
        a.setAttribute("href", "#")
        a.innerText = c
        a.addEventListener("click", (e) =>
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

        #@search_list = []
        #for item in @layouts
        #    @search_list.push(item)
        #    @search_list.push(DCore.Installer.get_layout_description(item))

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
        #@current.innerText = DCore.Installer.get_layout_description(layout)
        __selected_layout = layout

    execute_letter_query: (letter) ->
        try
            matched = @search_bus.SearchKeysByFirstLetter_sync(letter, @search_handle)
        catch error
            echo error
            matched = []
        @layout_list.innerHTML = ""
        @variant_list.innerHTML = ""
        matched_layouts = []
        for item in matched
            if item in @layouts
                matched_layouts.push(item)
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
        document.body.appendChild(@detect_dialog.element)
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
        @search_list = DCore.Installer.get_timezone_list()

        @tri = create_element("div", "TimezoneTri", @element)

        @query = create_element("div", "Query", @element)
        @query_div = create_element("div", "Left", @query)
        @query_wrap = create_element("div", "QueryWrap", @query_div)
        @query_input = create_element("input", "", @query_wrap)
        @query_input.addEventListener("keyup", (e) =>
            if e.which == 13
                @execute_query()
        )
        @query_img = create_element("div", "QueryImg", @query_wrap)
        @query_img.addEventListener("click", (e) =>
            @execute_query()
        )

        @query_txt = create_element("div", "Right", @query)
        @query_txt.innerText = _("Please select or search your location")

        @picker_wrap = create_element("div","TmezoneWrap", @element)
        @picker = create_element("div", "TimezonePicker", @picker_wrap)
        @canvas = create_element("canvas", "TimezoneCanvas", @picker)
        @canvas.setAttribute("width", 700)
        @canvas.setAttribute("height", 370)
        @img = create_img("TimezoneMap", "images/zonemap.png", @picker)
        @img.setAttribute("usemap", "#ImageMap")
        @construct_map()

        @current = create_element("div", "Current", @element)
        @current.innerText = _(__selected_timezone.split("/")[0], "tzdata") + "/" + _(__selected_timezone.split("/")[1], "tzdata")

        @hide()

    show: ->
        __keyboard_widget?.hide()
        __account_widget?.hide()
        @displayed = true
        update_el_attr(@element, "-webkit-transform", "translateX(-725px)")
        #update_el_attr(@element, "-webkit-transition", "all 0.3s cubic-bezier(0, 0, 0.35, -1)")
        #@element.style.display = "block"

    hide: ->
        #echo "timezone hide"
        @displayed = false
        update_el_attr(@element, "-webkit-transform", "translateX(0px)")
        #@element.style.display = "none"

    hover_timezone: (zone) ->
        #@current.innerHTML = "<span>Zone:" + __database[zone]["offset"] + "</span>"
        #@current.innerHTML += "<span>City:" + _(zone.split("/")[0], "tzdata") + "/" + _(zone.split("/")[1], "tzdata") + "</span>"
        #@current.innerHTML += "<span>Country:" + _(__database[zone]["country"], "tzdata") + "</span>"
        @current.innerHTML = "<span>" + _(zone.split("/")[0], "tzdata") + "/" + _(zone.split("/")[1], "tzdata") + "</span>"

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
        )
        area.addEventListener("mouseover", (e) =>
            @draw_timezone(area)
            @hover_timezone(area.getAttribute("data-timezone"))
        )
        area.addEventListener("mouseout", (e) =>
            @destroy_canvas(area)
        )
    
    show_pin: (area) ->
        pin = area.getAttribute("data-pin").split(",")
        @pin?.parentElement?.removeChild(@pin)
        @pin = null
        @pin = create_element("div", "Pin", @picker_wrap)
        @pin_img = create_img("", "images/pin.png", @pin)
        x = parseInt(pin[0]) - 6
        y = parseInt(pin[1]) + 50 - 6
        style = "left:" + x + "px;" + "top:" + y + "px"
        @pin.setAttribute("style", style)

        text = area.getAttribute("data-timezone").split("/")[1]
        if @tip?
            @tip.destroy()
        @tip = new TimezoneToolTip("timezone", _(text, "tzdata"))
        @pin.appendChild(@tip.element)
        left = @tip.content.clientWidth + 2 * (@tip.radius + @tip.xpadding) - 2
        @tip.element.style.top = "-40px"
        @tip.element.style.left = (0 - left / 2) + "px"
        @tip.draw()

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
            @hover_timezone(timezone)

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

    execute_query: ->
        key = @query_input.value
        matched = get_matched_items(key, @search_list)
        if matched.length == 1
            @set_timezone(matched[0])

class WelcomeFormItem extends Widget
    constructor: (@id)->
        super
        @input = create_element("input", "", @element)
        @change = false
        @fill_widget()
        @input.addEventListener("focus", (e) =>
            @input.setAttribute("style", "")
        )
        @input.addEventListener("blur", (e) =>
            @check_valid()
            @fill_item_data()
            Widget.look_up("account")?.check_start_ready()
        )
        @input.addEventListener("input", (e) =>
            if @id == "username" and Widget.look_up("account")?.hostname.change == false
                Widget.look_up("account")?.hostname.input.value = @input.value + "-pc"
            Widget.look_up("account")?.check_start_ready()
        )
        @input.addEventListener("change", (e) =>
            @change = true
        )

    fill_widget: ->
        if @id == "username"
            username_holder = _("User name") 
            @input.setAttribute("placeholder", username_holder)
        else if @id == "hostname"
            hostname_holder = _("Computer name")
            @input.setAttribute("placeholder", hostname_holder)
        else if @id == "password"
            password_holder = _("Password")
            @input.setAttribute("placeholder", password_holder)
            @input.setAttribute("type", "password")
        else if @id == "confirmpassword"
            confirm_holder = _("Retype password")
            @input.setAttribute("placeholder", confirm_holder)
            @input.setAttribute("type", "password")

    fill_item_data: ->
        if @id == "username"
            __selected_username = @input.value
        else if @id == "hostname"
            __selected_hostname = @input.value
        else if @id == "password"
            __selected_password = @input.value

    check_valid: ->
        if not @is_valid() 
            if @input.value.length != 0
                @input.setAttribute("style", "border:2px solid #F79C3B;border-radius:4px;background-position:-2px -2px;")

    is_valid: ->
        if not @input.value? or @input.value.length == 0
            return false
        if @id == "username"
            if @input.value == "deepin"
                return true
            if @input.value in DCore.Installer.get_system_users()
                return false
            for c in @input.value
                if c in __illegal_keys
                    return false
        else if @id == "confirmpassword"
            if @input.value != Widget.look_up("password")?.input.value
                return false
        return true

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

        @start = create_element("div", "Start", @element)
        @start_input = create_element("input", "InputBtn", @start)
        @start_input.setAttribute("type", "submit")
        next = _("Next")
        @start_input.setAttribute("value", next)
        @start.addEventListener("mousedown", (e) =>
            @start_input.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(#F8AD4B), to(#FFC040));color:rgba(0,0,0,1);")
        )
        @start.addEventListener("click", (e) =>
            @start_install_cb()
        )
        @start.setAttribute("style", "pointer-events:none")

    show: ->
        __timezone_widget?.hide()
        __keyboard_widget?.hide()
        apply_animation(@element, "pptin", "0.3s", "linear")
        @element.style.display = "block"

    hide: ->
        apply_animation(@element, "loadingout", "0.3s", "linear")
        setTimeout(=>
            @element.style.display = "none"
        , 300)

    check_start_ready: ->
        if @username.is_valid() and @hostname.is_valid() and @password.is_valid() and @confirmpassword.is_valid() 
            @start.setAttribute("style", "color:#00bdff;pointer-events:auto")
            @start_input.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(#F4C688), to(#FFBE57));color:rgba(0,0,0,1);")
            return true
        else
            @start.setAttribute("style", "pointer-events:none")
            @start_input.setAttribute("style", "background:-webkit-gradient(linear, left top, left bottom, from(rgba(244,198,136, 0.5)), to(rgba(255,190,87,0.5)))")
            return false

    start_install_cb: ->
        if @check_start_ready()
            undo_part_table_info()
            part_page = new Part("part")
            pc.remove_page(welcome_page)
            pc.add_page(part_page)
            __selected_item?.focus()
        else
            @username.check_valid()
            @hostname.check_valid()
            @password.check_valid()
            @confirmpassword.check_valid()


class Welcome extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_account.png", @titleprogress)

        @title_set = create_element("div", "TitleSet", @title)
        @keyboard_set = create_element("div", "KeyboardSet", @title_set)
        @keyboard_set.innerText = _("Keyboard")

        @timezone_set = create_element("div", "TimezoneSet", @title_set)
        @timezone_set.innerText = _("Time Zone", "INSTALLER")

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        @wrap = create_element("div", "WelcomeWrap", @element)

        @keyboard = new Keyboard("keyboard")
        @wrap.appendChild(@keyboard.element)
        __keyboard_widget = @keyboard

        @timezone = new Timezone("timezone")
        @wrap.appendChild(@timezone.element)
        __timezone_widget = @timezone

        @account = new Account("account")
        @wrap.appendChild(@account.element)
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

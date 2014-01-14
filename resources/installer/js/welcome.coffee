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
        @element.setAttribute("class", "VariantItem KeyboardActive")
        __selected_variant_item?.blur()
        __selected_variant_item = @
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

class Keyboard extends Widget
    constructor: (@id)->
        super
        @init_layouts()

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
        @query_txt.innerText = _("Please select your keyboard layout")

        @content = create_element("div", "KeyBoardContent", @element)
        @layout_list = create_element("div", "LayoutList", @content)
        @variant_list = create_element("div", "VariantList", @content)
        @current = create_element("div", "Current", @element)
        @current.innerText = DCore.Installer.get_layout_description(__selected_layout)

        @fill_layouts(@layouts)
        @hide()

    show: ->
        @displayed = true
        @element.style.display = "block"
        __selected_layout_item?.focus()

    hide: ->
        @displayed = false
        @element.style.display = "none"

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

        @search_list = []
        for item in @layouts
            @search_list.push(item)
            @search_list.push(DCore.Installer.get_layout_description(item))

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
        @current.innerText = DCore.Installer.get_layout_description(layout)
        __selected_layout = layout

    execute_query: ->
        key = @query_input.value
        matched = get_matched_items(key, @search_list)
        @layout_list.innerHTML = ""
        @variant_list.innerHTML = ""
        matched_layouts = []
        for item in matched
            if item in @layouts
                matched_layouts.push(item)
        @fill_layouts(matched_layouts)
        if matched_layouts.length > 0
            Widget.look_up("layoutitem_" + matched_layouts[0])?.focus()

class Timezone extends Widget
    constructor: (@id) ->
        super
        @search_list = DCore.Installer.get_timezone_list()

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
        @current.innerText = __selected_timezone

        @hide()

    show: ->
        @displayed = true
        @element.style.display = "block"

    hide: ->
        @displayed = false
        @element.style.display = "none"

    update_timezone: (zone) ->
        @current.innerHTML = "<span>Zone:" + __database[zone]["offset"] + "</span>"
        @current.innerHTML += "<span>City:" + _(zone) + "</span>"
        @current.innerHTML += "<span>Country:" + _(__database[zone]["country"]) + "</span>"
        __selected_timezone = zone

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
            @update_timezone(area.getAttribute("data-timezone"))
        )
        area.addEventListener("mouseover", (e) =>
            @draw_timezone(area)
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
        x = parseInt(pin[0]) - 7
        y = parseInt(pin[1]) + 50 - 7
        style = "left:" + x + "px;" + "top:" + y + "px"
        @pin.setAttribute("style", style)

        text = area.getAttribute("data-timezone").split("/")[1]
        if @tip?
            @tip.destroy()
        @tip = new TimezoneToolTip("timezone", text)
        @pin.appendChild(@tip.element)
        left = @tip.content.clientWidth + 2 * (@tip.radius + @tip.xpadding) - 2
        @tip.element.style.top = "-40px"
        @tip.element.style.left = (0 - left / 2) + "px"
        @tip.draw()

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
            @update_timezone(timezone)

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
        @fill_widget()
        @input.addEventListener("focus", (e) =>
            @input.setAttribute("style", "")
        )
        @input.addEventListener("blur", (e) =>
            @check_valid()
            @fill_item_data()
            Widget.look_up("welcome")?.check_start_ready()
        )
        @input.addEventListener("input", (e) =>
            if @id == "username"
                Widget.look_up("welcome")?.hostname.input.value = @input.value + "-pc"
            Widget.look_up("welcome")?.check_start_ready()
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
            @input.setAttribute("style", "border:2px solid #F79C3B;border-radius:4px;")

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

class Welcome extends Page
    constructor: (@id)->
        super
        @titleimg = create_img("", "images/progress_account.png", @titleprogress)

        @title_set = create_element("div", "TitleSet", @title)
        @keyboard_set = create_element("div", "KeyboardSet", @title_set)
        @keyboard_set.innerText = _("Keyboard")
        @keyboard_set.addEventListener("click", (e) =>
            if @keyboard.displayed
                @display_account()
            else
                @display_keyboard()
        )
        @keyboard_glue = create_element("div", "TitleGlue", @keyboard_set) 
        @keyboard_glue.style.display = "none"

        @timezone_set = create_element("div", "TimezoneSet", @title_set)
        @timezone_set.innerText = _("Time Zone")
        @timezone_set.addEventListener("click", (e) =>
            if @timezone.displayed
                @display_account()
            else
                @display_timezone()
        )
        @timezone_glue = create_element("div", "TitleGlue", @timezone_set) 
        @timezone_glue.style.display = "none"

        @close = create_element("div", "Close", @title)
        @close.addEventListener("click", (e) =>
            @exit_installer()
        )

        @keyboard = new Keyboard("keyboard")
        @element.appendChild(@keyboard.element)

        @timezone = new Timezone("timezone")
        @element.appendChild(@timezone.element)

        @account = create_element("div", "", @element)

        @form = create_element("div", "WelcomeForm", @account)

        @username = new WelcomeFormItem("username")
        @form.appendChild(@username.element)

        @hostname = new WelcomeFormItem("hostname")
        @form.appendChild(@hostname.element)

        @password = new WelcomeFormItem("password")
        @form.appendChild(@password.element)

        @confirmpassword = new WelcomeFormItem("confirmpassword")
        @form.appendChild(@confirmpassword.element)

        @start = create_element("div", "Start", @account)
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

    display_account: ->
        @keyboard.hide()
        @timezone.hide()
        @account.style.display = "block"
        @keyboard_set.setAttribute("class", "KeyboardSet")
        @timezone_set.setAttribute("class", "TimezoneSet")
        @keyboard_glue.style.display = "none"
        @timezone_glue.style.display = "none"

    hide_account: ->
        @account.style.display = "none"

    display_keyboard: ->
        @hide_account()
        @timezone.hide()
        @keyboard.show()
        @timezone_set.setAttribute("class", "TimezoneSet") 
        @keyboard_set.setAttribute("class", "KeyboardSet TitleSetActive")
        @keyboard_glue.style.display = "block"
        @timezone_glue.style.display = "none"

    display_timezone: ->
        @hide_account()
        @keyboard.hide()
        @timezone.show()
        @keyboard_set.setAttribute("class", "KeyboardSet")
        @timezone_set.setAttribute("class", "TimezoneSet TitleSetActive")
        @keyboard_glue.style.display = "none"
        @timezone_glue.style.display = "block"

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
            pc.add_page(part_page)
            pc.remove_page(welcome_page)
            __selected_item?.focus()
        else
            @username.check_valid()
            @hostname.check_valid()
            @password.check_valid()
            @confirmpassword.check_valid()

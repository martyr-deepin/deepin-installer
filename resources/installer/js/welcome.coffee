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

__selected_layout = "es"
__selected_timezone = "Asia/Shanghai"
__username = null
__hostname = null
__password = null
__illegal_keys='\t\n\r~`!@#$%^&*()+}{|\\\':;<,>.?/'

__database = JSON.parse timezone_json

class Keyboard extends Widget
    constructor: (@id)->
        super
        @init_default_layout()
        @current = create_element("div", "Current", @element)
        @current.innerText = __selected_layout

        @list = create_element("div", "KeyBoardList", @element)
        for layout in @get_layouts()
            @construct_item(layout)

    init_default_layout: ->
        lay_var = DCore.Installer.get_current_layout_variant()
        lay = lay_var["layouts"]
        if lay.length > 0
            mylay = lay[0]
        variant = lay_var["variants"]
        if variant.length > 0
            myvar = variant[0]
        if myvar.length > 0
            current = mylay + "," + myvar
        else
            current = mylay
        __selected_layout = current

    get_layouts: ->
        layouts = []
        for layout in DCore.Installer.get_keyboard_layouts()
            layouts.push(layout)
            for variant in DCore.Installer.get_layout_variants(layout)
                layouts.push(layout + "," + variant)
        return layouts

    construct_item: (layout) ->
        opt = create_element("div", "KeyboardItem", @list)
        opt.innerText = layout
        opt.addEventListener("click", (e) =>
            @update_layout(layout)
        )

    update_layout: (layout) ->
        @current.innerText = layout
        __selected_layout = layout

class Timezone extends Widget
    constructor: (@id) ->
        super
        @current = create_element("div", "Current", @element)
        @current.innerText = __selected_timezone
        @picker_wrap = create_element("div","TmezoneWrap", @element)
        @picker = create_element("div", "TimezonePicker", @picker_wrap)
        @canvas = create_element("canvas", "TimezoneCanvas", @picker)
        @canvas.setAttribute("width", 736)
        @canvas.setAttribute("height", 404)
        @img = create_img("TimezoneMap", "images/zonemap.png", @picker)
        @img.setAttribute("usemap", "#ImageMap")
        @construct_map()

    update_timezone: (zone) ->
        @current.innerHTML = "<span>Zone:" + __database[zone]["offset"] + "</span>"
        @current.innerHTML += "<span>City:" + zone + "</span>"
        @current.innerHTML += "<span>Country:" + __database[zone]["country"] = "</span>"
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
        @pin = create_img("", "images/pin.png", @picker_wrap)
        @pin.setAttribute("position", "absolute")
        @pin.setAttribute("left", pin[0] + "px")
        @pin.setAttribute("top", pin[1] + "px")
        @pin.setAttribute("width", "13px")
        @pin.setAttribute("height", "21px")

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
        ctx.fillStyle = "#FF0000"
        ctx.fill()

    destroy_canvas: (area) ->
        ctx = @canvas.getContext("2d")
        ctx.clearRect(0,0,736,404)

class WelcomeFormItem extends Widget
    constructor: (@id)->
        super
        @text = create_element("div", "", @element)
        @input = create_element("input", "", @element)
        @tip = create_element("div", "WelcomeTip", @element)
        @angle = create_element("span", "WelcomeAngle", @tip)
        @error = create_element("span", "WelcomeError", @tip)
        @fill_widget()
        @input.addEventListener("focus", (e) =>
            @tip.setAttribute("class", "WelcomeTip")
            @input.setAttribute("style", "")
        )
        @input.addEventListener("blur", (e) =>
            @check_valid()
        )
        @input.addEventListener("change", (e) =>

            Widget.look_up("welcome")?.check_start_ready()
        )

    fill_widget: ->
        if @id == "username"
            @text.innerText = "用户名"
            @input.setAttribute("placeholder", "2-14个字符，英文，数字，中文")
            @error.innerText = "用户名不合法"
        else if @id == "hostname"
            @text.innerText = "计算机名"
            @input.setAttribute("placeholder", "2-14个字符，英文，数字，中文")
            @error.innerText = "计算机不合法"
        else if @id == "password"
            @text.innerText = "登录密码"
            @input.setAttribute("placeholder", "请输入密码")
            @input.setAttribute("type", "password")
            @error.innerText = "密码不合法"
        else if @id == "confirmpassword"
            @text.innerText = "确认密码"
            @input.setAttribute("placeholder", "请再次输入密码")
            @input.setAttribute("type", "password")
            @error.innerText = "确认密码不合法"

    check_valid: ->
        if @is_valid()
            @tip.setAttribute("class", "WelcomeTip")
        else
            @tip.setAttribute("class", "WelcomeTipShow")
            @input.setAttribute("style", "border:2px solid red")

    is_valid: ->
        if not @input.value? or @input.value.length == 0
            return false
        if @id == "username"
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
        @keyboard_displayed = false
        @timezone_displayed = false

        @title_start = create_element("div", "", @title)
        @start_txt = create_element("p", "", @title_start)
        @start_txt.innerText = "安装向导"

        @title_set = create_element("div", "TitleSet", @title)
        @keyboard_set = create_element("span", "KeyboardSet", @title_set)
        @keyboard_set.innerHTML += "键盘"
        @keyboard_set.addEventListener("click", (e) =>
            @display_keyboard()
        )

        @timezone_set = create_element("span", "TimezoneSet", @title_set)
        @timezone_set.innerText = "时区"
        @timezone_set.addEventListener("click", (e) =>
            @display_timezone()
        )

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
        @start.innerText = "开始安装"
        @start.addEventListener("mouseover", (e) =>
            if @username.is_valid() and @hostname.is_valid() and @password.is_valid() and @confirmpassword.is_valid()
                @start.setAttribute("style", "color:#00bdff")
        )
        @start.addEventListener("mouseout", (e) =>
            @start.setAttribute("style", "")
        )
        @start.addEventListener("click", (e) =>
            @start_install_cb()
        )

    display_keyboard: ->
        @hide_keyboard()
        @keyboard = new Keyboard("keyboard")
        @element.appendChild(@keyboard.element)
        @keyboard_displayed = true
        @keyboard_set.setAttribute("style", "background-color:#D0D0D0")

    hide_keyboard: ->
        @keyboard?.destroy()
        @keyboard_displayed = false
        @keyboard = null
        @keyboard_set.setAttribute("style", "")

    display_timezone: ->
        @hide_timezone()
        @timezone = new Timezone("timezone")
        @element.appendChild(@timezone.element)
        @timezone_displayed = true
        @timezone_set.setAttribute("style", "background-color:#D0D0D0")

    hide_timezone: ->
        @timezone?.destroy()
        @timezone_displayed = false
        @timezone = null
        @timezone_set.setAttribute("style", "")

    do_click: (e) ->
        if @keyboard_displayed
            if e.target.className not in ["KeyboardItem", "Keyboard", "KeyboardSet"]
                @hide_keyboard()
        if @timezone_displayed
            if e.target.className not in ["TimezoneMap", "Timezone", "TimezoneSet", "ImageMap", "TimezoneArea"]
                @hide_timezone()

    check_start_ready: ->
        if @username.is_valid() and @hostname.is_valid() and @password.is_valid() and @confirmpassword.is_valid()
            @start.setAttribute("style", "color:#FFFFFF")
            return true
        else
            @start.setAttribute("style", "color:848383")
            return false

    start_install_cb: ->
        if @check_start_ready()
            pc.add_page(part_page)
            pc.remove_page(welcome_page)
            __selected_item?.focus()
        else
            @username.check_valid()
            @hostname.check_valid()
            @password.check_valid()
            @confirmpassword.check_valid()

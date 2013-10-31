#Copyright (c) 2011 ~ 2012 Deepin, Inc.
#              2011 ~ 2012 snyh
#
#Author:      snyh <snyh@snyh.org>
#Maintainer:  snyh <snyh@snyh.org>
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

Storage.prototype.setObject = (key, value) ->
    @setItem(key, JSON.stringify(value))

Storage.prototype.getObject = (key) ->
    JSON.parse(@getItem(key))

String.prototype.endsWith = (suffix)->
    return this.indexOf(suffix, this.length - suffix.length) != -1

Array.prototype.remove = (el)->
    i = this.indexOf(el)
    if i != -1
        this.splice(this.indexOf(el), 1)[0]

String::args = ->
    o = this
    len = arguments.length
    for i in [1..len]
        o = o.replace(new RegExp("%" + i, "g"), arguments[i - 1])

    return o

echo = (log) ->
    console.log log

assert = (value, msg) ->
    if not value
        throw new Error(msg)

# xgettext will extract the first argument if two is given
# so, if domain is given, it ought to be the second argument.
_ = (s, d)->
    if d
        DCore.dgettext(d, s)
    else
        DCore.gettext(s)

bindtextdomain = (domain, locale_dir) ->
    DCore.bindtextdomain(domain, locale_dir)

build_menu = (info) ->
    m = new DeepinMenu
    for v in info
        if v.length == 0  #separater item
            i = new DeepinMenuItem(2, 0, 0, 0)
        else if typeof v[0] == "number"  #normal item
            i = new DeepinMenuItem(0, v[0], v[1], null)
            if v.length > 2 and v[2] == false
                i.enabled = false
            else
                i.enabled = true
        else  #sub menu item
            sm = build_menu(v[1])
            i = new DeepinMenuItem(1, 0, v[0], sm)
        m.appendItem(i)
    return m

get_page_xy = (el, x, y) ->
    p = webkitConvertPointFromNodeToPage(el, new WebKitPoint(x, y))

find_drag_target = (el)->
    p = el
    if p.draggable
        return p
    while p = p.parentNode
        if p.draggable
            return p
    return null

swap_element = (c1, c2) ->
    if c1.parentNode == c2.parentNode
        tmp = document.createElement('div')
        c1.parentNode.insertBefore(tmp, c1)
        c2.parentNode.insertBefore(c1, c2)
        tmp.parentNode.insertBefore(c2, tmp)
        tmp.parentNode.removeChild(tmp)

#disable default body drop event
document.body.ondrop = (e) -> e.preventDefault()

run_post = (f, self)->
    f2 = f.bind(self or this)
    setTimeout(f2, 0)

create_element = (type, clss, parent)->
    el = document.createElement(type)
    el.setAttribute("class", clss) if clss
    if parent
        parent.appendChild(el)
    return el

create_img = (clss, src, parent)->
    el = create_element('img', clss, parent)
    el.src = src
    el.draggable = false
    return el

calc_text_size = (txt, width)->
    tmp = create_element('div', 'hidden_calc_text', document.body)
    tmp.innerText = txt
    tmp.style.width = "#{width}px"
    h = tmp.clientHeight
    document.body.removeChild(tmp)
    return h

clamp = (value, min, max)->
    return min if value < min
    return max if value > max
    return value

get_function_name = ->
    return "AnymouseFunction" if not arguments.caller
    /function (.*?)\(/.exec(arguments.caller.toString())[1]


DEEPIN_ITEM_ID = "deepin-item-id"
dnd_is_desktop = (e)->
    return e.dataTransfer.getData("text/uri-list").trim().endsWith(".desktop")
dnd_is_deepin_item = (e)->
    if e.dataTransfer.getData(DEEPIN_ITEM_ID)
        return true
    else
        return false
dnd_is_file = (e)->
    return e.dataTransfer.getData("text/uri-list").length != 0

ajax = (url,sync=true,callback,callback_error) ->
    xhr = new XMLHttpRequest()

    xhr.open("GET", url, sync)

    xhr.onload = ->
        # echo "callback： #{typeof callback}"
        echo "XMLHttpRequest onload"
        callback?(xhr)
        return

    xhr.onerror = ->
        echo "XMLHttpRequest onerror"
        callback_error?(xhr)
    xhr.send(null)

get_path_base = (path)->
    path.split('/').slice(0, -1).join('/')
get_path_name = (path)->
    dot_pos = path.lastIndexOf('.')
    if dot_pos == -1
        path.substring(path.lastIndexOf('/') + 1)
    else
        path.substring(path.lastIndexOf('/') + 1, dot_pos)

remove_element = (obj)->
    _parentElement = obj?.parentNode
    _parentElement?.removeChild(obj)

sortNumber = (a , b) ->
    return a - b
array_sort_min2max = (arr) ->
    arr.sort(sortNumber)

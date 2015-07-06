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

_events = [
    'blur',
    'change',
    'click',
    'contextmenu',
    'buildmenu',
    'rightclick',
    'copy',
    'cut',
    'dblclick',
    'error',
    'focus',
    'keydown',
    'keypress',
    'keyup',
    'mousedown',
    'mousemove',
    'mouseout',
    'mouseover',
    'mouseenter',
    'mouseleave',
    'mouseup',
    'mousewheel',
    'paste',
    'reset',
    'resize',
    'scroll',
    'select',
    'submit',
    'DOMActivate',
    'DOMAttrModified',
    'DOMCharacterDataModified',
    'DOMFocusIn',
    'DOMFocusOut',
    'DOMMouseScroll',
    'DOMNodeInserted',
    'DOMNodeRemoved',
    'DOMSubtreeModified',
    'textInput',
    'dragstart',
    'dragend',
    'dragover',
    'drag',
    'drop',
    'dragenter',
    'dragleave',
    'itemselected',
    'webkitTransitionEnd'
]


class Widget extends Module
    @object_table = {}
    @look_up = (id) ->
        @object_table[id]

    constructor: ->
        el = document.createElement('div')
        el.setAttribute('class',  @constructor.name)
        if @id
            el.id = @id
            Widget.object_table[@id] = this
        @element = el

        #there has an strange bug when use indexof instead search,
        # the key value will always be "constructor" without any other thing
        f_menu = null
        f_rclick = null

        for k,v of this.constructor.prototype when k.search("do_") == 0
            key = k.substr(3)
            if key in _events
                if key == "rightclick"
                    f_rclick = v.bind(this)
                else if key == "buildmenu"
                    f_menu = v.bind(this)
                else if key == "contextmenu"
                    "nothing should do"
                else
                    @element.addEventListener(key, v.bind(this))
            else
                console.error("[widget.coffee] Widget.constructor(), found the do_ prefix but the name #{key} is not an dom events")

        # filter unnecessary event binding.
        if f_menu || f_rclick
            @element.addEventListener("contextmenu", (e) =>
                if f_rclick
                    f_rclick(e)
                if f_menu
                    menu = f_menu()
                    menu.unshift(DEEPIN_MENU_TYPE.NORMAL)
                    build_menu(menu)
                        ?.addListener(@on_itemselected)
                        .showMenu(e.clientX, e.clientY)
                    e.preventDefault()
            )

    destroy: ->
        @element.parentElement?.removeChild(@element)
        delete Widget.object_table[@id]

    add_css_class: (name, el)->
        el = @element if not el
        el.classList.add(name)
    remove_css_class: (name, el)->
        el = @element if not el
        el.classList.remove(name)

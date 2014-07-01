#Copyright (c) 2011 ~ 2014 Deepin, Inc.
#              2011 ~ 2014 bluth
#
#encoding: utf-8
#Author:      bluth <yuanchenglu@linuxdeepin.com>
#Maintainer:  bluth <yuanchenglu@linuxdeepin.com>
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

class Language extends Widget

    constructor:->
        super
        #inject_js("js/jquery/jquery.min.js")
        #inject_js("js/jquery/jquery.nicescroll.js")
        inject_css(_b,"css/language.css")
        @local_list = []
        @lang_list = []
        document.body.style.width = screen.width
        document.body.style.height = screen.height

        document.body.appendChild(@element)
        @get_lang_list()
        @boxscroll_create()

    get_lang_list: ->
        @local_list = DCore.Greeter.get_local_list()
        @lang_list = DCore.Greeter.get_lang_list()

    select_lang: (name) ->
        lang = la["lang"] for la in @lang_list when la["name"] is name
        echo lang + "===for  lang_list  name===" + name
        DCore.Greeter.set_language(lang)
        @start_session()

    start_session: (@username = "deepin",@password = "",@session = "deepin") ->
        document.body.cursor = "wait"
        DCore.Greeter.start_session(@username, @password, @session)

    boxscroll_create: ->
        @wrap = create_element("div","wrap",@element)
        @wrap.setAttribute("draggable",false)
        @logo = create_element("div","logo",@wrap)
        @img = create_img("img","img/logo.png",@logo)
        @boxscroll = create_element("div","boxscroll",@wrap)
        @boxscroll.setAttribute("id","boxscroll")
        @li = []
        @a = []
        @ul = create_element("ul","",@boxscroll)
        @li = []
        @a = []
        @ul = create_element("ul","",@boxscroll)
        @ul.setAttribute("draggable",false)
        for local,i in @local_list
            @li[i] = create_element("li","",@ul)
            @a[i] = create_element("a","",@li[i])
            @li[i].setAttribute("id",local["name"])
            @a[i].innerText = local["local"]
            that = @
            @li[i].addEventListener("click",->
                that.select_lang(this.id)
            )
        document.body.addEventListener("keydown",(e)=>
            echo "keydown:#{e.which}"
            #if e.which == KEYCODE.ESC
            #    @start_session("ycl","1")
            if e.which == KEYCODE.ENTER
                if list.currentIndex < 0 then return
                local = @local_list[list.currentIndex]["name"]
                echo "#{list.currentIndex}:#{local}"
                @select_lang(local)
        )
 
document.body.addEventListener("contextmenu",(e)=>
    e.preventDefault()
    e.stopPropagation()
)
#document.ready = ->
#    nicesx = $("#boxscroll").niceScroll({touchbehavior:false,cursorcolor:"#fff",cursoropacitymax:0.6,cursorwidth:8})

new Language()

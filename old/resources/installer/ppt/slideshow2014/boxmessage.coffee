
_ = (s, d)->
    try
        if d
            DCore.dgettext(d, s)
        else
            DCore.gettext(s)
    catch e
        s

create_element = (opt, parent, compatible)->
    if typeof compatible != 'undefined' || typeof parent == 'string'
        opt = tag:opt, class: parent
        parent = compatible

    if not opt.tag?
        return null
    el = document.createElement(opt.tag)
    delete opt.tag
    for own k, v of opt
        el.setAttribute(k, v)

    if parent
        parent.appendChild(el)

    return el

getQueryParams = (qs) ->
    qs = qs.split("+").join(" ")
    params = {}
    re = /[?&]?([^=]+)=([^&]*)/g
    tokens = null
    while (tokens = re.exec(qs))
        params[decodeURIComponent(tokens[1])] = decodeURIComponent(tokens[2])
    params

lang = getQueryParams(document.location.search).lang
if not lang?
    lang = "en"
document.body.lang = lang
console.log("[boxmessage.coffee] slideshow2014 lang:#{document.body.lang}")

class BoxMessage
    constructor:(@id,@class,@parent) ->
        @boxmessage = create_element("div",@class,@parent)
        @boxmessage.setAttribute("id",@id)

    create_box_msg: (@msg_obj) ->
        @title = create_element("h1","",@boxmessage)
        @title.innerText = @msg_obj.title
        @ul = create_element("ul","",@boxmessage)
        @msg = []
        for msg,i in @msg_obj.msg
            @msg[i] = create_element("li","",@ul)
            @msg[i].innerText = msg

msg_obj_array = [
    {
        title:_("Deepin Store"),
        msg:[
            _("One-stop Software Management"),
            _("Continuous Update of Wonderful Software"),
            _("Unimpeded Online Communication")
        ]
    },
    {
        title:_("Deepin Movie")
        msg:[
            _("Comprehensive Audio-visual Shock"),
            _("New Interactive Experience"),
            _("Intelligent Subtitle Loading")
        ]
    },
    {
        title:_("Deepin Music")
        msg:[
            _("Ultimate Music Journey"),
            _("Rich Extended Functions"),
            _("Enjoy Online Strongest Voice")
        ]
    },
    {
        title:_("Deepin Community")
        msg:[
            _("Free and United Open Source Community"),
            _("Welcome to participate in Deepin Internationalization Project"),
            _("Welcome to follow us on Weibo, Twitter, Facebook, Google+, Mailing List and IRC")
        ]
    }
]



container = document.getElementById('container')
bmsg = []
for msg_obj,i in msg_obj_array
    bmsg[i] = new BoxMessage("text#{i + 1}", "text#{i + 1} messages", container)
    bmsg[i].create_box_msg(msg_obj)

document.body.addEventListener("contextmenu",(e)=>
    e.preventDefault()
    e.stopPropagation()
)

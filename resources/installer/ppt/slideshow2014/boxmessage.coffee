
class BoxMessage extends Widget
    constructor:(@id,@class) ->
        super
        @element.setAttribute("class",@class)

    set_msg_obj: (@msg_obj) ->

    create_box_msg: ->
        for obj in @msg_obj
            @title = create_element("h1","",@element)
            @title.innerText = obj.title
            @ul = create_element("ul","",@title)
            for msg,i in obj.msg
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
            _("Rich Extended Function"),
            _("Enjoy Online Strongest Voice")
        ]
    },
    {
        title:_("Deepin Game")
        msg:[
            _("Provide Latest and Hottest Games"),
            _("Gather Various Game Topics"),
            _("Collect Your Favorite Online")
        ]
    },
    {
        title:_("Deepin Community")
        msg:[
            _("Free and United Open Source Community"),
            _("Welcome to participate in Deepin International Project"),
            _("Welcome to follow us on Weibo, Twitter, Facebook, Google+, Mailing List and IRC")
        ]
    }
]

_b = document.body
container = create_element("div","container",_b)
canvas_slideStage = create_element("canvas","slideStage","@element")

bmsg = []
for msg_obj,i in msg_obj_array
    bmsg[i] = new BoxMessage("text#{i + 1}", "text#{i + 1} messages")
    container.appendChild(bmsg[i].element)
    bmsg[i].set_msg_obj(msg_obj)
    bmsg[i].create_box_msg()


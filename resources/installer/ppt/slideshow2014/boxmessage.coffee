
class BoxMessage
    constructor:(@id,@class,@parent) ->
        @boxmessage = create_element("div",@class,@parent)
        @boxmessage.setAttribute("id",@id)

    create_box_msg: (@msg_obj) ->
        @title = create_element("h1","",@boxmessage)
        @title.innerText = @msg_obj.title
        @ul = create_element("ul","",@title)
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
container = document.getElementById('container')
#container = create_element("div","container",_b)
#canvas_slideStage = create_element("canvas","slideStage",container)

bmsg = []
for msg_obj,i in msg_obj_array
    bmsg[i] = new BoxMessage("text#{i + 1}", "text#{i + 1} messages", container)
    bmsg[i].create_box_msg(msg_obj)


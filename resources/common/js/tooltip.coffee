tooltip_hide_id = null
class ToolTip extends Widget
    @tooltip: null
    @should_show_id: -1
    constructor: (@element, @text, @parent=document.body)->
        ToolTip.tooltip ?= create_element("div", "tooltip", @parent)

        @event_bind('dragstart', =>
            @hide()
        )
        @event_bind('dragenter', =>
            @hide()
        )
        @event_bind('dragover', =>
            @hide()
        )
        @event_bind('dragleave', =>
            @hide()
        )
        @event_bind('dragend', =>
            @hide()
        )
        @event_bind('contextmenu', =>
            @hide()
        )
        @event_bind('mouseout', =>
            @hide()
        )
        @event_bind('mouseover', =>
            ToolTip.should_show_id = setTimeout(=>
                @show()
            , 500)
        )
        @event_bind('click', =>
            @hide()
        )

    event_bind: (evt_name, callback) ->
        @element.addEventListener(evt_name, (e) ->
            callback()
        )

    show: ->
        ToolTip.tooltip.innerText = @text
        ToolTip.tooltip.style.display = "block"
        @_move_tooltip()
    hide: ->
        clearTimeout(ToolTip.should_show_id)
        ToolTip.tooltip?.style.display = "none"
    @move_to: (self, x, y) ->
        if y <= 0
            self.hide()
            return
        ToolTip.tooltip.style.left = "#{x}px"
        ToolTip.tooltip.style.bottom = "#{y}px"
    _move_tooltip: ->
        page_xy= get_page_xy(@element, 0, 0)
        offset = (@element.clientWidth - ToolTip.tooltip.clientWidth) / 2

        x = page_xy.x + offset + 4  # 4 for subtle adapt
        x = 0 if x < 0
        ToolTip.move_to(@, x.toFixed(), document.body.clientHeight - page_xy.y)

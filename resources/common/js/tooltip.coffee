tooltip_hide_id = null

$tooltip = null

class ToolTipBase extends Widget
    constructor: (@buddy, @text, @parent=document.body)->
        super
        @delay_time = 0
        @element.addEventListener("mouseover", @hide)
        @element.addEventListener("mousemove", @hide)
        @element.addEventListener("mouseenter", @hide)
        @element.addEventListener("mouseout", @hide)

    set_delay_time: (millseconds) ->
        @delay_time = millseconds

    set_text: (text)->
        @text = text

    bind_events: ->
        @buddy.addEventListener('dragstart', @hide)
        @buddy.addEventListener('dragenter', @hide)
        @buddy.addEventListener('dragover', @hide)
        @buddy.addEventListener('dragleave', @hide)
        @buddy.addEventListener('dragend', @hide)
        @buddy.addEventListener('contextmenu', @hide)
        @buddy.addEventListener('mouseout', @hide)
        @buddy.addEventListener('mouseover', @on_mouseover)
        @buddy.addEventListener('click', @hide)

    destroy:->
        @buddy.removeEventListener('dragstart', @hide)
        @buddy.removeEventListener('dragenter', @hide)
        @buddy.removeEventListener('dragover', @hide)
        @buddy.removeEventListener('dragleave', @hide)
        @buddy.removeEventListener('dragend', @hide)
        @buddy.removeEventListener('contextmenu', @hide)
        @buddy.removeEventListener('mouseout', @hide)
        @buddy.removeEventListener('mouseover', @on_mouseover)
        @buddy.removeEventListener('click', @hide)
        @buddy = null

    on_mouseover:=>
        if @text == ''
            return
        clearTimeout(tooltip_hide_id)
        tooltip_hide_id = setTimeout(=>
            @show()
        , @delay_time)

    hide: =>
        clearTimeout(tooltip_hide_id)

    show:=>
        $tooltip = @


class ToolTip extends ToolTipBase
    @tooltip: null
    constructor: (@buddy, @text, @parent=document.body)->
        super
        ToolTip.tooltip ?= create_element("div", "tooltip", @parent)
        @bind_events()

    show: =>
        super
        ToolTip.tooltip.innerText = @text
        ToolTip.tooltip.style.display = "block"
        @_move_tooltip()

    isShown:=>
        ToolTip.tooltip.style.display != "none"

    hide: =>
        super
        ToolTip.tooltip?.style.display = "none"

    @move_to: (self, x, y) ->
        if y <= 0
            self.hide()
            return
        ToolTip.tooltip.style.left = "#{x}px"
        ToolTip.tooltip.style.bottom = "#{y}px"

    _move_tooltip: =>
        page_xy= get_page_xy(@buddy, 0, 0)
        offset = (@buddy.clientWidth - ToolTip.tooltip.clientWidth) / 2

        x = parseInt((page_xy.x + offset).toFixed())
        x = 0 if x < 0
        ToolTip.move_to(@, x, document.body.clientHeight - page_xy.y)


class ArrowToolTip extends ToolTipBase
    @container: null
    @tooltip: null
    @arrow: null

    # Set direction of arrow inversed or not.
    @inversed: false

    constructor: (@buddy, @text, @inversed=false, @parent=document.body)->
        super(@buddy, @text, @parent)
        ArrowToolTip.container ?= create_element('div', 'arrow_tooltip_container ', @parent)
        ArrowToolTip.tooltip ?= create_element('canvas', 'arrow_tooltip', ArrowToolTip.container)
        ArrowToolTip.content ?= create_element('div', 'arrow_tooltip_content', ArrowToolTip.container)
        # content will show wried, have to use _hidden_content
        ArrowToolTip._hidden_content ?= create_element('div', 'arrow_tooltip_hidden_content', @parent)
        @bind_events()

    isShown: =>
        ArrowToolTip.container.style.display != 'none'

    setPointerEvents:(eventMask)->
        ArrowToolTip.container?.style.pointerEvents = eventMask
        ArrowToolTip.content?.style.pointerEvents = eventMask
        @buddy?.style.pointerEvents = eventMask

    draw: ->
        content = ArrowToolTip._hidden_content
        canvas = ArrowToolTip.tooltip
        ctx = canvas.getContext('2D')
        # triangle of tooltip
        # |<- width ->|
        # _________________
        # \           /   ^
        #  \         /    |
        #   \       /     |
        #    \     /    height
        #     \   /       |
        #      \ /        |
        #       v_________v
        triangle =
            width: 18
            height: 10
        if @inversed
            padding =
                horizontal: 5
                vertical: 2
                abs_horizontal: 25
        else
            padding =
                horizontal: 5
                vertical: 0
        radius = 4
        if @inversed
            offsetForShadow = 12
        else
            offsetForShadow = 6
        offsetForRadius = 0
        height = content.clientHeight - offsetForRadius * 2

        canvas.width = content.clientWidth + 2 * (padding.horizontal + radius + offsetForShadow)
        canvas.height = height + 2 * (padding.vertical + radius + offsetForShadow) + triangle.height
        ArrowToolTip.container.width = canvas.width
        ArrowToolTip.container.height = canvas.height

        topY = offsetForShadow + radius
        bottomY = topY + height + padding.vertical * 2
        leftX = offsetForShadow + radius
        rightX = leftX + 2 * padding.horizontal + content.clientWidth

        arch =
            TopLeft:
                ox: leftX
                oy: topY
                radius: radius
                startAngle: Math.PI
                endAngle: Math.PI * 1.5
            TopRight:
                ox: rightX
                oy: topY
                radius: radius
                startAngle: Math.PI * 1.5
                endAngle: Math.PI * 2
            BottomRight:
                ox: rightX
                oy: bottomY
                radius: radius
                startAngle: 0
                endAngle: Math.PI * 0.5
            BottomLeft:
                ox: leftX
                oy: bottomY
                radius: radius
                startAngle: Math.PI * 0.5
                endAngle: Math.PI

        ctx = canvas.getContext('2d')
        ctx.save()
        # ctx.globalAlpha = 0.8
        ctx.beginPath()

        ctx.moveTo(leftX - radius, topY)
        ctx.arc(arch['TopLeft'].ox, arch['TopLeft'].oy, arch['TopLeft'].radius,
                arch['TopLeft'].startAngle, arch['TopLeft'].endAngle)

        if @inversed
        # triangle
            ctx.lineTo(leftX + padding.abs_horizontal, topY - radius)
            ctx.lineTo(leftX + padding.abs_horizontal + triangle.width / 2, topY - radius - triangle.height)
            ctx.lineTo(leftX + padding.abs_horizontal + triangle.width, topY - radius)

        ctx.lineTo(rightX, topY - radius)

        ctx.arc(arch['TopRight'].ox, arch['TopRight'].oy, arch['TopRight'].radius,
                arch['TopRight'].startAngle, arch['TopRight'].endAngle)

        ctx.lineTo(rightX + radius, bottomY)

        ctx.arc(arch['BottomRight'].ox, arch['BottomRight'].oy, arch['BottomRight'].radius,
                arch['BottomRight'].startAngle, arch['BottomRight'].endAngle)

        # bottom line
        if ! @inversed
        # calculate the offset for trangle
            page_xy= get_page_xy(@buddy, 0, 0)
            offset = (@buddy.clientWidth - ArrowToolTip.container.clientWidth) / 2

            x = parseInt((page_xy.x + offset).toFixed())
            if x > 0
                x = x + ArrowToolTip.container.clientWidth
                if x > screen.width
                    x = x - screen.width
                else
                    x = 0

            ctx.lineTo(leftX + padding.horizontal + (content.clientWidth + triangle.width) / 2 + x,
                       bottomY + radius)

            # triangle
            ctx.lineTo(leftX + padding.horizontal + content.clientWidth / 2 + x,
                       bottomY + radius + triangle.height)

            ctx.lineTo(leftX + padding.horizontal + (content.clientWidth - triangle.width)/2 + x,
                       bottomY + radius)

        # bottom line
        ctx.lineTo(leftX, bottomY + radius)

        ctx.arc(arch['BottomLeft'].ox, arch['BottomLeft'].oy, arch['BottomLeft'].radius,
                arch['BottomLeft'].startAngle, arch['BottomLeft'].endAngle)
        ctx.closePath()

        ctx.shadowBlur = offsetForShadow
        ctx.shadowColor = 'black'
        ctx.shadowOffsetY = 2

        ctx.strokeStyle = 'rgba(255,255,255, 0.7)'
        ctx.lineWidth = 1
        ctx.stroke()

        grd = ctx.createLinearGradient(0, 0, 0, height + 2 * padding.vertical + radius * 2 + triangle.height)
        grd.addColorStop(0, 'rgba(0,0,0,0.7)')
        grd.addColorStop(1, 'rgba(0,0,0,0.9)')
        ctx.fillStyle = grd
        ctx.fill()

        ctx.restore()

        ArrowToolTip.content.style.top = topY + padding.vertical - offsetForRadius
        ArrowToolTip.content.style.left = leftX + padding.horizontal

    show: =>
        super
        ArrowToolTip.container.style.display = "block"
        ArrowToolTip.container.style.opacity = 1
        ArrowToolTip.content.style.display = "block"
        ArrowToolTip.content.textContent = @text
        ArrowToolTip._hidden_content.textContent = @text
        @draw()
        @_move_tooltip()

    hide: =>
        super
        ArrowToolTip.container.style.display = 'none'
        ArrowToolTip.container.style.opacity = 0

    get_xy: =>
        page_xy= get_page_xy(@buddy, 0, 0)
        offset = (@buddy.clientWidth - ArrowToolTip.container.clientWidth) / 2

        x = parseInt((page_xy.x + offset).toFixed())
        x = 0 if x < 0
        if x + ArrowToolTip.container.clientWidth > screen.width
            x = screen.width - ArrowToolTip.container.clientWidth
        y = document.body.clientHeight - page_xy.y
        {x:x, y:y}

    @move_to: (self, x, y) ->
        if y <= 0
            self.hide()
            return
        ArrowToolTip.container.style.left = "#{x}px"
        ArrowToolTip.container.style.bottom = "#{y}px"

    _move_tooltip: =>
        pos = @get_xy()
        ArrowToolTip.move_to(@, pos.x, pos.y)

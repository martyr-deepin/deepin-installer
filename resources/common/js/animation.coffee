apply_animation = (el, name, duration, timefunc)->
    el.style.webkitAnimationName = name
    el.style.webkitAnimationDuration = duration
    el.style.webkitAnimationTimingFunction = timefunc or "linear"

apply_rotate = (el, time)->
    apply_animation(el, "rotate", "#{time}s", "cubic-bezier(0, 0, 0.35, -1)")
    id = setTimeout(->
        el.style.webkitAnimation = ""
        clearTimeout(id)
    , time * 1000)

apply_flash = (el, time)->
    apply_animation(el, "flash", "#{time}s", "cubic-bezier(0, 0, 0.35, -1)")
    id = setTimeout(->
        el.style.webkitAnimation = ""
        clearTimeout(id)
    , time * 1000)


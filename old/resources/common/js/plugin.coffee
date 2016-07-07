get_name = (id) ->
    index = id.lastIndexOf(':')
    if index == -1
        return id
    else
        id.substring(index + 1)

class DDEPluginManager
    # key: plugin's name
    # value: Plugin class
    @_plugins: null

    constructor: ->
        DDEPluginManager._plugins = {} if not DDEPluginManager._plugins

    enable_plugin: (id, value)->
        DCore.enable_plugin(id, value)

    enable_plugin_front: (id, value) ->
        name = get_name(id)
        console.log("[plugin.coffee] DDEPluginManager.enable_plugin_front(), plugin name: #{name}")
        plugin = DDEPluginManager._plugins[name]
        if plugin
            if value
                console.log("[plugin.coffee] DDEPluginManager.enable_plugin_fron(), enable plugin: ", id)
                plugin.inject_css(name)
            else
                console.log("[plugin.coffee] DDEPluginManager.enable_plugin_fron(), disable plugin: ", id)
                plugin.destroy()
                delete DDEPluginManager._plugins[name]
                DDEPluginManager._plugins[name] = null
        else
            console.warn("[plugin.coffee] DDEPluginManager.enable_plugin_front(), plugin #{id} does not exist!")

    get_plugin: (name) ->
        DDEPluginManager._plugins[name]

    add_plugin: (name, obj) ->
        DDEPluginManager._plugins[name] = obj

    @plugin_changed_handler: (info) ->
        id_prefix = info.app_name + ":"
        all_plugins = DCore.get_plugins(info.app_name)
        delete info.app_name

        for plugin in all_plugins
            name = get_path_name(plugin)
            id = id_prefix + name
            if info[id]
                delete info[id]
                if not DDEPluginManager._plugins or not DDEPluginManager._plugins[name]
                    if id_prefix == 'desktop:'
                        new DesktopPlugin(get_path_base(plugin), name)
                        console.log("[plugin.coffee] DDEPluginManager.plugin_changed_handler() plugin id: #{id}")
                        PluginManager.enable_plugin_front(id, true)
                        place_all_widgets()

        for own k, v of info
            PluginManager.enable_plugin_front(k, false)

        return


class Plugin
    constructor: (@app_name, @path, @name, @host)->
        @id = @app_name + ':' + @name
        window.PluginManager = new DDEPluginManager() unless window.PluginManager
        PluginManager.add_plugin(@name, @)
        @info = DCore.get_plugin_info(@path)
        bindtextdomain(@info.textdomain, "#{@path}/locale/mo")
        @inject_js(@name)

    wrap_element: (child)->
        @host.appendChild(child)

    inject_js: (name) ->
        @js_element = create_element("script", null, document.body)
        @js_element.src = "#{@path}/#{name}.js"

    inject_css: (name)->
        @css_element = create_element('link', null, @host)
        @css_element.rel = "stylesheet"
        @css_element.href = "#{@path}/#{name}.css"

DCore.signal_connect("plugins_changed", DDEPluginManager.plugin_changed_handler)

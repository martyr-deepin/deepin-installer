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

guest_id = "999"
guest_name = _("Guest")
guest_name_in_lightdm = "guest"

class Accounts
    ACCOUNTS_DAEMON = "com.deepin.daemon.Accounts"
    ACCOUNTS_USER =
        name: ACCOUNTS_DAEMON
        path: "/com/deepin/daemon/Accounts/User1000"
        interface: "com.deepin.daemon.Accounts.User"

    GRAPHIC = "com.deepin.api.Graphic"
    APP = null

    constructor:(@id)->
        APP = @id#APP_NAME for DCore[APP]
        @get_dbus_failed = false
        @users_id = []
        @users_name = []
        @users_id_dbus = []
        @users_name_dbus = []

        @getDBus()

    getDBus:->
        try
            console.log(ACCOUNTS_DAEMON)
            @Dbus_Account = get_dbus("system", ACCOUNTS_DAEMON, "UserList")
            echo "ACCOUNTS_DAEMON succeed and then connect path" if @Dbus_Account
            for path in @Dbus_Account.UserList
                echo path
                ACCOUNTS_USER.path = path
                user_dbus = get_dbus("system", ACCOUNTS_USER, "UserName")
                echo "Uid:#{user_dbus.Uid}"
                echo "UserName:#{user_dbus.UserName}"
                @users_id.push(user_dbus.Uid)
                @users_name.push(user_dbus.UserName)
                @users_id_dbus[user_dbus.Uid] = user_dbus
                @users_name_dbus[user_dbus.UserName] = user_dbus
        catch e
            echo "Dbus_Account #{ACCOUNTS_DAEMON} ERROR: #{e}"
            @get_dbus_failed = true



    get_user_name: (uid) ->
        username = @users_id_dbus[uid].UserName
        return username

    get_user_id:(user)->
        if user is guest_name then return guest_id
        id = null
        try
            id = @users_name_dbus[user].Uid
        catch e
            echo "get_user_id #{e}"
        if not id? then id = "1000"
        return id

    is_user_logined:(uid)->
        is_logined = false
        LoginTime = @users_id_dbus[uid].LoginTime
        echo "LoginTime:#{LoginTime}"
        if LoginTime is null or LoginTime == 0 or LoginTime is undefined then is_logined = false
        else is_logined = true
        return is_logined

    is_user_sessioned_on:(uid)->
        if APP isnt "Greeter" then return true

        username = @users_id_dbus[uid].UserName
        try
            is_sessioned_on = DCore.Greeter.get_user_session_on(username)
        catch e
            echo "#{e}"
            is_sessioned_on = false
        return is_sessioned_on
    
    is_need_pwd: (uid) ->
        if uid is guest_id then return false
        else return true
        username = @get_user_name(uid)
        dbus = null
        try
            if APP is "Greeter"
                LIGHTDM = "com.deepin.dde.lightdm"
                dbus = DCore.DBus.sys(LIGHTDM)
            else
                LOCK = "com.deepin.dde.lock"
                dbus = DCore.DBus.sys(LOCK)
            return dbus?.NeedPwd_sync(username)
        catch e
            echo "is_need_pwd dbus error #{e}"
            return true


    get_user_icon:(uid)->
        icon = null
        try
            icon = @users_id_dbus[uid].IconFile
        catch e
            echo "get_user_bg #{e}"
        return icon


    get_user_bg:(uid)->
        bg = null
        try
            bg = @users_id_dbus[uid].BackgroundFile
        catch e
            echo "get_user_bg #{e}"
        return bg

    get_default_username:->
        try
            if APP is "Greeter"
                @_default_username = DCore[APP].get_default_user()
            else
                @_default_username = DCore[APP].get_username()
        catch e
            echo "#{APP}get_default_username:#{e}"
        echo "#{APP} get_default_username:---------#{@_default_username}-------------"
        return @_default_username

    get_current_user_background:->
        @_current_username = @get_default_username()
        @_current_userid = @get_user_id(@_current_username)
        return @get_user_bg(uid)

    is_disable_user :(uid)->
        disable = false
        user_dbus = @users_id_dbus[uid]
        if user_dbus.Locked is null then disable = false
        else if user_dbus.Locked is true then disable = true
        return disable

    isAllowGuest:->
        @AllowGuest = @Dbus_Account.AllowGuest

    getRandUserIcon:->
        icon = @Dbus_Account.RandUserIcon_sync()
        echo icon
        return icon

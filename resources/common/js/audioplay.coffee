#Copyright (c) 2011 ~ 2014 Deepin, Inc.
#              2011 ~ 2014 bluth
#
#Author:      bluth <yuanchenglu001@gmail.com>
#Maintainer:  bluth <yuanchenglu001@gmail.com>
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

class AudioPlay

    MPRIS_DBUS_MIN = "org.mpris.MediaPlayer2."

    DBUS =
        name:"org.freedesktop.DBus"
        path:"/"
        interface:"org.freedesktop.DBus"

    MPRIS_DBUS =
        name:"org.mpris.MediaPlayer2.dmusic"
        path:"/org/mpris/MediaPlayer2"
        interface:"org.mpris.MediaPlayer2.Player"

    constructor: ->
        @mpris_all = []
        @mpris_dbus = null
        @launched_status = false
        
        @now_mpris = @get_mpris_now()
        @now_mpris_dbus_name = @now_mpris?.mpris
        @now_mpris_name = @now_mpris?.name
        console.log("[audioplay.coffee] constructor() now_mpris_name: ", @now_mpris_name,
                    ", now_mpris_dbus_name: ", @now_mpris_dbus_name)
        @mpris_dbus = @get_mpris_dbus(@now_mpris_dbus_name)

    get_mpris_now:->
        #1.get all dbus_name_all and then search for MPRIS_DBUS_MIN
        freedesktop_dbus = DCore.DBus.session_object(
            DBUS.name,
            DBUS.path,
            DBUS.interface
        )
        dbus_name_all = []
        dbus_name_all = freedesktop_dbus.ListNames_sync()
        for dbus in dbus_name_all
            index = dbus.indexOf(MPRIS_DBUS_MIN)
            if index != -1
                name = dbus.substring(index + MPRIS_DBUS_MIN.length)
                mpris = {"name":name,"mpris":dbus}
                @mpris_all.push(mpris)
        
        console.log("[audioplay.coffee] get_mpris_now(), mpris_all: ", @mpris_all)

        #2.check which mpris is now playing
        switch(@mpris_all.length)
            when 0 then return null
            when 1 then return @mpris_all[0]
            else
                #1.if is dmusic then directly return
                for dbus in @mpris_all
                    if dbus?.name is "dmusic" then return dbus
                
                #2.if isnt Stopped then return
                for dbus in @mpris_all
                    mpris = dbus?.mpris
                    MPRIS_DBUS.name = mpris
                    try
                        mpris_dbus = DCore.DBus.session_object(
                            MPRIS_DBUS.name,
                            MPRIS_DBUS.path,
                            MPRIS_DBUS.interface
                        )
                        #if dbus.name is @get_default_audio_player_name return dbus
                        if mpris_dbus.PlaybackStatus isnt "Stopped" then return dbus
                    catch error
                        console.error("[audioplay.coffee] get_mpris_now() call session_object() error: ", error)
                        return null
                
                #3. else return null
                return null


    get_mpris_dbus:(dbus_name) ->
        try
            MPRIS_DBUS.name = dbus_name
            mpris_dbus = DCore.DBus.session_object(
                MPRIS_DBUS.name,
                MPRIS_DBUS.path,
                MPRIS_DBUS.interface
            )
            @launched_status = true
            return mpris_dbus
        catch error
            @launched_status = false
            console.error("[audioplay.coffee] get_mpris_dbus() error, mpris_dbus is null, the play is not launched: ", error)
            return null

    get_launched_status:->
        return @launched_status

    get_default_audio_player_name:->
        DCore.DEntry.get_default_audio_player_name().toLowerCase

    get_default_audio_player_icon:->
        DCore.DEntry.get_default_audio_player_icon()

    getPlaybackStatus:->
        @mpris_dbus.PlaybackStatus

    Next:->
        @mpris_dbus.Next()

    Pause:->
        @mpris_dbus.Pause()

    Play:->
        @mpris_dbus.Play()

    PlayPause:->
        @mpris_dbus.PlayPause()

    Previous:->
        @mpris_dbus.Previous()

    Seek:->
        @mpris_dbus.Seek()

    SetPosition:->
        @mpris_dbus.SetPosition()

    getPosition:->
        @mpris_dbus.Position

    Stop:->
        @mpris_dbus.Stop()

    getVolume:->
        @mpris_dbus.Volume

    setVolume:(val)->
        if val > 1 then val = 1
        else if val < 0 then val = 0
        @mpris_dbus.Volume = val

    getMetadata:->
        @mpris_dbus.Metadata

    getTitle:->
        @mpris_dbus.Metadata['xesam:title']

    getUrl:->
        #www url
        @mpris_dbus.Metadata['xesam:url']

    getalbum:->
        #zhuanji name
        @mpris_dbus.Metadata['xesam:album']

    getArtist:->
        #artist name
        @mpris_dbus.Metadata['xesam:artist']

    getArtUrl:->
        #artist img
        @mpris_dbus.Metadata['mpris:artUrl']

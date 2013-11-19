#Copyright (c) 2011 ~ 2012 Deepin, Inc.
#              2011 ~ 2012 snyh
#
#Author:      snyh <snyh@snyh.org>
#Maintainer:  snyh <snyh@snyh.org>
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

class Module
    moduleKeywords = ['extended', 'included']
    @extended : (obj) ->
        for key, value of obj when key not in moduleKeywords
            @[key] = value
        obj.extended?.apply(@)
        this

    @included: (obj, parms) ->
        for key, value of obj when key not in moduleKeywords
            @::[key] = value
        obj.included?.apply(@)
        obj.__init__?.call(@, parms)

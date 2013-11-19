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

$ = (q, o) ->
    return $s(q,o)?[0]

$s = (q, o) ->
    if typeof(q) != 'string'
        div = q
        selector = o
    else
        div = document
        selector = q

    switch selector.charAt(0)
        when '#' then return [div.getElementById(selector.substr(1))]
        when '.' then return div.getElementsByClassName(selector.substr(1))
        else
            return div.getElementsByTagName(selector)

/*
-*- coding: utf-8 -*-

Copyright (C) 2012~2013 Deepin, Inc.
              2012~2013 Long Wei

Author:     Long Wei <yilang2007lw@gmail.com>
Maintainer: Long Wei <yilang2007lw@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gio/gio.h>
#include <assert.h>

GUnixMountEntry * get_mount_entry(char *mpath,char *devpath,char *fstype,gboolean read_only,gboolean system_internal)
{
    GUnixMountEntry *gMountEntry=(struct GUnixMountEntry *)malloc(sizeof(GUnixMountEntry));
    assert(gMountEntry!=NULL);
    gMountEntry->mount_path=mpath;
    gMountEntry->device_path=devpath;
    gMountEntry->filesystem_type=fstype;
    gMountEntry->is_read_only=read_only;
    gMountEntry->is_system_internal=system_internal;

    return gMountEntry;
}

GVolume * get_mount_volume(char *devpath)
{
     GVolume *volume=NULL;
     GVolumeMonitor *monitor=g_volume_monitor_get();
     GList * volume_list=g_volume_monitor_get_volumes();
     for(i=0;i<volume_list.length();i++)
     {
          if (g_volume_get_identifier(volume_list[i],G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE)==devpath)
          {
               volume=volume_list[i];
          }
     }
     return volume;
}

int mount_volume(GUnixMountEntry *gMountEntry)
{
    


}


int main(int argc,char **argv)
{
    ;
    return 0;
}
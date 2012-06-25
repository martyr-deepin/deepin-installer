#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012~2013 Deepin, Inc.
#               2012~2013 Long Wei
#
# Author:     Long Wei <yilang2007lw@gmail.com>
# Maintainer: Long Wei <yilang2007lw@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TARGET="/target"
PART_TYPE_LIST=["primary","logical","extend","freespace","metadata","protect"]
import os
from basic_utils import run_os_command,get_os_command_output
import parted
import re
from log_util import LogUtil

class PartUtil:
    '''user interface communicate with backend via disk_partition_info_tab,mark each partition in the table:
    keep:the origin partition,we don't want and don't need to change
    add:new added partition,we should do add partition
    delete:to added partition,just remove from the table;else mark delete flag,then do real delete operation
    '''
    def __init__(self):
    #for temporay used variable    
        self.device=""
        self.devices=[]
        self.disk=""
        self.disks=[]
        self.partition=""
        self.partitions=[]
    #{dev_path:disk}
        self.path_disks=self.__get_path_disks()
    #{disk:[partition1,partition2,],disk2:[]}
        self.path_disks_partitions=self.__get_path_disks_partitions()
    #{disk:[ partition,part_type,part_size,part_tuple,part_fs,
    #        part_format,part_name,part_mountpoint,part_location,part_flag
    #      ]
    #}
        self.disk_partition_info_tab=self.init_disk_partition_info_tab()
    #{disk:{partition:part_path}}
        self.disk_part_display_path=self.init_disk_part_display_path()
    #{disk:["freepart",geometry]}    
        self.disk_geom_info_tab={}
        self.init_disk_geom_info_tab()

        self.lu=LogUtil()
        self.logger=self.lu.create_logger("part_logger","debug")
        self.handler=self.lu.create_logger_handler(self.logger,"file","debug","lineno",None)

        self.backup_disk_partition_info_tab=self.init_backup_disk_partition_info_tab()
        # self.backup_disk_partition_info_tab=copy.deepcopy(self.disk_partition_info_tab)

    #######################fill in data relative to disk,mostly in physical#######################
    def __get_path_disks(self):
        '''return{ path:disk} dict,called this only once to make sure the Ped object id will not change'''
        self.path_disks={}
        self.devices=parted.getAllDevices()
        for device in self.devices:
            if "/dev/sd" in device.path or "/dev/hd" in device.path:
                try:
                    self.disk=parted.disk.Disk(device,None)
                except:
                    self.disk=self.set_disk_label(device)
                self.path_disks[device.path]=self.disk   

        return self.path_disks

    def __get_path_disk_partitions(self,disk):
        '''return [partition],only called by __get_path_disks_partitons'''
        partitions = []
        partition = disk.getFirstPartition()
        while partition:
            if partition.type & parted.PARTITION_FREESPACE or \
               partition.type & parted.PARTITION_METADATA or \
               partition.type & parted.PARTITION_PROTECTED:
                partition = partition.nextPartition()
                continue
            partitions.append(partition)
            partition = partition.nextPartition()

        return partitions

    def __get_path_disks_partitions(self):
        '''return {disk:[partition1,partition2],disk2:[partition1]},calld this only once to keep Ped partition id uniquee'''
        path_disks_partitons={}
        for disk in self.get_system_disks():
            path_disks_partitons[disk]=self.__get_path_disk_partitions(disk)
            
        return path_disks_partitons    

    def get_install_device_info(self):
        '''return dict {/dev/sda:size,/dev/sdb:size} to choose which to install linux deepin'''
        self.devices=parted.getAllDevices()
        self.install_info={}
        for device in self.devices:
            if "/dev/sd" in device.path or "/dev/hd" in self.device.path:
                dev_path=device.path
                dev_size=str(device.getSize(unit="GB"))+"GB"
                self.install_info[dev_path]=dev_size

        return self.install_info

    def get_system_disks(self):
        '''return list of system disks,fetch the global path_disks'''
        self.disks=[]
        for item in self.path_disks.values():
            self.disks.append(item)
        return self.disks    

    def get_disk_from_path(self,dev_path):
        '''from path:/dev/sda to get disk,fetch the global path_disks need to keep the only Ped id'''
        if dev_path in self.path_disks.keys():
            return self.path_disks[dev_path]
        else:
            print "there's no disk specified by the path"
            self.lu.do_log_msg(self.logger,"critical","there's no disk specified by the path")
            return None

    def get_device_from_path(self,dev_path):
        '''from path:/dev/sda to get device,old function,avoid to use this'''
        return parted.device.Device(dev_path,None)

    def get_disk_size(self,disk):
        '''get disk size,used to compare with the optimize swap size'''
        return disk.device.getSize("MB")

    def get_disk_max_primary_count(self,disk):
        '''return max primary NO. the disk can hold '''
        self.max_primary_count=disk.MaxPrimaryPartitionCount()
        if len(self.max_primary_count)==0:
            self.max_primary_count=4
        return self.max_primary_count    

    def get_disk_max_support_count(self,disk):
        '''return max support NO. the disk can hold'''
        self.max_support_count=disk.maxSupportedPartitionCount()
        return self.max_support_count

    ##################disk->partiton structure,in logical concept,not react to the physical disk################
    def get_disk_partitions(self,disk):
        '''return partitions of the given disk,get value from path_disks_partitions'''
        return self.path_disks_partitions[disk]

    def get_disk_partition_mount(self,partition):
        '''get partition mount info,need consider multiple mount '''
        mountinfo=[]
        try:
            part_path=partition.path
            mtab=get_os_command_output("cat /etc/mtab")
            for item in mtab:
                if item.startswith(part_path):
                    mountinfo.append(item.split())
        except:
            print "cann't get mount info"
        return mountinfo        

    def get_disk_primary_list(self,disk):
        '''return list of disk primary partitions,not include marked delete'''
        disk_primary_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_primary_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="primary" or item[0].type==0:
                disk_primary_list.append(item)

        return disk_primary_list                        

    def get_disk_logical_list(self,disk):
        '''return list of disk logical partitions,not include marked delete'''
        disk_logical_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_logical_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="logical" or item[0].type==1:
                disk_logical_list.append(item)
        return disk_logical_list

    def get_disk_main_list(self,disk):
        '''return list of disk primary/extend partitions,not include marked delete'''
        disk_main_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_main_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="primary" or item[1]=="extend":
                disk_main_list.append(item)
        return disk_main_list        

    def get_disk_extend_list(self,disk):
        '''return list of disk extend partitions,not include marked delete'''
        disk_extend_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_extend_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="extend" or item[0].type==2:
                disk_extend_list.append(item)
        return disk_extend_list

    ##################generate part path when have add or delete partition###################
    def init_disk_part_display_path(self):
        '''display_path for vitual path to display in UI listview'''
        disk_part_display_path={}
        for disk in self.get_system_disks():
            disk_part_display_path[disk]={}
            for part in self.get_disk_partitions(disk):
                disk_part_display_path[disk][part]=part.path

        return disk_part_display_path       

    def get_new_add_part_path(self,disk,part_obj):
        '''get new added part path of disk_part_display_path and react to the dict'''
        main_part_list=filter(lambda item :item.type==0 or item.type==2,self.disk_part_display_path[disk].keys())
        extend_part_list=filter(lambda item: item.type==2,self.disk_part_display_path[disk].keys())
        logical_part_list=filter(lambda item :item.type==1,self.disk_part_display_path[disk].keys())

        new_part_path=""
        max_num=0
        if part_obj.type==0:
            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                new_part_path=disk.device.path+str(1)
            else:
                for part in main_part_list:
                    part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                    if part_no > max_num:
                        max_num=part_no

                new_part_path=disk.device.path+str(int(max_num)+1)

            self.disk_part_display_path[disk][part_obj]=new_part_path
            return self.disk_part_display_path

        elif part_obj.type==2:
            for part in main_part_list:
                if part.type==2:
                    print "can have only one extend "
            if len(main_part_list) > 3:
                print "can at most have 4 primary partition"
            elif len(main_part_list)==0:
                new_part_path=disk.device.path+str(1)
            else:
                for part in main_part_list:
                    part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                    if part_no > max_num:
                        max_num=part_no

                new_part_path=disk.device.path+str(int(max_num)+1)

            self.disk_part_display_path[disk][part_obj]=new_part_path
            return self.disk_part_display_path

        elif part_obj.type==1:
            if len(extend_part_list)!=1:
                print "must have only one extend part first"
                return self.disk_part_display_path

            if len(logical_part_list)==0:
                new_part_path=disk.device.path+str(5)
            else:
                for part in logical_part_list:
                    part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                    if part_no > max_num:
                        max_num=part_no

                new_part_path=disk.device.path+str(int(max_num)+1)

            self.disk_part_display_path[disk][part_obj]=new_part_path
            return self.disk_part_display_path

        else:
            print "invalid part type"
            return self.disk_part_display_path

    def get_delete_part_other_path(self,disk,part_obj):
        '''update the disk_part_display_path when delete a partition'''
        main_part_list=filter(lambda item :item.type==0 or item.type==2,self.disk_part_display_path[disk].keys())
        extend_part_list=filter(lambda item: item.type==2,self.disk_part_display_path[disk].keys())
        logical_part_list=filter(lambda item :item.type==1,self.disk_part_display_path[disk].keys())

        if part_obj not in self.disk_part_display_path[disk].keys():
            print "part_obj not in disk_part_display_path,some error occurs"

        current_num=int(filter(str.isdigit,self.disk_part_display_path[disk][part_obj])[:])    

        if part_obj.type==0:
            if len(main_part_list)==1:
                del self.disk_part_display_path[disk][part_obj]
                return self.disk_part_display_path
            else:    
                part_prefix=re.findall(r'[^0-9]+',self.disk_part_display_path[disk][part_obj])[0]
                for part in main_part_list:
                    part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                if part_no > current_num:
                        self.disk_part_display_path[disk][part]=part_prefix+str(part_no-1)
                del self.disk_part_display_path[disk][part_obj]
                return self.disk_part_display_path

        elif part_obj.type==2:
            if len(logical_part_list)!=0:
                for part in logical_part_list:
                    del self.disk_part_display_path[disk][part]

            part_prefix=re.findall(r'[^0-9]+',self.disk_part_display_path[disk][part_obj])[0]
            for part in main_part_list:
                part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                if part_no > current_num:
                    self.disk_part_display_path[disk][part]=part_prefix+str(part_no-1)

            del self.disk_part_display_path[disk][part_obj]
            return self.disk_part_display_path

        elif part_obj.type==1:
            if len(extend_part_list)!=1:
                print "must have one extend part first"
                return self.disk_part_display_path
            if len(logical_part_list)==1:
                del self.disk_part_display_path[disk][part_obj]
            else:
                part_prefix=re.findall(r'[^0-9]+',self.disk_part_display_path[disk][part_obj])[0]
                for part in logical_part_list:
                    part_no=int(filter(str.isdigit,self.disk_part_display_path[disk][part])[:])
                    if part_no > current_num:
                        self.disk_part_display_path[disk][part]=part_prefix+str(part_no-1)
                del self.disk_part_display_path[disk][part_obj]
                return self.disk_part_display_path

        else:
            print "invalid part type"
            return self.disk_part_display_path


    ################################# disk_partition_info_tab operations:#############################
    def init_disk_partition_info_tab(self):
        '''read origin disk partition info'''
        disk_partition_info_tab={}
        disk_partition_info_tab_item=[]

        for disk in self.get_system_disks():
            disk_partition_info_tab[disk]=[]

            if disk.getFirstPartition()==None:
                self.set_disk_label(disk.device)
                continue 
            for part in self.get_disk_partitions(disk):
                part_type=PART_TYPE_LIST[part.type]
                part_size=part.getSize(unit="MB")
                start=part.geometry.start
                length=part.geometry.length
                end=part.geometry.end
                part_tuple=(start,length,end)
                
                try:
                    part_fs=part.fileSystem.type#just for primary and logical partition
                except:
                    part_fs=None
                part_format=False#donn't format origin partition
                try:
                    part_name=part.name
                except:
                    part_name=""
                try:
                    part_mountpoint=self.get_disk_partition_mount(part)[0][1]
                except:
                    part_mountpoint=""
                part_location="start"#start/end
                part_flag="keep"   #flag:keep,new,delete 

                disk_partition_info_tab_item=[part,part_type,part_size,part_tuple,part_fs,part_format,
                                              part_name,part_mountpoint,part_location,part_flag]

                disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

        return disk_partition_info_tab

    def rebuild_disk_partition_info_tab(self,disk):
        '''backend operation for UI:create new disk partition tab'''
        self.disk_partition_info_tab[disk].clear()
        self.disk_part_display_path[disk].clear()        
        self.path_disks_partitions[disk].clear()
        self.disk_geom_info_tab[disk].clear()
            
    def recovery_disk_partition_info_tab(self,disk):
        '''backend operation for UI:recovery edited disk partition tab'''
        #init disk_partition_info_tab
        del self.disk_partition_info_tab[disk]
        self.disk_partition_info_tab[disk]=[]

        for item in self.backup_disk_partition_info_tab[disk]:
            item_list=[]
            for i in item:
                item_list.append(i)
            self.disk_partition_info_tab[disk].append(item_list)    

        #init disk_part_display_path        
        if disk in self.disk_part_display_path.keys():
            self.disk_part_display_path[disk].clear()
            del self.disk_part_display_path[disk]
        self.disk_part_display_path[disk]={}    
        for part in self.get_disk_partitions(disk):
            self.disk_part_display_path[disk][part]=part.path

        self.path_disks_partitions[disk]=self.__get_path_disk_partitions(disk)
        ###to be implemented
        # self.disk_geom_info_tab[disk]=self.init_disk_geom_info_tab()

    def get_disk_partition_info_tab_item(self,disk,part):
        '''get the item of disk_partition_info_tab specified by part'''
        for item in self.disk_partition_info_tab[disk]:
            if item[0]==part:
                return item
            else:
                continue
        else:
            print "no item in disk_partition_info_tab specified by part"

    def update_disk_partition_info_tab_item(self,disk,part,item_info):
        '''update the item info of disk_partition_info_tab specified by part'''
        item=self.get_disk_partition_info_tab_item(disk,part)
        if item==None or len(item)==0:
            print "doesn't find the part in disk_partition_info_tab"
        else:
            self.disk_partition_info_tab[disk].remove(item)
            self.disk_partition_info_tab[disk].append(item_info)
        return self.disk_partition_info_tab    

    def add_disk_partition_info_tab(self,disk,part_type,part_size,part_tuple,part_fs,part_format,part_name,part_mountpoint,part_location):
        '''add partition to the table,insert_path_disks_partitions in get_disk_partition_object because it's used
        not only for add_disk_partition_info_tab,but also the real add partition operate'''
        if part_type=="logical" and len(self.get_disk_extend_list)==0:
            geometry=parted.geometry.Geometry(disk.device,part_tuple[0],part_tuple[1],part_tuple[2],None)
            self.add_disk_extended_partition(disk,geometry)

        self.to_add_partition=self.get_disk_partition_object(disk,part_type,part_size,part_tuple,part_fs,part_location)
        if self.to_add_partition==None:
            print "partition is null"
            return 

        part_flag="add"   
        disk_partition_info_tab_item=[self.to_add_partition,part_type,part_size,part_tuple,part_fs,part_format,
                                      part_name,part_mountpoint,part_location,part_flag]
        self.disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

        if part_type!=2:
            self.add_part_geom_info_tab(disk,self.to_add_partition.geometry)
        else:
            pass
        return self.disk_partition_info_tab

    def mark_disk_partition_info_tab(self,part,part_flag):
        '''part_flag:keep,add,delete,according the flag to adjust the disk_partition_info_tab
           Then I can do partition add delete without bother the existed partition
        '''
        if(part==None):
            print "partition doesn't exist"
            self.lu.do_log_msg(self.logger,"error","partition doesn't exist")
        for item in self.disk_partition_info_tab[part.disk]:
            if item[0]==part:
                item[-1]=part_flag
            else:
                continue

        return self.disk_partition_info_tab

    def delete_disk_partition_info_tab(self,part):
        '''for origin partition,marked delete;for to added partition,rm from the table;'''
        if(part==None):
            print "partition doesn't exist"
            return

        for item in self.disk_partition_info_tab[part.disk]:
            if item[0]==part and item[-1]=="add":
                self.disk_partition_info_tab[part.disk].remove(item)
                self.delete_path_disks_partitions(part.disk,part)
                if part.type!=2:
                    self.delete_part_geom_info_tab(part.disk,part.geometry)
            elif item[0]==part and item[-1]=="keep":
                self.mark_disk_partition_info_tab(part,"delete")
                if part.type!=2:
                    self.delete_part_geom_info_tab(part.disk,part.geometry)
            else:
                print "invalid,if partition marked delete,you wonn't see it"
                self.lu.do_log_msg(self.logger,"error","invalid,if partition marked delete,you wonn't see it ")
                break
            
        return self.disk_partition_info_tab

    def probe_tab_disk_has_extend(self,disk):
        '''probe extend partition,for except handle add logical without extend'''
        Flag=False
        for item in self.disk_partition_info_tab[disk]:
            if item[0].type==parted.PARTITION_EXTENDED or item[1]=="extend":
                Flag=True
            else:
                continue
        return Flag    

    def init_backup_disk_partition_info_tab(self):
        '''copy from init_disk_partition_info_tab,as _Ped object not support simply deepcopy'''
        disk_partition_info_tab={}
        disk_partition_info_tab_item=[]

        for disk in self.get_system_disks():
            disk_partition_info_tab[disk]=[]

            if disk.getFirstPartition()==None:
                self.set_disk_label(disk.device)
                continue 
            for part in self.get_disk_partitions(disk):
                part_type=PART_TYPE_LIST[part.type]
                part_size=part.getSize(unit="MB")
                start=part.geometry.start
                length=part.geometry.length
                end=part.geometry.end
                part_tuple=(start,length,end)
                
                try:
                    part_fs=part.fileSystem.type#just for primary and logical partition
                except:
                    part_fs=None
                part_format=False#donn't format origin partition
                try:
                    part_name=part.name
                except:
                    part_name=""
                try:
                    part_mountpoint=self.get_disk_partition_mount(part)[0][1]
                except:
                    part_mountpoint=""
                part_location="start"#start/end
                part_flag="keep"   #flag:keep,new,delete 

                disk_partition_info_tab_item=[part,part_type,part_size,part_tuple,part_fs,part_format,
                                              part_name,part_mountpoint,part_location,part_flag]

                disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

        return disk_partition_info_tab
    
    ###################update disk extended part when necessary###################################
    def add_disk_extended_partition(self,disk,geometry):
        '''add extended partition,called this function passive,when add logical and doesn't have an extend
           add-->disk_partition_info_tab
              -->path_disks_partitions
        '''
        if len(self.get_disk_extend_list[disk])!=0:
            print "no need to add extend partition"
        else:
            extend_part=parted.partition.Partition(disk,parted.PARTITION_EXTENDED,None,geometry,None)
            disk_partition_info_tab_item=[extend_part,"extend",geometry.length*disk.device.sectorSize,
                                          (geometry.start,geometry.length,geometry.end),None,False,None,None,"start","add"]
            self.disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

            self.insert_path_disks_partitions(disk,extend_part)

    def delete_disk_extended_partition(self,disk,geometry):
        '''delete extened partition,called this function passive'''
        if len(self.get_disk_logical_list[disk])!=0:
            print "cann't delete extended part since there still have logical one"
        else:
            print "to be implemented"

    def update_extended_disk_partition_info_tab(self,disk,extend_part):
        '''update extended part info in disk_partition_info_tab,mostly update the geometry'''
        pass

    def get_disk_extended_partition_geometry(self,disk,extend_part):
        '''get the geometry of the current extended part'''
        if extend_part.geometry!=None:
            return extend_part.geometry

    def set_disk_extended_partition_geometry(self,disk,extend_part,geom):
        '''set the geometry of the current extended part'''
        constraint=parted.constraint.Constraint(exactGeom=geom)
        start=geom.start
        end=geom.end
        disk.setPartitionGeometry(extend_part,constraint,start,end)
        for item in self.disk_partition_info_tab[disk]:
            if item[0]==extend_part and item[1]=="extend":
                item[3]=(geom.start,geom.length,geom.end)

    def grown_disk_extended_partition_geometry(self,disk,extend_part,geom_tuple):
        '''grown extended geometry since add logical in extra freespace'''
        if extend_part!=None:
            ori_start=extend_part.geometry.start
            ori_end=extend_part.geometry.end
        else:
            print "error,invalid extend_part argument"

        if len(self.get_disk_logical_list(disk))==0 or len(self.get_disk_extend_list)==0:
            print "no need to grown extended geometry as no logical part"
        elif ori_start < geom_tuple[0] and ori_end > geom_tuple[2]:
            print "no need to grown as origin size bigger"
        elif ori_start > geom_tuple[0] and self.get_prev_geom_info_tab_item(disk,extend_part.geometry)[0]=="part":
            print "cann't grown size to prev used space"
        elif ori_end < geom_tuple[0] and self.get_next_geom_info_tab_item(disk,extend_part.geometry)[0]=="part":
            print "cann't grown size to next used space"
        else:
            start=min(ori_start,geom_tuple[0])
            end=max(ori_end,geom_tuple[2])
            length=end-start+1
            new_geom=parted.geometry.Geometry(disk.device,start,length,end,None)
            self.set_disk_extended_partition_geometry(self,disk,extend_part,new_geom)
            for item in self.disk_partition_info_tab[disk]:
                if item[0]==extend_part:
                    item[3]=(extend_part.geometry.start,extend_part.geometry.length,extend_part.geometry.end)

    def reduce_disk_exended_partition_geometry(self,disk,extend_part,geom_tuple):
        '''reduce extended geometry since add primary with freespace in origin extended'''
        if len(self.get_disk_extend_list(disk))==0:
            print "no extend part"
        elif len(self.get_disk_logical_list(disk))==0:
            start=max(extend_part.geometry.start,geom_tuple[0])
            end=min(extend_part.geometry.end,geom_tuple[2])
            length=end-start+1
            new_geom=parted.geometry.Geometry(disk.device,start,length,end,None)
            self.set_disk_extended_partition_geometry(disk,extend_part,new_geom)
        else:
            logical_start=min(part.geometry.start for part in self.get_disk_logical_list(disk))
            logical_end=max(part.geometry.end for part in self.get_disk_logical_list(disk))
            ori_start=extend_part.geometry.start
            ori_end=extend_part.geometry.end
            first_logical=filter(lambda part:part.geometry.start==logical_start,self.get_disk_logical_list(disk))[0]
            last_logical=filter(lambda part:part.geometry.end==logical_end,self.get_disk_logical_list(disk))[0]

            if ori_start > logical_start or ori_end < logical_end:
                print "logical geometry not in extend geometry,error"
            elif geom_tuple[0]> logical_start or geom_tuple[2] < logical_end:
                print "cann't reduce geometry into logical"
            elif geom_tuple[0] <ori_start or geom_tuple[2] > ori_end:
                print "actually grown size"
            elif ori_start <= geom_tuple[0] <= logical_start and self.get_prev_geom_info_tab_item(disk,first_logical.geometry)[0]=="part":
                print "no space before first logical"
            elif logical_end <=geom_tuple[2] <=ori_end and self.get_next_geom_info_tab_item(disk,last_logical.geometry)[0]=="part"            :
                print "no space after last logical"
            else:
                new_geom=parted.geometry.Geometry(disk.device,geom_tuple[0],geom_tuple[1],geom_tuple[2],None)
                self.set_disk_extended_partition_geometry(self,disk,extend_part,new_geom)
                for item in self.disk_partition_info_tab[disk]:
                    if item[0]==extend_part:
                        item[3]=(extend_part.geometry.start,extend_part.geometry.length,extend_part.geometry.end)

    ##############update path_disks_partitons structure when add/delete partition##################

    def insert_path_disks_partitions(self,disk,partition):
        '''insert new added partition to path_disk_partitions variable,keep partition id uniquee'''
        if disk.getPedDisk!=partition.disk.getPedDisk:
            print "the partition(id) not in the disk(id)"
            self.lu.do_log_msg(self.logger,"error","somewhere id not match")
        self.path_disks_partitions[disk].append(partition)    

    def delete_path_disks_partitions(self,disk,partition):
        '''delete partition from path_disks_partitions variable,keep partition id uniquee'''
        if disk.getPedDisk!=partition.disk.getPedDisk:
            print "the partition(id) not in the disk(id)"
            self.lu.do_log_msg(self.logger,"error","somewhere id not match")
        self.path_disks_partitions[disk].remove(partition)


    def set_disk_label(self,device):
        '''set disk label:gpt or msdos,for blank disk'''
        if device.getSize() >= 1.5*1024*1024:
            self.disk=parted.freshDisk(device,"gpt")
        else:
            self.disk=parted.freshDisk(device,"msdos")
            
        return self.disk

    def get_disk_partition_size(self,partition):
        '''return partition size'''
        return partition.getSize("MB")

    #####################probe geom layout of the disk########################################
    def init_disk_geom_info_tab(self):
        '''init_disk_geom_info_tab,not include the extended partition'''
        self.disk_geom_info_tab={}#

        for disk in self.get_system_disks():
            self.disk_geom_info_tab[disk]=[]
            main_part_list=self.get_disk_main_list(disk)

            part_geom_list=[]#[["part",geometry]]
            gap_geom_list=[]#[["freespace",geometry]]
            space_geom_list=[]

            if len(main_part_list==0):
                print "a whole blank disk"
                part_geom_list=[]
            else:
                for part in self.get_disk_partitions(disk):
                    if part.type==2:
                        pass
                    else:
                        part_geom_list.append(["part",part.geometry])
                part_geom_list.sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
                
            if len(part_geom_list)==0 or len(main_part_list)==0:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=disk.getFreeSpaceRegions()[-1].end-4
                length=end-start+1
                gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])

            else:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=part_geom_list[0].start-4
                length=end-start+1
                if length > 0:
                    gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                else:
                    print "start of disk have geometry overlap or disk size too small"
                    self.lu.do_log_msg(self.logger,"warning","start of disk have geometry overlap or disk size too small")

                for i in range(len(part_geom_list)-1):
                    start=part_geom_list[i].end+4
                    end=part_geom_list[i+1].start-4
                    length=end-start+1
                    if length > 0:
                        gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                    else:
                        print "the size between two partition is too small"
                        self.logger.warning("the size between two partition is too small")
                        self.lu.do_log_msg(self.logger,"warning","the size between two partition is too small")
                    i=i+1    
        
                start=part_geom_list[-1].end+4    
                end=disk.getFreeSpaceRegions()[-1].end-4 
                length=end-start+1
                if length > 0:
                    gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                else:
                    print "end of disk have geometry overlap or disk size too small"
                    self.logger.warning("end of disk have geometry overlap or disk size too small")
                    self.lu.do_log_msg(self.logger,"warning","end of disk have geometry overlap or disk size too small")

            space_geom_list=part_geom_list.extend(gap_geom_list)      
            space_geom_list.sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
            self.disk_geom_info_tab[disk]=space_geom_list

        return self.disk_geom_info_tab    

    def get_disk_geom_info_tab(self,disk):
        '''get geom layout of the disk:include partition and freespace'''
        # self.init_disk_geom_info_tab()
        return self.disk_geom_info_tab[disk]

    def get_prev_geom_info_tab_item(self,disk,geometry):
        '''get previous space block of the geometry:part or freespace'''
        self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        prev_item=filter(lambda item:item[-1].end <= geometry.start,self.disk_geom_info_tab[disk])[-1]
        return prev_item

    def get_next_geom_info_tab_item(self,disk,geometry):
        '''get next space block of the geometry:part or freespace'''
        self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        next_item=filter(lambda item:item[-1].start >= geometry.end,self.disk_geom_info_tab[disk])[0]
        return next_item

    def get_current_geom_info_tab_item(self,disk,geometry):
        '''get current space block of the geometry:part or freespace'''
        self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        current_item=filter(lambda item:item[-1].start <=geometry.start and item[-1].end >=geometry.end,
                            self.disk_geom_info_tab[disk])[0]
        return current_item

    def get_space_geom_size(self,disk,geometry):
        '''get size of the geometry:part or freespace'''
        pass

    def get_part_from_geom_info_tab(self,disk,geometry):
        '''return part obj match the given geometry'''
        pass

    def add_part_geom_info_tab(self,disk,geometry):
        '''update disk_geom_info_tab when add partition from UI'''
        for item in self.disk_geom_info_tab[disk]:
            if item[0]=="part":
                pass
            elif item[0]=="freespace":
                if item[-1].start <=geometry.start and item[-1].end >= geometry.end:
                    if geometry.start - item[-1].start > 8:
                        begin_start=item[-1].start
                        begin_end=geometry.start
                        begin_length=begin_end-begin_start+1
                        begin_geometry=parted.geometry.Geometry(disk.device,begin_start,begin_length,begin_end,None)
                        self.disk_geom_info_tab[disk].append(["freespace",begin_geometry])

                    self.disk_geom_info_tab[disk].append(["part",geometry])    

                    if  item[-1].end - geometry.end > 8:
                        after_start=geometry.end
                        after_end=item[-1].end
                        after_length=after_end-after_start+1
                        after_geometry=parted.geometry.Geometry(disk.device,after_start,after_length,after_end,None)
                        self.disk_geom_info_tab[disk].append(["freespace",after_geometry])

                    self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))    
                else:
                    print "invalid freespace"
            else:
                print "unknown space"

    def delete_part_geom_info_tab(self,disk,geometry):
        '''update disk_geom_info_tab when delete partition from UI'''
        for item in self.disk_geom_info_tab[disk]:
            if item[0]=="freespace":
                pass
            elif item[0]=="part":
                if item[-1].start<=geometry.start and item[-1].end>=geometry.end:
                    self.disk_geom_info_tab[disk].remove(item)
                    prev_item=self.get_prev_geom_info_tab_item(disk,geometry)
                    if prev_item[0]=="freepart":
                        start=prev_item[-1].start
                        self.disk_geom_info_tab[disk].remove(prev_item)
                    else:
                        start=geometry.start
                    next_item=self.get_next_geom_info_tab_item(disk,geometry)
                    if next_item[0]=="freepart":
                        end=next_item[-1].end
                        self.disk_geom_info_tab[disk].remove(next_item)
                    else:
                        end=geometry.end

                    length=end-start+1
                    geom=parted.geometry.Geometry(disk.device,start,length,end,None)
                    self.disk_geom_info_tab[disk].append("freepart",geom)
                    self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
                else:
                    continue
            else:
                print "unknown space"
                

    ################set disk partition attribute before add or modify########################
    def get_disk_partition_object(self,disk,part_type,part_size,geom_tuple,part_fs,part_location):
        '''get partition_object for add to the disk_partition_info_tab,also for actual partition add operation
           add the part_obj to path_disks_partitions when it birth
        '''
        self.type=self.set_disk_partition_type(disk,part_type)
        self.geometry=self.set_disk_partition_geometry(disk,part_size,geom_tuple,part_location)
        self.fs=parted.filesystem.FileSystem(part_fs,self.geometry,False,None)
        self.partition=parted.partition.Partition(disk,self.type,self.fs,self.geometry,None)
        self.insert_path_disks_partitions(disk,self.partition)
        
        return self.partition

    def set_disk_partition_type(self,disk,part_type):
        '''check the to added partition type,need consider the count of primary,extend,etc...'''
        if part_type=="primary":
            self.type=parted.PARTITION_NORMAL
        elif part_type=="extend":
            self.type=parted.PARTITION_EXTENDED
        elif part_type=="logical":
            self.type=parted.PARTITION_LOGICAL
        else:
            print "part type error"
            self.lu.do_log_msg(self.logger,"error","part type error")
        return self.type    

    def get_part_geom_tuple(self,geometry):
        '''return tuple (start,length,end) of the geometry object'''
        start=geometry.start
        length=geometry.length
        end=geometry.end

        return (start,length,end)

    def set_disk_partition_geometry(self,disk,part_size,geom_tuple,part_location):
        '''to get geometry of new added partition'''
        import math
        minlength=math.floor((part_size*1024*1024)/(disk.device.sectorSize)+1)
        (start,length,end)=geom_tuple
        if minlength > length:
            print "the free space too small to hold the partition"
            part_start=start
            part_length=length
            part_end=end
        else:
            if part_location=="start" or len(part_location)==0:
                part_start=start
                part_length=minlength
                part_end=part_start+part_end-1
            elif part_location=="end":
                part_end=end
                part_length=minlength
                part_start=part_end-part_length+1

        self.geometry=parted.geometry.Geometry(disk.device,part_start,part_length,part_end,None)
        return self.geometry

    def set_disk_partition_name(self,partition,part_name):
        '''cann't set this attribute,need to fix'''
        if not partition.disk.supportsFeature(parted.DISK_TYPE_PARTITION_NAME):
            print "sorry,can't set partition name"
            self.lu.do_log_msg(self.logger,"warning","sorry,conn't set partition name")
        elif part_name==None or len(part_name)==0:
            partition.name=""
        else:
            partition.name=part_name

    def set_disk_partition_fstype(self,partition,fstype):
        '''format the partition to given fstype,not create the parted fs object'''
        if partition.type!=parted.PARTITION_NORMAL and partition.type!=parted.PARTITION_LOGICAL:
            print "you can only set filesystem for primary and logical partition"
            self.lu.do_log_msg(self.logger,"error","can only set fs for primary/logical partition")
            return
        for item in self.disk_partition_info_tab[partition.disk]:
            if item[0]==partition and item[5]==False:
                print "no need to format the partition"
                return 
        if fstype==None or len(fstype)==0:
            print "no filesystem specified"
            self.lu.do_log_msg(self.logger,"error","no filesystem specified")
            return

        part_path=partition.path
        #swapoff the partition before change filesystem,if not work,simply reduce code
        try:
            origin_fs_type=partition.fileSystem.type
            if origin_fs_type==fstype:
                # print "filesystem not changed"
                self.lu.do_log_msg(self.logger,"warning","filesystem not changed")
                return
            elif origin_fs_type=="linux-swap":
                swap_data=get_os_command_output("grep "+part_path+" /proc/swaps")
                if len(swap_data)!=0:
                    swap_off_command="sudo swapoff "+part_path
                    run_os_command(swap_off_command)
                else:    
                    print "swap not on"
        except:
            print "get origin_fs_type error"
            self.lu.do_log_msg(self.logger,"error","get origin_fs_type error")

        #set fstype
        if fstype=="linux-swap":
            format_command="sudo mkswap "+part_path
        elif fstype=="fat32":
            format_command="sudo mkfs.vfat "+part_path
        elif fstype in ["ext2","ext3","ext4","reiserfs","xfs","ntfs"]:
            format_command="sudo mkfs."+fstype+" -f "+part_path
        else:
            # print "invalid fstype"
            self.lu.do_log_msg(self.logger,"error","invalid fstype")
        try:    
            run_os_command(format_command)
        except:
             #umount a partition before format it to make a filesystem
            self.set_disk_partition_umount(partition)
            run_os_command(format_command)

        if fstype=="swap":
            swap_command="sudo swapon "+part_path
            run_os_command(swap_command)

    def set_disk_partition_mount(self,partition,fstype,mountpoint):
        '''mount partition to mp:new or modify,need consider various situation'''
        if mountpoint==None or len(mountpoint)==0:
            print "need mountpoint,not given"
            self.lu.do_log_msg(self.logger,"error","need mountpoint,not given")
            return 
        if partition.type==parted.PARTITION_EXTENDED:
            print "cann't mount extended partition"
            self.lu.do_log_msg(self.logger,"error","cann't mount extended partition")
            return
        part_path=partition.path
        mp=TARGET+mountpoint
        if not os.path.exists(mp):
            mkdir_command="sudo mkdir -p "+mp
            run_os_command(mkdir_command)
        if not os.path.exists(part_path):
            print "partition not exists,commit to os first"
            self.lu.do_log_msg(self.logger,"error","partition not exists,commit to os first")
            return 
        mount_command="sudo mount -t "+fstype+" "+part_path+" "+mp
        run_os_command(mount_command)
        
    def set_disk_partition_umount(self,partition):
        '''umount the partition,may used before remove a partition'''
        part_path=partition.path
        mount_flag=False
        mtab=get_os_command_output("cat /etc/mtab")
        umount_command="sudo umount "+part_path
        umount_busy_command="sudo umount -l "+part_path
        # umount_busy_command="sudo fuser -mk "+part_path
        for item in mtab:
            if item.startswith(part_path):
                mount_flag=True
            else:
                continue
        if mount_flag==True:        
            if partition.busy:
                run_os_command(umount_busy_command)
                # run_os_command(umount_command)
            else:
                run_os_command(umount_command)
        else:
            print "don't need to umount"+part_path
            return

    def set_disk_partition_flag(self,partition,flag,state):
        '''set the partition status to state'''
        if flag not in partition.getFlagsAsString():
            print "flag invalid"
            self.lu.do_log_msg(self.logger,"warning","flag invalid")
        if not partition.isFlagAvailable():
            print "flag not available"
            self.lu.do_log_msg(self.logger,"warning","flag not available")
        else:    
            if state=="on" | state=="True":
                partition.setFlag()
            else:
                partition.unsetFlag()


    #####################backend operation for add/delete partition###########################
    def delete_disk_partition(self,disk,partition):
        '''atom function:delete the given partition,called only because need delete original partition'''
        self.partitions=self.get_disk_partitions(disk)
        if partition not in self.partitions:
            print "partition not in the disk"
            self.lu.do_log_msg(self.logger,"error","partition not in the disk")
            return
        if partition.type==parted.PARTITION_EXTENDED and disk.getLogicalPartitions()!=[]:
            print "need delete all logical partitions before delete extend partition"
            self.lu.do_log_msg(self.logger,"info","delete logical partitions before delete extend")
            for logical_part in disk.getLogicalPartitions():
                self.delete_path_disks_partitions(disk,logical_part)
                self.disk_partition_info_tab=filter(lambda info:info[0]!=logical_part,self.disk_partition_info_tab[disk])
                try:
                    self.set_disk_partition_umount(logical_part)
                    disk.deletePartition(logical_part)
                except:
                    print "delete logical_part failed"
                    self.lu.do_log_msg(self.logger,"error","delete logical_part failed")
        self.delete_path_disks_partitions(disk,partition)    
        self.disk_partition_info_tab=filter(lambda info:info[0]!=partition,self.disk_partition_info_tab[disk])
        try:
            self.set_disk_partition_umount(partition)
            disk.deletePartition(partition)
        except:
            print "delete partition error occurs"
            self.lu.do_log_msg(self.logger,"error","delete partition error occurs")
        disk.commit()

    def delete_custom_partition(self):
        '''batch delete origin disk partitions:'''
        for disk in self.get_system_disks():
            for item in filter(lambda info:info[-1]=="delete",self.disk_partition_info_tab[disk]):
                if item[0]==None:
                    print "partition doesn't exist"
                    self.lu.do_log_msg(self.logger,"warning","partition doesn't exist")
                else:
                    try:
                        self.delete_disk_partition(disk,item[0])
                    except:
                        print "batch delete origin partition error!"

    def add_disk_partition(self):
        '''atom function:add disk partition'''
        pass
                
    def add_custom_partition(self):
        '''batch add partition according to disk_partition_info_tab,then mount them,every disk batch commit'''
        for disk in self.get_system_disks():
            for item in filter(lambda info:info[-1]=="add",self.disk_partition_info_tab[disk]):
                if self.probe_tab_disk_has_extend(disk):
                    # print "must add extend first"
                    if item[0].type==parted.PARTITION_EXTENDED:
                        self.partition=item[0]
                        self.geometry=self.partition.geometry
                        self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                        disk.addPartition(self.partition,self.constraint)
                if item[0].type==parted.PARTITION_LOGICAL and not self.probe_tab_disk_has_extend(disk):
                    print "can't add logical partition as there is no extended partition"
                    self.lu.do_log_msg(self.logger,"error","can't add logical as no extended")
                    break
                else:
                    self.partition=item[0]
                    self.geometry=self.partition.geometry
                    self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                    disk.addPartition(self.partition,self.constraint)
            disk.commit()        
            #mkfs/setname/mount disk partitions
            for item in filter(lambda info:info[-1]=="add",self.disk_partition_info_tab[disk]):
                if item[0].type==parted.PARTITION_LOGICAL and not self.probe_tab_disk_has_extend(disk):
                    print "as no extended partition to add logical,no need to mkfs and mount"
                    break
                else:
                    self.partition=item[0]
                    self.part_fs=item[4]
                    self.part_name=item[6]
                    self.part_mountpoint=item[7]
                    self.set_disk_partition_fstype(self.partition,self.part_fs)
                    self.set_disk_partition_name(self.partition,self.part_name)
                    self.set_disk_partition_mount(self.partition,self.part_fs,self.part_mountpoint)

    ################################auto partition tools################################
    def get_memory_total(self):
        '''get physical total memory size,unit:MB'''
        memory_total_command="grep MemTotal /proc/meminfo"
        total_size=get_os_command_output(memory_total_command)
        if len(total_size)==0:
            # print "cann't get the size of total memory"
            self.lu.do_log_msg(self.logger,"error","cann't get the size of total memory")
        else:
            self.total_memory=int(filter(lambda c:c in "0123456789.",total_size[0]))

        return self.total_memory/1024

    def if_need_swap(self):
        '''decide whether to make a swap,only used in auto partition'''
        self.total_memory=self.get_memory_total()
        self.need_swap=False
        if self.total_memory < 1024:
            self.need_swap=True

        return self.need_swap    

    def adapt_autoswap_size(self,disk):
        '''get the optimize size of the swap'''
        if self.if_need_swap()==False:
            # print "no need to make swap"
            self.lu.do_log_msg(self.logger,"warning","no need to make swap")
        else:    
            self.total_memory=self.get_memory_total()
            self.disk_size=self.get_disk_size(disk)
            if self.disk_size < 40*1024:
                self.swap_size=self.total_memory
            else:
                self.swap_size=self.total_memory*2
                
        return self.swap_size    

    def auto_partition(self,disk_path):
        '''auto partition,use a single disk,ignore the variety info tab,force format'''
        self.disk=self.get_disk_from_path(disk_path)
        self.disk_size=self.get_disk_size(self.disk)
        self.total_memory=self.get_memory_total()
        #delete all partition first,the path_disks_partitions,disk_partition_info_tab automatic sync
        for part in self.get_disk_partitions[self.disk]:
            self.delete_disk_partition(self.disk,part)

        if self.if_need_swap:
            self.swap_size=self.adapt_autoswap_size(self.disk)
            self.add_disk_partition_info_tab(disk_path,"primary",self.swap_size,"linux-swap",None,None,None)

        if self.disk_size > 40*1024:
            self.add_disk_partition_info_tab(disk_path,"primary",5*1024,"ext4",None,None,"/")
            self.add_disk_partition_info_tab(disk_path,"primary",self.disk_size-self.swap_size-5*1024,"ext4",None,None,"/home")
        else:
            self.add_disk_partition_info_tab(disk_path,"primary",self.disk_size-self.swap_size,"ext4",None,None,"/")
            
        self.add_custom_disk_partition(self.disk_partition_info_tab)

#should use global part_util to keep disk/partition/device id uniquee
global_part_util=PartUtil()

def test():
    '''test function'''
    pass

if  __name__=="__main__":
    test()

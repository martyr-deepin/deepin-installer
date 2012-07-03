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
import copy

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

        self.lu=LogUtil()
        self.logger=self.lu.create_logger("part_logger","debug")
        self.handler=self.lu.create_logger_handler(self.logger,"file","debug","lineno",None)

    #{dev_path:disk}
        self.path_disks=self.__get_path_disks()
    #{disk:[partition1,partition2,],disk2:[]}
        self.path_disks_partitions=self.__get_path_disks_partitions()
    #{disk:[ partition,part_type,part_size,space_geom,part_fs,
    #        part_format,part_name,part_mountpoint,part_location,part_flag
    #      ]
    #}
        self.disk_partition_info_tab=self.init_disk_partition_info_tab()
    #{disk:{partition:part_path}}
        self.disk_part_display_path=self.init_disk_part_display_path()
    #{disk:["freespace",geometry]},use geometry object to keep match with disk_partition_info_tab    
    #attention:the geometry id of freespace is variable#
        self.disk_geom_info_tab=self.init_disk_geom_info_tab()


        self.backup_disk_partition_info_tab=self.init_disk_partition_info_tab()
        # self.backup_disk_partition_info_tab=copy.deepcopy(self.disk_partition_info_tab)
        self.backup_disk_geom_info_tab=self.init_disk_geom_info_tab()
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
                disk_primary_list.append(item[0])

        return disk_primary_list                        

    def get_disk_logical_list(self,disk):
        '''return list of disk logical partitions,not include marked delete'''
        disk_logical_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_logical_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="logical" or item[0].type==1:
                disk_logical_list.append(item[0])
        return disk_logical_list

    def get_disk_main_list(self,disk):
        '''return list of disk primary/extend partitions,not include marked delete'''
        disk_main_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_main_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="primary" or item[1]=="extend":
                disk_main_list.append(item[0])
        return disk_main_list        

    def get_disk_extend_list(self,disk):
        '''return list of disk extend partitions,not include marked delete'''
        disk_extend_list=[]
        if len(self.disk_partition_info_tab[disk])==0:
            return disk_extend_list
        for item in filter(lambda info:info[-1]!="delete",self.disk_partition_info_tab[disk]):
            if item[1]=="extend" or item[0].type==2:
                disk_extend_list.append(item[0])
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
                #need generate an geometry object have uniquee id
                space_geom=part.geometry
                # space_geom=parted.geometry.Geometry(disk.device,part.geometry.start,part.geometry.length,part.geometry.end,None)
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

                # disk_partition_info_tab_item=[part,part_type,part_size,part_tuple,part_fs,part_format,
                #                               part_name,part_mountpoint,part_location,part_flag]

                disk_partition_info_tab_item=[part,part_type,part_size,space_geom,part_fs,part_format,
                                              part_name,part_mountpoint,part_location,part_flag]

                disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

        return disk_partition_info_tab

    def rebuild_disk_partition_info_tab(self,disk):
        '''backend operation for UI:create new disk partition tab'''
        del self.disk_partition_info_tab[disk]
        self.disk_partition_info_tab[disk]=[]
        
        del self.disk_part_display_path[disk]
        self.disk_part_display_path[disk]={}

        del self.path_disks_partitions[disk]
        self.path_disks_partitions[disk]=[]
        
        del self.disk_geom_info_tab[disk]
        self.disk_geom_info_tab[disk]=[]

    def recovery_disk_partition_info_tab(self,disk):
        '''backend operation for UI:recovery edited disk partition tab'''
        self.rebuild_disk_partition_info_tab(disk)

        #init disk_partition_info_tab
        for item in self.backup_disk_partition_info_tab[disk]:
            item_list=[]
            for i in item:
                item_list.append(i)
            self.disk_partition_info_tab[disk].append(item_list)    

        #init path_disks_partitions    
        self.path_disks_partitions[disk]=[]    
        for item in self.disk_partition_info_tab[disk]:
            self.path_disks_partitions[disk].append(item[0])

        #init disk_part_display_path        
        if disk in self.disk_part_display_path.keys():
            self.disk_part_display_path[disk].clear()
            del self.disk_part_display_path[disk]
        self.disk_part_display_path[disk]={}
        for part in self.get_disk_partitions(disk):
            self.disk_part_display_path[disk][part]=part.path
    
        #init disk_geom_info_tab
        for item in self.backup_disk_geom_info_tab[disk]:    
            item_list=[]
            for i in item:
                item_list.append(i)
            self.disk_geom_info_tab[disk].append(item_list)

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

    def add_disk_partition_info_tab(self,disk,part_type,part_size,space_geom,part_fs,part_format,part_name,part_mountpoint,part_location):
        '''add partition to the table,insert_path_disks_partitions in get_disk_partition_object because it's used
        not only for add_disk_partition_info_tab,but also the real add partition operate'''
        ####attention:space_geom is a deepcopy of the geometry object of disk_geom_info_tab######
        if part_type=="logical" and len(self.get_disk_extend_list(disk))==0:
            self.add_disk_extended_partition(disk,space_geom)

        elif part_type=="logical" and len(self.get_disk_extend_list(disk))!=0:
            extend_part=self.get_disk_extend_list(disk)[0]
            if space_geom.start -4 > extend_part.geometry.start and space_geom.end + 4 < extend_part.geometry.end:
                print "extend_part space is big enough to add the logical"
            else:
                print "need grown_disk_extended_partition_geometry"
                self.grown_disk_extended_partition_geometry(disk,extend_part,space_geom)

        elif part_type=="primary" and len(self.get_disk_extend_list(disk))!=0:
            extend_part=self.get_disk_extend_list(disk)[0]
            start=extend_part.geometry.start
            end=extend_part.geometry.end
            if space_geom.start -4 >= end or space_geom.end +4 <= start:
                print "no need to smaller extend_part when add primary"
            else:
                print "need reduce extend_part geometry when add primary"
                self.reduce_disk_extended_partition_geometry(disk,extend_part,space_geom)

        self.to_add_partition=self.get_disk_partition_object(disk,part_type,part_size,space_geom,part_fs,part_location)
        if self.to_add_partition==None:
            print "error,partition is null"

        part_flag="add"   
        disk_partition_info_tab_item=[self.to_add_partition,part_type,part_size,space_geom,part_fs,part_format,
                                      part_name,part_mountpoint,part_location,part_flag]

        self.disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

        self.get_new_add_part_path(disk,self.to_add_partition)

        if part_type!=2:
            print "begin add_part_geom_info_tab when add new partitions"
            self.add_part_geom_info_tab(disk,self.to_add_partition.geometry)
            print "finish add_part_geom_info_tab when add new partition"
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
            if item[0]==part:
                if item[-1]=="add":
                    self.disk_partition_info_tab[part.disk].remove(item)
                    self.delete_path_disks_partitions(part.disk,part)
                    if part.type!=2:
                        self.delete_part_geom_info_tab(part.disk,part.geometry)
                    self.get_delete_part_other_path(part.disk,part)    

                elif item[-1]=="keep":
                    self.mark_disk_partition_info_tab(part,"delete")
                    if part.type!=2:
                        self.delete_part_geom_info_tab(part.disk,part.geometry)
                    self.get_delete_part_other_path(part.disk,part)    

                else:
                    print "invalid,if partition marked delete,you wonn't got it in UI listview"
                    self.lu.do_log_msg(self.logger,"error","invalid,if partition marked delete,you wonn't see it ")
                break
            else:
                continue
        else:    
            print "doesn't find the partition in disk_partition_info_tab to delete"

        #delete extend_part when no logical    
        if len(self.get_disk_extend_list(part.disk))==1 and len(self.get_disk_logical_list(part.disk))==0:
            print "delete extend part as no logical,when add new logical then add new extend"
            self.delete_disk_extended_partition(part.disk,self.get_disk_extend_list(part.disk)[0])

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

    
    ###################update disk extended part when necessary###################################
    def add_disk_extended_partition(self,disk,geometry):
        '''add extended partition,called this function passive,when add logical and doesn't have an extend
           add-->disk_partition_info_tab
              -->path_disks_partitions
        '''
        if len(self.get_disk_extend_list(disk))!=0:
            print "no need to add extend partition"
        else:
            start=copy.deepcopy(geometry.start)
            end=copy.deepcopy(geometry.end)
            length=end - start + 1
            
            geometry.start=geometry.start+4
            geometry.end=geometry.end-4
            geometry.length=geometry.end - geometry.start + 1

            extend_geometry=parted.geometry.Geometry(disk.device,start,length,end,None)
            extend_part=parted.partition.Partition(disk,parted.PARTITION_EXTENDED,None,extend_geometry,None)
            disk_partition_info_tab_item=[extend_part,"extend",extend_geometry.length*disk.device.sectorSize/1024*1024,
                                          extend_geometry,None,False,None,None,"start","add"]
            self.disk_partition_info_tab[disk].append(disk_partition_info_tab_item)

            self.insert_path_disks_partitions(disk,extend_part)
            self.get_new_add_part_path(disk,extend_part)

    def delete_disk_extended_partition(self,disk,extend_part):
        '''delete extened partition,when delete all logical part in the table'''
        if(extend_part==None or extend_part.type!=2):
            print "the partition already deleted or partition type invalid"
        elif len(self.get_disk_logical_list(disk))!=0:
            print "cann't delete extended part since there still have logical one"
        else:
            for item in self.disk_partition_info_tab[disk]:
                if item[0]==extend_part:
                    if item[-1]=="add":
                        self.disk_partition_info_tab[disk].remove(item)
                        self.delete_path_disks_partitions(disk,extend_part)
                        self.get_delete_part_other_path(disk,extend_part)    
                    elif item[-1]=="keep":
                        self.mark_disk_partition_info_tab(extend_part,"delete")
                        self.get_delete_part_other_path(disk,extend_part)    
                    else:
                        print "invalid,if partition marked delete,you wonn't got it in UI listview"
                        self.lu.do_log_msg(self.logger,"error","invalid,if partition marked delete,you wonn't see it ")
                    break
                else:
                    continue
            else:    
                print "doesn't find the extend partition in disk_partition_info_tab to delete"
            print "successfully delete the extended part from table"    
        return self.disk_partition_info_tab


    def update_extended_disk_partition_info_tab(self,disk,extend_part):
        '''update extended part info in disk_partition_info_tab,mostly update the geometry'''
        pass

    def get_disk_extended_partition_geometry(self,disk,extend_part):
        '''get the geometry of the current extended part'''
        if extend_part.geometry!=None:
            return extend_part.geometry

    def grown_disk_extended_partition_geometry(self,disk,extend_part,space_geom):
        '''grown extended geometry since add logical in extra freespace'''
        if extend_part!=None:
            ori_start=extend_part.geometry.start
            ori_end=extend_part.geometry.end
            logical_list=sorted(self.get_disk_logical_list(disk),key=lambda x:x.geometry.start)
            if len(logical_list)!=0:
                prev_item=self.get_prev_geom_info_tab_item(disk,logical_list[0].geometry)
                next_item=self.get_next_geom_info_tab_item(disk,logical_list[-1].geometry)
            else:
                print "grown extend_part as no logical,maybe an error occurs"
                prev_item=self.get_prev_geom_info_tab_item(disk,extend_part.geometry)
                next_item=self.get_next_geom_info_tab_item(disk,extend_part.geometry)
        else:
            print "error,invalid extend_part argument"
            return 

        if len(self.get_disk_extend_list(disk))==0:
            print "no need to grown as no extended part,error!!!"
        elif ori_start +4 < space_geom.start and ori_end -4 > space_geom.end:
            print "no need to grown as origin size bigger"
        else:
            if ori_start +4 >= space_geom.start:
                if prev_item[0]=="freespace":
                    if prev_item[-1].start <=space_geom.start -4:
                        start=space_geom.start-4
                    else:
                        start=prev_item[-1].start
                        space_geom.start=prev_item[-1].start+4
                        space_geom.length=space_geom.end - space_geom.start +1
                elif prev_item[0]=="part" and self.get_part_from_geom(disk,prev_item[-1]).type==1:
                    print "grown extended with prev logical,this should be error"
                    if prev_item[-1].start <=space_geom.start -4:
                        start=space_geom.start-4
                    else:
                        start=prev_item[-1].start
                        space_geom.start=prev_item[-1].start+4
                        space_geom.length=space_geom.end - space_geom.start + 1
                else:
                    print "cann't grown size to prev used space"
                    start=ori_start
            else:
                start=ori_start

            if ori_end -4 <= space_geom.end:
                if next_item[0]=="freespace":
                    if next_item[-1].end >= space_geom.end +4:
                        end=space_geom.end +4
                    else:
                        end=next_item[-1].end
                        space_geom.end=next_item[-1].end -4
                        space_geom.length=space_geom.end - space_geom.start + 1
                elif next_item[0]=="part" and self.get_part_from_geom(disk,next_item[-1]).type==1:
                    print "grown extended with next logical,this should be error"
                    if next_item[-1].end >= space_geom.end +4:
                        end=space_geom.end +4
                    else:
                        end=next_item[-1].end
                        space_geom.end=next_item[-1].end -4
                        space_geom.length=space_geom.end - space_geom.start + 1
                else:
                    print "cann't grown size to next used space"
                    end=ori_end
            else:
                end=ori_end
            length=end-start+1

            extend_part.geometry.start=start
            extend_part.geometry.length=length
            extend_part.geometry.end=end

    def reduce_disk_extended_partition_geometry(self,disk,extend_part,primary_geom):
        '''reduce extended geometry since add primary with freespace in origin extended'''
        if len(self.get_disk_extend_list(disk))==0:
            print "error,no extend part"
        else:
            start=extend_part.geometry.start
            length=extend_part.geometry.length
            end=extend_part.geometry.end
            logical_list=sorted(self.get_disk_logical_list(disk),key=lambda x:x.geometry.start)

            if primary_geom.start <= start and start < primary_geom.end <= end:
                print "need reduce size at the start of the extend_part"
                if len(logical_list)!=0:
                    if primary_geom.end >= logical_list[0].geometry.start - 4:
                        # print "add primary to space used by first logical,smaller the primary end to to added part"
                        # primary_geom.end = logical_list[0].geometry.start-4
                        # primary_geom.length=primary_geom.end - primary_geom.start + 1
                        print "error,cann't add primary into first logical"
                    else:    
                        extend_part.geometry.start=min(primary_geom.end+4,logical_list[0].geometry.start-4)
                        extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start + 1

                extend_part.geometry.start=primary_geom.end+4
                extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start + 1

            elif primary_geom.start > start and start < primary_geom.end <=end:
                print "add primary into old extend_part,need reduce size at the start of the extend_part"
                if len(logical_list)!=0:
                    if primary_geom.end <= logical_list[0].geometry.start - 4:
                        start=max(primary_geom.end+4,logical_list[0].geometry.start-4)
                        length=end-start+1
                        extend_part.geometry.start=start
                        extend_part.geometry.length=length
                    elif primary_geom.start >= logical_list[-1].geometry.end +4:
                        end=min(logical_list[-1].geometry.end +4,primary_geom.start-4)
                        length=end-start+1
                        extend_part.geometry.end=end
                        extend_part.geometry.length=length
                    else:
                        print "error,cann't add primary into logical"
                        return 
                else:
                    print "choose the bigger part to be new extend_part"
                    start_gap=primary_geom.start - start
                    end_gap=end - primary_geom.end
                    if start_gap <= end_gap:
                        extend_part.geometry.start=primary_geom.end+4
                        extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start +1
                    else:    
                        extend_part.geometry.end=primary_geom.start-4
                        extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start + 1

            elif start < primary_geom.start <= end and primary_geom.end > end:
                print "need reduce size at the end to the extend_part"
                if len(logical_list)!=0:
                    if start < logical_list[-1].geometry.end +4:
                        print "error ,add primary to space used by last logical"
                        return 
                    else:
                        extend_part.geometry.end=max(logical_list[-1].geometry.end+4,primary_geom.start-4)
                        extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start + 1
                else:
                    extend_part.geometry.end=primary_geom.start-4
                    extend_part.geometry.length=extend_part.geometry.end - extend_part.geometry.start + 1

            elif primary_geom.start <= start and primary_geom.end >=end:
                print "the primary geometry contains the extend one"
                if len(logical_list)==0:
                    self.disk_partition_info_tab[disk]=filter(lambda info:info[0].type!=2,self.disk_partition_info_tab[disk])
                    print "you should delete the extend part to get the space to add primary"
                else:
                    print "invalid,cann't add primary whoes geometry contains logicals" 

            else:
                print "invalid situation,you wonn't see this "

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
        disk_geom_info_tab={}#

        for disk in self.get_system_disks():
            disk_geom_info_tab[disk]=[]
            main_part_list=self.get_disk_main_list(disk)

            part_geom_list=[]#[["part",geometry]]
            gap_geom_list=[]#[["freespace",geometry]]
            space_geom_list=[]

            if len(main_part_list)==0:
                print "a whole blank disk"
                part_geom_list=[]
            else:
                for part in self.get_disk_partitions(disk):
                    if part.type==2:
                        pass
                    else:
                        part_geom_list.append(["part",part.geometry])
                part_geom_list=sorted(part_geom_list,key=lambda x:x[-1].start)        
                
            if len(part_geom_list)==0 or len(main_part_list)==0:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=disk.getFreeSpaceRegions()[-1].end-4
                length=end-start+1
                if length > 2048:
                    gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                else:
                    print "the whole blank disk is too small"
            else:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=part_geom_list[0][-1].start-4
                length=end-start+1
                if length > 2048:
                    gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                else:
                    print "start of disk have geometry gap or disk size too small"
                    self.lu.do_log_msg(self.logger,"warning","start of disk have geometry gap or disk size too small")

                for i in range(len(part_geom_list)-1):
                    start=part_geom_list[i][-1].end+4
                    end=part_geom_list[i+1][-1].start-4
                    length=end-start+1
                    if length > 2048:
                        gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                    else:
                        # print "the size between two partition is too small"
                        self.logger.warning("the size between two partition is too small")
                        self.lu.do_log_msg(self.logger,"warning","the size between two partition is too small")
                    i=i+1    
        
                start=part_geom_list[-1][-1].end+4    
                end=disk.getFreeSpaceRegions()[-1].end-4 
                length=end-start+1
                if length > 2048:
                    gap_geom_list.append(["freespace",parted.geometry.Geometry(disk.device,start,length,end,None)])
                else:
                    # print "end of disk have geometry overlap or disk size too small"
                    self.logger.warning("end of disk have geometry overlap or disk size too small")
                    self.lu.do_log_msg(self.logger,"warning","end of disk have geometry overlap or disk size too small")
            for item in part_geom_list:
                space_geom_list.append(item)
            for item in gap_geom_list:
                space_geom_list.append(item)
            space_geom_list=sorted(space_geom_list,key=lambda x:x[-1].start)
            disk_geom_info_tab[disk]=space_geom_list

        return disk_geom_info_tab    

    def get_disk_geom_info_tab(self,disk):
        '''get geom layout of the disk:include partition and freespace'''
        # self.init_disk_geom_info_tab()
        return self.disk_geom_info_tab[disk]

    def get_prev_geom_info_tab_item(self,disk,geometry):
        '''get previous space block of the geometry:part or freespace'''
        # self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
        if geometry.start <= self.disk_geom_info_tab[disk][0][-1].end:
            prev_item=self.disk_geom_info_tab[disk][0]
        else:
            prev_item=filter(lambda item:item[-1].end <= geometry.start,self.disk_geom_info_tab[disk])[-1]
        return prev_item

    def get_next_geom_info_tab_item(self,disk,geometry):
        '''get next space block of the geometry:part or freespace'''
        # self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
        if geometry.end >= self.disk_geom_info_tab[disk][-1][-1].start:
            next_item=self.disk_geom_info_tab[disk][-1]
        else:    
            next_item=filter(lambda item:item[-1].start >= geometry.end,self.disk_geom_info_tab[disk])[0]
        return next_item

    def get_start_geom_info_tab_item(self,disk,geometry):
        '''get current space block of the geometry:return the start one'''
        # self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
        return filter(lambda item:item[-1].start <= geometry.start,self.disk_geom_info_tab[disk])[-1]

    def get_end_geom_info_tab_item(self,disk,geometry):
        '''get current space block of the geometry:return the end one'''
        self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
        # self.disk_geom_info_tab[disk].sort(cmp=lambda x,y:cmp(x[-1].start,y[-1].start))
        return filter(lambda item:item[-1].end >= geometry.end,self.disk_geom_info_tab[disk])[0]

    def get_space_geom_size(self,disk,geometry):
        '''get size of the geometry:part or freespace'''
        space_size=(float)(geometry.length*disk.device.sectorSize)/(float)(1024*1024)
        return space_size

    def get_part_from_geom(self,disk,geometry):
        '''return part obj match the given geometry'''
        current_start_item=self.get_start_geom_info_tab_item(disk,geometry)
        current_end_item=self.get_end_geom_info_tab_item(disk,geometry)
        disk_partition_info_item=self.disk_partition_info_tab[disk]
        if current_start_item==current_end_item:
            if current_start_item[0]=="freespace":
                print "the block of the geometry is freespace"
                return
            elif current_start_item[0]=="part":
                for item in disk_partition_info_item:
                    if item[0].geometry==geometry:
                        return item[0]
                    else:
                        continue
                else:
                    print "do not find a part match the given geometry"
        else:
            print "invalid geometry arg,have part block overlaps"

    def add_part_geom_info_tab(self,disk,geometry):
        '''update disk_geom_info_tab when add partition from UI'''
        for item in self.disk_geom_info_tab[disk]:
            if item[0]=="part":
                print "part area,geometry not match,goto find the freespace one"
                continue
            elif item[0]=="freespace":
                if item[-1].start +4 <=geometry.start and item[-1].end -4 >= geometry.end:
                    print "find the match freespace block to add new part"
                    if geometry.start - item[-1].start > 4:
                        begin_start=item[-1].start+4
                        begin_end=geometry.start-4
                        begin_length=begin_end-begin_start+1
                        if begin_length > 2048:
                            begin_geometry=parted.geometry.Geometry(disk.device,begin_start,begin_length,begin_end,None)
                            self.disk_geom_info_tab[disk].append(["freespace",begin_geometry])
                        else:
                            pass
                    self.new_add_geom_item=["part",geometry]
                    self.disk_geom_info_tab[disk].append(self.new_add_geom_item)    

                    if  item[-1].end - geometry.end > 4:
                        after_start=geometry.end+4
                        after_end=item[-1].end-4
                        after_length=after_end-after_start+1
                        if after_length > 2048:
                            after_geometry=parted.geometry.Geometry(disk.device,after_start,after_length,after_end,None)
                            self.disk_geom_info_tab[disk].append(["freespace",after_geometry])
                        else:
                            pass

                    self.disk_geom_info_tab[disk].remove(item)    
                    self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)

                    print "finish find the match freespace block to add new part"    
                    break

                elif geometry.start < item[-1].start <geometry.end and item[-1].end >=geometry.end :
                    print "error,start of geometry out of range can allocate:need let start bigger"
                    continue
                elif item[-1].start <= geometry.start and geometry.start< item[-1].end < geometry.end:
                    print "error,end of geometry out of range can allocate:need let end smaller"
                    continue
                else:
                    print "geometry not match freespace,goto find the next block"
                    continue
            else:
                print "unknown space type,not part or freespace"
                continue
        else:
            print "error,doesn't find the matched freespace in disk_geom_info_tab"


    def delete_part_geom_info_tab(self,disk,geometry):
        '''update disk_geom_info_tab when delete partition from UI'''
        current_item=self.get_start_geom_info_tab_item(disk,geometry)
        current_end_item=self.get_end_geom_info_tab_item(disk,geometry)
        prev_item=self.get_prev_geom_info_tab_item(disk,geometry)
        next_item=self.get_next_geom_info_tab_item(disk,geometry)

        if current_item==None or len(current_item)==0:
            print "no space specified by the geometry"
            return
        if current_item==current_end_item:
            if current_item[0]=="freespace":
                print "freespace,no need to delete"
            else:
                if prev_item[0]=="freespace":
                    start=prev_item[-1].start+4
                    self.disk_geom_info_tab[disk].remove(prev_item)
                else:
                    start=current_item[-1].start+4
                if next_item[0]=="freespace":
                    end=next_item[-1].end-4
                    self.disk_geom_info_tab[disk].remove(next_item)
                else:
                    end=current_item[-1].end-4
                length=end-start+1    

                geom=parted.geometry.Geometry(disk.device,start,length,end,None)
                self.disk_geom_info_tab[disk].remove(current_item)
                self.disk_geom_info_tab[disk].append(["freespace",geom])
                self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
        else:
            print "error,the geometry arg should indicate only one block"
            if current_end_item[0]=="freespace":
                if prev_item[0]=="freespace":
                    start=prev_item[-1].start+4
                    self.disk_geom_info_tab[disk].remove(prev_item)
                else:
                    start=current_item[-1].start+4
                if next_item[0]=="freespace":
                    end=next_item[-1].end-4
                    self.disk_geom_info_tab[disk].remove(next_item)
                else:
                    end=current_end_item[-1].end-4
                length=end-start+1    
                geom=parted.geometry.Geometry(disk.device,start,length,end,None)
                self.disk_geom_info_tab[disk].remove(current_item)
                self.disk_geom_info_tab[disk].remove(current_end_item)
                self.disk_geom_info_tab[disk].append(["freespace",geom])

                self.disk_geom_info_tab[disk]=sorted(self.disk_geom_info_tab[disk],key=lambda x:x[-1].start)
            else:    
                print "can operate only one block one time,please adapt the geometry"

    ################set disk partition attribute before add or modify########################
    def get_disk_partition_object(self,disk,part_type,part_size,space_geom,part_fs,part_location):
        '''get partition_object for add to the disk_partition_info_tab,also for actual partition add operation
           add the part_obj to path_disks_partitions when it birth
        '''
        self.type=self.set_disk_partition_type(disk,part_type)
        self.geometry=self.set_disk_partition_geometry(disk,part_size,space_geom,part_location)
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

    def set_disk_partition_geometry(self,disk,part_size,space_geom,part_location):
        '''to get geometry of new added partition'''
        import math
        minlength=math.floor((float)((float)(part_size)*1024*1024)/(float)(disk.device.sectorSize))
        (start,length,end)=(space_geom.start,space_geom.length,space_geom.end)
        if length==None or length < end-start+1:
            length=end-start+1
        if minlength + 8 > length:
            print "the free space too small to hold the partition"
            part_start=start+4
            part_end=end-4
            part_length=part_end-part_start+1
        else:
            if part_location=="start" or len(part_location)==0:
                part_start=start+4
                part_length=minlength
                part_end=part_start+part_length-1
            elif part_location=="end":
                part_end=end-4
                part_length=minlength
                part_start=part_end-part_length+1
        space_geom.start=part_start
        space_geom.length=part_length
        space_geom.end=part_end
        return space_geom

    def set_disk_partition_name(self,partition,part_name):
        '''cann't set this attribute,need to fix'''
        if not partition.disk.supportsFeature(parted.DISK_TYPE_PARTITION_NAME):
            print "sorry,the disk doesn't support set partition name"
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
                print "filesystem not changed or partition hadnn't been formated"
                self.lu.do_log_msg(self.logger,"warning","filesystem not changed")
            elif origin_fs_type=="linux-swap":
                swap_data=get_os_command_output("grep "+part_path+" /proc/swaps")
                if len(swap_data)!=0:
                    swap_off_command="sudo swapoff "+part_path
                    run_os_command(swap_off_command)
                else:    
                    print "swap not on"
            else:
                print "fstype had changed,need format"
        except:
            print "get origin_fs_type error"
            self.lu.do_log_msg(self.logger,"error","get origin_fs_type error")
        #set fstype
        if fstype=="linux-swap":
            format_command="sudo mkswap "+part_path
        elif fstype=="fat32":
            format_command="sudo mkfs.vfat "+part_path
        elif fstype in ["ext2","ext3","ext4","reiserfs","xfs","ntfs"]:
            format_command="sudo mkfs."+fstype+" "+part_path
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
        '''umount the partition,return True when umount successfully'''
        if partition.type==2:
            print "no need to umount extended part"
            return True
        part_path=partition.path
        for item in get_os_command_output("cat /proc/mounts"):
            if item.startswith(part_path):
                mount_point=item.split()[1]
                break
            else:    
                continue
        else:
            return True

        def is_umount():        
            after_mtab=get_os_command_output("cat /proc/mounts")
            for item in after_mtab:
                if item.startswith(part_path):
                    return False
                    break
                else:
                    continue
            else:
                return True

        while not is_umount():
            run_os_command("sudo umount "+part_path)
            if not is_umount():
                run_os_command("sudo umount -l "+part_path)
            else:
                return True
            if not is_umount() and mount_point!=None:
                run_os_command("sudo umount -f "+mount_point)
        else:
            return True

        return is_umount()                

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
        if partition.disk!=disk:
            print "partition not in the disk"
            self.lu.do_log_msg(self.logger,"error","partiton not in the disk")
            return 
        if partition.type==2 and len(self.get_disk_logical_list(disk))!=0:
            print "error,need delete all logical partitions before delete extend partition"
            self.lu.do_log_msg(self.logger,"error","delete logical partitions before delete extend")

        self.delete_path_disks_partitions(disk,partition)    
        self.disk_partition_info_tab[disk]=filter(lambda info:info[0]!=partition,self.disk_partition_info_tab[disk])
        try:
            if self.set_disk_partition_umount(partition)==True:
                disk.deletePartition(partition)
            else:
                print "error,need umount partition before delete"
                self.set_disk_partition_umount(partition)
                disk.deletePartition(partition)
        except:
            print "error,delete partition error occurs,device may busy"
            self.lu.do_log_msg(self.logger,"error","delete partition error occurs")


    def delete_custom_partition(self):
        '''batch delete origin disk partitions:'''
        for disk in self.get_system_disks():
            for item in filter(lambda info:info[-1]=="delete",self.disk_partition_info_tab[disk]):
                if item[0]==None:
                    print "error,the partition object should not be null"
                    continue
                elif item[0].type==1:
                    print "delete logical partition"
                    self.delete_disk_partition(disk,item[0])
                    # try:
                    #     self.delete_disk_partition(disk,item[0])
                    #     print "successfully delete logical partition"    
                    # except:
                    #     print "delete logical partition error!"
                else:
                    continue

            disk.commit()    
            for item in filter(lambda info:info[-1]=="delete",self.disk_partition_info_tab[disk]):
                if item[0].type==1:
                    print "error,you should have already delete the logical partition"
                    print item[0]
                    continue
                elif item[0].type==0 or item[0].type==2:
                    print "delete primary/extended partition"
                    self.delete_disk_partition(disk,item[0])
                    # try:
                    #     self.delete_disk_partition(disk,item[0])
                    #     print "successfully delete primary/extended partition"    
                    # except:
                    #     print "delete primary/extend partition error!"
                else:
                    continue
            disk.commit()        

    def add_custom_partition(self):
        '''batch add partition according to disk_partition_info_tab,then mount them,every disk batch commit'''
        print "begin batch add partition"
        for disk in self.get_system_disks():
            for item in filter(lambda info:info[-1]=="add" and info[0].type==2,self.disk_partition_info_tab[disk]):
                print "add extended partition first"
                self.partition=item[0]
                self.geometry=self.partition.geometry
                self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                disk.addPartition(self.partition,self.constraint)
                # try:
                #     disk.addPartition(self.partition,self.constraint)
                #     print "successfully add extended partition for %s" % disk.device.path
                # except:
                #     print "add extended partition error"
                disk.commit()

            for item in filter(lambda info:info[-1]=="add" and info[0].type==1,self.disk_partition_info_tab[disk]):    
                print "add logical partition"
                self.partition=item[0]
                self.geometry=self.partition.geometry
                self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                disk.addPartition(self.partition,self.constraint)
                # try:
                #     disk.addPartition(self.partition,self.constraint)
                #     print "successfully add logical partition for %s" % disk.device.path
                # except:
                #     print "add logical partition error"

            for item in filter(lambda info:info[-1]=="add" and info[0].type==0,self.disk_partition_info_tab[disk]):    
                print "add primary partition"
                self.partition=item[0]
                self.geometry=self.partition.geometry
                self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                disk.addPartition(self.partition,self.constraint)
                # try:
                #     disk.addPartition(self.partition,self.constraint)
                #     print "successfully add primary partition for %s" % disk.device.path
                # except:
                #     print "add primary partition error"

            disk.commit()        
            print "add all partitions for the disk %s ok" % disk.device.path
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

        
    ###############################do real partition operations#########################                
    def do_advance_partition(self):
        '''do in fact part operations,delete old first,then add new ones'''
        self.delete_custom_partition()
        self.add_custom_partition()

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

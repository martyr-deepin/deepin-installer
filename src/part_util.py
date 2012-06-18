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
import copy
import parted
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

    #{dev_path:disk}   must need this because you refs the same disk object,but they have different Ped id ,
    #and cann't share the id to add partition,so never call the get_path_disks function again,just get the variable value
        self.path_disks=self.get_path_disks()

    #{disk:[partition1,partition2,],disk2:[]},to the same reason like above,keep the Ped partition id uniquee,
    #you can update the variable,but never call the get_path_disks_partitions function again
        self.path_disks_partitions=self.get_path_disks_partitions()

    # #machine based:[disk,geometry,partition],to do the backend operations,may not need this table,consider to delete
    #     self.disk_partition_tab =[]

    #user based operation frontend
    #[   partition,part_disk_path,part_type,part_size,part_fs,
    #    part_format,part_name,part_mountpoint,part_flag
    #]

        self.disk_partition_info_tab=self.init_disk_partition_info_tab()
        
    #get disk space_info,used in display available size and constraint the new creating partition size    
    # {disk:[   [main_part_list,logical_part],
    #           [main_geom_list,logical_geom_list],
    #           [main_geom_gap_list,logical_geom_gap_list]
    #       ],
    #    
    #  }   
        self.disk_space_info_tab={}

        self.lu=LogUtil()
        self.logger=self.lu.create_logger("part_logger","debug")
        self.handler=self.lu.create_logger_handler(self.logger,"file","debug","lineno",None)

    #newly port from part.py for share use
        self.disk_part_display_path={}
        self.init_disk_part_display_path()#{disk:{partition:part_path}}
        self.backup_disk_partition_info_tab=self.init_backup_disk_partition_info_tab()

    #disk_partition_tab && disk_partition_info operations:
    def init_disk_partition_info_tab(self):
        '''read origin disk partition info'''
        disk_partition_info_tab=[]
        disk_partition_info_tab_item=[]
        
        for disk in self.get_system_disks():
            if disk.getFirstPartition()==None:
                self.set_disk_label(disk.device)
                continue 
            for part in self.get_disk_partitions(disk):
                self.partition=part
                self.part_disk_path=part.disk.device.path
                self.part_type=PART_TYPE_LIST[part.type]
                self.part_size=part.getSize(unit="MB")
                try:
                    self.part_fs=part.fileSystem.type#just for primary and logical partition
                except:
                    self.part_fs=None
                self.part_format=False#donn't format origin partition
                self.part_name=part.name

                try:
                    self.part_mountpoint=self.get_disk_partition_mount(self.partition)[0][1]
                except:
                    self.part_mountpoint=""
                    
                self.part_flag="keep"   #flag:keep,new,delete 
                # self.disk_partition_info_tab_item=[self.partition.path,self.part_disk_path,self.part_type,
                #                                    self.part_size,self.part_fs,self.part_format,
                #                                    self.part_name,self.part_mountpoint]

                disk_partition_info_tab_item=[self.partition,self.part_disk_path,self.part_type,
                                              self.part_size,self.part_fs,self.part_format,
                                              self.part_name,self.part_mountpoint,self.part_flag]

                disk_partition_info_tab.append(disk_partition_info_tab_item)

        return disk_partition_info_tab

    def init_backup_disk_partition_info_tab(self):
        '''copy from init_disk_partition_info_tab,as _Ped object not support simply deepcopy'''
        disk_partition_info_tab=[]
        disk_partition_info_tab_item=[]
        
        for disk in self.get_system_disks():
            if disk.getFirstPartition()==None:
                self.set_disk_label(disk.device)
                continue 
            for part in self.get_disk_partitions(disk):
                self.partition=part
                self.part_disk_path=part.disk.device.path
                self.part_type=PART_TYPE_LIST[part.type]
                self.part_size=part.getSize(unit="MB")
                try:
                    self.part_fs=part.fileSystem.type#just for primary and logical partition
                except:
                    self.part_fs=None
                self.part_format=False#donn't format origin partition
                self.part_name=part.name

                try:
                    self.part_mountpoint=self.get_disk_partition_mount(self.partition)[0][1]
                except:
                    self.part_mountpoint=""
                    
                self.part_flag="keep"   #flag:keep,new,delete 
                # self.disk_partition_info_tab_item=[self.partition.path,self.part_disk_path,self.part_type,
                #                                    self.part_size,self.part_fs,self.part_format,
                #                                    self.part_name,self.part_mountpoint]

                disk_partition_info_tab_item=[self.partition,self.part_disk_path,self.part_type,
                                              self.part_size,self.part_fs,self.part_format,
                                              self.part_name,self.part_mountpoint,self.part_flag]

                disk_partition_info_tab.append(disk_partition_info_tab_item)

        return disk_partition_info_tab


    def refresh_disk_partition_info_tab(self,disk_partition_info_tab):
        '''sort the table according to partition number,update the table after add or delete operation'''
        #to be implemented
        parts_path=[]
        for item in self.disk_partition_info_tab:
            parts_path.append(item[0].path)
        parts_path.sort()    
        return parts_path

    def get_disk_free_space_info(self):
        '''basic function help for get available size to UI,not really in the disk:
           get_disk_free_geometry
           get_disk_single_available_space_size
           get_disk_total_available_space_size
        '''
    #get disk space_info,used in display available size and constraint the new creating partition size    
    # {disk:[   [main_part_list,logical_part],
    #           [main_geom_list,logical_geom_list],
    #           [main_geom_gap_list,logical_geom_gap_list]
    #       ],
    #    
    #  }   
        self.disk_space_info_tab={}

        for disk in self.get_system_disks():

            self.primary_part=[]
            self.extended_part=[]
            self.extend_part=""
    
            self.main_part_list=[]
            self.logical_part=[]
    
            self.main_geom_list=[]
            self.logical_geom_list=[]   
    
            self.main_geom_gap_list=[]
            self.logical_geom_gap_list=[]
            #middleware list
            self.disk_space_info_list=[[],[],[]]
            
        #get disk main_part_list and logical_part,#fix:use space now in the to deleted part
            # for item in filter(lambda info:info[0].disk==disk,self.disk_partition_info_tab):
            for item in filter(lambda info:info[0].disk==disk and info[-1]!="delete",self.disk_partition_info_tab):
                if item[2]=="primary":
                    self.primary_part.append(item[0])
                    continue
                elif item[2]=="logical":
                    self.logical_part.append(item[0])
                    continue
                elif item[2]=="extend":
                    self.extended_part.append(item[0])
                    continue
                else:
                    print "invalid part type"
                    self.lu.do_log_msg(self.logger,"warning","invalid part type")
                    continue

            if len(self.extended_part)>1:        
                print "can have only one extended_part"
                self.lu.do_log_msg(self.logger,"error","can have only one extended_part")
            elif len(self.extended_part)==0:
                self.logger.info("the disk doesn't have extended_part")
                print "the disk "+disk.device.path+" doesn't have extended_part"
            else:
                self.extend_part=self.extended_part[0]
        
            if len(self.primary_part)!=0:
                self.main_part_list=filter(lambda item:item in self.primary_part,self.primary_part)
                if len(self.extended_part)!=0:
                    self.main_part_list.append(self.extended_part[0])
            elif len(self.primary_part)==0 and len(self.extended_part)!=0:
                self.main_part_list=self.extended_part
            else:#blank disk
                self.primary_part=[]
                self.extended_part=[]
                self.logical_part=[]
                self.main_part_list=[]

        #put main_part_list and logical_part into disk_space_info_tab_info_tab    
            self.disk_space_info_list[0].append(self.main_part_list)
            self.disk_space_info_list[0].append(self.logical_part)
            
            # self.disk_space_info_tab[disk][0].append(self.main_part_list)    
            # self.disk_space_info_tab[disk][0].append(self.logical_part)


        #get main_geom_list and logical_geom_list     
            if len(self.main_part_list)==0:#blank list
                self.main_geom_list=[]
            else:
                for part in self.main_part_list:
                    self.main_geom_list.append(part.geometry)
                self.main_geom_list.sort(cmp=lambda x,y:cmp(x.start,y.start))

            if len(self.logical_part)==0:#no logical part    
                self.logical_geom_list=[]
            else:
                for part in self.logical_part:
                    self.logical_geom_list.append(part.geometry)
                self.logical_geom_list.sort(cmp=lambda x,y:cmp(x.start,y.start))    
    
        #put main_geom_list and logical_geom_list into disk_space_info_tab        
            self.disk_space_info_list[1].append(self.main_geom_list)
            self.disk_space_info_list[1].append(self.logical_geom_list)
            # self.disk_space_info_tab[disk][1].append(self.main_geom_list)
            # self.disk_space_info_tab[disk][1].append(self.logical_geom_list)

        #get main_geom_gap_list and logical_geom_gap_list    
            #get main_geom_gap_list
            if len(self.main_geom_list)==0 or len(self.main_part_list)==0:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=disk.getFreeSpaceRegions()[-1].end-4
                length=end-start+1
                if length > 0:
                    self.main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    self.main_geom_gap_list=[]
            else:
                start=disk.getFreeSpaceRegions()[0].start+4
                end=self.main_geom_list[0].start-4
                length=end-start+1
                if length > 0:
                    self.main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    # print "start of disk have geometry overlap or disk size too small"
                    self.lu.do_log_msg(self.logger,"warning","start of disk have geometry overlap or disk size too small")
                for i in range(len(self.main_geom_list)-1):
                    start=self.main_geom_list[i].end+4
                    end=self.main_geom_list[i+1].start-4
                    length=end-start+1
                    if length > 0:
                        self.main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                    else:
                        # print "the size between two partition is too small"
                        self.logger.warning("the size between two partition is too small")
                        # self.lu.do_log_msg(self.logger,"warning","the size between two partition is too small")
                    i=i+1    
        
                start=self.main_geom_list[-1].end+4    
                end=disk.getFreeSpaceRegions()[-1].end-4 
                length=end-start+1
                if length > 0:
                    self.main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    # print "end of disk have geometry overlap or disk size too small"
                    self.logger.warning("end of disk have geometry overlap or disk size too small")
                    # self.lu.do_log_msg(self.logger,"warning","end of disk have geometry overlap or disk size too small")
            #get logical_geom_gap_list        
            if len(self.logical_geom_list)==0 and len(self.extended_part)!=0:
                start=self.extend_part.geometry.start+4
                end=self.extend_part.geometry.end-4
                length=end-start+1
                if length > 0:
                    self.logical_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    self.logical_geom_gap_list=[]
                    # print "the hole extend_part size is too small"
                    self.lu.do_log_msg(self.logger,"error","the hole extend_part size is too small")
            elif len(self.extended_part)!=0:
                start=self.extend_part.geometry.start+4
                end=self.logical_geom_list[0].start-4
                length=end-start
                if length > 0:
                    self.logical_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    # print "the space between start of extend_part and the first logical is too small"
                    self.lu.do_log_msg(self.logger,"warning","the space between start of extended_part and the first logical is too small")
                for i in range(len(self.logical_geom_list)-1):
                    start=self.logical_geom_list[i].end+4
                    end=self.logical_geom_list[i+1].start-4
                    length=end-start+1
                    if length > 0:
                        self.logical_geom_gap=parted.geometry.Geometry(disk.device,start,length,end,None)
                        self.logical_geom_gap_list.append(self.logical_geom_gap)
                    else:
                        # print "the size between two logical partition is too small"
                        self.lu.do_log_msg(self.logger,"warning","the size between two logical partition is too small")
                    i=i+1    
        
                start=self.logical_geom_list[-1].end+4
                end=self.extend_part.geometry.end-4
                length=end-start
                if length > 0:
                    self.logical_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
                else:
                    # print "end of extend_part have geometry overlap or disk is too small to satisfy the minlength"
                    self.lu.do_log_msg(self.logger,"warning","end of extended_part have geometry overlap")
            else:
                # print "the disk "+disk.device.path+" has no extend_part,no need to add logical_geom_gap_list"
                self.lu.do_log_msg(self.logger,"warning","no extend_part,no need to add logical_geom_gap_list")
                self.logical_geom_gap_list=[]

            #put main_geom_gap_list and logical_geom_gap_list into self.disk_space_info_tab
            # self.disk_space_info_tab[disk][2].append(self.main_geom_gap_list)
            # self.disk_space_info_tab[disk][2].append(self.logical_geom_gap_list)
            self.disk_space_info_list[2].append(self.main_geom_gap_list)
            self.disk_space_info_list[2].append(self.logical_geom_gap_list)

            self.disk_space_info_tab[disk]=self.disk_space_info_list
            
        return self.disk_space_info_tab    


    def get_disk_total_available_space_size(self,disk):
        '''return the total available space size for user to create his primary/extended/logical partition'''
        disk_space_info_tab=self.get_disk_free_space_info()
        self.main_available_space_size=""
        self.logical_available_space_size=""
        self.total_available_space_size=()
        main_length=0
        logical_length=0
        if disk_space_info_tab.has_key(disk):
            disk_space_info_list=disk_space_info_tab[disk]
            for geom in disk_space_info_list[2][0]:#main_geom_gap_list
                main_length+=geom.length
            for geom in disk_space_info_list[2][1]:#logical_geom_gap_list
                logical_length+=geom.length
        else:
            print "cann't find disk in the disk_free_space_info"
            self.lu.do_log_msg(self.logger,"error","cann't find disk in the disk_free_space_info")
        self.main_available_space_size=main_length*disk.device.sectorSize/(1024*1024)    
        self.logical_available_space_size=logical_length*disk.device.sectorSize/(1024*1024)
        self.total_available_space_size=(self.main_available_space_size,self.logical_available_space_size)

        return self.total_available_space_size

    def get_disk_single_available_space_size(self,disk,part_type):
        '''return the max size for user to create a single partition,not used in the tree view list'''
        disk_space_info_tab=self.get_disk_free_space_info()
        import math
        self.single_available_space_size=0
        if disk_space_info_tab.has_key(disk):
            disk_space_info_list=disk_space_info_tab[disk]
            if part_type=="primary" or part_type=="extend":
                max_geo=disk_space_info_list[2][0][0]
                for geom in disk_space_info_list[2][0]:
                    if geom.length > max_geo.length:
                        max_geo=geom
                    else:
                        continue
                self.single_available_space_size=math.floor((max_geo.length*disk.device.sectorSize)/(1024*1024))
                return self.single_available_space_size

            elif part_type=="logical":
                max_geo=disk_space_info_list[2][1][0]
                for geom in disk_space_info_list[2][1]:
                    if geom.length > max_geo.length:
                        max_geo=geom
                    else:
                        continue
                self.single_available_space_size=math.floor((max_geo.length*disk.device.sectorSize)/(1024*1024))
                return self.single_available_space_size
    
            else:
                print "invalid part_type"
                self.lu.do_log_msg(self.logger,"error","invalid part_type")
        else:
            print "cann't find disk in the disk_free_space_info"
            self.lu.do_log_msg(self.logger,"error","cann't find disk in the disk_free_space_info")

    def get_disk_free_geometry(self,disk,part_type,minlength):
        '''get free geometry to add part:specified by part_type and minlength,use disk_free_space_info'''
        disk_space_info_tab=self.get_disk_free_space_info()

        self.geometry=""
        if disk_space_info_tab.has_key(disk):
            disk_space_info_list=disk_space_info_tab[disk]
            if part_type=="primary" or part_type=="extend":
                max_geo=disk_space_info_list[2][0][0]
                for geom in disk_space_info_list[2][0]:
                    if geom.length > max_geo.length:
                        max_geo=geom
                    else:
                        continue
                self.geometry=max_geo
                if self.geometry.length >= minlength:
                    return self.geometry
                else:
                    # print "cann't get a main part satisfy the minlength"
                    self.lu.do_log_msg(self.logger,"error","cann't get a main part satisfy the minlength")
    
            elif part_type=="logical":
                max_geo=disk_space_info_list[2][1][0]
                for geom in disk_space_info_list[2][1]:
                    if geom.length > max_geo.length:
                        max_geo=geom
                    else:
                        continue
                self.geometry=max_geo
                if self.geometry.length >= minlength:
                    return self.geometry
                else:
                    # print "cann't get a logical part satisfy the minlength"
                    self.lu.do_log_msg(self.logger,"error","cann't get a logical part satisfy the minlength")
            else:
                # print "invalid part type"
                self.lu.do_log_msg(self.logger,"error","invalid part type")
        else:
            # print "get_disk_free_geometry:cann't find disk in the disk_free_space_info"
            self.lu.do_log_msg(self.logger,"critical","cann't find disk in the disk_free_space_info")

    def get_disk_logical_freepart(self,disk):
        '''attention:this freepart are logical,not in the path_disk_partitions and disk_partition_info_tab,
        used for locate geometry to create new partition'''
#to be implemented,consider to delete this,just use get_disk_free_geometry
        pass

    def get_disk_partition_object(self,part_disk_path,part_type,part_size,part_fs):
        '''get partition_object for add to the disk_partition_info_tab,also for actual partition add operation
           don't worry about the existed partition object,can get from user view partition path
           but now the new added partition path may like /dev/sda-1,not /dev/sda1
        '''

        self.disk=self.get_disk_from_path(part_disk_path)
        self.type=self.set_disk_partition_type(self.disk,part_type)

        import math
        minlength=math.floor((part_size*1024*1024)/(self.disk.device.sectorSize)+1)
        
        self.free_geometry=self.get_disk_free_geometry(self.disk,part_type,minlength)
        # self.free_geometry=self.get_disk_free_geometry(self.disk,part_type)

        self.geometry=self.set_disk_partition_geometry(self.disk,self.free_geometry,part_size)

        self.fs=parted.filesystem.FileSystem(part_fs,self.geometry,False,None)
        self.partition=parted.partition.Partition(self.disk,self.type,self.fs,self.geometry,None)
        #track the Partition ped id
        self.insert_path_disks_partitions(self.disk,self.partition)
        
        return self.partition

        
    def add_disk_partition_info_tab(self,part_disk_path,part_type,part_size,part_fs,part_format,part_name,part_mountpoint):
        '''add partition to the table,insert_path_disks_partitions in get_disk_partition_object because it's used
        not only for add_disk_partition_info_tab,but also the real add partition operate'''
        self.to_add_partition=self.get_disk_partition_object(part_disk_path,part_type,part_size,part_fs)
        if self.to_add_partition==None:
            print "partition is null"
            return 
        part_flag="add"   
        self.disk_partition_info_tab_item=[self.to_add_partition,part_disk_path,part_type,part_size,part_fs,
                                           part_format,part_name,part_mountpoint,part_flag]
        self.disk_partition_info_tab.append(self.disk_partition_info_tab_item)

        return self.disk_partition_info_tab

    def mark_disk_partition_info_tab(self,partition,part_flag):
        '''part_flag:keep,add,delete,according the flag to adjust the disk_partition_info_tab
           Then I can do partition add delete without bother the existed partition
        '''
        if(partition==None):
            # print "partition doesn't exist"
            self.lu.do_log_msg(self.logger,"error","partition doesn't exist")
            return
        for item in filter(lambda info:partition.__eq__(info[0]),self.disk_partition_info_tab):
            item[-1]=part_flag   

        return  self.disk_partition_info_tab

    def delete_disk_partition_info_tab(self,partition):
        '''for origin partition,marked delete;for to added partition,rm from the table;'''
        if(partition==None):
            print "partition doesn't exist"
            return

        for item in filter(lambda info:partition.__eq__(info[0]),self.disk_partition_info_tab):
            if item[-1]=="add":
                self.disk_partition_info_tab=filter(lambda i:i!=item,self.disk_partition_info_tab)
                self.delete_path_disks_partitions(partition.disk,partition)
            elif item[-1]=="keep":
                self.mark_disk_partition_info_tab(partition,"delete")
            else:
                # print "invalid,if partition marked delete,you wonn't see it"
                self.lu.do_log_msg(self.logger,"error","invalid,if partition marked delete,you wonn't see it ")
                break

        return self.disk_partition_info_tab

#may not need this table
    def add_disk_partition_tab(self,disk,geometry,partition):
        '''add disk part tab,need consider handle except and part mount info'''
        disk_partition_tab_item=(disk,geometry,partition)
        try:
            self.disk_partition_tab.append(disk_partition_tab_item)
        except:
            # print "add disk partition tab failed!"
            self.lu.do_log_msg(self.logger,"error","add disk partition tab failed")
        # return self.disk_partition_tab    


    def delete_disk_partition_tab(self,disk,partition):
        '''delete disk part tab item'''
        for disk_partition_tab_item in self.disk_partition_tab:
            if disk in disk_partition_tab_item and partition in disk_partition_tab_item:
                self.disk_partition_tab.remove(disk_partition_tab_item)
        else:
            # print "the partition is not in the disk_partition_tab"
            self.lu.do_log_msg(self.logger,"error","the partition is not in the disk_partition_tab")
        # print self.disk_partition_tab            
                    
    def change_disk_partition_tab(self,disk,partition):
        pass


    #disk operations    
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

    def get_path_disks(self):
        '''return path:disk dict,called this only once to make sure the Ped object id will not change'''
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

    def get_path_disk_partitions(self,disk):
        '''return [partition],only called by get_path_disks_partitons'''
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

    def insert_path_disks_partitions(self,disk,partition):
        '''insert new added partition to path_disk_partitions variable,keep partition id uniquee'''
        #to add exceptionl handle
        if disk.getPedDisk!=partition.disk.getPedDisk:
            # print "somewhere id not match"
            self.lu.do_log_msg(self.logger,"error","somewhere id not match")
        self.path_disks_partitions[disk].append(partition)    

    def delete_path_disks_partitions(self,disk,partition):
        '''delete partition from path_disks_partitions variable,keep partition id uniquee'''
        #to add exceptionl handle
        if disk.getPedDisk!=partition.disk.getPedDisk:
            # print "somewhere id not match"
            self.lu.do_log_msg(self.logger,"error","somewhere id not match")
        self.path_disks_partitions[disk].remove(partition)


    def get_path_disks_partitions(self):
        '''return {disk:[partition1,partition2],disk2:[partition1]},calld this only once to keep Ped partition id uniquee'''
        path_disks_partitons={}

        for disk in self.get_system_disks():
            path_disks_partitons[disk]=self.get_path_disk_partitions(disk)
            
        return path_disks_partitons    
        
    def get_disk_from_path(self,dev_path):
        '''from path:/dev/sda to get disk,fetch the global path_disks need to keep the only Ped id'''
        if dev_path in self.path_disks.keys():
            return self.path_disks[dev_path]
        else:
            # print "there's no disk specified by the path"
            self.lu.do_log_msg(self.logger,"critical","there's no disk specified by the path")
            return 

    def get_device_from_path(self,dev_path):
        '''from path:/dev/sda to get device,old function,avoid to use this'''
        return parted.device.Device(dev_path,None)

    def get_disk_size(self,disk):
        '''get disk size,used to compare with the optimize swap size'''
        # '''test function ,never used'''
        # print disk.device.path+" disk size:"+ str(disk.device.getSize("GB")*1.0)+"GB"
       
        # for part in self.get_disk_partitions(disk):
        #     print self.get_disk_partition_size(part)
        return disk.device.getSize("MB")


    def set_disk_label(self,device):
        '''set disk label:gpt or msdos,to be extended'''
        
        if device.getSize() >= 1.5*1024*1024:
            self.disk=parted.freshDisk(device,"gpt")
        else:
            self.disk=parted.freshDisk(device,"msdos")
            
        return self.disk

    def recovery_disk_partitions(self,disk):
        pass


    def get_disk_partitions(self,disk):
        '''return partitions of the given disk,get value from path_disks_partitions'''
        return self.path_disks_partitions[disk]

    #partition operations
    def get_disk_partition_size(self,partition):
        print partition.path+" partition size:"+str(partition.getSize("GB")*1.0)+"GB"

    def delete_disk_partition(self,disk,partition):
        '''atom function:delete the given partition,called only because need delete original partition'''
        self.partitions=self.get_disk_partitions(disk)
        if partition not in self.partitions:
            # print "partition not in the disk"
            self.lu.do_log_msg(self.logger,"error","partition not in the disk")
            return

        if partition.type==parted.PARTITION_EXTENDED and disk.getLogicalPartitions()!=[]:
            # print "need delete all logical partitions before delete extend partition"
            self.lu.do_log_msg(self.logger,"info","delete logical partitions before delete extend")
            for logical_part in disk.getLogicalPartitions():
                self.delete_path_disks_partitions(disk,logical_part)
                self.disk_partition_info_tab=filter(lambda info:info[0]!=logical_part,self.disk_partition_info_tab)
                try:
                    self.set_disk_partition_umount(logical_part)
                    disk.deletePartition(logical_part)
                except:
                    # print "delete logical_part failed"
                    self.lu.do_log_msg(self.logger,"error","delete logical_part failed")
        self.delete_path_disks_partitions(disk,partition)    
        self.disk_partition_info_tab=filter(lambda info:info[0]!=partition,self.disk_partition_info_tab)
        try:
            self.set_disk_partition_umount(partition)
            disk.deletePartition(partition)
        except:
            # print "error occurs"
            self.lu.do_log_msg(self.logger,"error","error occurs")
        disk.commit()

    def delete_custom_partition(self,partition_path):
        '''delete disk partition:get partition_path from ui
           This function can be called only because you want to delete the original partition
           umount it first,handle the table delete in delete_disk_partition
        '''
        for item in filter(lambda info:info[0].path==partition_path,self.disk_partition_info_tab):
            if item[-1]=="delete":
                self.partition=item[0]
                break
            else:
                continue
        if self.partition==None:
            # print "partition doesn't exist"
            self.lu.do_log_msg(self.logger,"warning","partition doesn't exist")
            return
        self.disk=self.partition.disk
        self.delete_disk_partition(self.disk,self.partition)

    def recovery_disk_partition(self,disk,partition):
        pass

    def edit_disk_partition(self,disk,partition,info):
        '''edit partitiion size and file system,actually edit fs and geometry'''
        self.part_info=self.get_disk_partition_info(partition)
        self.part_size=partition.getSize()
        self.part_maxava_size=partition.getMaxAvailableSize(unit="MB")

    def probe_tab_has_extend(self,disk,disk_partition_info_tab):
        '''probe extend partition,for except handle add logical without extend'''
        Flag=False
        for item in filter(lambda info:info[0].disk==disk,disk_partition_info_tab):
            if item[0].type==parted.PARTITION_EXTENDED:
                Flag=True
            else:
                continue
        return Flag    

    def add_custom_disk_partition(self,disk,disk_partition_info_tab):
        '''help for add_custom_partition'''
        for item in filter(lambda info:info[-1]=="add" and info[0].disk==disk,disk_partition_info_tab):
            if item[0].type==parted.PARTITION_LOGICAL and not self.probe_tab_has_extend(disk,disk_partition_info_tab):
                # print "can't add logical partition as there is no extended partition"
                self.lu.do_log_msg(self.logger,"error","can't add logical as no extended")
                break
            else:
                self.partition=item[0]
                self.geometry=self.partition.geometry
                print self.geometry
                self.constraint=parted.constraint.Constraint(exactGeom=self.geometry)
                disk.addPartition(self.partition,self.constraint)
        disk.commit()        
        #mkfs/setname/mount disk partitions
        for item in filter(lambda info:info[-1]=="add" and info[0].disk==disk,disk_partition_info_tab):
            if item[0].type==parted.PARTITION_LOGICAL and not self.probe_tab_has_extend(disk,disk_partition_info_tab):
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
                
                
    def add_custom_partition(self,disk_partition_info_tab):
        '''add partition according to disk_partition_info_tab,then mount them,every disk batch commit'''
        for disk in self.get_system_disks():
            self.add_custom_disk_partition(disk,disk_partition_info_tab)

    def set_disk_partition_type(self,disk,part_type):
        '''check the to added partition type,need consider the count of primary,extend,etc...'''
        if part_type=="primary":
            self.type=parted.PARTITION_NORMAL
        elif part_type=="extend":
            self.type=parted.PARTITION_EXTENDED
        elif part_type=="logical":
            self.type=parted.PARTITION_LOGICAL
        else:
            # print "part type error"
            self.lu.do_log_msg(self.logger,"error","part type error")
        return self.type    

    def set_disk_partition_geometry(self,disk,free_geometry,size):
        '''to get free_geometry'''
        if free_geometry==None:
            free_geometry=disk.getFreeSpaceRegions()[0]#this have to be fixed
        self.start=free_geometry.start
        self.length=long(free_geometry.length*size/free_geometry.getSize())
        self.end=self.start+self.length-1
        
        if self.end > free_geometry.end:
            self.end=free_geometry.end
            self.length=self.end-self.start+1

        self.geometry=parted.geometry.Geometry(disk.device,self.start,self.length,self.end,None)
        return self.geometry
    

    def set_disk_partition_name(self,partition,part_name):
        '''cann't set this attribute,need to fix'''
        if part_name==None or len(part_name)==0:
            # print "don't need to set partition name"
            return

        if not partition.disk.supportsFeature(parted.DISK_TYPE_PARTITION_NAME):
            # print "sorry,can't set partition name"
            self.lu.do_log_msg(self.logger,"warning","sorry,conn't set partition name")
        else:
            partition.name=part_name
        

    def set_disk_partition_fstype(self,partition,fstype):
        '''format the partition to given fstype,not create the parted fs object'''
        if partition.type!=parted.PARTITION_NORMAL and partition.type!=parted.PARTITION_LOGICAL:
            # print "you can only set filesystem for primary and logical partitionn"
            self.lu.do_log_msg(self.logger,"error","can only set fs for primary/logical partitionn")
            return

        for item in self.disk_partition_info_tab:
            if item[0]==partition and item[5]==False:
                print "no need to format the partition"
                return 

        if fstype==None or len(fstype)==0:
            # print "no filesystem specified"
            self.lu.do_log_msg(self.logger,"error","no filesystem specified")
            return
        part_path=partition.path
        #swapoff the partition before change filesystem,if not work,simply reduce code
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
            # print "need mountpoint,not given"
            self.lu.do_log_msg(self.logger,"error","need mountpoint,not given")
            return 

        if partition.type==parted.PARTITION_EXTENDED:
            # print "cann't mount extended partition"
            self.lu.do_log_msg(self.logger,"error","cann't mount extended partition")
            return

        part_path=partition.path
        mp=TARGET+mountpoint
        if not os.path.exists(mp):
            mkdir_command="sudo mkdir -p "+mp
            run_os_command(mkdir_command)
        if not os.path.exists(part_path):
            # print "partition not exists,commit to os first"
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
            # print "don't need to umount"+part_path
            return

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

    def set_disk_partition_flag(self,partition,flag,state):
        '''set the partition status to state'''
        if flag not in partition.getFlagsAsString():
            # print "flag invalid"
            self.lu.do_log_msg(self.logger,"warning","flag invalid")
        if not partition.isFlagAvailable():
            # print "flag not available"
            self.lu.do_log_msg(self.logger,"warning","flag not available")
        else:    
            if state=="on" | state=="True":
                partition.setFlag()
            else:
                partition.unsetFlag()

    def write_partition_table(self):
        '''write partition change info to /etc/fstab'''
        pass

    def generate_view_partition_path(self):
        '''calc partition path display to user,and as arg to manage disk_partition_info_tab
        set/get partition path for the tree view'''
        # to be implement
        pass

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

    def get_disk_main_partitions(self,disk):
        '''get primary and extended part list,attention:not include marked delete part'''

        return filter(lambda item:item[-1]!="delete" and item[0].disk==disk 
                                and (item[0].type ==0 or item[0].type==2),self.disk_partition_info_tab)

    def get_disk_primary_partitions(self,disk):
        '''get primary part list'''

        return filter(lambda item:item[-1]!="delete" and item[0].disk==disk 
                                and (item[0].type ==0),self.disk_partition_info_tab)

    def get_disk_extended_partition(self,disk):
        '''return extended partition of the disk'''

        return filter(lambda item:item[-1]!="delete" and item[0].disk==disk 
                                and (item[0].type ==2),self.disk_partition_info_tab)


    def get_disk_logical_partitions(self,disk):
        '''get disk logical part list'''

        return filter(lambda item:item[-1]!="delete" and item[0].disk==disk 
                                and (item[0].type ==1),self.disk_partition_info_tab)


    def init_disk_part_display_path(self):
        '''display_path for vitual path to display in UI listview'''
        for disk in self.get_system_disks():
            self.disk_part_display_path[disk]={}
            for part in self.get_disk_partitions(disk):
                self.disk_part_display_path[disk][part]=part.path

        return self.disk_part_display_path       

    def rebuild_disk_partition_info_tab(self,disk):
        '''backend operation for UI:create new disk partition tab'''
        for item in self.disk_partition_info_tab:
            if item[0].disk==disk:
                self.delete_disk_partition_info_tab(item[0])

        self.disk_part_display_path[disk].clear()        
            
    def recovery_disk_partition_info_tab(self,disk):
        '''backend operation for UI:recovery edited disk partition tab'''
        #init disk_partition_info_tab
        self.disk_partition_info_tab=filter(lambda item:item[0].disk!=disk,self.disk_partition_info_tab)
        for item in self.disk_partition_info_tab:
            if item[0].disk==disk:
                print "already removed item in disk_partition_info_tab,you wonn't see this"
                self.disk_partition_info_tab.remove(item)

        for item in self.backup_disk_partition_info_tab:
            if item[0].disk==disk:
                self.disk_partition_info_tab.append(item)
        #init disk_part_display_path        
        if disk in self.disk_part_display_path.keys():
            self.disk_part_display_path[disk].clear()
            del self.disk_part_display_path[disk]
        self.disk_part_display_path[disk]={}    
        for part in self.get_disk_partitions(disk):
            self.disk_part_display_path[disk][part]=part.path


#should use global part_util to keep disk/partition/device id uniquee
global_part_util=PartUtil()


def test_operate_disk_partition_info_tab_path_disks_partitions():
    pu=PartUtil()
    print "system disks:"
    print pu.path_disks
    print "\n"
    print "system_disks partitions:"
    print pu.path_disks_partitions
    print "\n"
    print "disk_partition_info_tab"
    print pu.disk_partition_info_tab
    print "\n"
    print "add disk_partition_info_tab"
    pu.add_disk_partition_info_tab("/dev/sda","primary",1024,"ext4",None,None,"/")
    pu.add_disk_partition_info_tab("/dev/sda","primary",2048,"ext3",None,None,"/home")
    print pu.disk_partition_info_tab
    print "\n"
    print "after add system_disks partitions:"
    print pu.path_disks_partitions
    print "\n"
    print "delete new added disk_partition_info_tab"
    disk=pu.get_disk_from_path("/dev/sda")
    part=pu.path_disks_partitions[disk][0]
    pu.delete_disk_partition_info_tab(part)
    print pu.disk_partition_info_tab
    print "\n"
    print "after delete system_disks partitions"
    print pu.path_disks_partitions
    print "\n"
    print "delete original disk_partition_info_tab"
    disk=pu.get_disk_from_path("/dev/sdb")
    part=pu.path_disks_partitions[disk][0]
    pu.delete_disk_partition_info_tab(part)
    print pu.disk_partition_info_tab
    print "\n"
    print "after delete original system_disks partitions"
    print pu.path_disks_partitions

def test_delete_new_partition():
    pu=PartUtil()    
    pu.add_disk_partition_info_tab("/dev/sda","primary",1024,"ext4",None,None,"/")
    pu.add_disk_partition_info_tab("/dev/sda","extend",4096,"ext4",None,None,None)
    pu.add_disk_partition_info_tab("/dev/sda","logical",1024,"ext4",None,None,"/home")
    pu.add_disk_partition_info_tab("/dev/sda","logical",1024,"ext3",None,None,None)

    disk=pu.get_disk_from_path("/dev/sda")
    part=pu.path_disks_partitions[disk][2]

    pu.delete_disk_partition_info_tab(part)
    # print pu.path_disks_partitions[disk]
    for item in pu.disk_partition_info_tab:
        print item
    pu.add_custom_partition(pu.disk_partition_info_tab)

def test_delete_mount_extend_partition():
    pu=PartUtil()    
    pu.add_disk_partition_info_tab("/dev/sda","primary",1024,"ext4",None,None,"/")
    pu.add_disk_partition_info_tab("/dev/sda","extend",4096,"ext4",None,None,None)
    pu.add_disk_partition_info_tab("/dev/sda","logical",1024,"ext4",None,None,"/home")
    pu.add_disk_partition_info_tab("/dev/sda","logical",1024,"ext3",None,None,None)
    pu.add_disk_partition_info_tab("/dev/sda","primary",1024,"ext2",None,None,None)

    disk=pu.get_disk_from_path("/dev/sda")
    part=pu.path_disks_partitions[disk][2]
    pu.delete_disk_partition_info_tab(part)

    disk2=pu.get_disk_from_path("/dev/sdb")
    part2=pu.get_disk_partitions(disk2)[2]
    pu.delete_disk_partition_info_tab(part2)

    pu.delete_custom_partition("/dev/sdb5")

    pu.add_disk_partition_info_tab("/dev/sdb","logical",1000,"ext3",None,None,"/home")
    pu.add_disk_partition_info_tab("/dev/sdb","primary",1000,"ext3",None,None,None)

    pu.add_custom_partition(pu.disk_partition_info_tab)

    
if  __name__=="__main__":
    test_delete_mount_extend_partition()
    pu=PartUtil()
    # print pu.get_disk_free_space_info()
    # print "/dev/sdb:"
    # disk=pu.get_disk_from_path("/dev/sdb")
    # print pu.get_disk_total_available_space_size(disk)
    # print pu.get_disk_single_available_space_size(disk,"primary")

    # pu.if_need_swap()
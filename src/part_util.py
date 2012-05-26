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

    #machine based:[disk,geometry,partition],to do the backend operations,may not need this table,consider to delete
        self.disk_partition_tab =[]

    #user based:(partition,part_disk_path,part_type,part_size,part_fs,part_format,part_name,part_mountpoint,part_flag),frontend 
        self.disk_partition_info_tab=self.init_disk_partition_info_tab()

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
                self.part_format=True
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

    def get_disk_free_geometry(self,disk,part_type,minlength):
        '''get free geometry to add part:specified by part_type and length'''
        #get current disk info first
        primary_part=[]
        logical_part=[]
        extended_part=[]
        extend_part=""
        main_part_list=[]

        for item in filter(lambda info:info[0].disk==disk,self.disk_partition_info_tab):
            if item[2]=="primary":
                primary_part.append(item[0])
                continue
            elif item[2]=="logical":
                logical_part.append(item[0])
                continue
            elif item[2]=="extend":
                extended_part.append(item[0])
                continue
            else:
                print "invalid part type"
                continue
        if len(extended_part)>1:        
            print "can have only one extended_part"
            return
        elif len(extended_part)==0:
            print "the disk "+disk.device.path+" doesn't have extended_part"
        else:
            extend_part=extended_part[0]

        #consider extend part as primary,because they have the same way to use disk geometry space   
        if len(primary_part)!=0:
            main_part_list=filter(lambda item:item in primary_part,primary_part)
            if len(extended_part)!=0:
                main_part_list.append(extended_part[0])
        elif len(primary_part)==0 and len(extended_part)!=0:
            main_part_list=extended_part
        else:#blank disk
            return disk.getFreeSpaceRegions()[0]
        
        main_geom_list=[]
        logical_geom_list=[]   
        main_geom_gap_list=[]
        logical_geom_gap_list=[]

        if part_type=="primary" or part_type=="extend":
            for part in main_part_list:
                main_geom_list.sort(cmp=lambda x,y:cmp(x.start,y.start))
                main_geom_list.append(part.geometry)

            start=disk.getFreeSpaceRegions()[0].start+4
            end=main_geom_list[0].start-4
            length=end-start+1
            if length > minlength:
                print "there have enough space between the start of the disk and the first partition"
                main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))

            for i in range(len(main_geom_list)-2):
                start=main_geom_list[i].end+4
                end=main_geom_list[i+1].start-4
                length=end-start+1
                if length > minlength:
                    main_geom_gap=parted.geometry.Geometry(disk.device,start,length,end,None)
                    main_geom_gap_list.append(main_geom_gap)
                else:
                    print "have geometry overlap or disk size too small to satisfy minlength"
                i=i+1    

            start=main_geom_list[-1].end+4    
            end=disk.getFreeSpaceRegions()[-1].end-4 
            length=end-start+1
            if length > minlength:
                main_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
            else:
                print "end of disk have geometry overlap or disk size too small to satisfy the minlength"
                
            if len(main_geom_gap_list)==0:
                print "cann't get enough space to create the partition"
                return

            main_geom_max=main_geom_gap_list[0]    
            for geom in main_geom_gap_list:    
                if geom.length > main_geom_max.length:
                    main_geom_max=geom

            return main_geom_max
        
        elif part_type=="logical":
            if extended_part==[] or extend_part==None:
                print "must have an extended partition before add logical partition"
                return 

            for part in logical_part:
                logical_geom_list.append(part.geometry)
                logical_geom_list.sort(cmp=lambda x,y:cmp(x.start,y.start))
            #if there is no logical partition,just return the hole extended partition geometry
            if len(logical_geom_list)==0:
                return extend_part.geometry
            
            start=extend_part.geometry.start+4
            end=logical_geom_list[0].start-4
            length=end-start
            if length > minlength:
                print "there have enough space between the begin of extend_part and the first logical_part"
                logical_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))


            for i in range(len(logical_geom_list)-2):
                start=logical_geom_list[i].end+4
                end=logical_geom_list[i+1]-4
                length=end-start+1
                if length > minlength:
                    logical_geom_gap=parted.geometry.Geometry(disk.device,start,length,end,None)
                    logical_geom_gap_list.append(logical_geom_gap)
                else:
                    print "have geometry overlap or disk is too small to satisfy the minlength"
                i=i+1    

            start=logical_geom_list[-1].end+4
            end=extend_part.geometry.end-4
            length=end-start
            if length > minlength:
                logical_geom_gap_list.append(parted.geometry.Geometry(disk.device,start,length,end,None))
            else:
                print "end of extend_part have geometry overlap or disk is too small to satisfy the minlength"
            
            if len(logical_geom_gap_list)==0:
                print "cann't get enough space to create the logical partition"
                return

            logical_geom_max=logical_geom_gap_list[0]
            for geom in logical_geom_gap_list:
                if geom.length > logical_geom_max.length:
                    logical_geom_max=geom

            return logical_geom_max        
        else:
            print "invalid partition type"
            return

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
        minlength=math.floor(part_size*1024*1024/self.disk.device.sectorSize)+1

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
        partition=self.get_disk_partition_object(part_disk_path,part_type,part_size,part_fs)
        if partition==None:
            print "partition is null"
            return 
        part_flag="add"   
        self.disk_partition_info_tab_item=[partition,part_disk_path,part_type,part_size,part_fs,
                                           part_format,part_name,part_mountpoint,part_flag]
        self.disk_partition_info_tab.append(self.disk_partition_info_tab_item)
        return self.disk_partition_info_tab

    def mark_disk_partition_info_tab(self,partition,part_flag):
        '''part_flag:keep,add,delete,according the flag to adjust the disk_partition_info_tab
           Then I can do partition add delete without bother the existed partition
        '''
        if(partition==None):
            print "partition doesn't exist"
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
                print "invalid,if partition marked delete,you wonn't see it"
                break

        return self.disk_partition_info_tab

#may not need this table
    def add_disk_partition_tab(self,disk,geometry,partition):
        '''add disk part tab,need consider handle except and part mount info'''
        disk_partition_tab_item=(disk,geometry,partition)
        try:
            self.disk_partition_tab.append(disk_partition_tab_item)
        except:
            print "add disk partition tab failed!"
        # return self.disk_partition_tab    


    def delete_disk_partition_tab(self,disk,partition):
        '''delete disk part tab item'''
        for disk_partition_tab_item in self.disk_partition_tab:
            if disk in disk_partition_tab_item and partition in disk_partition_tab_item:
                self.disk_partition_tab.remove(disk_partition_tab_item)
        else:
            print "the partition is not in the disk_partition_tab"
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
            print "somewhere id not match"

        self.path_disks_partitions[disk].append(partition)    

    def delete_path_disks_partitions(self,disk,partition):
        '''delete partition from path_disks_partitions variable,keep partition id uniquee'''
        #to add exceptionl handle
        if disk.getPedDisk!=partition.disk.getPedDisk:
            print "somewhere id not match"
        
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
            print "there's no disk specified by the path"
            return 


    def get_device_from_path(self,dev_path):
        '''from path:/dev/sda to get device,old function,avoid to use this'''
        return parted.device.Device(dev_path,None)

    def get_disk_size(self,disk):
        '''test function ,never used'''
        print disk.device.path+" disk size:"+ str(disk.device.getSize("GB")*1.0)+"GB"
       
        for part in self.get_disk_partitions(disk):
            print self.get_disk_partition_size(part)

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
            print "partition not in the disk"
            return

        if partition.type==parted.PARTITION_EXTENDED and disk.getLogicalPartitions()!=[]:
            print "need delete all logical partitions before delete extend partition"
            for logical_part in disk.getLogicalPartitions():
                self.delete_path_disks_partitions(disk,logical_part)
                self.disk_partition_info_tab=filter(lambda info:info[0]!=logical_part,self.disk_partition_info_tab)
                try:
                    self.set_disk_partition_umount(logical_part)
                    disk.deletePartition(logical_part)
                except:
                    print "delete logical_part failed"

        self.delete_path_disks_partitions(disk,partition)    
        self.disk_partition_info_tab=filter(lambda info:info[0]!=partition,self.disk_partition_info_tab)
        try:
            self.set_disk_partition_umount(partition)
            disk.deletePartition(partition)
        except:
            print "error occurs"
        disk.commit()

    def delete_custom_partition(self,partition_path):
        '''delete disk partition:get partition_path from ui
           This function can be called only because you want to delete the original partition
           umount it first
        '''
        for item in filter(lambda info:info[0].path==partition_path,self.disk_partition_info_tab):
            if item[-1]=="delete":
                self.partition=item[0]
                break
            else:
                continue
        if self.partition==None:
            print "partition doesn't exist"
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
                print "can't add logical partition as there is no extended partition"
                break
            else:
                self.partition=item[0]
                self.geometry=self.partition.geometry
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
            print "part type error"
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
            print "sorry,can't set partition name"
        else:
            partition.name=part_name
        

    def set_disk_partition_fstype(self,partition,fstype):
        '''format the partition to given fstype,not create the parted fs object'''
        if partition.type!=parted.PARTITION_NORMAL and partition.type!=parted.PARTITION_LOGICAL:
            print "you can only set filesystem for primary and logical partitionn"
            return
        if fstype==None or len(fstype)==0:
            print "no filesystem specified"
            return

        part_path=partition.path
        format_command="sudo mkfs -t "+fstype+" "+part_path
        run_os_command(format_command)
    

    def set_disk_partition_mount(self,partition,fstype,mountpoint):
        '''mount partition to mp:new or modify,need consider various situation'''

        if partition.type==parted.PARTITION_EXTENDED:
            print "cann't mount extended partition"
            return
        if mountpoint==None or len(mountpoint)==0:
            print "need mountpoint,not given"
            return 
        part_path=partition.path
        mp=TARGET+mountpoint
        if not os.path.exists(mp):
            mkdir_command="sudo mkdir -p "+mp
            run_os_command(mkdir_command)
        if not os.path.exists(part_path):
            print "partition not exists,commit to os first"
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
            print "flag invalid"
        if not partition.isFlagAvailable():
            print "flag not available"
        else:    
            if state=="on" | state=="True":
                partition.setFlag()
            else:
                partition.unsetFlag()


    def generate_view_partition_path(self):
        '''calc partition path display to user,and as arg to manage disk_partition_info_tab
        set/get partition path for the tree view'''
        # to be implement
        pass

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
    part2=pu.get_disk_partitions(disk2)[0]
    pu.delete_disk_partition_info_tab(part2)
    pu.delete_custom_partition("/dev/sdb2")

    pu.add_disk_partition_info_tab("/dev/sdb","logical",1024,"ext2",None,None,"/home")
    pu.add_disk_partition_info_tab("/dev/sdb","primary",1000,"ext3",None,None,None)

    pu.add_custom_partition(pu.disk_partition_info_tab)
    
if  __name__=="__main__":
    test_delete_mount_extend_partition()


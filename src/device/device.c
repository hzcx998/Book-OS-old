/*
File:		device/device.c
Contains:	all device set here
Auther:		Hu Zicheng
Time:		2019/3/31
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_


#include <string.h>

#include <sys/fs.h>
#include <sys/dev.h>

/*device talbe*/
static struct disk_device disk_device_table[MAX_DEVICE_ID_NR];

/*init device talbe*/
void init_disk_device()
{
	int i;
	/*init every disk device*/
	for(i = 0; i < MAX_DEVICE_ID_NR; i++){
		/*set master and slave id*/
		disk_device_table[i].dev_master = DEVICE_ID_TO_MASTER(i);
		disk_device_table[i].dev_slave = DEVICE_ID_TO_SLAVE(i);
		/*default -1, when we use it, it is device id*/
		disk_device_table[i].dev_id = -1;
	}
}
/*
install a kind of device to disk device table
master is types. slave is offset.
return 0 is sucess, -1 is failed
*/
int disk_device_install(uint32 master, uint32 slave)
{
	int i;
	struct disk_device *dd = &disk_device_table[0];
	for(i = 0; i < MAX_DEVICE_ID_NR; i++){
		/*if device id is -1, it means that this disk device is free*/
		if(dd->dev_id == -1){
			/*if master and slave are same*/
			if(dd->dev_master == master &&\
				dd->dev_slave == slave){
				/*set device id by master and slave*/
				dd->dev_id = MAKE_DEVICE_ID(master, slave);
				//printk("device[%d] master:%d slave:%d id:%d\n", i, master, slave, disk_device_table[i].dev_id);
				
				return 0;
			}
		}
		dd++;
	}
	return -1;
}

/*
uninstall a kind of device from disk device table
by device id.
return 0 is sucess, -1 is failed
*/
int disk_device_uninstall(uint32 dev_id)
{
	int i;
	struct disk_device *dd = &disk_device_table[0];
	for(i = 0; i < MAX_DEVICE_ID_NR; i++){
		/*if device id is -1, it means that this disk device is free*/
		if(dd->dev_id != -1 && dd->dev_id == dev_id){
			/*set device id to -1*/
			dd->dev_id = -1;
			return 0;
		}
		dd++;
	}
	return -1;
}

/*
get a disk device by device id
*/
struct disk_device *disk_device_get_by_id(uint32 dev_id)
{
	/*device id error*/
	if(dev_id < 0 || dev_id >= MAX_DEVICE_ID_NR){
		return NULL;
	}
	//printk("device[%d] master:%d slave:%d id:%d\n", id, disk_device_table[id].dev_master, disk_device_table[id].dev_slave, disk_device_table[id].dev_id);
	return &disk_device_table[dev_id];
}
/*
select a disk by device id 
*/
int select_disk_device(int dev_id)
{
	/*device id error*/
	if(dev_id < 0 || dev_id >= MAX_DEVICE_ID_NR){
		return -1;
	}

	/*get disk device*/
	struct disk_device *dd = &disk_device_table[dev_id];
	
	/*check device exist*/
	if(dd->dev_id == -1){
		return -1;
	}
	
	/*if this is harddisk*/
	if(dd->dev_master == DEVICE_HARDDISK){
		/*
		if this kind of disk device has many disk.
		we set harddisk by device slave
		*/
		if(dd->dev_slave == 0){
			set_current_hd("hda");
		}else if(dd->dev_slave == 1){
			set_current_hd("hdb");
		}else if(dd->dev_slave == 2){
			set_current_hd("hdc");
		}else if(dd->dev_slave == 3){
			set_current_hd("hdd");
		}
		/*select a device type we will operate*/
		select_disk_device_type("harddisk");
		
	}else if(dd->dev_master == DEVICE_RAMDISK){
		/*select a device type we will operate*/
		select_disk_device_type("ramdisk");
	}
	return 0;
}

/*select a device*/
void select_disk_device_type(char *name)
{
	if(!strcmp(name, "ramdisk")){
		sector_read = ramdisk_read;
		sector_write = ramdisk_write;
	}else if(!strcmp(name, "harddisk")){
		sector_read = hd_read_sectors;
		sector_write = hd_write_sectors;
	}
}

#endif //_CONFIG_DEVIVE_

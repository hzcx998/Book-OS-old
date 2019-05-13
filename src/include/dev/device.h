#ifndef _DEVICE_H
#define _DEVICE_H
#include <types.h>
#include <stdint.h>

/*a kind of device has how many disk, here we have DEVICE_MAX_SLAVE_NR*/
#define DEVICE_MAX_SLAVE_NR 8

/*get master num by device id.
if id = 7: master = 0	(7/8)
if id = 15: master = 1 	(15/8)	
*/
#define DEVICE_ID_TO_MASTER(id) (id/DEVICE_MAX_SLAVE_NR)

/*get slave num by device id
if id = 7: slave = 7	(7%8)
if id = 15: slave = 7 	(15%8)	
*/

#define DEVICE_ID_TO_SLAVE(id) (id%DEVICE_MAX_SLAVE_NR)

/*make a device id by master and slave
if master = 1, slave = 5: device id = 13	(1*8+5)
if master = 3, slave = 2: device id = 26	(3*8+2)
*/
#define MAKE_DEVICE_ID(master, slave) (master*DEVICE_MAX_SLAVE_NR + slave)

/*device type*/
#define DEVICE_FLOPPY 		0		/*this is floppy device*/
#define DEVICE_CDROM 		1		/*this is cdrom device*/
#define DEVICE_HARDDISK 	2		/*this is harddisk device*/
#define DEVICE_RAMDISK 		3		/*this is ramdisk device*/

/*the max types of device*/
#define DEVICE_MAX_TYPES_NR 4
/*
we have device id for a hole system. 
if we add device type, we need change DEVICE_MAX_TYPES_NR to make
MAX_DEVICE_ID_NR correctly.
*/
#define MAX_DEVICE_ID_NR (DEVICE_MAX_TYPES_NR*DEVICE_MAX_SLAVE_NR)

#define DEVICE_NAME_LEN 12

/*
device struct
master is using for device type.
if 0: DEVICE_FLOPPY
if 2: DEVICE_HARDDSIK

slave is using for this type device disks.
if we have 4 harddisk in our computer, slave is 0, 1, 2, 3

device id is make by master and slave.
it's unique for disk device.
*/
struct disk_device
{
	uint32 dev_master;	/*master num*/
	uint32 dev_slave;	/*slave num*/
	uint32 dev_id;		/*device id*/
};

void init_disk_device();
int disk_device_install(uint32 master, uint32 slave);
int disk_device_uninstall(uint32 dev_id);
struct disk_device *disk_device_get_by_id(uint32 dev_id);
int select_disk_device(int dev_id);

void select_disk_device_type(char *name);

void (*sector_read)(uint32 lba, void *buf, uint32 counts);
void (*sector_write)(uint32 lba, void *buf, uint32 counts);



#endif


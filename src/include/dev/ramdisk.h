#ifndef _DEVICE_RAMDISK_H_
#define _DEVICE_RAMDISK_H_
#include <types.h>

#include <sys/mm.h>

#define RAM_BLOCK_SIZE 512


#define RAMDISK_START 0x300000
#define RAMDISK_SIZE (PHY_MEM_BASE_ADDR-RAMDISK_START)



struct ramdisk
{
	uint32 size;
	uint32 blocks;
	uint8 *start_addr;
	
};
extern struct ramdisk ramdisk;
void init_ramdisk();

void ramdisk_read(uint32 lba, void *buf, uint32 counts);
void ramdisk_write(uint32 lba, void *buf, uint32 counts);


#endif


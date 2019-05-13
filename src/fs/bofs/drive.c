/*
File:		fs/bofs/dir.c
Contains:	bofs's dir
Auther:		Hu Zicheng
Time:		2019/3/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <sys/fs.h>

#include <string.h>
#include <sys/dev.h>
#include <sys/core.h>

/*this is drive table, all info are here!*/
struct bofs_drive bofs_drive_table[BOFS_MAX_DRIVE_NR];
/*init drive talbe*/
void bofs_init_drive()
{
	int i;
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*set name to drive*/
		bofs_drive_table[i].name[0] = 'a'+i;
		bofs_drive_table[i].name[1] = 'A'+i;
		/*set id is -1, when using, it is >= 0*/
		bofs_drive_table[i].dev_id = -1;

		/*set type name to zero*/
		memset(bofs_drive_table[i].type_name,  0, BOFS_DRIVE_TYPE_NAME_LEN);
	}
}
/*
install a drive, after that, we can operate a file like this.
C:/ or D:/ and so on...
return dirve (c,d,e,f ...) is sucess, return 0 is failed
*/
char bofs_install_drive(char start ,int dev_id)
{
	int i;
	int master;

	/*scan from start. we don't scan from 0 every time.
	sometimes we want scan from other palce*/
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*if drive name are same, we break out. we use value i to start our scan.*/
		if(bofs_drive_table[i].name[0] == start ||\
			bofs_drive_table[i].name[1] == start
		){
			break;
		}
	}
	/*set drive with drive id*/
	for(; i < BOFS_MAX_DRIVE_NR; i++){
		/*if it is a free dev id, we will use it*/
		if(bofs_drive_table[i].dev_id == -1){
			/*set dev id and return 0 for sucess*/
			bofs_drive_table[i].dev_id = dev_id;

			/*set drive disk type name*/
			master = DEVICE_ID_TO_MASTER(dev_id);
			switch(master){
				case DEVICE_HARDDISK:
					strcpy(bofs_drive_table[i].type_name,  "harddisk");
					break;
				case DEVICE_RAMDISK:
					strcpy(bofs_drive_table[i].type_name,  "ramdisk");
					break;
				default:
					break;
			}

			return bofs_drive_table[i].name[0];
		}
	}
	/*install failed!*/
	return 0;
}

/*
uninstall a drive
return device id.
*/
int bofs_uninstall_drive(char drive)
{
	int i;
	/*scan from start. we don't scan from 0 every time.
	sometimes we want scan from other palce*/
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*if drive name are same, we break out. we use value i to start our scan.*/
		if(bofs_drive_table[i].name[0] == drive ||\
			bofs_drive_table[i].name[1] == drive
		){
			/*we return dev id*/
			i = bofs_drive_table[i].dev_id;
			//disk_device_uninstall(i);
			bofs_drive_table[i].dev_id = -1;
			return i;
		}
	}
	return -1;
}

/*
select a disk drive.
we set current drive's super block, we will operate it latter
!NOTE we can't select a drive we didn't installed
*/
int bofs_select_drive(char drive)
{
	int i;
	/*search from talbe*/
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*if it's not a avaliable drive.*/
		if(bofs_drive_table[i].dev_id != -1){
			/*if the name are same.*/
			if(drive == bofs_drive_table[i].name[0] || drive == bofs_drive_table[i].name[1]){
				/*set a superblock which we will operate*/
				bofs_select_current_super_block(bofs_drive_table[i].dev_id);
				/*select a disk which we want operate on*/
				if(select_disk_device(bofs_drive_table[i].dev_id) == -1){
					printk("drive: %c not exist!\n", drive);
					return -1;
				}
				//printk("The drive[%d] is %c, dev id:%d\n", i, bofs_drive_table[i].name[0], bofs_drive_table[i].dev_id);
				return 0;
			}
		}	
	}
	printk("drive: %c not exist!\n", drive);
	return -1;
}

struct bofs_drive *bofs_drive_connect()
{
	return bofs_drive_table;	
}

struct bofs_drive *bofs_drive_get(struct bofs_drive *drive)
{
	int i;
	
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*if it's not a avaliable drive.*/
		if(bofs_drive_table[i].dev_id != -1){
			/*if the name are same.*/
			if (&bofs_drive_table[i] > drive) {
				return &bofs_drive_table[i];
			}
		}	
	}

	return NULL;	
}

int bofs_list_drive()
{
	printk("=====Disk Drive=====\n");
	printk("-Drive-   -Type-\n");

	int drive_nr = 0;
	
	struct bofs_drive *drive = bofs_drive_connect();


	do {
		drive = bofs_drive_get(drive);
		if (drive != NULL) {
			printk(" %c:        %s\n", \
				drive->name[1], drive->type_name);
			drive_nr++;
		}
	} while (drive != NULL);

	return drive_nr;
}

int bofs_list_drive2()
{
	int i;
	printk("=====Disk Drive=====\n");
	printk("-Drive-   -Type-\n");

	int drive_nr = 0;
	
	/*search from talbe*/
	for(i = 0; i < BOFS_MAX_DRIVE_NR; i++){
		/*if it's not a avaliable drive.*/
		if(bofs_drive_table[i].dev_id != -1){
			/*if the name are same.*/

			printk(" %c:        %s\n", \
				bofs_drive_table[i].name[1], bofs_drive_table[i].type_name);
			drive_nr++;
		}	
	}
	return drive_nr;
}
#endif


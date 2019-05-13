/*
File:		fs/bofs/bofs.c
Contains:	new file system
Auther:		Hu Zicheng
Time:		2019/3/12
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <sys/fs.h>

#include <sys/dev.h>
#include <sys/core.h>

#include <sys/gui.h>
#include <sys/mm.h>
#include <sys/debug.h>
#include <string.h>

/*we have serval devices, so we have serval bsb*/

void *bofs_dir_entry_io_buf;
void *bofs_inode_io_buf;
void *bofs_file_io_buf;

void *bofs_io_buf, *bofs_io_buf_table[IO_BUF_NR];
struct bofs_super_block *bofs_current_super_block;

#define DISK_SIZE 10*1024*1024

void init_bofs()
{
	printk("> init bofs start.\n");
	
	/*init io buf for different part of fs*/
	bofs_dir_entry_io_buf = mm.malloc(SECTOR_SIZE*2);
	bofs_inode_io_buf = mm.malloc(SECTOR_SIZE*2);
	bofs_file_io_buf = mm.malloc(SECTOR_SIZE*2);
	bofs_io_buf = mm.malloc(SECTOR_SIZE*2);
	/*if we alloc failed, we panic*/ 
	if(bofs_dir_entry_io_buf == NULL \
		|| bofs_inode_io_buf == NULL \
		|| bofs_file_io_buf == NULL \
		|| bofs_io_buf == NULL
	){
		panic("fs alloc mem failed!\n");
	}
	
	int i;
	for(i = 0; i < IO_BUF_NR; i++){
		bofs_io_buf_table[i] = mm.malloc(SECTOR_SIZE*2);
		if(bofs_io_buf_table[i] == NULL){
			panic("fs alloc mem failed!\n");
		}
	}
	/*init bofs super block*/
	bofs_init_super_block();
	
	/*init bofs drive manage*/
	bofs_init_drive();
	
	/*init bofs file descriptor*/
	bofs_init_fd_table();
	
	struct disk_device *dd;
	char drive;
	char start_drive;
	for(i = 0; i < MAX_DEVICE_ID_NR; i++, dd++){
		/*get a disk device*/
		dd = disk_device_get_by_id(i);
		/*if deivce id used*/
		if(dd->dev_id != -1){
			
			/*select a disk we will operate on*/
			select_disk_device(dd->dev_id);
			
			/*select start drive type*/
			switch(dd->dev_master){
				case DEVICE_HARDDISK:
					start_drive = BOFS_HARDDISK_DRIVE;
					break;
				case DEVICE_RAMDISK:
					start_drive = BOFS_RAMDISK_DRIVE;
					break;
				default: break;	
			}
			
			/*install drive to device*/
			drive = bofs_install_drive(start_drive, dd->dev_id);
			#ifdef _DEBUG_BOFS_
				printk("format fs on harddisk with dev:%d drive:%c\n", dd->dev_id, drive);
			#endif
			/*format a fs on disk*/
			bofs_format(dd->dev_id, 0, DISK_SIZE/SECTOR_SIZE, DEFAULT_MAX_INODE_NR);
		}
	}
	printk("< init bofs done.\n");
	
}

/*format a disk*/
void bofs_format(int dev_id, uint32 start_sec, uint32 total_sec, uint32 inode_nr)
{
	struct bofs_super_block *bsb = bofs_alloc_super_block();
		/*read bsb on disk*/
		sector_read(start_sec, bofs_io_buf, 1);  
		memcpy(bsb, bofs_io_buf, sizeof(struct bofs_super_block));
		if(bsb->magic != BSB_MAGIC){
			
			/*-----init super block-----*/
			
			bsb->magic = BSB_MAGIC;
			bsb->dev_id = dev_id;
			
			bsb->super_block_lba = start_sec;
			bsb->all_sectors = total_sec;	/*assume 10MB*/
			bsb->sector_bitmap_lba = bsb->super_block_lba+1;
			/*All sectors / 8bits / 512sectors = sector bitmap sectors*/
			bsb->sector_bitmap_sectors = total_sec/(8*SECTOR_SIZE);
			
			bsb->inode_bitmap_lba = bsb->sector_bitmap_lba+bsb->sector_bitmap_sectors;
			bsb->inode_bitmap_sectors = inode_nr/(8*SECTOR_SIZE);
			
			bsb->inode_table_lba = bsb->inode_bitmap_lba+bsb->inode_bitmap_sectors;

			bsb->inode_table_sectors = (inode_nr*sizeof(struct bofs_inode) )/(8*SECTOR_SIZE);
			
			bsb->data_start_lba = bsb->inode_table_lba + bsb->inode_table_sectors;
			
			bsb->root_inode_id = 0;
			bsb->inode_nr_in_sector = SECTOR_SIZE/sizeof(struct bofs_inode);
			
			
   			#ifdef _DEBUG_BOFS_
			   /*no fs on disk, format it*/
				printk("No fs on disk!\n");

				printk("BookOS file system super block:\n");
				printk("dev id:%d sectors:%d\nsector bitmap lba:%d sector bitmap sectors:%d\n", 
				bsb->dev_id, bsb->all_sectors, bsb->sector_bitmap_lba, bsb->sector_bitmap_sectors);
				printk("inode bitmap lba:%d inode bitmap sectors:%d\ninode table lba:%d inode table sectors:%d data start lba:%d\n", 
				bsb->inode_bitmap_lba, bsb->inode_bitmap_sectors, bsb->inode_table_lba, bsb->inode_table_sectors, bsb->data_start_lba);
			#endif	

			/*write bsb into disk*/
			memcpy(bofs_io_buf, bsb, sizeof(struct bofs_super_block));
			sector_write(bsb->super_block_lba, bofs_io_buf, 1);
			
			/*-----set sector bitmap -----*/
			char *buf = (char *)bofs_io_buf;
			
			memset(buf, 0, SECTOR_SIZE);
			
			buf[0] |= 1;
			
			sector_write(bsb->sector_bitmap_lba, buf, 1);
			
			/*-----set inode bitmap -----*/
			memset(buf, 0, SECTOR_SIZE);
			
			buf[0] |= 1;
			
			sector_write(bsb->inode_bitmap_lba, buf, 1);
			
			/*-----create root inode-----*/
			struct bofs_inode root_inode;
			bofs_create_inode(&root_inode);
			root_inode.id = 0;
			
			root_inode.mode |= (BOFS_IMODE_D|BOFS_IMODE_R|BOFS_IMODE_W);

			/*set time*/
			root_inode.crttime = datetime_to_data();
			root_inode.acstime = root_inode.mdftime = root_inode.crttime;

			root_inode.block[0] = bsb->data_start_lba;
			
			root_inode.size = sizeof(struct bofs_dir_entry);
			
			memset(buf, 0, SECTOR_SIZE);
			memcpy(buf, &root_inode, sizeof(struct bofs_inode));
			
			sector_write(bsb->inode_table_lba, buf, 1);
			
   			#ifdef _DEBUG_BOFS_
				printk("dir time: %d/%d/%d",
					DATA16_TO_DATE_YEA(root_inode.crttime>>16),
					DATA16_TO_DATE_MON(root_inode.crttime>>16),
					DATA16_TO_DATE_DAY(root_inode.crttime>>16));
				printk(" %d:%d:%d\n",
					DATA16_TO_TIME_HOU(root_inode.crttime),
					DATA16_TO_TIME_MIN(root_inode.crttime),
					DATA16_TO_TIME_SEC(root_inode.crttime));
			#endif
			
			/*-----create root dir-----*/
			
			struct bofs_dir_entry root_dir;
			root_dir.inode = 0;
			
			root_dir.file_type = BOFS_FILE_TYPE_DIRECTORY;
			
			memset(root_dir.name, 0, BOFS_NAME_LEN);
			strcpy(root_dir.name, "/");
			
			memset(buf, 0, SECTOR_SIZE);
			memcpy(buf, &root_dir, sizeof(struct bofs_dir_entry));
			sector_write(bsb->data_start_lba, buf, 1);
			
		}else{
			
    		#ifdef _DEBUG_BOFS_
				printk("Have fs on disk!\n");
				
				printk("BookOS file system super block:\n");
				printk("dev id:%d sectors:%d\nsector bitmap lba:%d sector bitmap sectors:%d\n", 
				bsb->dev_id, bsb->all_sectors, bsb->sector_bitmap_lba, bsb->sector_bitmap_sectors);
				printk("inode bitmap lba:%d inode bitmap sectors:%d\ninode table lba:%d inode table sectors:%d data start lba:%d\n", 
				bsb->inode_bitmap_lba, bsb->inode_bitmap_sectors, bsb->inode_table_lba, bsb->inode_table_sectors, bsb->data_start_lba);
			#endif

		}
	/*load bitmap from disk*/
	bofs_load_bitmap(bsb);
	/*set this bsb and we will operate on this disk*/
	bofs_select_current_super_block(dev_id);
	
	bofs_open_root_dir();
}

#endif


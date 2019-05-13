/*
File:		fs/bofs/bitmap.c
Contains:	bofs's bitmap
Auther:		Hu Zicheng
Time:		2019/3/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <sys/fs.h>
#include <sys/mm.h>

#include <string.h>

#include <sys/core.h>

void bofs_load_bitmap(struct bofs_super_block *bsb)
{
	//printk("Ready load bitmap.\n");
	/*read sector bitmap*/
	bsb->sector_bitmap.btmp_bytes_len = bsb->sector_bitmap_sectors*SECTOR_SIZE;
	bsb->sector_bitmap.bits = (uint8 *)mm.malloc(bsb->sector_bitmap.btmp_bytes_len);
	bitmap_init(&bsb->sector_bitmap);
	
	sector_read(bsb->sector_bitmap_lba, bsb->sector_bitmap.bits, bsb->sector_bitmap_sectors);
	
	/*read inode bitmap*/
	bsb->inode_bitmap.btmp_bytes_len = bsb->inode_bitmap_sectors*SECTOR_SIZE;
	bsb->inode_bitmap.bits = (uint8 *)mm.malloc(bsb->inode_bitmap.btmp_bytes_len);
	bitmap_init(&bsb->inode_bitmap);
	
	sector_read(bsb->inode_bitmap_lba, bsb->inode_bitmap.bits, bsb->inode_bitmap_sectors);
}

int bofs_alloc_bitmap(enum bofs_bitmap_type bitmap_type, uint32 counts)
{
	int idx, i;
	if(bitmap_type == SECTOR_BITMAP){
		idx = bitmap_scan(&bofs_current_super_block->sector_bitmap, counts);
		//printk("counts:%d idx:%d\n", counts, idx);
		//printk("<<<alloc sector idx:%d\n", idx);
		if(idx == -1){
			printk("alloc sector bitmap failed!\n");
			return -1;
		}
		for(i = 0; i < counts; i++){
			bitmap_set(&bofs_current_super_block->sector_bitmap, idx + i, 1);
		}
		return idx;
	}else if(bitmap_type == INODE_BITMAP){
		
		idx = bitmap_scan(&bofs_current_super_block->inode_bitmap, counts);
		//printk("counts:%d idx:%d\n", counts, idx);
		
		if(idx == -1){
			printk("alloc inode bitmap failed!\n");
			return -1;
		}
		for(i = 0; i < counts; i++){
			bitmap_set(&bofs_current_super_block->inode_bitmap, idx + i, 1);
		}
		return idx;
	}
	return -1;
}

int bofs_free_bitmap(enum bofs_bitmap_type bitmap_type, uint32 idx)
{
	if(bitmap_type == SECTOR_BITMAP){
		//printk(">>>free sector idx:%d\n", idx);
		if(idx == -1){
			printk("free sector bitmap failed!\n");
			return -1;
		}
		bitmap_set(&bofs_current_super_block->sector_bitmap, idx, 0);
		return idx;
	}else if(bitmap_type == INODE_BITMAP){
		if(idx == -1){
			printk("free inode bitmap failed!\n");
			return -1;
		}
		bitmap_set(&bofs_current_super_block->inode_bitmap, idx, 0);
		return idx;
	}
	return -1;
}

int bofs_sync_bitmap(enum bofs_bitmap_type bitmap_type, uint32 idx)
{
	uint32 off_sec = idx / (8*SECTOR_SIZE);
	uint32 off_size = off_sec * SECTOR_SIZE;
	uint32 sec_lba;
	uint8 *bitmap_off;
	if(bitmap_type == SECTOR_BITMAP){
		//printk("^^^sync sector idx:%d\n", idx);
		sec_lba = bofs_current_super_block->sector_bitmap_lba + off_sec;
		bitmap_off = bofs_current_super_block->sector_bitmap.bits + off_size;
		
		sector_write(sec_lba, bitmap_off, 1);

		return true;
	}else if(bitmap_type == INODE_BITMAP){
		
		sec_lba = bofs_current_super_block->inode_bitmap_lba + off_sec;
		bitmap_off = bofs_current_super_block->inode_bitmap.bits + off_size;
		
		sector_write(sec_lba, bitmap_off, 1);

		return true;
	}
	return false;
}

int bofs_idx_to_lba(enum bofs_bitmap_type bitmap_type, uint32 idx)
{
	if(bitmap_type == SECTOR_BITMAP){
		return bofs_current_super_block->data_start_lba + idx;
	}else if(bitmap_type == INODE_BITMAP){
		return bofs_current_super_block->inode_bitmap_lba + idx;
	}
	return -1;
}

int bofs_lba_to_idx(enum bofs_bitmap_type bitmap_type, uint32 lba)
{
	if(bitmap_type == SECTOR_BITMAP){
		return (lba - bofs_current_super_block->data_start_lba);
	}else if(bitmap_type == INODE_BITMAP){
		return (lba - bofs_current_super_block->inode_bitmap_lba);
	}
	return -1;
}

#endif


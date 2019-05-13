#ifndef _BOFS_SUPER_BLOCK_H
#define _BOFS_SUPER_BLOCK_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <stdint.h>
#include <sys/core.h>

#include <fs/bofs/dir.h>

/*book os file system super block*/
#define BSB_MAGIC 0x19980325

/*we have 26 super blocks in memory*/
#define MAX_BSB_NR 26

struct bofs_super_block
{
	/*disk info*/
	uint32 magic;
	uint32 dev_id;		/*this fs install on which dev*/
	uint32 all_sectors;	/*will use how many sectors*/
	
	uint32 super_block_lba;
	
	uint32 sector_bitmap_lba;	/*sector bitmap start at lba*/
	uint32 sector_bitmap_sectors;	/*sector bitmap use sectors*/
	
	/*fs info*/
	uint32 inode_bitmap_lba;	/*inode bitmap start at lba*/
	uint32 inode_bitmap_sectors;	/*inode bitmap use sectors*/
	
	uint32 inode_table_lba;	/*inode table start at lba*/
	uint32 inode_table_sectors;	/*inode table use sectors*/
	
	uint32 data_start_lba;
	/*inode info*/
	uint32 root_inode_id;	/*root inode start at lba*/
	uint32 inode_nr_in_sector;	/*how many inode a sector can store*/
	
	/*use in ram*/
	struct bitmap sector_bitmap;	/*sector manager bitmap*/
	struct bitmap inode_bitmap;		/*inode manager bitmap*/
	struct bofs_dir *root_dir;
	
};

void bofs_init_super_block();
struct bofs_super_block *bofs_alloc_super_block();
void bofs_select_current_super_block(uint32 dev_id);
void bofs_free_super_block(struct bofs_super_block *sb);
#endif

#endif


/*
File:		fs/bofs/surper_block.c
Contains:	bofs's surper_block
Auther:		Hu Zicheng
Time:		2019/3/24
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <sys/fs.h>

#include <string.h>

/*super block talbe*/
struct bofs_super_block bofs_super_block_table[MAX_BSB_NR];

void bofs_init_super_block()
{
	int i;
	for(i = 0; i < MAX_BSB_NR; i++){
		bofs_super_block_table[i].magic = 0;
	}
}
/*
set a super block by device id.
if we want operate a disk, we need set super block
*/
void bofs_select_current_super_block(uint32 dev_id)
{
	if(dev_id < 0 || dev_id >= MAX_BSB_NR){
		return;
	}
	int i;
	for(i = 0; i < MAX_BSB_NR; i++){
		/*if device id is same*/
		if(bofs_super_block_table[i].dev_id == dev_id){
			//printk("set bsb: dev:%d idx:%d\n", bofs_super_block_table[i].dev_id, i);
			
			/*set global value bofs_current_super_block with right device id*/
			bofs_current_super_block = &bofs_super_block_table[i];
			break;
		}
	}
}
/*
alloc a super block for a file system
*/
struct bofs_super_block *bofs_alloc_super_block()
{
	int i;
	for(i = 0; i < MAX_BSB_NR; i++){
		/*if no magic, we alloc it*/
		if(bofs_super_block_table[i].magic == 0){
			/*set magic and return*/
			bofs_super_block_table[i].magic = BSB_MAGIC;
			return &bofs_super_block_table[i];
		}
	}
	return NULL;
}

void bofs_free_super_block(struct bofs_super_block *sb)
{
	memset(sb, 0, sizeof(struct bofs_super_block));
	sb->magic = 0;
}

#endif


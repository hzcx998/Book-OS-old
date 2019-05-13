/*
File:		fs/bofs/dir_entry.c
Contains:	bofs's dir_entry
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
#include <math.h>

#include <sys/dev.h>
#include <sys/core.h>

void bofs_create_dir_entry(struct bofs_dir_entry *dir_entry, char *name, uint32 flags)
{
	/*set dir entry info*/
	memset(dir_entry->name, 0, BOFS_NAME_LEN);
	strcpy(dir_entry->name, name);
	
	dir_entry->file_type = (flags>>16)&0xffff;
	
	/*alloc a inode*/
	dir_entry->inode = bofs_alloc_bitmap(INODE_BITMAP, 1);
	bofs_sync_bitmap(INODE_BITMAP, dir_entry->inode);
	
	struct bofs_inode inode;
	bofs_create_inode(&inode);
	inode.id = dir_entry->inode;
	inode.mode = flags&0xffff;
	if(dir_entry->file_type&BOFS_FILE_TYPE_NORMAL){
		inode.mode |= BOFS_IMODE_F;
	}else if(dir_entry->file_type&BOFS_FILE_TYPE_DIRECTORY){
		inode.mode |= BOFS_IMODE_D;
	}
	
	inode.crttime = datetime_to_data();
	inode.acstime = inode.mdftime = inode.crttime;
	/*
	printk("dir time: %d/%d/%d",
		DATA16_TO_DATE_YEA(inode.crttime>>16),
		DATA16_TO_DATE_MON(inode.crttime>>16),
		DATA16_TO_DATE_DAY(inode.crttime>>16));
	printk(" %d:%d:%d\n",
		DATA16_TO_TIME_HOU(inode.crttime),
		DATA16_TO_TIME_MIN(inode.crttime),
		DATA16_TO_TIME_SEC(inode.crttime));
	*/		
	bofs_sync_inode(&inode);
	
	//printk("new dir entry: name:%s inode:%d\n", dir_entry->name, dir_entry->inode);
}

void bofs_close_dir_entry(struct bofs_dir_entry *dir_entry)
{
	if(dir_entry == bofs_current_super_block->root_dir->m_dir_entry || dir_entry == NULL){	//can't close root dir
		return;
	}
	mm.free(dir_entry);
}

void bofs_copy_dir_entry(struct bofs_dir_entry *dir_a, struct bofs_dir_entry *dir_b, char copy_inode)
{
	/*copy dir entry*/
	memset(dir_a, 0, sizeof(struct bofs_dir_entry));
	memcpy(dir_a, dir_b,sizeof(struct bofs_dir_entry));
	
	/*if we copy dir for a new file, we nee copy inode.
	if we just copy dir for dir info, we don't need inode*/
	if(copy_inode){
		struct bofs_inode inode_a, inode_b;
		/*load inode*/
		bofs_load_inode_by_id(&inode_b, dir_b->inode);
		
		bofs_copy_inode(&inode_a, &inode_b);
		
		/*alloc inode for new one*/
		inode_a.id = bofs_alloc_bitmap(INODE_BITMAP, 1);
		dir_a->inode = inode_a.id;
		bofs_sync_bitmap(INODE_BITMAP, dir_a->inode);
		
		/*sync new inode*/
		bofs_sync_inode(&inode_a);
	}
}

/*
sync a child dir entry to parent dir entry
return 0 is failed, 
return 1 is success with get a not invalid for dir entry, 
return 2 is success with get a invalid for dir entry.
*/
int bofs_sync_dir_entry(struct bofs_dir_entry *parent_dir, struct bofs_dir_entry *child_dir)
{
	/*1.read parent data*/
	struct bofs_inode parent_inode;
	
	bofs_load_inode_by_id(&parent_inode, parent_dir->inode);
	
	/*we need read data in parent inode
	we read a sector once.if not finish, read again.
	*/
	uint32 lba;
	uint32 block_id = 0;
	struct bofs_dir_entry *dir_buf = (struct bofs_dir_entry *)bofs_dir_entry_io_buf;
	int i;
	
	uint32 blocks = DIV_ROUND_UP(parent_inode.size, SECTOR_SIZE);
	/*we check for blocks + 1, we need a new palce to store the dir entry*/
	while(block_id <= blocks){	
		bofs_get_inode_data(&parent_inode, block_id, &lba);
		
		//printk("inode data: id:%d lba:%d\n", block_id, lba);
		
		memset(bofs_dir_entry_io_buf, 0, SECTOR_SIZE);
		sector_read(lba, bofs_dir_entry_io_buf, 1);
		
		/*scan a sector*/
		for(i = 0; i < BOFS_DIR_NR_IN_SECTOR; i++){
			if(!strcmp(dir_buf[i].name, child_dir->name) &&\
				dir_buf[i].file_type != BOFS_FILE_TYPE_INVALID){
				/*same dir entry
				
				change a file info
				
				*/
				memcpy(&dir_buf[i], child_dir, sizeof(struct bofs_dir_entry));
				sector_write(lba, bofs_dir_entry_io_buf, 1);
				//printk("same dir entry\n");
				
				//printk("scan: id:%d lba:%d\n", block_id, lba);
				
				return 1;
				
			}else if(!strcmp(dir_buf[i].name, child_dir->name) &&\
				dir_buf[i].file_type == BOFS_FILE_TYPE_INVALID){
				/*same dir entry
				
				new dir entry pos
				
				*/
				memcpy(&dir_buf[i], child_dir, sizeof(struct bofs_dir_entry));
				sector_write(lba, bofs_dir_entry_io_buf, 1);
				
				/*we will use a dir entry which is invaild!*/
				return 2;
			}else if(dir_buf[i].name[0] != '\0' &&\
				dir_buf[i].inode == child_dir->inode &&\
				child_dir->file_type != BOFS_FILE_TYPE_INVALID){
				/*same dir entry but name different and not invaild
				rename a file
				*/
				memcpy(&dir_buf[i], child_dir, sizeof(struct bofs_dir_entry));
				sector_write(lba, bofs_dir_entry_io_buf, 1);
				//printk("same dir entry but name different\n");
				//printk(">>disk inode:%d child dir inode:%d\n", dir_buf[i].inode, child_dir->inode);
				//printk("scan: id:%d lba:%d\n", block_id, lba);
				
				return 1;
			}else if(dir_buf[i].name[0] == '\0'){
				/*empty dir entry
				new a file
				*/
				memcpy(&dir_buf[i], child_dir, sizeof(struct bofs_dir_entry));
				sector_write(lba, bofs_dir_entry_io_buf, 1);
				//printk("empty dir entry\n");
				
				//printk("scan: id:%d lba:%d\n", block_id, lba);
				return 1;
			}
			
		}
		block_id++;
	}
	return 0;
}

void bofs_release_dir_entry(struct bofs_dir_entry *child_dir)
{
	/*1.read parent data*/
	struct bofs_inode child_inode;
	bofs_load_inode_by_id(&child_inode, child_dir->inode);
	
	/*release child data*/
	bofs_release_inode_data(&child_inode);
	
	/*free inode bitmap*/
	bofs_free_bitmap(INODE_BITMAP, child_dir->inode);
	/*free inode bitmap*/
	bofs_sync_bitmap(INODE_BITMAP, child_dir->inode);
	
	/*we can empty inode for debug
	if we not empty inode , we can recover dir inode info
	*/
	//bofs_empty_inode(&child_inode);
	
	/*
	we do not set inode to 0, so that we can recover 
	dir inode, but we can't recover data.
	*/
	/*child_dir->inode = 0;*/

	/*change file type to INVALID, so that we can search it but not use it*/
	child_dir->file_type = BOFS_FILE_TYPE_INVALID;
	
}

bool bofs_search_dir_entry(struct bofs_dir_entry *parent_dir, struct bofs_dir_entry *child_dir, char *name)
{
	/*1.read parent data*/
	struct bofs_inode parent_inode;
	bofs_load_inode_by_id(&parent_inode, parent_dir->inode);
	
	/*we need read data in parent inode
	we read a sector once.if not finish, read again.
	*/
	uint32 lba;
	uint32 block_id = 0;
	struct bofs_dir_entry *dir_buf = (struct bofs_dir_entry *)bofs_dir_entry_io_buf;
	int i;
	
	uint32 blocks = DIV_ROUND_UP(parent_inode.size, SECTOR_SIZE);
	/*we check for blocks, we need search in old dir entry*/
	while(block_id < blocks){
		bofs_get_inode_data(&parent_inode, block_id, &lba);
		//printk("inode data: id:%d lba:%d\n", block_id, lba);
		
		memset(bofs_dir_entry_io_buf, 0, SECTOR_SIZE);
		sector_read(lba, bofs_dir_entry_io_buf, 1);
		
		/*scan a sector*/
		for(i = 0; i < BOFS_DIR_NR_IN_SECTOR; i++){
			/*this dir has name*/
			if(dir_buf[i].name[0] != '\0'){
				if(!strcmp(dir_buf[i].name, name) && dir_buf[i].file_type != BOFS_FILE_TYPE_INVALID){
					/*same dir entry*/
					memcpy(child_dir, &dir_buf[i], sizeof(struct bofs_dir_entry));
					//printk("search success!\n");
					
					return true;
				}
			}else{
				//printk("search failed!\n");
				return false;
			}
		}
		block_id++;
	}
	
	return false;
}

void bofs_exhibit_dir_entry(struct bofs_dir_entry *dir_entry)
{
	if(dir_entry == NULL){
		return;
	}
	printk("=====dir entry=====\n");
	printk("name:%s file_type:%x inode:%d\n",\
		dir_entry->name, dir_entry->file_type,dir_entry->inode);
}

bool bofs_load_dir_entry(struct bofs_dir_entry *parent_dir, char *name, struct bofs_dir_entry *child_dir)
{
	/*1.read parent data*/
	struct bofs_inode parent_inode;
	
	bofs_load_inode_by_id(&parent_inode, parent_dir->inode);
	
	/*we need read data in parent inode
	we read a sector once.if not finish, read again.
	*/
	uint32 lba;
	uint32 block_id = 0;
	struct bofs_dir_entry *dir_buf = (struct bofs_dir_entry *)bofs_dir_entry_io_buf;
	int i;
	
	uint32 blocks = DIV_ROUND_UP(parent_inode.size, SECTOR_SIZE);
	//printk(">>>load dir entry: inode size:%d blocks:%d\n", parent_inode.size, blocks);
	/*we check for blocks, we need load in old dir entry*/
	while(block_id < blocks){
		bofs_get_inode_data(&parent_inode, block_id, &lba);
		
		//printk("inode data: id:%d lba:%d\n", block_id, lba);
		memset(bofs_dir_entry_io_buf, 0, SECTOR_SIZE);
		sector_read(lba, bofs_dir_entry_io_buf, 1);
		
		/*scan a sector*/
		for(i = 0; i < BOFS_DIR_NR_IN_SECTOR; i++){
			
			if(!strcmp(dir_buf[i].name, name) && dir_buf[i].file_type != BOFS_FILE_TYPE_INVALID){
				//printk(">>load dir entry %s %s\n", dir_buf[i].name, name);
				/*copy data to child dir*/
				memcpy(child_dir, &dir_buf[i], sizeof(struct bofs_dir_entry));
				
				/*change access time*/
				parent_inode.acstime = datetime_to_data();
				bofs_sync_inode(&parent_inode);
				return true;
			}
			
		}
		block_id++;
	}
	return false;
}

#endif


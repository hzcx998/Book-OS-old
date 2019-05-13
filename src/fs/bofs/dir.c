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
#include <sys/mm.h>

#include <string.h>
#include <math.h>

#include <sys/dev.h>
#include <sys/core.h>

/*
get the name from path
*/
int bofs_path_to_name(const char *pathname, char *name_buf)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[BOFS_NAME_LEN];
	int i,j;
	
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	//printk("deep:%d \n",deep);
	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		
		memset(name, 0, BOFS_NAME_LEN);
		
		p++;	//skip '/'
		j = 0;
		//get a dir name
		while(*p != '/' && j < BOFS_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;
			j++;
			p++;
		}
		name[j] = 0;
		//printk("name:%s %d\n",name, i);
		if(name[0] == 0){	//no name
			return -1;
		}
		
		if(i == deep-1){	//name is what we need
			j = 0;
			while(name[j]){	//if not arrive next '/'
				name_buf[j] = name[j];	// transform to A~Z
				j++;
			}
			name_buf[j] = 0;
			return 0;	//success
		}
	}
	return -1;
}


void bofs_exhibit_dir(struct bofs_dir *dir)
{
	if(dir == NULL){
		return;
	}
	printk("=====dir=====\n");
	/*printk("dir_entry:%x block_id:%d dir_pos:%d\n",\
		dir->m_dir_entry, dir->block_id,dir->dir_pos);*/
}

void bofs_scan_dir(const char *pathname)
{
	//open a dir
	struct bofs_dir *dir = bofs_open_dir(pathname);

	bofs_rewind_dir(dir);
	
	struct bofs_dir_entry* dir_entry;

	dir_entry = bofs_read_dir(dir);
	while(dir_entry != NULL){
		//bofs_exhibit_dir_entry(dir_entry);
		dir_entry = bofs_read_dir(dir);
	}
	
	bofs_close_dir(dir);

}

void bofs_list_dir(const char *pathname, int level)
{
	//open a dir
	struct bofs_dir *dir = bofs_open_dir(pathname);

	if(dir == NULL){
		return;
	}
	
	bofs_rewind_dir(dir);
	
	struct bofs_dir_entry* dir_entry;
	char type;
	
	struct bofs_inode inode;
	
	dir_entry = bofs_read_dir(dir);
	while(dir_entry != NULL){
		//bofs_exhibit_dir_entry(dir_entry);
		if(level == 2){
			if(dir_entry->file_type == BOFS_FILE_TYPE_DIRECTORY){
				type = 'd';
			}else if(dir_entry->file_type == BOFS_FILE_TYPE_NORMAL){
				type = 'f';
			}else if(dir_entry->file_type == BOFS_FILE_TYPE_INVALID){
				type = 'i';
			}
			bofs_load_inode_by_id(&inode, dir_entry->inode);
			
			printk("%d/%d/%d ",
				DATA16_TO_DATE_YEA(inode.crttime>>16),
				DATA16_TO_DATE_MON(inode.crttime>>16),
				DATA16_TO_DATE_DAY(inode.crttime>>16));
			printk("%d:%d:%d ",
				DATA16_TO_TIME_HOU(inode.crttime&0xffff),
				DATA16_TO_TIME_MIN(inode.crttime&0xffff),
				DATA16_TO_TIME_SEC(inode.crttime&0xffff));
			printk("%c %d %s \n", type, inode.size, dir_entry->name);
		}else if(level == 1){
			if(dir_entry->file_type != BOFS_FILE_TYPE_INVALID){
				if(dir_entry->file_type == BOFS_FILE_TYPE_DIRECTORY){
					type = 'd';
				}else if(dir_entry->file_type == BOFS_FILE_TYPE_NORMAL){
					type = 'f';
				}
				bofs_load_inode_by_id(&inode, dir_entry->inode);
				
				printk("%d/%d/%d ",
					DATA16_TO_DATE_YEA(inode.crttime>>16),
					DATA16_TO_DATE_MON(inode.crttime>>16),
					DATA16_TO_DATE_DAY(inode.crttime>>16));
				printk("%d:%d:%d ",
					DATA16_TO_TIME_HOU(inode.crttime&0xffff),
					DATA16_TO_TIME_MIN(inode.crttime&0xffff),
					DATA16_TO_TIME_SEC(inode.crttime&0xffff));
				printk("%c %d %s \n", type, inode.size, dir_entry->name);
			}
		}else if(level == 0){
			if(dir_entry->file_type != BOFS_FILE_TYPE_INVALID){
				printk("%s ", dir_entry->name);
			}
		}
		dir_entry = bofs_read_dir(dir);
	}
	
	bofs_close_dir(dir);

}

void bofs_rewind_dir(struct bofs_dir *dir)
{
	dir->dir_pos = 0;
	dir->buf_status = 0;
	dir->block_id = 0;
	memset(dir->dir_buf, 0, BOFS_BLOCK_SIZE);
}

void bofs_close_dir(struct bofs_dir* dir)
{
	if(dir == NULL || dir == bofs_current_super_block->root_dir){	// can't close root dir
		return;
	}
	//free all
	if(dir->m_dir_entry != NULL){
		bofs_close_dir_entry(dir->m_dir_entry);
	}
	mm.free(dir);
}

struct bofs_dir *bofs_open_dir(const char *pathname)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return NULL;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	//if it's root , we return root dir
	if (path[0] == '/' && path[1] == 0) {
		return bofs_current_super_block->root_dir;
	}
	
	struct bofs_dir *dir = NULL;
	
	struct bofs_dir_search_record record;
	int found = bofs_search_dir(path, &record);
	
	if(found == 1){	//fount
		if (record.child_dir->file_type == BOFS_FILE_TYPE_NORMAL) {	///nomal file
			//printk("%s is regular file!\n", path);
			
			bofs_close_dir_entry(record.child_dir);
		} else if (record.child_dir->file_type == BOFS_FILE_TYPE_DIRECTORY) {
			//printk("%s is dir file!\n", path);
			dir = (struct bofs_dir *)mm.malloc(sizeof(struct bofs_dir));
			
			if(dir == NULL){
				bofs_close_dir_entry(record.parent_dir);
				bofs_close_dir_entry(record.child_dir);
				return NULL;
			}
			dir->m_dir_entry = record.child_dir;
			
			dir->block_id = 0;
			dir->dir_pos = 0;
			dir->buf_status = 0;
			memset(dir->dir_buf, 0, BOFS_BLOCK_SIZE);
		}
	}else{
		printk("in path %s not exist\n", path); 
	}
	bofs_close_dir_entry(record.parent_dir);
	
	return dir;
}

struct bofs_dir_entry *bofs_read_dir(struct bofs_dir *dir)
{
	if(dir == NULL){	
		return NULL;
	}
	/*alloc for dir entry*/
	struct bofs_dir_entry *dir_entry = (struct bofs_dir_entry *)mm.malloc(sizeof(struct bofs_dir_entry));
	if(dir_entry == NULL){	
		return NULL;
	}
	/*get a dir entry*/   
	
	struct bofs_inode inode;
	bofs_load_inode_by_id(&inode, dir->m_dir_entry->inode);
	
	uint32 data_blocks = DIV_ROUND_UP(inode.size, SECTOR_SIZE);
	
	uint32 lba;
	
	while(1){
		if(dir->buf_status == 0){
			/*if block id is less than data blocks, we can get data*/
			if(dir->block_id < data_blocks){
				/*get a sector*/
				bofs_get_inode_data(&inode, dir->block_id, &lba);
				//printk("get a sector id:%d lba:%d\n", dir->block_id, lba);
				
				dir->block_id++;
				
				/*read sector*/
				memset(dir->dir_buf, 0, SECTOR_SIZE);
				sector_read(lba, dir->dir_buf, 1);
				
				/*now buf has data*/
				dir->buf_status = 1;
				dir->dir_pos = 0;
			}
		}else{
			//printk("dir buf is full!\n");
		}
		
		/*get a dir entry from buf*/
		memcpy(dir_entry, &dir->dir_buf[dir->dir_pos], sizeof(struct bofs_dir_entry));
		dir->dir_pos += sizeof(struct bofs_dir_entry);
		
		/*if read buf to end, we reset buf*/
		if(dir->dir_pos >= BOFS_BLOCK_SIZE){
			//printk("dir in buf scan over, reset buf.\n");
			dir->dir_pos = 0;
			dir->buf_status = 0;
			memset(dir->dir_buf, 0, BOFS_BLOCK_SIZE);
		}
		
		/*if name is empty, we return NULL, or not we return a dir entry we want.*/
		if(dir_entry->name[0] == '\0'){
			//printk("dir no data left!\n");
			
			mm.free(dir_entry);
			return NULL;
		}else{
			//printk("read name:%s\n", dir_entry->name);
			break;
		}
	}
	return dir_entry;
}

int bofs_search_dir(const char* pathname, struct bofs_dir_search_record *record)
{
	char *p = (char *)pathname;
	char deep = 0;
	char name[BOFS_NAME_LEN];
	int i,j;
	
	/*alloc dir entry space*/
	struct bofs_dir_entry *parent_dir = (struct bofs_dir_entry *)mm.malloc(sizeof(struct bofs_dir_entry));
	struct bofs_dir_entry *child_dir = (struct bofs_dir_entry *)mm.malloc(sizeof(struct bofs_dir_entry));
	
	memset(parent_dir,0, sizeof(struct bofs_dir_entry));
	memset(child_dir,0, sizeof(struct bofs_dir_entry));
	
	if(*p != '/'){	//First must be /
		return 0;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	
	//printk("search ing!\n");
	
	p = (char *)pathname;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'

		memset(name,0,BOFS_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < BOFS_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;	// transform to A~Z
			j++;
			p++;
		}
		
		if(j == 0){	//no name
			printk("no name\n");
			return 0;
		}
		//判断是不是只有根目录
		if(deep == 1 && pathname[0] == '/' && pathname[1] == 0){
			printk("search: pathname is only root dir %s\n", pathname);
			return 1;
		}
		
		if(i == 0){	//root dir 
			//printk("in root dir\n");
			
			//get root dir entry
			if(bofs_search_dir_entry(bofs_current_super_block->root_dir->m_dir_entry, child_dir, name)){	//find
				//printk("search:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//printk("search:child name:%s attr:%x\n",child_dir->name,child_dir->attributes);
				
				if(i == deep - 1){	//finally
					//printk("search:found!\n");
					
					record->parent_dir = bofs_current_super_block->root_dir->m_dir_entry;
					
					record->child_dir = child_dir;
					mm.free(parent_dir);
					return 1;
				}else{
					//printk("search:continue!\n");
					memcpy(parent_dir, child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
				}
				//printk("find\n");
			}else{
				//printk("not found\n");
	
				//printk("search:error!\n");
				record->parent_dir = bofs_current_super_block->root_dir->m_dir_entry;
				record->child_dir = NULL;
				
				mm.free(parent_dir);
				mm.free(child_dir);
				
				return 0;
			}
		}else{	//if not under the root dir 
			
			//parent_dir we have gotten under root dir
			//get root dir entry
			if(bofs_search_dir_entry(parent_dir, child_dir, name)){	//find
				//printk("search:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
				//printk("search:child name:%s attr:%x\n",child_dir->name,child_dir->attributes);
				
				if(i == deep - 1){	//finally
					//printk("search:found!\n");
					
					record->parent_dir = parent_dir;
					record->child_dir = child_dir;
					
					
					return 1;
				}else{
					//printk("search:continue!\n");
					memcpy(parent_dir, child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
				}
			}else{
				record->parent_dir = parent_dir;
				record->child_dir = NULL;	//error
				
				//printk("search:error!\n");
				mm.free(child_dir);
				return 0;
			}
		}
	}
	return 0;
}

void bofs_open_root_dir()
{
	/*read inode*/
	/*bofs_current_super_block->root_inode = (struct bofs_inode *)mm.malloc(sizeof(struct bofs_inode));
	bofs_load_inode_by_id(bofs_current_super_block->root_inode, bofs_current_super_block->root_inode_id);
	printk("root inode: blocks:%d data:%d\n", \
	bofs_current_super_block->root_inode->blocks, bofs_current_super_block->root_inode->block[0]);
	*/
	
	struct bofs_dir_entry *dir_entry;
	
	bofs_current_super_block->root_dir = (struct bofs_dir *)mm.malloc(sizeof(struct bofs_dir));
	
	memset(bofs_io_buf, 0, SECTOR_SIZE);
	sector_read(bofs_current_super_block->data_start_lba, bofs_io_buf, 1);
	
	dir_entry = bofs_current_super_block->root_dir->m_dir_entry;
	
	memcpy(dir_entry, bofs_io_buf, sizeof(struct bofs_dir_entry));
	
	#ifdef _DEBUG_BOFS_
		printk("root dir: name:%s inode:%d type:%d\n", \
		dir_entry->name, dir_entry->inode, dir_entry->file_type);
	#endif
}

/*
C:/a/b
D:/a
E:/a/b/c
*/
int bofs_make_dir(const char *pathname)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	int i, j;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	char *p = (char *)path;
	char deep = 0;
	char name[BOFS_NAME_LEN];
	
	struct bofs_dir_entry parent_dir, child_dir;
	
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	
	p = (char *)path;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'
		
		memset(name,0,BOFS_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < BOFS_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;
			j++;
			p++;
		}
		
		if(name[0] == 0){	//no name
			return -1;
		}
		
		if(i == 0){	//root dir , C:/a
			//get root dir entry
			if(bofs_search_dir_entry(bofs_current_super_block->root_dir->m_dir_entry, &child_dir, name)){	//find, C:/a/b
				memcpy(&parent_dir, &child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
				//printk("bofs_create_dir:deep:%d path:%s name:%s has exsit! find\n",deep, pathname, name);
			}else{
				//printk("bofs_create_dir:deep:%d path:%s name:%s not exsit! not find\n",deep, pathname, name);
				
				/*check whether is last dir name*/
				if(i == deep - 1){
					//printk("bofs_create_dir: create path:%s name:%s\n", pathname, name);
					//create 
					bofs_create_dir_entry(&child_dir, name, \
					BOFS_FILE_TYPE_DIRECTORY<<16|(BOFS_IMODE_R|BOFS_IMODE_W));
					
					struct bofs_inode inode;
					
					/*change child dir inode size*/
					
					bofs_load_inode_by_id(&inode, child_dir.inode);
					
					inode.size += sizeof(struct bofs_dir_entry)*2;
					bofs_sync_inode(&inode);

					//we need create . and .. under child 
					struct bofs_dir_entry dir_0, dir_1;
					
					// . point to child_dir .. point to parent_dir
					//copy data
					bofs_copy_dir_entry(&dir_0, &child_dir, 0);
					//reset name
					memset(dir_0.name, 0,BOFS_NAME_LEN);
					//set name
					strcpy(dir_0.name, ".");
					
					bofs_copy_dir_entry(&dir_1, bofs_current_super_block->root_dir->m_dir_entry, 0);
					memset(dir_1.name,0,BOFS_NAME_LEN);
					strcpy(dir_1.name, "..");
					
					/*sync . and .. to child dir*/
					//printk("sync . dir >>>");
					bofs_sync_dir_entry(&child_dir, &dir_0);
					//printk("sync .. dir>>>");
					bofs_sync_dir_entry(&child_dir, &dir_1);
					//printk("sync child dir>>>");
					
					int sync_ret = bofs_sync_dir_entry(bofs_current_super_block->root_dir->m_dir_entry, &child_dir);
					
					if(sync_ret > 0){	//successed
						if(sync_ret != 2){
							/*change parent dir inode size*/
							bofs_load_inode_by_id(&inode, bofs_current_super_block->root_dir->m_dir_entry->inode);
							
							inode.size += sizeof(struct bofs_dir_entry);
							bofs_sync_inode(&inode);
							
						}
						return 0;
					}
				}else{
					printk("mkdir:can create path:%s name:%s\n", pathname, name);
					
					return -1;
				}
			}
		}else{	//if not under the root dir 
			//parent_dir we have gotten under root dir
			if(bofs_search_dir_entry(&parent_dir, &child_dir, name)){	//find
				memcpy(&parent_dir, &child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
				//printk("bofs_create_dir:deep:%d path:%s name:%s has exsit!\n",deep, pathname, name);
			}else{
				//printk("bofs_create_dir:deep:%d path:%s name:%s not exsit!\n",deep, pathname, name);
				/*check whether is last dir name*/
				if(i == deep - 1){	//create it
					bofs_create_dir_entry(&child_dir, name, \
					BOFS_FILE_TYPE_DIRECTORY<<16|(BOFS_IMODE_R|BOFS_IMODE_W));
					
					//we need create . and .. under child 
					struct bofs_dir_entry dir_0, dir_1;
					
					/*change inode size*/
					struct bofs_inode inode;

					/*change child dir inode size*/
					
					bofs_load_inode_by_id(&inode, child_dir.inode);
					
					inode.size += sizeof(struct bofs_dir_entry)*2;
					bofs_sync_inode(&inode);
					
					// . point to child_dir .. point to parent_dir
					//copy data
					bofs_copy_dir_entry(&dir_0, &child_dir, 0);
					//reset name
					memset(dir_0.name,0,BOFS_NAME_LEN);
					//set name
					strcpy(dir_0.name, ".");

					bofs_copy_dir_entry(&dir_1, &parent_dir, 0);
					memset(dir_1.name,0,BOFS_NAME_LEN);
					strcpy(dir_1.name, "..");
					
					/*sync . and .. to child dir*/
					//printk("sync . dir >>>");
					bofs_sync_dir_entry(&child_dir, &dir_0);
					//printk("sync .. dir>>>");
					bofs_sync_dir_entry(&child_dir, &dir_1);
					//printk("sync child dir>>>");
					
					int sync_ret = bofs_sync_dir_entry(&parent_dir, &child_dir);
					
					if(sync_ret > 0){	//successed
						if(sync_ret != 2){
							/*change parent dir inode size*/
							bofs_load_inode_by_id(&inode, parent_dir.inode);
							
							inode.size += sizeof(struct bofs_dir_entry);
							bofs_sync_inode(&inode);
							
						}
						return 0;
					}
				}else{
					printk("bofs_create_dir:can create path:%s name:%s\n", pathname, name);
					return -1;
				}
			}
		}
	}
	/*if dir has exist, it will arrive here.*/
	return 0;
}

/*
C:/a/b
D:/a
E:/a/b/c
*/

int bofs_remove_dir(const char *pathname)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	int i, j;
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	char *p = (char *)path;
	char deep = 0;
	char name[BOFS_NAME_LEN];
	
	struct bofs_dir_entry parent_dir, child_dir;
	
	if(*p != '/'){	//First must be /
		return -1;
	}
	//Count how many dir 
	while(*p){
		if(*p == '/'){
			deep++;
		}
		p++;
	}
	
	p = (char *)path;
	for(i = 0; i < deep; i++){
		p++;	//skip '/'
		
		memset(name,0,BOFS_NAME_LEN);
		j = 0;
		//get a dir name
		while(*p != '/' && j < BOFS_NAME_LEN){	//if not arrive next '/'
			name[j] = *p;
			j++;
			p++;
		}
		
		if(name[0] == 0){	//no name
			return -1;
		}
		
		if(i == 0){	//root dir , C:/a
			//get root dir entry
			if(bofs_search_dir_entry(bofs_current_super_block->root_dir->m_dir_entry, &child_dir, name)){	//find, C:/a/b
				//printk("bofs_remove_dir:deep:%d path:%s name:%s exsit! find\n",deep, pathname, name);
				
				/*check whether is last dir name*/
				if(i == deep - 1){
					
					/*change parent dir inode size*/
					/*struct bofs_inode inode;
					
					bofs_load_inode_by_id(&inode, bofs_current_super_block->root_dir->inode);
					
					inode.size -= sizeof(struct bofs_dir_entry);
					bofs_sync_inode(&inode);
					*/
					bofs_release_dir_entry(&child_dir);
					
					if(bofs_sync_dir_entry(bofs_current_super_block->root_dir->m_dir_entry, &child_dir)){
						return 0;
					}
				}else{
					memcpy(&parent_dir, &child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
					//printk("bofs_remove_dir:deep:%d path:%s name:%s has exsit! find\n",deep, pathname, name);
				}
			}else{
				printk("bofs_remove_dir:deep:%d path:%s name:%s not exsit! not find\n",deep, pathname, name);
				
			}
		}else{	//if not under the root dir 
			//parent_dir we have gotten under root dir
			if(bofs_search_dir_entry(&parent_dir, &child_dir, name)){	//find
				//printk("bofs_remove_dir:deep:%d path:%s name:%s exsit! find\n",deep, pathname, name);
				
				/*check whether is last dir name*/
				if(i == deep - 1){
					
					/*change parent dir inode size*/
					/*
					struct bofs_inode inode;
					
					bofs_load_inode_by_id(&inode, parent_dir.inode);
					
					inode.size -= sizeof(struct bofs_dir_entry);
					bofs_sync_inode(&inode);
					*/
					
					bofs_release_dir_entry(&child_dir);
					
					if(bofs_sync_dir_entry(&parent_dir, &child_dir)){
						return 0;
					}
				}else{
					memcpy(&parent_dir, &child_dir, sizeof(struct bofs_dir_entry));	//child_dir become parent_dir
					//printk("bofs_remove_dir:deep:%d path:%s name:%s has exsit! find\n",deep, pathname, name);
				}
			}else{
				printk("bofs_remove_dir:deep:%d path:%s name:%s not exsit! not find\n",deep, pathname, name);
				
			}
		}
	}
	/*if dir has not exist, it will arrive here.*/
	return -1;
}
/*
change current work dir
@return: success(0), failed(-1)
*/
int bofs_chdir(const char *pathname)
{
	struct thread *cur = thread_current();
	int ret = -1;
	
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	/*root dir*/
	if(path[0] == '/' && path[1] == 0){
		memset(cur->cwd, 0, BOFS_PATH_LEN);
		strcpy(cur->cwd, pathname);
		ret = 0;
	}else{
		/*search dir*/
		struct bofs_dir_search_record record;  
		memset(&record, 0, sizeof(struct bofs_dir_search_record));
		int found = bofs_search_dir(path, &record);
		//printk("bofs_chdir: %s is found!\n", path);
		if (found == 1) {
			//printk("bofs_chdir: %s is found!\n", path);
			if(record.child_dir->file_type == BOFS_FILE_TYPE_DIRECTORY){
				
				memset(cur->cwd, 0, BOFS_PATH_LEN);
				strcpy(cur->cwd, pathname);
				//printk("bofs_chdir: thread pwd %s \n", cur->cwd);
				ret = 0;
			}
			bofs_close_dir_entry(record.child_dir);
		}else {	
			printk("bofs chdir: %s isn't exist!\n", pathname);
		}
		bofs_close_dir_entry(record.parent_dir);
	}
	return ret;
}

int bofs_getcwd(char* buf, uint32 size)
{
	struct thread *cur = thread_current();
	memcpy(buf, cur->cwd, size);
	return 0;
}

int bofs_reset_name(const char *pathname, char *name)
{
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	
	char *path = (char *)(pathname+2);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		printk("rename: can't reset '/' name!\n");
		return -1;
	}
	
	char new_path[BOFS_PATH_LEN] = {0};
	memset(new_path, 0, BOFS_PATH_LEN);
	strcpy(new_path, pathname);
	
	int i;
	/*从后往前寻找'/'*/
	for(i = strlen(new_path)-1; i >= 0; i--){
		if(new_path[i] == '/'){
			break;
		}
	}
	i++; /*跳过'/'*/
	while(i < BOFS_PATH_LEN){
		new_path[i] = 0;
		i++;
	}
	
	strcat(new_path, name);
	
	printk("new path :%s\n", new_path);
	/*check the name after rename*/
	if(!bofs_is_path_exist(new_path)){
		printk("rename: the path name:%s has exist!\n", new_path);
		return -1;
	}
	
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		//printk("rename: find dir entry %s.\n", record.child_dir->name);
		/*reset name*/
		memset(record.child_dir->name, 0, BOFS_NAME_LEN);
		strcpy(record.child_dir->name, name);
		/*sync dir entry*/
		if(bofs_sync_dir_entry(record.parent_dir, record.child_dir)){
			ret = 0;
		}
		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
	return ret;
}

#endif


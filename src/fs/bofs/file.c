/*
File:		fs/bofs/file.c
Contains:	bofs's file
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

#include <sys/core.h>

struct bofs_file_descriptor bofs_fd_table[BOFS_MAX_FD_NR];

void bofs_init_fd_table()
{
	int fd_idx = 0;
	while (fd_idx < BOFS_MAX_FD_NR) {
		
		bofs_fd_table[fd_idx].fd_status = BOFS_FD_FREE;
		bofs_fd_table[fd_idx].fd_dir = NULL;
		bofs_fd_table[fd_idx].fd_parent = NULL;
		bofs_fd_table[fd_idx].fd_inode = NULL;
		
		fd_idx++;
	}
}

int bofs_alloc_fd_global()
{
	uint32 fd_idx = 0;
	while (fd_idx < BOFS_MAX_FD_NR) {
		if (bofs_fd_table[fd_idx].fd_status == BOFS_FD_FREE) {
			bofs_fd_table[fd_idx].fd_status = BOFS_FD_USING;
			return fd_idx;
		}
		fd_idx++;
	}
	
	printk("alloc fd over max open files!\n");
	return -1;
}

struct bofs_file_descriptor *bofs_get_file_by_fd(int fd)
{
	if(fd < 0 || fd >= BOFS_MAX_FD_NR) {
		printk("fd error\n");
		return NULL;
	}
	return &bofs_fd_table[fd];
}


int bofs_create_file(struct bofs_dir_entry *parent_dir, char *name, uint16 mode)
{
	/*1.create a dir entry*/
	
	struct bofs_dir_entry *dir_entry = (struct bofs_dir_entry *)mm.malloc(sizeof(struct bofs_dir_entry));
	if(dir_entry == NULL){
		printk("alloc memory for dir_entry failed!\n");
		return -1;
	}
	
	struct bofs_inode *inode = (struct bofs_inode *)mm.malloc(sizeof(struct bofs_inode));
	if(inode == NULL){
		printk("alloc memory for inode failed!\n");
		mm.free(dir_entry);
		return -1;
	}

	bofs_create_dir_entry(dir_entry, name, (BOFS_FILE_TYPE_NORMAL<<16)|mode);
	
	/*2.sync dir entry to parent dir*/
	int sync_ret = bofs_sync_dir_entry(parent_dir, dir_entry);
	
	if(sync_ret > 0){
		if(sync_ret != 2){
			/*3.change parent dir size*/
			struct bofs_inode parent_inode;
			/*change parent dir parent inode size*/
			bofs_load_inode_by_id(&parent_inode, parent_dir->inode);
			
			parent_inode.size += sizeof(struct bofs_dir_entry);
			bofs_sync_inode(&parent_inode);
		}
		
		/*4.open file*/
		int fd_idx = bofs_alloc_fd_global();
		if (fd_idx == -1) {
			printk("alloc fd for failed!\n");
		
			mm.free(dir_entry);
			mm.free(inode);
			return -1;
		}
		
		bofs_fd_table[fd_idx].fd_parent = parent_dir;
		bofs_fd_table[fd_idx].fd_dir = dir_entry;
		
		bofs_load_inode_by_id(inode, dir_entry->inode);
		
		bofs_fd_table[fd_idx].fd_inode = inode;
		
		bofs_fd_table[fd_idx].fd_pos = 0;
		
		bofs_fd_table[fd_idx].fd_flags = mode;
		//we can't free dir_entry and inode, because we will use it in fd
		
		//bofs_exhibit_fd(fd_idx);
		return fd_idx;
		
	}else{
		mm.free(dir_entry);
		mm.free(inode);
		return -1;
	}
	
}

int bofs_open_file(struct bofs_dir_entry *parent_dir, char *name, uint16 mode)
{
	/*1.alloc memory for dir entry and inode*/
	struct bofs_dir_entry *dir_entry = (struct bofs_dir_entry *)mm.malloc(sizeof(struct bofs_dir_entry));
	if(dir_entry == NULL){
		printk("alloc memory for dir_entry failed!\n");
		return -1;
	}
	
	struct bofs_inode *inode = (struct bofs_inode *)mm.malloc(sizeof(struct bofs_inode));
	if(inode == NULL){
		printk("alloc memory for inode failed!\n");
		mm.free(dir_entry);
		return -1;
	}
	
	/*2.load dir entry to parent dir*/
	if(bofs_load_dir_entry(parent_dir, name, dir_entry)){
		
		/*3.open file*/
		int fd_idx = bofs_alloc_fd_global();
		if (fd_idx == -1) {
			printk("alloc fd for failed!\n");
		
			mm.free(dir_entry);
			mm.free(inode);
			return -1;
		}
		
		bofs_fd_table[fd_idx].fd_parent = parent_dir;
		bofs_fd_table[fd_idx].fd_dir = dir_entry;
		
		bofs_load_inode_by_id(inode, dir_entry->inode);
		
		bofs_fd_table[fd_idx].fd_inode = inode;
		
		bofs_fd_table[fd_idx].fd_pos = 0;
		
		bofs_fd_table[fd_idx].fd_flags = mode;
		//we can't free dir_entry and inode, because we will use it in fd
		//bofs_exhibit_fd(fd_idx);
		return fd_idx;
		
	}else{
		printk("load dir entry failed!\n");
		mm.free(dir_entry);
		mm.free(inode);
		return -1;
	}
	
}

int bofs_close_file(struct bofs_file_descriptor *fd)
{
	if(fd == NULL) {
		return -1;
	}
	
	fd->fd_pos = 0;
	fd->fd_flags = 0;
	/*close parent and child dir*/
	bofs_close_dir_entry(fd->fd_dir);
	bofs_close_dir_entry(fd->fd_parent);
	/*close inode*/
	bofs_close_inode(fd->fd_inode);
	
	/*set to NULL*/
	fd->fd_dir = NULL;  
	fd->fd_parent = NULL;  
	fd->fd_inode = NULL;  
	
	return 0;
}

int bofs_open(const char *pathname, uint32 flags)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	struct bofs_dir_search_record record;
	
	int fd = -1;	   //default open failed
	
	char name[BOFS_NAME_LEN];
	memset(name,0,BOFS_NAME_LEN);
	if(!bofs_path_to_name(path, name)){
		//printk("path to name sucess!\n");
		
	}else{
		//printk("path to name failed!\n");
		return -1;
	}
	
	/*
	if file has exist, we will reopen it(reopen = 1).
	if file not exist, we have falgs O_CREAT, we create a file
	*/
	uint8 reopen = 0;
	
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		//printk("find dir entry!\n");
		
		if(record.child_dir->file_type == BOFS_FILE_TYPE_DIRECTORY){	//found a dir
			printk("fopen: can't open a direcotry with fopen(), use opendir() to instead!\n");
			bofs_close_dir_entry(record.parent_dir);
			bofs_close_dir_entry(record.child_dir);
			
			return -1;
		}
		if(record.child_dir->file_type == BOFS_FILE_TYPE_INVALID){	//found a dir
			printk("fopen: can't open a invalid file or direcotry!\n");
			bofs_close_dir_entry(record.parent_dir);
			bofs_close_dir_entry(record.child_dir);
			return -1;
		}
		if ((flags & BOFS_O_CREAT)) {
			printk("fopen: %s has already exist!\n", pathname);
			if((flags & BOFS_O_RDONLY) || (flags & BOFS_O_WRONLY) || (flags & BOFS_O_RDWR)){
				reopen = 1;
				printk("%s %s can be reopen.\n", pathname, name);
			}else{
				
				bofs_close_dir_entry(record.parent_dir);
				bofs_close_dir_entry(record.child_dir);
				printk("open: file %s has exist, can't create it!\n", pathname);
				return -1;
			}
		}
	}else{
		if (!found && !(flags & BOFS_O_CREAT)) {	//not found ,not create
			printk("fopen: path %s, file isn't exist and without O_CR!\n", pathname);
			bofs_close_dir_entry(record.parent_dir);
			bofs_close_dir_entry(record.child_dir);
			
			return -1;
		}
	}
	
	/*make file mode*/
	uint32 mode = BOFS_FILE_TYPE_NORMAL<<16;
	/*read and write set*/
	if(flags & BOFS_O_RDONLY){
		mode |= BOFS_IMODE_R;
	}else if(flags & BOFS_O_WRONLY){
		mode |= BOFS_IMODE_W;
	}else if(flags & BOFS_O_RDWR){
		mode |= BOFS_IMODE_R;
		mode |= BOFS_IMODE_W;
	}
	/*execute flags*/
	if(flags & BOFS_O_EXEC){
		mode |= BOFS_IMODE_X;
	}
	
	uint8 append = 0;
	if(flags & BOFS_O_APPEDN){
		append = 1;
	}
	
	//printk("file mode:%x\n", mode);
	if(flags & BOFS_O_CREAT) {	
		if(!reopen){
			fd = bofs_create_file(record.parent_dir, name, mode);
			
			if(fd != -1){
				/*create sucess! we can't close record parent dir, we will use in fd_parent*/
				
				/*if need append, we set it here*/
				if(append){
					bofs_lseek(fd, 0, BOFS_SEEK_END);
				}
				
				/*set disk drive*/
				bofs_fd_table[fd].fd_drive = drive;
				
				/*close record child dir, we don't use it
				fd_dir was alloced in bofs_create_file
				*/
				
				//printk("create path %s file %s sucess!\n", pathname, name);
				bofs_close_dir_entry(record.child_dir);
			}else{	
				/*create file failed, we close record dir*/
				printk("create path %s file %s failed!\n", pathname, name);
				bofs_close_dir_entry(record.parent_dir);
				bofs_close_dir_entry(record.child_dir);
			}
			return fd;
		}
	}
	
	if((flags & BOFS_O_RDONLY) || (flags & BOFS_O_WRONLY) || (flags & BOFS_O_RDWR)){
		/*open an exsit file with O_RO,O_WO,O_RW*/
		fd = bofs_open_file(record.parent_dir, name, mode);
		if(fd != -1){
			/*open sucess! we can't close record parent dir, we will use it in fd_parent*/
			/*if need append, we set it here*/
			if(append){
				bofs_lseek(fd, 0, BOFS_SEEK_END);
			}
			/*set disk drive*/
			bofs_fd_table[fd].fd_drive = drive;
			
			/*close record child dir, we don't use it
			fd_dir was alloced in bofs_create_file
			*/
			//printk("open path %s file %s sucess!\n", pathname, name);
			bofs_close_dir_entry(record.child_dir);
		}else{
			/*open file failed, we close record dir*/
			printk("open path %s file %s failed!\n", pathname, name);
			bofs_close_dir_entry(record.parent_dir);
			bofs_close_dir_entry(record.child_dir);
		}
	}
	return fd;
}

int bofs_close(int fd)
{
	int ret = -1;   // defaut -1,error
	if (fd >= 0 && fd < BOFS_MAX_FD_NR) {
		ret = bofs_close_file(&bofs_fd_table[fd]);
		bofs_fd_table[fd].fd_status = BOFS_FD_FREE;
		//printk("close fd:%d success!\n", fd);
	}else{
		printk("close fd:%d failed!\n", fd);
	}
	return ret;
}

int bofs_unlink(const char *pathname)
{
	/*judge witch drive we will operate*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	if(path[0] == '/' && path[1] == 0){
		printk("unlink: can't delelt / dir!\n");
		return -1;
	}
	
	struct bofs_dir_search_record record;
	
	//1.is file exist?
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		//printk("find dir entry!\n");
		if(record.child_dir->file_type == BOFS_FILE_TYPE_DIRECTORY){	//found a dir
			printk("unlink: can't delete a direcotry with unlink(), use rmdir() to instead!\n");
			bofs_close_dir_entry(record.parent_dir);
			bofs_close_dir_entry(record.child_dir);
			
			return -1;
		}
	}else{
		printk("unlink: file %s not found!\n", pathname);
		bofs_close_dir_entry(record.parent_dir);
		bofs_close_dir_entry(record.child_dir);
		return -1;
	}
	
	//2.file is in file table?
	uint32 fd_idx = 0;
	while (fd_idx < BOFS_MAX_FD_NR) {
		//if name is same and inode same, that the file we want
		if (bofs_fd_table[fd_idx].fd_status == BOFS_FD_USING){
			//printk("scan fd %d name:%s \n", fd_idx, bofs_fd_table[fd_idx].fd_dir->name);
			if(record.child_dir->inode == bofs_fd_table[fd_idx].fd_dir->inode){
				//printk("find a open file with fd %d !\n", fd_idx);
				break;
			}
		}
		fd_idx++;
	}
	if (fd_idx < BOFS_MAX_FD_NR) {
		bofs_close_dir_entry(record.parent_dir);
		bofs_close_dir_entry(record.child_dir);
		printk("unlink: file %s is in use, not allow to delete!\n", pathname);
		return -1;
	}
	/*3.empty file data*/
	bofs_release_dir_entry(record.child_dir);
	
	/*4.sync child dir to parent dir*/
	if(bofs_sync_dir_entry(record.parent_dir, record.child_dir)){
		printk("unlink: delete file %s done.\n",pathname);
	
		bofs_close_dir_entry(record.parent_dir);
		bofs_close_dir_entry(record.child_dir);
		return 0;
	}
	bofs_close_dir_entry(record.parent_dir);
	bofs_close_dir_entry(record.child_dir);
	printk("unlink: delete file %s faild!\n",pathname);
	
	return -1;
}

void bofs_exhibit_fd(int fd)
{
	if(fd < 0 || fd >= BOFS_MAX_FD_NR){
		return;
	}
	struct bofs_file_descriptor *pfd = &bofs_fd_table[fd];
	printk("=====file descriptor=====\n");
	printk("fd:%d fd_pos:%d fd_flags:%d\n", fd, pfd->fd_pos, pfd->fd_flags);
	if(pfd->fd_dir != NULL){
		printk("dir name:%s inode:%d\n", pfd->fd_dir->name, pfd->fd_dir->inode);
	}
	if(pfd->fd_parent != NULL){
		printk("parent dir name:%s inode:%d\n", pfd->fd_parent->name, pfd->fd_parent->inode);
	}
}

int bofs_write(int fd, void* buf, uint32 count)
{
	if (fd < 0 || fd >= BOFS_MAX_FD_NR) {
		printk("bofs fwrite: fd error\n");
		return -1;
	}
	if(count == 0) {
		printk("bofs fwrite: count zero\n");
		return 0;
	}
	
    struct bofs_file_descriptor* wr_file = &bofs_fd_table[fd];
	
	/*set disk drive*/
	if(bofs_select_drive(wr_file->fd_drive)){
		return -1;
	}
	/*we need compare inode mode with flags*/
	/*flags have write*/
    if(wr_file->fd_flags & BOFS_O_WRONLY || wr_file->fd_flags & BOFS_O_RDWR){
		/*inode mode can write*/
		if(wr_file->fd_inode->mode & BOFS_IMODE_W){
			int bytes_written  = bofs_file_write(wr_file, buf, count);
			return bytes_written;
		}else{
			printk("not allowed to write inode without BOFS_IMODE_W.\n");
		}
	} else {
		printk("bofs fwrite: not allowed to write file without flag BOFS_O_RDWR or BOFS_O_WRONLY\n");
	}
	return -1;
}

int bofs_file_write( struct bofs_file_descriptor *fd, void* buf, uint32 count)
{
	/*step 1:
	if pos > file size, we should correcte it*/
	if(fd->fd_pos > fd->fd_inode->size){
		fd->fd_pos = fd->fd_inode->size;
	}
	
	/*step 2:
	we need to get file's data old blocks, 
	after that we can calculate data new blocks by pos and count.
	*/
	//get old blocks
	//uint32 all_blocks_old = DIV_ROUND_UP(fd->fd_inode->size, SECTOR_SIZE);
	//printk("all_blocks_old:%d\n", all_blocks_old);
	//we will beyond how many bytes, we need add how many bytes beyond file size
	/*uint32 will_beyond_bytes;	
	if(fd->fd_pos < fd->fd_inode->size){//pos is in file size
		if((fd->fd_pos + count) < fd->fd_inode->size){	
			will_beyond_bytes = 0;
			//printk("below pos start:%d pos end:%d cunt:%d\n",fd->fd_pos, fd->fd_pos + count, count);
		}else{	//end >= size
			will_beyond_bytes = count - (fd->fd_inode->size - fd->fd_pos);
			//printk("above pos start:%d pos end:%d cunt:%d\n",fd->fd_pos, fd->fd_pos + count, count);
		}
	}else{	//pos is equal to size
		will_beyond_bytes = count;
	}*/
	/*
	uint32 will_beyond_sectors = 0;	//default is 0 sectors
	if(will_beyond_bytes > 0){//if have beyond bytes
		will_beyond_sectors = DIV_ROUND_UP(will_beyond_bytes, SECTOR_SIZE);
	}*/
	
	//uint32 all_blocks_new = all_blocks_old + will_beyond_sectors;
	
	//printk("will_beyond_bytes:%x will_beyond_sectors:%d\n",will_beyond_bytes, will_beyond_sectors);

	/*step 3:
	judge whether need we change file's size. if pos+count > size, change size!
	*/
	uint8 change_file_size = 0;
	if(fd->fd_pos + count > fd->fd_inode->size){
		change_file_size = 1;
	}
	
	/*step 4:
	write data to file
	*/
	uint8 *src = buf;        // src is buffer we will write to file 
	uint32 bytes_written = 0;	    // we have written n bytes
	uint32 size_left = count;	    // how many bytes we didn't write
	uint32 sec_lba;	      // we will write data to a sector 
	uint32 sec_off_bytes;    // bytes offset in a sector 
	uint32 sec_left_bytes;   // bytes left in a sector
	uint32 chunk_size;	      // every time will write chunk size to disk
	
	/*we will write data to a sector, but not every time we write is a whole sector.
	maybe it is like this. a sector, we will write to half of it. so we will clear a
	sector buf first, and then write data into sector buf, and write into disk.
	if we don't judge whether it's first time to write sector, we may clean old data
	in the disk. so we judge it use value first_write_sector.*/
	uint8 first_write_sector = 1;
	
	/*block id for get file data block.
	we need set it by fd pos, we will start at a pos, not always 0.
	because sometimes, pos is not start from 0, so we should set it use 
	pos/SECTOR_SIZE.
	for example:
		if (0<= pos < 512) : id = 0
		if (512<= pos < 1024) : id = 1
		if (1024<= pos < 1536) : id = 2
		...
	*/
	uint32 block_id = fd->fd_pos/SECTOR_SIZE;    
	
	//printk(">>>start block id:%d\n", block_id);
	
	while (bytes_written < count) {
		memset(bofs_file_io_buf, 0, SECTOR_SIZE);
		
		bofs_get_inode_data(fd->fd_inode, block_id, &sec_lba);
		
		//get remainder of pos = pos/512
		sec_off_bytes = fd->fd_pos % SECTOR_SIZE;	
		sec_left_bytes = SECTOR_SIZE - sec_off_bytes;
		
		chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
		
		//printk("sector:%d off:%d left:%d chunk:%d\n", sec_lba, sec_off_bytes, sec_left_bytes, chunk_size);
		
		if (first_write_sector) {	//we need to keep first sector not
			sector_read(sec_lba, bofs_file_io_buf, 1);
			first_write_sector = 0;
			//printk("first write, need to read old data!\n");
		}
		memcpy(bofs_file_io_buf + sec_off_bytes, src, chunk_size);
		sector_write(sec_lba, bofs_file_io_buf, 1);

		src += chunk_size;   // set src to next pos
		fd->fd_pos += chunk_size;   
		if(change_file_size){
			fd->fd_inode->size  = fd->fd_pos;    // updata file's size
		}
		bytes_written += chunk_size;
		size_left -= chunk_size;
		block_id++;
	}
	/*step 5:
	updata inode size
	*/
	bofs_sync_inode(fd->fd_inode);
	
	return bytes_written;
}

int bofs_read(int fd, void* buf, uint32 count)
{
	if (fd < 0 || fd >= BOFS_MAX_FD_NR) {
		printk("bofs fread: fd error\n");
		return -1;
	}
	if (count == 0) {
		printk("fread: count zero\n");
		return 0;
	}
	
	struct bofs_file_descriptor* rd_file = &bofs_fd_table[fd];
	
	/*set disk drive*/
	if(bofs_select_drive(rd_file->fd_drive)){
		return -1;
	}
	/*we need compare inode mode with flags*/
	/*flags have read*/
    if(rd_file->fd_flags & BOFS_O_RDONLY || rd_file->fd_flags & BOFS_O_RDWR){
		/*inode mode can read*/
		if(rd_file->fd_inode->mode & BOFS_IMODE_R){
			uint32 bytes_read  = bofs_file_read(rd_file, buf, count);
			return bytes_read;
		}else{
			printk("not allowed to read inode without BOFS_IMODE_R.\n");
		}
	} else {
		printk("fread: not allowed to read file without flag BOFS_O_RDONLY or BOFS_O_RDWR.\n");
	}
	return -1;
}

int bofs_file_read( struct bofs_file_descriptor *fd, void* buf, uint32 count)
{
	/*step 1:
	calculate that we can read how many bytes
	*/
	uint32 size = count, size_left = count;
	//printk("pos:%d count:%d size:%d\n", fd->fd_pos, count, fd->fd_inode->size);

	//check read bytes
	if ((fd->fd_pos + count) > fd->fd_inode->size){
		size = fd->fd_inode->size - fd->fd_pos;
		size_left = size;
		//printk("size_left:%d\n", size_left);
		
		if (size == 0) {	   // if read at the end of file, return 0
			return -1;
		}
	}
	
	/*step 2:
	calculate that we read data from which block id(sector)
	*/
	/*block id for get file data block.
	we need set it by fd pos, we will start at a pos, not always 0.
	because sometimes, pos is not start from 0, so we should set it use 
	pos/SECTOR_SIZE.
	for example:
		if (0<= pos < 512) : id = 0
		if (512<= pos < 1024) : id = 1
		if (1024<= pos < 1536) : id = 2
		...
	*/
	uint32 block_id = fd->fd_pos/SECTOR_SIZE;    
	//printk(">>>start block id:%d\n", block_id);
	
	/*step 3:
	read data to buf
	*/
	uint8 *dst = buf;        // src is buffer we will write to file 
	uint32 bytes_read = 0;	    // we have written n bytes
	uint32 sec_lba;	      // we will write data to a sector 
	uint32 sec_off_bytes;    // bytes offset in a sector 
	uint32 sec_left_bytes;   // bytes left in a sector
	uint32 chunk_size;	      // every time will write chunk size to disk
	
	while (bytes_read < size) {
		bofs_get_inode_data(fd->fd_inode, block_id, &sec_lba);
		
		//get remainder of pos = pos/512
		sec_off_bytes = fd->fd_pos % SECTOR_SIZE;	
		sec_left_bytes = SECTOR_SIZE - sec_off_bytes;
		
		chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;
		
		//printk("sector:%d off:%d left:%d chunk:%d\n", sec_lba, sec_off_bytes, sec_left_bytes, chunk_size);
		
		//clean buf and read data to buf
		memset(bofs_file_io_buf, 0, SECTOR_SIZE);
		sector_read(sec_lba, bofs_file_io_buf, 1);
		/*copy disk data to out buf*/
		memcpy(dst, bofs_file_io_buf + sec_off_bytes, chunk_size);
		
		dst += chunk_size;
		fd->fd_pos += chunk_size;
		bytes_read += chunk_size;
		size_left -= chunk_size;
		block_id++;
	}
	
	return bytes_read;
}

int bofs_lseek(int fd, int offset, uint8 whence)
{
	if (fd < 0 || fd >= BOFS_MAX_FD_NR) {
		printk("bofs lseek: fd error\n");
		return -1;
	}

	struct bofs_file_descriptor* pf = &bofs_fd_table[fd];
	
	//printk("seek file %s\n",pf->fd_dir->name);
	int new_pos = 0;   //new pos must < file size
	int file_size = (int)pf->fd_inode->size;
	
	switch (whence) {
		case BOFS_SEEK_SET: 
			new_pos = offset; 
			break;
		case BOFS_SEEK_CUR: 
			new_pos = (int)pf->fd_pos + offset; 
			break;
		case BOFS_SEEK_END: 
			new_pos = file_size + offset;
			break;
		default :
			printk("bofs lseek: unknown whence!\n");
	
			break;
	}
	
	if (new_pos < 0 || new_pos > file_size) {	 
		return -1;
	}
	
	pf->fd_pos = new_pos;
	return pf->fd_pos;
}

int bofs_stat(const char *pathname, struct bofs_stat *buf)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	struct bofs_inode inode;
	
	if(!strcmp(path, "/")){
		buf->st_type = BOFS_FILE_TYPE_DIRECTORY;
		
		bofs_load_inode_by_id(&inode, bofs_current_super_block->root_dir->m_dir_entry->inode);
		
		buf->st_size = inode.size;
		buf->st_mode = inode.mode;
		return 0;
	}
	
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));
	int found = bofs_search_dir(path, &record);
	if (found == 1) {
		if (record.child_dir->file_type == BOFS_FILE_TYPE_DIRECTORY){
			//printk("dir file!\n");
			buf->st_type = BOFS_FILE_TYPE_DIRECTORY;
		}else if (record.child_dir->file_type == BOFS_FILE_TYPE_NORMAL){
			//printk("normal file!\n");
			buf->st_type = BOFS_FILE_TYPE_NORMAL;
		}
		
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		
		buf->st_size = inode.size;
		buf->st_mode = inode.mode;
		
		bofs_close_dir_entry(record.child_dir);
		ret = 0;
	} else {
		printk("stat: %s not found\n", path);
		
	}
	bofs_close_dir_entry(record.parent_dir);
	
	return ret;
}

int bofs_is_path_exist(const char* pathname)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = -1;	//default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		bofs_close_dir_entry(record.child_dir);
		ret = 0;
	}else{
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
	return ret;
}

int bofs_is_path_executable(const char* pathname)
{
   /*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		struct bofs_inode inode;
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		
		if(inode.mode&BOFS_IMODE_X){
			ret = 0;
		}

		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
   return ret;
}

int bofs_is_path_readable(const char* pathname)
{
   /*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		struct bofs_inode inode;
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		
		if(inode.mode&BOFS_IMODE_R){
			ret = 0;
		}
		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
   return ret;
}

int bofs_is_path_writable(const char* pathname)
{
   /*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		struct bofs_inode inode;
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		
		if(inode.mode&BOFS_IMODE_W){
			ret = 0;
		}

		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
   return ret;
}

int bofs_access(const char *pathname, int mode)
{
	int ret = -1;
	switch(mode){
		case BOFS_F_OK:
			if(!bofs_is_path_exist(pathname)){
				ret = 0;
			}
			break;
		case BOFS_X_OK:
			if(!bofs_is_path_executable(pathname)){
				ret = 0;
			}
			break;
		case BOFS_W_OK:
			if(!bofs_is_path_writable(pathname)){
				ret = 0;
			}
			break;
		case BOFS_R_OK:
			if(!bofs_is_path_readable(pathname)){
				ret = 0;
			}
			break;
		default:
			printk("access mode unknown!\n");
			break;
	}
	return ret;
}
/*
set new mode.
NOTE! if file is opening in fd, we need load mode to fd
*/
int bofs_set_mode(const char* pathname, int mode)
{
   /*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = -1;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		struct bofs_inode inode;
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		/*set mode and sync to disk*/
		inode.mode = mode;
		bofs_sync_inode(&inode);
		
		/*check whether in file descriptor talbe*/
		int fd_idx;
		for(fd_idx = 0; fd_idx < BOFS_MAX_FD_NR; fd_idx++){
			if (bofs_fd_table[fd_idx].fd_status == BOFS_FD_USING) {
				bofs_fd_table[fd_idx].fd_inode->mode = mode;
				break;
			}
		}
		
		ret = 0;
		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
	return ret;
}

int bofs_get_mode(const char* pathname)
{
	/*judge witch drive we will create dir on*/
	char drive;
	drive = *pathname;
	
	if(bofs_select_drive(drive)){
		return -1;
	}
	char *path = (char *)(pathname+2);
	//printk("The path is %s\n", pathname);
	
	/*if dir is '/' */
	if(!strcmp(path, "/")){
		return 0;
	}
	int ret = 0;	// default -1
	struct bofs_dir_search_record record;
	memset(&record, 0, sizeof(struct bofs_dir_search_record));   // 记得初始化或清0,否则栈中信息不知道是什么
	int found = bofs_search_dir(path, &record);
	if(found == 1){
		struct bofs_inode inode;
		bofs_load_inode_by_id(&inode, record.child_dir->inode);
		/*get mode*/
		ret = inode.mode;
		bofs_close_dir_entry(record.child_dir);
	} else {
		printk("%s not exist!\n", pathname);
	}
	bofs_close_dir_entry(record.parent_dir);
	return ret;
}

/*
@Name:		bofs_copy_file

@Param:		src_pathname:the source path
			dst_pathname:the destnation path

@Function: 	copy a file from src to dst

@Return: 	sucess(0), faild(-1)

@Time: 		2019/4/1

@Author: 	Hu Zicheng
*/
int bofs_copy_file(const char *src_pathname, const char *dst_pathname)
{
	
	//printk("copy file: src %s,dst %s\n", src_pathname, dst_pathname);

	/*1.make dst pathname by dst path and src file*/
	/*get file name*/
	char name[BOFS_NAME_LEN];
	memset(name, 0, BOFS_NAME_LEN);
	bofs_path_to_name(src_pathname+2, name);
	
	//printk("copy file: file name:%s\n", name);
	/*make pathname*/
	char dst_path[BOFS_PATH_LEN];
	memset(dst_path, 0, BOFS_PATH_LEN);
	strcpy(dst_path, dst_pathname);
	
	/*if not '/', we will cat a '/'*/
	if(dst_path[strlen(dst_pathname)-1] != '/'){
		strcat(dst_path, "/");
	}
	/*cat file name to path to make pathname*/
	strcat(dst_path, name);
	/*if name are same, we return*/
	if(!strcmp(dst_path, src_pathname)){
		return -1;
	}
	
	/*2.open a file as read file*/
	int fd_rd = bofs_open(src_pathname, BOFS_O_CREAT|BOFS_O_RDWR);
	if(fd_rd == -1){
		return -1;
	}
	//printk("copy file: dst pathname:%s\n", dst_path);
	/*3.open a file as write file*/
	int fd_wr = bofs_open(dst_path, BOFS_O_CREAT|BOFS_O_RDWR);
	if(fd_wr == -1){
		bofs_close(fd_rd);
		return -1;
	}
	/*get src file size*/
	struct bofs_stat fstat;
	bofs_stat(src_pathname, &fstat);
	//printk("file size:%d\n", fstat.st_size);
	
	/*4.alloc a buf for file data*/
	uint8 *copy_buf = mm.malloc(fstat.st_size);
	if(copy_buf == NULL){
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		return -1;
	}
	memset(copy_buf, 0, fstat.st_size);
	/*5.read data from src file*/
	if(bofs_read(fd_rd, copy_buf, fstat.st_size) != fstat.st_size){
		//printk("bofs read size different!\n");
		/*read failed*/
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		mm.free(copy_buf);
		return -1;
	}
	//bofs_exhibit_fd(fd_rd);
	
	/*6.write data to dst file*/
	if(bofs_write(fd_wr, copy_buf, fstat.st_size) == -1){
		//printk("bofs write size different!\n");
		
		/*write failed*/
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		mm.free(copy_buf);
	
		return -1;
	}
	//bofs_exhibit_fd(fd_wr);
	
	//printk("copy file sucess!\n");

	/*change time info*/
	struct bofs_file_descriptor *rd_file, *wr_file;
	rd_file = bofs_get_file_by_fd(fd_rd);
	wr_file = bofs_get_file_by_fd(fd_wr);
	
	wr_file->fd_inode->mdftime = rd_file->fd_inode->mdftime;
	bofs_sync_inode(wr_file->fd_inode);
	
	bofs_close(fd_rd);
	bofs_close(fd_wr);
	mm.free(copy_buf);
	return 0;
}

/*
@Name:		bofs_move_file

@Param:		src_pathname:the source path
			dst_pathname:the destnation path

@Function: 	move a file from src to dst

@Return: 	sucess(0), faild(-1)

@Time: 		2019/4/1

@Author: 	Hu Zicheng
*/
int bofs_move_file(const char *src_pathname, const char *dst_pathname)
{
	//printk("move file: src %s,dst %s\n", src_pathname, dst_pathname);
	
	/*1.make dst pathname by dst path and src file*/
	/*get file name*/
	char name[BOFS_NAME_LEN];
	memset(name, 0, BOFS_NAME_LEN);
	bofs_path_to_name(src_pathname+2, name);
	
	//printk("copy file: file name:%s\n", name);
	/*make pathname*/
	char dst_path[BOFS_PATH_LEN];
	memset(dst_path, 0, BOFS_PATH_LEN);
	strcpy(dst_path, dst_pathname);
	
	/*if not '/', we will cat a '/'*/
	if(dst_path[strlen(dst_pathname)-1] != '/'){
		strcat(dst_path, "/");
	}
	/*cat file name to path to make pathname*/
	strcat(dst_path, name);
	
	/*if name are same, we return*/
	if(!strcmp(dst_path, src_pathname)){
		return -1;
	}
	/*2.open a file as read file*/
	int fd_rd = bofs_open(src_pathname, BOFS_O_CREAT|BOFS_O_RDWR);
	if(fd_rd == -1){
		return -1;
	}
	//printk("move file: dst pathname:%s\n", dst_path);
	/*3.open a file as write file*/
	int fd_wr = bofs_open(dst_path, BOFS_O_CREAT|BOFS_O_RDWR);
	if(fd_wr == -1){
		bofs_close(fd_rd);
		return -1;
	}
	/*get src file size*/
	struct bofs_stat fstat;
	bofs_stat(src_pathname, &fstat);
	//printk("file size:%d\n", fstat.st_size);
	
	/*4.alloc a buf for file data*/
	uint8 *copy_buf = mm.malloc(fstat.st_size);
	if(copy_buf == NULL){
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		return -1;
	}
	memset(copy_buf, 0, fstat.st_size);
	/*5.read data from src file*/
	if(bofs_read(fd_rd, copy_buf, fstat.st_size) != fstat.st_size){
		//printk("bofs read size different!\n");
		/*read failed*/
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		mm.free(copy_buf);
		return -1;
	}
	//bofs_exhibit_fd(fd_rd);
	
	/*6.write data to dst file*/
	if(bofs_write(fd_wr, copy_buf, fstat.st_size) == -1){
		//printk("bofs write size different!\n");
		
		/*write failed*/
		bofs_close(fd_rd);
		bofs_close(fd_wr);
		mm.free(copy_buf);
	
		return -1;
	}
	//bofs_exhibit_fd(fd_wr);
	
	/*change time info*/
	struct bofs_file_descriptor *rd_file, *wr_file;
	rd_file = bofs_get_file_by_fd(fd_rd);
	wr_file = bofs_get_file_by_fd(fd_wr);
	
	wr_file->fd_inode->crttime = rd_file->fd_inode->crttime;
	wr_file->fd_inode->mdftime = rd_file->fd_inode->mdftime;
	bofs_sync_inode(wr_file->fd_inode);
	
	//printk("move file sucess!\n");

	bofs_close(fd_rd);
	bofs_close(fd_wr);
	mm.free(copy_buf);
	

	/*delete src file*/
	if(bofs_unlink(src_pathname) == -1){
		return -1;
	}
	
	return 0;
}

#endif


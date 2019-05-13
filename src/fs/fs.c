/*
File:		fs/fs.c
Contains:	file system module
Auther:		Hu Zicheng
Time:		2019/4/2
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/fs.h>
#include <sys/core.h>

#include <sys/dev.h>

#include <string.h>

struct fs fs;

void init_fs()
{
	fs_environment_init();
	fs_interface_init();
	
}

void fs_environment_init()
{
	#ifdef _CONFIG_FS_BOFS_
		init_bofs();
	#endif
}

void fs_interface_init()
{
	#ifdef _CONFIG_FS_BOFS_
		/*dir*/
		fs.mkdir = bofs_make_dir;
		fs.rmdir = bofs_remove_dir;
		fs.opendir = bofs_open_dir;
		fs.readdir = bofs_read_dir;
		fs.rewinddir = bofs_rewind_dir;
		fs.closedir = bofs_close_dir;
		fs.getcwd = bofs_getcwd;
		fs.chdir = bofs_chdir;
		fs.lsdir = bofs_list_dir;
		fs.rename = bofs_reset_name;
		/*file*/
		fs.open = bofs_open;
		fs.close = bofs_close;
		fs.unlink = bofs_unlink;
		fs.write = bofs_write;
		fs.read = bofs_read;
		fs.lseek = bofs_lseek;
		fs.stat = bofs_stat;
		fs.access = bofs_access;
		fs.setmod = bofs_set_mode;
		fs.getmod = bofs_get_mode;
		fs.copy = bofs_copy_file;
		fs.move = bofs_move_file;
		
		fs.path_to_name = bofs_path_to_name;
		fs.lsdisk = bofs_list_drive;

		fs.load_inode = bofs_load_inode_by_id;
		fs.drive_connect = bofs_drive_connect;
		fs.drive_get = bofs_drive_get;
		
	#endif
}

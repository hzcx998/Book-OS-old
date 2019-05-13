#ifndef _BOFS_FILE_H
#define _BOFS_FILE_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <stdint.h>
#include <fs/bofs/dir_entry.h>
#include <fs/bofs/inode.h>

#define BOFS_MAX_FD_NR 32

#define BOFS_O_RDONLY 0x01 /*file open with read only*/
#define BOFS_O_WRONLY 0x02 /*file open with write only*/
#define BOFS_O_RDWR 0x04 /*file open with read and write*/
#define BOFS_O_CREAT 0x08 /*file open with create*/
#define BOFS_O_TRUNC 0x10 /*file open with trunc file size to 0*/
#define BOFS_O_APPEDN 0x20 /*file open with append pos to end*/
#define BOFS_O_EXEC 0x80 /*file open with execute*/

#define BOFS_SEEK_SET 1 	/*set a pos from 0*/
#define BOFS_SEEK_CUR 2		/*set a pos from cur pos*/
#define BOFS_SEEK_END 3		/*set a pos from end pos back*/

#define BOFS_F_OK 1		/*file existence*/
#define BOFS_X_OK 2		/*file executable*/
#define BOFS_R_OK 3		/*file readable*/
#define BOFS_W_OK 4		/*file writable*/

#define BOFS_FD_FREE 0
#define BOFS_FD_USING 1

struct bofs_file_descriptor
{
	uint8 fd_status; /*fd is free or using*/
	uint32 fd_pos;	/*file cursor pos*/
	uint32 fd_flags; /*file operate flags*/
	char fd_drive; /*file operate drive*/
	
	struct bofs_dir_entry *fd_dir;	/*dir entry*/
	struct bofs_dir_entry *fd_parent;	/*parent dir entry*/
	struct bofs_inode *fd_inode;	/*dir inode*/
};

struct bofs_stat
{
	uint32 st_type;
	uint32 st_size;
	uint32 st_mode;
};

void bofs_init_fd_table();
int bofs_alloc_fd_global();
struct bofs_file_descriptor *bofs_get_file_by_fd(int fd);

int bofs_create_file(struct bofs_dir_entry *parent_dir, char *name, uint16 mode);
int bofs_open_file(struct bofs_dir_entry *parent_dir, char *name, uint16 mode);
int bofs_close_file(struct bofs_file_descriptor *fd);
int bofs_file_write( struct bofs_file_descriptor *fd, void* buf, uint32 count);
int bofs_file_read( struct bofs_file_descriptor *fd, void* buf, uint32 count);

int bofs_is_path_exist(const char* pathname);
int bofs_is_path_executable(const char* pathname);
int bofs_is_path_readable(const char* pathname);
int bofs_is_path_writable(const char* pathname);

int bofs_open(const char *pathname, uint32 flags);
int bofs_close(int fd);
int bofs_unlink(const char *pathname);
int bofs_write(int fd, void* buf, uint32 count);
int bofs_read(int fd, void* buf, uint32 count);
int bofs_lseek(int fd, int offset, uint8 whence);
int bofs_stat(const char *pathname, struct bofs_stat *buf);
int bofs_access(const char *pathname, int mode);
void bofs_exhibit_fd(int fd);

int bofs_set_mode(const char* pathname, int mode);
int bofs_get_mode(const char* pathname);

int bofs_copy_file(const char *src_pathname, const char *dst_pathname);
int bofs_move_file(const char *src_pathname, const char *dst_pathname);
#endif

#endif


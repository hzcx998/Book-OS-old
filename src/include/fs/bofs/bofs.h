#ifndef _FS_BOFS_H
#define _FS_BOFS_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <types.h>
#include <stdint.h>

#include <fs/bofs/super_block.h>

#include <sys/dev.h>

#define IO_BUF_NR 3

/*default fs inode nr*/
#define DEFAULT_MAX_INODE_NR 4096

/*
-----
super block
-----
sector bitmap
-----
inode bitmap
-----
inode table
-----
data
-----
*/
extern void *bofs_dir_entry_io_buf;
extern void *bofs_inode_io_buf;
extern void *bofs_file_io_buf;

extern void *bofs_io_buf, *bofs_io_buf_table[IO_BUF_NR];
extern struct bofs_super_block *bofs_current_super_block;

/*bofs basic*/
void init_bofs();
void bofs_format(int dev_id, uint32 start_sec, uint32 total_sec, uint32 inode_nr);

/*input*/
void * (*bofs_malloc)(uint32 size);
int (*bofs_free)(void * addr);
void (*bofs_sector_read)(uint32 lba, void *buf, uint32 counts);
void (*bofs_sector_write)(uint32 lba, void *buf, uint32 counts);
#endif

#endif


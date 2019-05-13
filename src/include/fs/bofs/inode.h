#ifndef _BOFS_INODE_H
#define _BOFS_INODE_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <stdint.h>


/*book os file system inode*/
#define BOFS_BLOCK_NR 3
#define BOFS_INODE_RESERVED 6

#define BOFS_SECTOR_BLOCK_NR  128	/*a sector max block nr*/

/*we assume that a file max size is 8MB-64KB-512B*/

#define BOFS_BLOCK_LEVEL0  0
#define BOFS_BLOCK_LEVEL1  BOFS_SECTOR_BLOCK_NR
#define BOFS_BLOCK_LEVEL2  (BOFS_SECTOR_BLOCK_NR + BOFS_SECTOR_BLOCK_NR*BOFS_SECTOR_BLOCK_NR)

#define BOFS_IMODE_U 0X00 /*unknown mode*/
#define BOFS_IMODE_R 0X01 /*read mode*/
#define BOFS_IMODE_W 0X02 /*write mode*/
#define BOFS_IMODE_X 0X04 /*read and write mode*/

#define BOFS_IMODE_F 0X10 /*file type mode*/
#define BOFS_IMODE_D 0X20 /*directory type mode*/

/*
we assume a inode is 64 bytes
*/

struct bofs_inode 
{
	uint32 id;	/*inode id*/
	
	uint16 mode;	/*file mode*/
	uint16 links;	/*links number*/
	
	uint32 size;	/*file size*/
	
	uint32 crttime;	/*create time*/
	uint32 mdftime;	/*modify time*/
	uint32 acstime;	/*access time*/
	
	uint32 flags;	/*access time*/
	
	uint32 block[BOFS_BLOCK_NR];	/*data block*/
	
	uint32 reserved[BOFS_INODE_RESERVED];	/*data block*/
};

void bofs_create_inode(struct bofs_inode *inode);

void bofs_sync_inode(struct bofs_inode *inode);
void bofs_load_inode_by_id(struct bofs_inode *inode, uint32 id);
void bofs_copy_inode(struct bofs_inode *inode_a, struct bofs_inode *inode_b);
void bofs_release_inode_data(struct bofs_inode *inode);
void bofs_empty_inode(struct bofs_inode *inode);
void bofs_close_inode(struct bofs_inode *inode);

int bofs_get_inode_data(struct bofs_inode *inode, uint32 block_id, uint32 *data);
int bofs_free_inode_data(struct bofs_inode *inode, uint32 block_id);
void bofs_copy_inode_data(struct bofs_inode *inode_a, struct bofs_inode *inode_b);
#endif

#endif


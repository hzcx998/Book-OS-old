#ifndef _BOFS_BITMAP_H
#define _BOFS_BITMAP_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <stdint.h>

#include <fs/bofs/super_block.h>

/*bofs bitmap*/
enum bofs_bitmap_type
{
	SECTOR_BITMAP,
	INODE_BITMAP,
};
void bofs_load_bitmap(struct bofs_super_block *bsb);

int bofs_alloc_bitmap(enum bofs_bitmap_type bitmap_type, uint32 counts);
int bofs_free_bitmap(enum bofs_bitmap_type bitmap_type, uint32 idx);
int bofs_sync_bitmap(enum bofs_bitmap_type bitmap_type, uint32 idx);

int bofs_idx_to_lba(enum bofs_bitmap_type bitmap_type, uint32 idx);
int bofs_lba_to_idx(enum bofs_bitmap_type bitmap_type, uint32 lba);
#endif

#endif


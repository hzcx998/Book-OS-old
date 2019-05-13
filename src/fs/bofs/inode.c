/*
File:		fs/bofs/inode.c
Contains:	bofs's inode
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

void bofs_close_inode(struct bofs_inode *inode)
{
	if(inode != NULL){
		mm.free(inode);
	}
}

void bofs_create_inode(struct bofs_inode *inode)
{
	memset(inode, 0, sizeof(struct bofs_inode));
	inode->id = 0;
	inode->mode = BOFS_IMODE_U;
	inode->links = 0;
	inode->size = 0;
	
	inode->crttime = 0;	/*create time*/
	inode->mdftime = 0;	/*modify time*/
	inode->acstime = 0;	/*access time*/
	
	inode->flags = 0;	/*access time*/
	
	//inode->blocks = 0;	/*file blocks*/
	
	int i;
	for(i = 0; i < BOFS_BLOCK_NR; i++){
		inode->block[i] = 0;
	}
}

void bofs_sync_inode(struct bofs_inode *inode)
{
	uint32 sec_off = inode->id/bofs_current_super_block->inode_nr_in_sector;
	uint32 lba = bofs_current_super_block->inode_table_lba + sec_off;
	uint32 buf_off = inode->id%bofs_current_super_block->inode_nr_in_sector;
	
	//printk("bofs sync inode: sec off:%d lba:%d buf off:%d\n", sec_off, lba, buf_off);
	
	memset(bofs_inode_io_buf, 0, SECTOR_SIZE);
	sector_read(lba, bofs_inode_io_buf, 1);
	
	struct bofs_inode *inode_buf = (struct bofs_inode *)bofs_inode_io_buf;
	inode_buf[buf_off] = *inode;
	
	sector_write(lba, bofs_inode_io_buf, 1);
}

/*
we need inode size to release data
*/
void bofs_release_inode_data(struct bofs_inode *inode)
{
	int i;
	uint32 blocks = DIV_ROUND_UP(inode->size, SECTOR_SIZE);
	
	//printk("inode:%d size:%d blocks:%d\n",inode->id,inode->size, blocks);
	/*scan back to*/
	for(i = 0; i < blocks; i++){
		bofs_free_inode_data(inode, i);
	}
}

void bofs_copy_inode(struct bofs_inode *inode_a, struct bofs_inode *inode_b)
{
	memset(inode_a, 0, sizeof(struct bofs_inode));
	
	inode_a->id = inode_b->id;
	
	inode_a->mode = inode_b->mode;
	inode_a->links = inode_b->links;
	
	inode_a->size = inode_b->size;
	
	inode_a->crttime = inode_b->crttime;
	inode_a->mdftime = inode_b->mdftime;
	inode_a->acstime = inode_b->acstime;
	
	inode_a->flags = inode_b->flags;
	
	//inode_a->blocks = inode_a->blocks;
	
	/*
	copy data block
	*/
}

void bofs_load_inode_by_id(struct bofs_inode *inode, uint32 id)
{
	uint32 sec_off = id/bofs_current_super_block->inode_nr_in_sector;
	uint32 lba = bofs_current_super_block->inode_table_lba + sec_off;
	uint32 buf_off = id%bofs_current_super_block->inode_nr_in_sector;
	
	//printk("bofs sync: sec off:%d lba:%d buf off:%d\n", sec_off, lba, buf_off);
	
	memset(bofs_inode_io_buf, 0, SECTOR_SIZE);
	sector_read(lba, bofs_inode_io_buf, 1);
	
	struct bofs_inode *inode_buf = (struct bofs_inode *)bofs_inode_io_buf;
	*inode = inode_buf[buf_off];
}

void bofs_empty_inode(struct bofs_inode *inode)
{
	
	uint32 sec_off = inode->id/bofs_current_super_block->inode_nr_in_sector;
	uint32 lba = bofs_current_super_block->inode_table_lba + sec_off;
	uint32 buf_off = inode->id%bofs_current_super_block->inode_nr_in_sector;
	
	//printk("bofs sync: sec off:%d lba:%d buf off:%d\n", sec_off, lba, buf_off);
	
	memset(bofs_inode_io_buf, 0, SECTOR_SIZE);
	sector_read(lba, bofs_inode_io_buf, 1);
	
	struct bofs_inode *inode_buf = (struct bofs_inode *)bofs_inode_io_buf;

	memset(&inode_buf[buf_off], 0, sizeof(struct bofs_inode));
	sector_write(lba, bofs_inode_io_buf, 1);
	
}


/*
for data read or write, we should make a way to do it.
in a inode, we can read max sectors below blocks, so we can get 
sector use bofs_get_inode_data(); to get data lba
*/

int bofs_get_inode_data(struct bofs_inode *inode, uint32 block_id, uint32 *data)
{
	uint32 indirect[3];
	uint32 offset[2];
	uint32 *data_buf0, *data_buf1;
	
	uint32 lba, idx;
	//printk("bofs_get_inode_data> inode:%d ",inode->id);
	if(block_id == BOFS_BLOCK_LEVEL0){
		//printk("level:%d off:%d\n", 0, block_id);
		
		/*get indirect0*/
		indirect[0] = inode->block[0];
		if(indirect[0] == 0){
			//printk("no data, now create a new sector to save it.\n");
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[0] = inode->block[0] = lba;
			
			//printk(">>>lba:%d\n", lba);
			
			/*sync inode to save new data*/
			bofs_sync_inode(inode);
		}else{
			//printk("have data.\n");
			
		}
		
		*data = indirect[0];
		
	}else if(BOFS_BLOCK_LEVEL0 < block_id && block_id <= BOFS_BLOCK_LEVEL1){
		
		block_id -= (BOFS_BLOCK_LEVEL0+1);
		
		offset[0] = block_id;
		
		//printk("level:%d off:%d\n", 1, offset[0]);
		/*get indirect0*/
		indirect[0] = inode->block[1];

		if(indirect[0] == 0){
			//printk("no indirect 0 data, now create a new sector to save it.\n");
			
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[0] = inode->block[1] = lba;
			/*sync inode to save new data*/
			//printk(">>>sync inode block %d %d %d", inode->block[0], inode->block[1], inode->block[2]);
		
			bofs_sync_inode(inode);
		}else{
			//printk("have data.\n");
			
		}
		
		memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
		sector_read(indirect[0], bofs_io_buf_table[0], 1);
		
		data_buf0 = bofs_io_buf_table[0];
		
		/*get indirect1*/
		indirect[1] = data_buf0[offset[0]];
		if(indirect[1] == 0){
			//printk("no indirect 1 data, now create a new sector to save it.\n");
			
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[1] = data_buf0[offset[0]] = lba;
			/*sync buf to save new data*/
			
			sector_write(indirect[0], bofs_io_buf_table[0], 1);
		}else{
			//printk("have data.\n");
			
		}
		//printk("block %d %d %d", inode->block[0], inode->block[1], inode->block[2]);
		
		*data = indirect[1];
		
	}else if(BOFS_BLOCK_LEVEL1 < block_id && block_id <= BOFS_BLOCK_LEVEL2){
		/*
		we do not test here, if a dir has at least 129 dir , it will run here.
		so now, we don't test it, but we image it is OK.
		2019/3/24 by Hu Zicheng @.@
		*/
		
		block_id -= (BOFS_BLOCK_LEVEL1+1);
		
		offset[0] = block_id/BOFS_SECTOR_BLOCK_NR;
		offset[1] = block_id%BOFS_SECTOR_BLOCK_NR;
		
		//printk("level:%d off:%d off:%d\n", 2, offset[0], offset[1]);
		/*get indirect0*/
		indirect[0] = inode->block[2];
		if(indirect[0] == 0){
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[0] = inode->block[2] = lba;
			/*sync inode to save new data*/
			bofs_sync_inode(inode);
		}
		
		memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
		sector_read(indirect[0], bofs_io_buf_table[0], 1);
		
		data_buf0 = bofs_io_buf_table[0];
		
		/*get indirect1*/
		indirect[1] = data_buf0[offset[0]];
		if(indirect[1] == 0){
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[1] = data_buf0[offset[0]] = lba;
			/*sync buf to save new data*/
			sector_write(indirect[0], bofs_io_buf_table[0], 1);
		}
		
		memset(bofs_io_buf_table[1], 0, SECTOR_SIZE);
		sector_read(indirect[1], bofs_io_buf_table[1], 1);
		
		data_buf1 = bofs_io_buf_table[1];
		
		/*get indirect2*/
		indirect[2] = data_buf1[offset[1]];
		if(indirect[2] == 0){
			/*no data, alloc data*/
			idx = bofs_alloc_bitmap(SECTOR_BITMAP, 1);
			if(idx == -1){
				printk("alloc sector bitmap failed!\n");
				return -1;
			}
			/*alloc a sector*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			lba = bofs_idx_to_lba(SECTOR_BITMAP, idx);
			
			indirect[2] = data_buf1[offset[1]] = lba;
			/*sync buf to save new data*/
			
			sector_write(indirect[1], bofs_io_buf_table[1], 1);
		}
		*data = indirect[2];
	}
	return 0;
}

/*

*/
int bofs_free_inode_data(struct bofs_inode *inode, uint32 block_id)
{
	uint32 indirect[3];
	uint32 offset[2];
	uint32 *data_buf0, *data_buf1;
	
	uint32 idx;
	int i;
	uint8 flags;
	
	//printk("bofs_free_inode_data> ");
	
	if(block_id == BOFS_BLOCK_LEVEL0){
		/*get indirect0*/
		indirect[0] = inode->block[0];
		if(indirect[0] != 0){
			//printk("have data:%d\n", indirect[0]);
			memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
			sector_write(indirect[0], bofs_io_buf_table[0], 1);
			
			idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[0]);
			/*free sector bitmap*/
			bofs_free_bitmap(SECTOR_BITMAP, idx);
			/*we need sync to disk*/
			bofs_sync_bitmap(SECTOR_BITMAP, idx);
			
			inode->block[0] = 0;
			/*sync inode to save empty data*/
			bofs_sync_inode(inode);
			//printk("<<<lba:%d\n", indirect[0]);
			
		}else{
			//printk("no data.\n");
		}
		return indirect[0];
	}else if(BOFS_BLOCK_LEVEL0 < block_id && block_id <= BOFS_BLOCK_LEVEL1){
		
		block_id -= (BOFS_BLOCK_LEVEL0+1);
		
		offset[0] = block_id;
		
		//printk("level:%d off:%d\n", 1, offset[0]);
		/*get indirect0*/
		indirect[0] = inode->block[1];
		if(indirect[0] != 0){
			//printk("have indirect 0 data:%d\n", indirect[0]);
			memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
			sector_read(indirect[0], bofs_io_buf_table[0], 1);
			
			data_buf0 = bofs_io_buf_table[0];
			
			/*get indirect1*/
			indirect[1] = data_buf0[offset[0]];
			if(indirect[1] != 0){
				//printk("have indirect 1 data:%d\n", indirect[1]);
				memset(bofs_io_buf_table[1], 0, SECTOR_SIZE);
				sector_write(indirect[1], bofs_io_buf_table[1], 1);
				
				idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[1]);
				/*free sector bitmap*/
				bofs_free_bitmap(SECTOR_BITMAP, idx);
				/*we need sync to disk*/
				bofs_sync_bitmap(SECTOR_BITMAP, idx);
			
				data_buf0[offset[0]] = 0;
				/*write to disk for next time check.
				every time we change indirect block's data,
				we need write to disk to save this result.
				*/
				sector_write(indirect[0], bofs_io_buf_table[0], 1);
				
				//printk("@@@check empty!\n");
				flags = 0;
				/*check indirect, if empty, we release it*/
				for(i = 0; i < BOFS_SECTOR_BLOCK_NR; i++){
					if(data_buf0[i] != 0){
						/*not empty*/
						flags = 1;
						//printk("buf[%d] = %d ", i, data_buf0[i]);
						//printk(">>indirect block no empty!\n");
						break;
					}
				}
				/*indirect sector is empty, we will release it*/
				if(!flags){
					//printk("level 1 indirect 1 is empty!\n");
					memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
					sector_write(indirect[0], bofs_io_buf_table[0], 1);
					//printk("##sector!\n");
					idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[0]);
					/*free sector bitmap*/
					bofs_free_bitmap(SECTOR_BITMAP, idx);
					/*we need sync to disk*/
					bofs_sync_bitmap(SECTOR_BITMAP, idx);
			
					inode->block[1] = 0;
					//printk("&&&&&sync to inode!\n");
					/*sync inode to save empty data*/
					bofs_sync_inode(inode);
				}
			}else{
				//printk("no data.\n");
			}
		}else{
			//printk("no data.\n");
		}
		return indirect[1];
	}else if(BOFS_BLOCK_LEVEL1 < block_id && block_id <= BOFS_BLOCK_LEVEL2){
		/*
		we do not test here, if a dir has at least 129 dir , it will run here.
		so now, we don't test it, but we image it is OK.
		2019/3/24 by Hu Zicheng @.@
		*/
		
		block_id -= (BOFS_BLOCK_LEVEL1+1);
		
		offset[0] = block_id/BOFS_SECTOR_BLOCK_NR;
		offset[1] = block_id%BOFS_SECTOR_BLOCK_NR;
		
		//printk("level:%d off:%d off:%d\n", 2, offset[0], offset[1]);
		/*get indirect0*/
		indirect[0] = inode->block[2];
		if(indirect[0] != 0){
			
			memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
			sector_read(indirect[0], bofs_io_buf_table[0], 1);
			
			data_buf0 = bofs_io_buf_table[0];
			
			/*get indirect1*/
			indirect[1] = data_buf0[offset[0]];
			if(indirect[1] != 0){
				
				memset(bofs_io_buf_table[1], 0, SECTOR_SIZE);
				sector_read(indirect[1], bofs_io_buf_table[1], 1);
				
				data_buf1 = bofs_io_buf_table[1];
				
				indirect[2] = data_buf1[offset[1]];
				
				if(indirect[2] != 0){
					memset(bofs_io_buf_table[2], 0, SECTOR_SIZE);
					sector_write(indirect[2], bofs_io_buf_table[2], 1);
					
					idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[2]);
					/*free sector bitmap*/
					bofs_free_bitmap(SECTOR_BITMAP, idx);
					/*we need sync to disk*/
					bofs_sync_bitmap(SECTOR_BITMAP, idx);
			
					data_buf1[offset[1]] = 0;
					/*write to disk for next time check.
					every time we change indirect block's data,
					we need write to disk to save this result.
					*/
					sector_write(indirect[1], bofs_io_buf_table[1], 1);
					
					flags = 0;
					/*check indirect, if empty, we release it*/
					for(i = 0; i < BOFS_SECTOR_BLOCK_NR; i++){
						if(data_buf1[i] != 0){
							/*not empty*/
							flags = 1;
						}
					}
					/*indirect sector is empty, we will release it*/
					if(!flags){
						memset(bofs_io_buf_table[1], 0, SECTOR_SIZE);
						sector_write(indirect[1], bofs_io_buf_table[1], 1);
						
						idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[1]);
						/*free sector bitmap*/
						bofs_free_bitmap(SECTOR_BITMAP, idx);
						/*we need sync to disk*/
						bofs_sync_bitmap(SECTOR_BITMAP, idx);
						
						data_buf0[offset[0]] = 0;
						/*write to disk for next time check.
						every time we change indirect block's data,
						we need write to disk to save this result.
						*/
						sector_write(indirect[0], bofs_io_buf_table[0], 1);
						
						flags = 0;
						/*check indirect, if empty, we release it*/
						for(i = 0; i < BOFS_SECTOR_BLOCK_NR; i++){
							if(data_buf0[i] != 0){
								/*no empty*/
								flags = 1;
							}
						}
						/*indirect sector is empty, we will release it*/
						if(!flags){
							//printk("level 2 indirect 1 is empty!\n");
							
							memset(bofs_io_buf_table[0], 0, SECTOR_SIZE);
							sector_write(indirect[0], bofs_io_buf_table[0], 1);
							
							idx = bofs_lba_to_idx(SECTOR_BITMAP, indirect[0]);
							/*free sector bitmap*/
							bofs_free_bitmap(SECTOR_BITMAP, idx);
							/*we need sync to disk*/
							bofs_sync_bitmap(SECTOR_BITMAP, idx);
			
							inode->block[2] = 0;
							
							/*sync inode to save empty data*/
							bofs_sync_inode(inode);
						}
					}
					
				}
			}
		}
		return indirect[2];
	}
	return -1;
}

void bofs_copy_inode_data(struct bofs_inode *inode_a, struct bofs_inode *inode_b)
{
	/*if size is 0, don't copy data*/
	if(inode_b->size == 0){
		return;
	}
	/*inode data blocks*/
	uint32 blocks = DIV_ROUND_UP(inode_b->size, SECTOR_SIZE);
	uint32 block_id = 0;
	
	uint32 src_lba, dst_lba;
	
	while(block_id < blocks){
		/*get a lba and read data*/
		bofs_get_inode_data(inode_b, block_id, &src_lba);
		memset(bofs_inode_io_buf, 0, SECTOR_SIZE);
		sector_read(src_lba, bofs_inode_io_buf, 1);
		
		/*get a lba and write data*/
		bofs_get_inode_data(inode_a, block_id, &dst_lba);
		sector_write(dst_lba, bofs_inode_io_buf, 1);
		
		block_id++;
	}
}

#endif



#ifndef _BOFS_DIR_ENTRY_H
#define _BOFS_DIR_ENTRY_H

#include <sys/config.h>

#ifdef _CONFIG_FS_BOFS_

#include <stdint.h>

/*book os file system dir entry*/

#define BOFS_NAME_LEN (256-6)

#define BOFS_FILE_TYPE_UNKNOWN 		0X00
#define BOFS_FILE_TYPE_NORMAL 		0X01
#define BOFS_FILE_TYPE_DIRECTORY 	0X02
#define BOFS_FILE_TYPE_INVALID 		0X80

#define BOFS_PATH_LEN 	256

struct bofs_dir_entry
{
	uint32 inode;	/*inode number*/
	uint16 file_type;	/*file type*/
	char name[BOFS_NAME_LEN];	/*name length(0~255)*/
};

#define BOFS_DIR_NR_IN_SECTOR 	(SECTOR_SIZE/(sizeof(struct bofs_dir_entry)))

int bofs_set_dir_name(char *buf, char *name);
void bofs_open_root_dir();
void bofs_create_dir_entry(struct bofs_dir_entry *dir_entry, char *name, uint32 flags);
void bofs_copy_dir_entry(struct bofs_dir_entry *dir_a, struct bofs_dir_entry *dir_b, char copy_inode);
int bofs_sync_dir_entry(struct bofs_dir_entry* parent_dir, struct bofs_dir_entry* child_dir);
bool bofs_search_dir_entry(struct bofs_dir_entry *parent_dir, struct bofs_dir_entry *child_dir, char *name);
void bofs_release_dir_entry(struct bofs_dir_entry *child_dir);
void bofs_close_dir_entry(struct bofs_dir_entry *dir_entry);
bool bofs_load_dir_entry(struct bofs_dir_entry *parent_dir, char *name, struct bofs_dir_entry *child_dir);
void bofs_exhibit_dir_entry(struct bofs_dir_entry *dir_entry);
#endif

#endif


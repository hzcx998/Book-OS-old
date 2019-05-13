#ifndef _BOFS_DIR_H
#define _BOFS_DIR_H

#include <stdint.h>
#include <fs/bofs/dir_entry.h>


#define BOFS_BLOCK_SIZE 512

/*book os file system dir*/
struct bofs_dir
{
	struct bofs_dir_entry *m_dir_entry;
	uint32 block_id;
	uint32 dir_pos;
	uint32 buf_status;
	uint8 dir_buf[BOFS_BLOCK_SIZE];
};

struct bofs_dir_search_record
{
   struct bofs_dir_entry *parent_dir;
   struct bofs_dir_entry *child_dir;
};

/*dir */
int bofs_search_dir(const char* pathname, struct bofs_dir_search_record *record);
void bofs_exhibit_dir(struct bofs_dir *dir);

int bofs_make_dir(const char *pathname);
int bofs_remove_dir(const char *pathname);
struct bofs_dir *bofs_open_dir(const char *pathname);
struct bofs_dir_entry *bofs_read_dir(struct bofs_dir *dir);
void bofs_scan_dir(const char *pathname);
void bofs_rewind_dir(struct bofs_dir *dir);
void bofs_close_dir(struct bofs_dir* dir);
void bofs_list_dir(const char *pathname, int level);

int bofs_path_to_name(const char *pathname, char *name_buf);
int bofs_reset_name(const char *pathname, char *name);


int bofs_chdir(const char *pathname);
int bofs_getcwd(char* buf, uint32 size);
#endif


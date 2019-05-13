#ifndef _STDIO_H_
#define _STDIO_H_

#include <types.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/fs.h>

#define STR_DEFAULT_LEN 256

/*conio*/
int write(char *str);

int getchar();
void putchar(char ch);
/*print*/
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);
int speak(const char *fmt, ...);


/*files*/

int32_t fopen(const char *pathname,uint8_t flags);
int32_t fclose(int32_t fd);
int32_t fwrite(int32_t fd, void* buf, uint32_t count);
int32_t fread(int32_t fd, void* buf, uint32_t count);
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t fstat(char* path, struct stat* buf);
int32_t unlink(const char* pathname);

/*dir*/
struct dir* opendir(const char* name);
struct dir_entry* readdir(struct dir* dir);
void rewinddir(struct dir* dir);
void closedir(struct dir* dir);
int32_t chdir(const char* path);
int32_t getcwd(char* buf, uint32_t size);

int32_t rename(const char *pathname, char *new_name);
int32_t move(char* oldpath, char* newpath);
int32_t copy(char* src_path, char* dst_path);

int32_t mkdir(const char *pathname);
int32_t rmdir(const char *pathname);

int32_t access(const char *pathname, int mode);
void lsdir(const char *pathname, int level);

int lsdisk();

/*inode*/
void fs_load_inode(struct inode_s *buf, unsigned int id);

/*drive*/
struct drive_s *fs_drive_connect();
struct drive_s *fs_drive_get(struct drive_s *buf);


/*mouse*/
int get_mouse_pos(int *x, int *y);
void get_mouse_btn(char buf[]);

#endif

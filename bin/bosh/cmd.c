#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/const.h>
#include <graphic.h>
#include "gui.h"
#include "bosh.h"
#include "cmd.h"

extern char final_path[MAX_PATH_LEN];

char final_path2[MAX_PATH_LEN] = {0}; // 用于清洗路径时的缓冲

int cmd_kill(uint32_t argc, char** argv)
{
	if(argc < 2){
		gui_printf("kill: too few arguments.\n");	
		return -1;
	}

	//默认 argv[1]是进程的pid
	
	//把传递过来的字符串转换成数字

	int pid = atoi(argv[1]);
	gui_printf("kill: arg %s pid %d.\n", argv[1], pid);	

	if (thread_kill(pid) == -1) {
		gui_printf("kill: pid %d failed.\n", pid);	
	}
	return 0;
}



void cmd_mm(uint32_t argc, char** argv)
{
	int mem_size, mem_free;
	get_memory(&mem_size, &mem_free);
	gui_printf("physical memory size: %d bytes ( %d mb)\nphysical memory free: %d bytes ( %d mb)\n",\
	mem_size, mem_size/(1024*1024), mem_free, mem_free/(1024*1024));
	
}

void cmd_exit(uint32_t argc, char** argv)
{
	//关闭pipe
	pipe_close(pipe_id);
	//printf("close pipe %d\n", pipe_id);
	//关闭窗口
	gui_window_close();
	//while(1);
	//关闭进程
	exit(0);
}

void cmd_reboot(uint32_t argc, char** argv)
{
	reboot(BOOT_KBD);
}

int cmd_copy(uint32_t argc, char** argv)
{
	if(argc < 3){
		gui_printf("copy: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		gui_printf("copy: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		gui_printf("copy: dst pathname can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	make_clear_abs_path(argv[2], final_path2);

	if(!copy(final_path, final_path2)){
		gui_printf("copy: %s to", final_path);	
		gui_printf(" %s sucess!\n", final_path2);
		return 0;
	}else{
		gui_printf("copy: %s to", final_path);	
		gui_printf(" %s faild!\n", final_path2);
		return -1;
	}
}

int cmd_move(uint32_t argc, char** argv)
{
	//gui_printf("echo: arguments %d\n", argc);
	if(argc < 3){
		gui_printf("move: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		gui_printf("move: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		gui_printf("move: dst pathname can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	make_clear_abs_path(argv[2], final_path2);

	if(!move(final_path, final_path2)){
		gui_printf("move: %s to", final_path);	
		gui_printf(" %s sucess!\n", final_path2);
		return 0;
	}else{
		gui_printf("move: %s to", final_path);	
		gui_printf(" %s faild!\n", final_path2);
		return -1;
	}
}

int cmd_rename(uint32_t argc, char** argv)
{
	//gui_printf("echo: arguments %d\n", argc);
	if(argc < 3){
		gui_printf("rename: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		gui_printf("rename: pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		gui_printf("rename: new name can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	if(!rename(final_path, argv[2])){
		gui_printf("rename: %s to", final_path);	
		gui_printf(" %s sucess!\n", argv[2]);
		return 0;
	}else{
		gui_printf("rename: %s to", final_path);	
		gui_printf(" %s faild!\n", argv[2]);
		return -1;
	}
}


void cmd_time(uint32_t argc, char** argv)
{
	struct time tm;
	gettime(&tm);
	
	gui_printf("current time: ");
	gui_printf("%d:",tm.hour);
	gui_printf("%d:",tm.minute);
	gui_printf("%d\n",tm.second);
	
}

void cmd_date(uint32_t argc, char** argv)
{
	struct time tm;
	gettime(&tm);
	
	gui_printf("current date: ");
	gui_printf("%d/",tm.year);
	gui_printf("%d/",tm.month);
	gui_printf("%d\n",tm.day);
	
}

void cmd_ver(uint32_t argc, char** argv)
{
	
	gui_printf("\n%s ",OS_NAME);
	gui_printf("[Version %s]\n",OS_VERSION);
	
}

void cmd_dir(uint32_t argc, char** argv)
{
	char *pathname = NULL;
	
	pathname = argv[1];
	if(pathname == NULL){
		
		if(!getcwd(final_path, MAX_PATH_LEN)){
			pathname = final_path;
			
		}else{
			gui_printf("ls: getcwd for default path faild!\n");
			return;
		}
	}else{
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
		
		
	}
	gui_printf("dir path:%s\n", pathname);
	

	//gui_printf("ls: path:%s\n", pathname);
	cmd_ls_sub(pathname, 1);
	
	if(!1){
		gui_printf("\n");
	}
}
/*
cat: print a file
*/
int cmd_cat(int argc, char *argv[])
{
	//gui_printf("argc: %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		gui_printf("cat: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		gui_printf("cat: only support 2 argument!\n");
		return -1;
	}
	
	make_clear_abs_path(argv[1], final_path);

	int fd = fopen(final_path, O_RDONLY);
	if(fd == -1){
		gui_printf("cat: fd %d error\n", fd);
		return 0;
	}
	
	struct stat stat;
	fstat(final_path, &stat);
	
	char *buf = (char *)malloc(stat.st_size);
	
	int bytes = fread(fd, buf, stat.st_size);
	//gui_printf("read:%d\n", bytes);
	fclose(fd);
	
	int i = 0;
	while(i < bytes){
		window_put_word(buf[i]);
		i++;
	}
	free(buf);
	gui_printf("\n");
	return 0;
}

/*
type filename
*/
int cmd_type(int argc, char *argv[])
{
	//gui_printf("argc: %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		gui_printf("type: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		gui_printf("cat: only support 2 argument!\n");
		return -1;
	}
	
	int fd = fopen(final_path, O_RDONLY);
	if(fd == -1){
		gui_printf("cat: fd %d error\n", fd);
		return 0;
	}
	
	struct stat stat;
	fstat(final_path, &stat);
	
	char *buf = (char *)malloc(stat.st_size);
	
	int bytes = fread(fd, buf, stat.st_size);
	//gui_printf("read:%d\n", bytes);
	fclose(fd);
	
	int i = 0;
	while(i < bytes){
		putchar(buf[i]);
		i++;
	}
	free(buf);
	
	gui_printf("\n");
	return 0;
}

/*
echo string			//show a string
echo string>file 	//output a string to file
echo string>>file	//apend a string to file
*/
int cmd_echo(int argc, char *argv[])
{
	//gui_printf("echo: arguments %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		gui_printf("echo: no arguments.\n");
		return -1;
	}
	if(argc > 2){
		gui_printf("echo: only support 2 argument!");
		return -1;
	}
	char operate = 0;
	//获取操作类型
	char *p = argv[1];
	while(*p){
		if(*p == '>'){
			operate++;
		}
		p++;
	}
	
	//让p指向文件名
	p = argv[1];
	//知道遇到>才停止，如果没有>就遇到结束符停止
	while(*p != '>' && *p != '\0'){
		p++;
	}
	
	char pathname[256] = {0};

	if(operate == 0){//直接显示
		gui_printf("%s\n", argv[1]);
	}else if(operate == 1){	//输出到文件
		//先获取文件路径
		strcpy(pathname, p+1);	//跳过一个>
		//gui_printf("output: %s\n", pathname);
		
		make_clear_abs_path(pathname, final_path);
		
		//gui_printf("abs: %s\n", abs_path);
		//gui_printf("abs: %s\n", abs_path);
		
		//在文本最后添加'\0'
		*p = '\0';
		p = argv[1];
	
		int fd = fopen(final_path, O_CREAT|O_RDWR);
		//gui_printf("fd:%d\n", fd);
		fwrite(fd, p, strlen(p));
		//gui_printf("write:%d\n", bytes);
		fclose(fd);
		//gui_printf("%s", p);
	}else if(operate == 2){	//输出到文件
		//先获取文件路径
		strcpy(pathname, p+2);	//跳过两个>
		//gui_printf("output: %s\n", pathname);
		
		make_clear_abs_path(pathname, final_path);
		//gui_printf("abs: %s\n", abs_path);
		
		//在文本最后添加'\0'
		*p = '\0';
		p = argv[1];
		
		int fd = fopen(final_path, O_CREAT|O_RDWR);
		//gui_printf("fd:%d\n", fd);
		//指向末尾
		lseek(fd, 0, SEEK_END);
		fwrite(fd, p, strlen(p));
		//gui_printf("write:%d at pos:%d\n", bytes, pos);
		fclose(fd);
		//gui_printf("%s", p);
	}
	//gui_printf("\n");
	return 0;
}

void cmd_help(uint32_t argc, char** argv)
{
	if(argc != 1){
		gui_printf("help: no argument support!\n");
		return;
	}
	gui_printf("  cat         print a file.\n");
	gui_printf("  cls         clean screen.\n");
	gui_printf("  cd          change current work dirctory.\n");
	gui_printf("  copy        copy a file.\n");
	gui_printf("  date        get current date.\n");
	gui_printf("  dir         list files in current dirctory.\n");
	gui_printf("  exit        exit shell.\n");
	gui_printf("  kill        close a thread.\n");
	gui_printf("  ls          list files in current dirctory.\n");
	gui_printf("  lsdisk      list disk drives.\n");
	gui_printf("  mkdir       create a dir.\n");
	gui_printf("  mm          print memory info.\n");
	gui_printf("  mv          move a file.\n");
	gui_printf("  ps          print tasks.\n");
	gui_printf("  pwd         print work directory.\n");
	gui_printf("  rmdir       remove a dir.\n");
	gui_printf("  rename      reset file/dirctory name.\n");
	gui_printf("  reboot      reboot system.\n");
	gui_printf("  rm          delete a file.\n");
	gui_printf("  time        get current time.\n");
	gui_printf("  ver         show os version.\n");
	
	
}

void cmd_ps(uint32_t argc, char** argv)
{
	if(argc != 1){
		gui_printf("ps: no argument support!\n");
		return;
	}

	gui_printf("=====Thread info=====\n");

	struct thread *buf = thread_connect();

	do {
		if (buf != NULL) {
			if (buf->status != THREAD_UNUSED) {
				gui_printf("name:%s  pid:%d  status:%d\n", buf->name, buf->pid, buf->status);
			}
		}
		buf = thread_getinfo(buf);
	} while (buf != NULL);

	//ps();
}

void cmd_ls(uint32_t argc, char** argv)
{
	char *pathname = NULL;
	int detail = 0;
	uint32_t arg_path_nr = 0;
	uint32_t arg_idx = 1;	//跳过argv[0]
	while(arg_idx < argc){
		if(argv[arg_idx][0] == '-'){	//参数形式
			if(!strcmp(argv[arg_idx], "-a")){
				detail = 2;
			}else if(!strcmp(argv[arg_idx], "-l")){
				detail = 1;
			}else if(!strcmp(argv[arg_idx], "-h")){
				gui_printf("  -l list all infomation about the file.\n\
  -h get command  help\n\
  list all files in the current dirctory if no option.\n");
				return;
			}
		}else {	//是个路径
			if(arg_path_nr == 0){
				pathname = argv[arg_idx];
				
				arg_path_nr = 1;
			}else{
				gui_printf("ls: only support one path!\n");
				return;
			}
		}
		arg_idx++;
	}
	if(pathname == NULL){
		if(!getcwd(final_path, MAX_PATH_LEN)){
			pathname = final_path;
		}else{
			gui_printf("ls: getcwd for default path faild!\n");
			return;
		}
	}else{
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
	}
	//gui_printf("ls: path:%s\n", pathname);
	cmd_ls_sub(pathname, detail);
	
	if(!detail){
		gui_printf("\n");
	}
	
}

char *cmd_cd(uint32_t argc, char** argv)
{
	//gui_printf("pwd: argc %d\n", argc);
	if(argc > 2){
		gui_printf("cd: only support 1 argument!\n");
		return NULL;
	}
	memset(final_path, 0, MAX_PATH_LEN);
	if(argc == 1){	//只有cd
		final_path[0] = 'c';
		final_path[1] = ':';
		final_path[2] = '/';
		
	}else{
		//不只有cd ，例如 cd a
		make_clear_abs_path(argv[1], final_path);
	}
	//现在已经有了绝对路径
	//尝试改变目录，如果没有就失败
	//gui_printf("cd: operate %s\n",final_path);
	
	if(chdir(final_path) == -1){
		gui_printf("cd: no such directory %s\n",final_path);
		return NULL;
	}
	return final_path;
}

void cmd_pwd(uint32_t argc, char** argv)
{
	//gui_printf("pwd: argc %d\n", argc);
	if(argc != 1){
		gui_printf("pwd: no argument support!\n");
		return;
	}else{
		if(!getcwd(final_path, MAX_PATH_LEN)){
			gui_printf("%s\n", final_path);
		}else{
			gui_printf("pwd: get current work directory failed!\n");
		}
	}
}

void cmd_cls(uint32_t argc, char** argv)
{
	//gui_printf("cls: argc %d\n", argc);
	if(argc != 1){
		gui_printf("cls: no argument support!\n");
		return;
	}
	window_clean_screen();
}

int cmd_mkdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		gui_printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		
		/*如果不是根目录*/
		if(strcmp(final_path, "c:/")){
			if(mkdir(final_path) == 0){
				gui_printf("mkdir: create a dir %s success.\n", final_path);
				ret = 0;
			}else{
				gui_printf("mkdir: create directory %s faild!\n", argv[1]);
			}
			
		}
		
	}
	return ret;
}

int cmd_rmdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		gui_printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(rmdir(final_path) == 0){
				gui_printf("rmdir: remove %s success.\n", final_path);
				ret = 0;
			}else{
				gui_printf("rmdir: remove %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

int cmd_rm(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		gui_printf("rm: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		//gui_printf("rm: path %s\n", final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(unlink(final_path) == 0){
				gui_printf("rm: delete %s success.\n", final_path);
				ret = 0;
			}else{
				gui_printf("rm: delete %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

void cmd_ls_sub(char *pathname, int detail)
{
	struct dir *dir = opendir(pathname);
	if(dir == NULL){
		gui_printf("opendir failed!\n");
	}
	rewinddir(dir);
	
	struct inode_s inode;
	
	struct dir_entry *de;
	char type;
	de = readdir(dir);
	while(de != NULL){
		if(detail == 2){
			if(de->file_type == BOFS_FILE_TYPE_DIRECTORY){
				type = 'd';
			}else if(de->file_type == BOFS_FILE_TYPE_NORMAL){
				type = 'f';
			}else if(de->file_type == BOFS_FILE_TYPE_INVALID){
				type = 'i';
			}
			fs_load_inode(&inode, de->inode);
			
			gui_printf("%d/%d/%d ",
				DATA16_TO_DATE_YEA(inode.crttime>>16),
				DATA16_TO_DATE_MON(inode.crttime>>16),
				DATA16_TO_DATE_DAY(inode.crttime>>16));
			gui_printf("%d:%d:%d ",
				DATA16_TO_TIME_HOU(inode.crttime&0xffff),
				DATA16_TO_TIME_MIN(inode.crttime&0xffff),
				DATA16_TO_TIME_SEC(inode.crttime&0xffff));
			gui_printf("%c %d %s \n", type, inode.size, de->name);
		}else if(detail == 1){
			if(de->file_type != BOFS_FILE_TYPE_INVALID){
				if(de->file_type == BOFS_FILE_TYPE_DIRECTORY){
					type = 'd';
				}else if(de->file_type == BOFS_FILE_TYPE_NORMAL){
					type = 'f';
				}
				fs_load_inode(&inode, de->inode);
				
				gui_printf("%d/%d/%d ",
					DATA16_TO_DATE_YEA(inode.crttime>>16),
					DATA16_TO_DATE_MON(inode.crttime>>16),
					DATA16_TO_DATE_DAY(inode.crttime>>16));
				gui_printf("%d:%d:%d ",
					DATA16_TO_TIME_HOU(inode.crttime&0xffff),
					DATA16_TO_TIME_MIN(inode.crttime&0xffff),
					DATA16_TO_TIME_SEC(inode.crttime&0xffff));
				gui_printf("%c %d %s \n", type, inode.size, de->name);
			}
		}else if(detail == 0){
			if(de->file_type != BOFS_FILE_TYPE_INVALID){
				gui_printf("%s ", de->name);
			}
		}
		de = readdir(dir);
	}
	closedir(dir);
}

void cmd_lsdisk(uint32_t argc, char** argv)
{
	gui_printf("=====Disk Drive=====\n");
	gui_printf("-Drive-   -Type-\n");

	int drive_nr = 0;
	
	struct drive_s *buf = fs_drive_connect();

	do {
		if (buf != NULL) {
			if (buf->dev_id != -1) {
				gui_printf(" %c:        %s\n", \
					buf->name[1], buf->type_name);
				drive_nr++;	
			}
			
		}
		buf = fs_drive_get(buf);

	} while (buf != NULL);

	gui_printf("current disk number is %d.\n", drive_nr);

}

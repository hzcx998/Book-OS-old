#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/const.h>
#include <graphic.h>
#include "gui.h"
#include "cmd.h"
#include "bosh.h"

//var
char final_path[MAX_PATH_LEN] = {0}; // 用于清洗路径时的缓冲

char cmd_line[CMD_LINE_LEN] = {0};
char cwd_cache[MAX_PATH_LEN] = {0};
char *cmd_argv[MAX_ARG_NR];

char pipe_buf[PIPE_BUF_LEN] = {0};
int argc = -1;

int pipe_id;

int main(int argc, char *argv0[])
{
	//printf("bosh is start\n");
	/*
	初始化图形界面环境
	*/

	//初始化窗口信息
	if (init_window(WIN_WIDTH, WIN_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT) == -1) {
		printf("init window failed!\n");
		return -1;
	}

	//创建pipe，用于和子进程进行交互
	pipe_id = pipe_create(128);

	//printf("bosh pipe id %d\n", pipe_id);

	memset(cwd_cache, 0, MAX_PATH_LEN);
	//strcpy(cwd_cache, "/");
	getcwd(cwd_cache, 32);
	//gui_printf("cwd:%s\n", cwd_cache);
	
	int arg_idx = 0;

	int next_line = 1;
	int status = 0;
	int daemon = 0;	/*后台程序*/
	
	//clear();
	while(1){ 
		print_prompt();
		memset(cmd_line, 0, CMD_LINE_LEN);
		
		//读取命令行
		
		readline(cmd_line, CMD_LINE_LEN);
		


		if(cmd_line[0] == 0){
			continue;
		}
		
		argc = -1;
		argc = cmd_parse(cmd_line, cmd_argv, ' ');
		
		if(argc == -1){
			gui_printf("shell: num of arguments exceed %d\n",MAX_ARG_NR);
			continue;
		}

		arg_idx = 0;
		/*while(arg_idx < argc){
			//make_clear_abs_path(cmd_argv[arg_idx], buf);
			//gui_printf("%s ",buf);
			cmd_argv[arg_idx] = NULL;
			arg_idx++;
		}*/
		
		if(!strcmp("cls", cmd_argv[0])){
			cmd_cls(argc, cmd_argv);
			next_line = 0;
		}else if(!strcmp("pwd", cmd_argv[0])){
			cmd_pwd(argc, cmd_argv);
		}else if(!strcmp("cd", cmd_argv[0])){
			if(cmd_cd(argc, cmd_argv) != NULL){
				memset(cwd_cache,0, MAX_PATH_LEN);
				strcpy(cwd_cache, final_path);
			}
		}else if(!strcmp("ls", cmd_argv[0])){
			cmd_ls(argc, cmd_argv);
			
		}else if(!strcmp("ps", cmd_argv[0])){
			cmd_ps(argc, cmd_argv);
			
		}else if(!strcmp("help", cmd_argv[0])){
			cmd_help(argc, cmd_argv);
			
		}else if(!strcmp("mkdir", cmd_argv[0])){
			cmd_mkdir(argc, cmd_argv);
			
		}else if(!strcmp("rmdir", cmd_argv[0])){
			cmd_rmdir(argc, cmd_argv);
			
		}else if(!strcmp("rm", cmd_argv[0])){
			cmd_rm(argc, cmd_argv);
			
		}else if(!strcmp("echo", cmd_argv[0])){
			cmd_echo(argc, cmd_argv);
			
		}else if(!strcmp("type", cmd_argv[0])){
			cmd_type(argc, cmd_argv);
		}else if(!strcmp("cat", cmd_argv[0])){
			cmd_cat(argc, cmd_argv);	
		}else if(!strcmp("dir", cmd_argv[0])){
			cmd_dir(argc, cmd_argv);
		}else if(!strcmp("ver", cmd_argv[0])){
			cmd_ver(argc, cmd_argv);
		}else if(!strcmp("time", cmd_argv[0])){
			cmd_time(argc, cmd_argv);
		}else if(!strcmp("date", cmd_argv[0])){
			cmd_date(argc, cmd_argv);
		}else if(!strcmp("rename", cmd_argv[0])){
			cmd_rename(argc, cmd_argv);
		}else if(!strcmp("mv", cmd_argv[0])){
			cmd_move(argc, cmd_argv);
		}else if(!strcmp("copy", cmd_argv[0])){
			cmd_copy(argc, cmd_argv);
		}else if(!strcmp("reboot", cmd_argv[0])){
			cmd_reboot(argc, cmd_argv);
		}else if(!strcmp("exit", cmd_argv[0])){
			cmd_exit(argc, cmd_argv);
		}else if(!strcmp("mm", cmd_argv[0])){
			cmd_mm(argc, cmd_argv);
		}else if(!strcmp("lsdisk", cmd_argv[0])){
			cmd_lsdisk(argc, cmd_argv);
		}else if(!strcmp("kill", cmd_argv[0])){
			cmd_kill(argc, cmd_argv);
		}else{
			make_clear_abs_path(cmd_argv[0], final_path);
			//gui_printf("operate %s\n",final_path);
			//while(1);
			cmd_argv[argc] = NULL;
			
			int pid = execv(( char *)final_path, ( char **)cmd_argv);
			/*是否把进程当作后台程序运行*/
			arg_idx = 0;
			while(arg_idx < argc){
				if(!strcmp(cmd_argv[arg_idx], "&")){
					daemon = 1;
				}
				arg_idx++;
			}
			
			if(pid != -1){
				//run success
			
				//gui_printf("child pid %d.\n",pid);
				if(!daemon){
					//循环检测子进程是否为退出状态，如果是就跳出往后面不执行，不是就检测pipe数据
					//_wait(&status);
					while(wait_child_exit() == -1){
						//可以在这里面接收子进程发送的数据
						receive_child_message();

						//等待子程序时可以检测
						force_exit_check();
					}
					//gui_printf("child exit.\n");
				}
				
				//gui_printf("status %d.\n",status);
			}else{
				gui_printf("bosh: unknown cmd!");
			}
		}
		//如果是清屏就不换行
		
		if(next_line){
			gui_printf("\n");
		}
		next_line = 1;
		daemon = 0;	
	}
}

/*
c:/b/..
*/
void make_clear_abs_path(char *path, char *final_path)
{
	/*绝对路径变量*/
	char abs_path[MAX_PATH_LEN] = {0};
	
	/*绝对路径判断*/
	if(!(path[1] == ':' && path[2] == '/')){
		/*不是绝对路径，就从工作目录中读取并且生成目录*/
		memset(abs_path,0, MAX_PATH_LEN);
		/*从当前任务中获取*/
		if (!getcwd(abs_path, MAX_PATH_LEN)) {
			if (!((abs_path[1] == ':') && (abs_path[2] == '/') && (abs_path[3] == 0))) {
				/*若abs_path表示的当前目录不是根目录'/'，我们就会在后面添加一个'/'
				例如：c:/ 就不添加'/'
					c:/a 就要添加'/'变成c:/a/
				*/
				strcat(abs_path, "/");
			}
		}
		
	}
	/*在这儿把新的路径添加到绝对路径后面*/
	strcat(abs_path, path);

	/*解析每一个目录*/
	char name[MAX_FILE_NAME_LEN]= {0};
	memset(name, 0, MAX_FILE_NAME_LEN);
	char *path_sub = (char *)(abs_path + 2);
	/*解析出一个目录的名字*/
	path_sub = path_parse(path_sub, name);
	if(name[0] == 0){	//只有根目录"/"
		/*把根目录添加进去*/
		final_path[0] = *abs_path;
		final_path[1] = ':';
		final_path[2] = '/';
		final_path[3] = 0;
		
		return;
	}
	/*清空缓冲区*/
	memset(final_path, 0, MAX_PATH_LEN);
	/*填写磁盘符以及根目录*/
	final_path[0] = *abs_path;
	final_path[1] = ':';
	strcat(final_path, "/");
	while(name[0]){
		//如果是上一级目录
		if(!strcmp(name,"..")){
			/*判断是否是根目录后面的那个，获取最后面的'/'，返回它的指针位置*/
			char *slash_ptr = (char *)strrchr(final_path, '/');
			/*如果未到 final_path 中的顶层目录，就将最右边的'/'替换为 0，
			这样便去除了 final_path 中最后一层路径，相当于到了上一级目录 
			*/
			/*
			如果指针位置到达了第一个'/'，也就是磁盘符c:/后面的根目录，因此比较的时候
			就要比较final_path+2的位置
			*/
			if(slash_ptr != final_path+2){
				/* 如 final_path 为/a/b， ..之后则变为/a*/
				*slash_ptr = 0;
			}else{ /* 如 final_path 为/a， ..之后则变为/*/
				/*
				若 final_path 中只有 1 个'/'，即表示已经到了顶层目录,
				就将下一个字符置为结束符 0 
				*/
				*(slash_ptr + 1) = 0;
			}
			
		}else if(strcmp(name,".")){
			/*
			如果路径是. 就代表当前目录，则不管
			如果路径不是. ，就将 name 拼接到 final_path
			*/
			if (!(final_path[1] == ':' && final_path[2] == '/' && final_path[3] == 0)) {
				// 如果 final_path 不是"c:/"
				// 就拼接一个"/",此处的判断是为了避免路径开头变成这样"//"
				strcat(final_path, "/");
			}
			strcat(final_path, name);
		}
		/*清空名字后再次解析*/
		memset(name, 0, MAX_FILE_NAME_LEN);
		if(path_sub){
			path_sub = path_parse(path_sub, name);
		}
	}
	//gui_printf("finally %s\n", final_path);
}

int cmd_parse(char * cmd_str, char **argv, char token)
{
	if(cmd_str == NULL){
		return -1;
	}
	int arg_idx = 0;
	while(arg_idx < MAX_ARG_NR){
		argv[arg_idx] = NULL;
		arg_idx++;
	}
	char *next = cmd_str;
	int argc = 0;
	while(*next){
		//跳过token字符
		while(*next == token){
			next++;
		}
		//如果最后一个参数后有空格 例如"cd / "
		if(*next ==0){
			break;
		}
		//存入一个字符串地址，保存一个参数项
		argv[argc] = next;
		
		//每一个参数确定后，next跳到这个参数的最后面
		while(*next && *next != token){
			next++;
		}
		//如果此时还没有解析完，就把这个空格变成'\0'，当做字符串结尾
		if(*next){
			*next++ = 0;
		}
		//参数越界，解析失败
		if(argc > MAX_ARG_NR){
			return -1;
		}
		//指向下一个参数
		argc++;
		//让下一个字符串指向0
		argv[argc] = 0;
	}
	return argc;
}

void readline( char *buf, uint32_t count)
{
	char *pos = buf;
	while(read_key(buf, pos, count) && (pos - buf) < count){
		switch(*pos){
			case '\n':
				//当到达底部了就不在继续了，目前还没有设定
				*pos = 0;
				gui_printf("\n");
				return;
			case '\b':
				if(buf[0] != '\b'){
					--pos;
					gui_printf("\b");
				}
				break;
			default:
				gui_printf("%c", *pos);
				pos++;
		}
	}
	gui_printf("\nreadline: error!\n");
}

int read_key(char *start, char *buf, int count)
{
	//从键盘获取按键
	int key, modi;;
	int status;
	//循环获取按键

	do {
		status = gui_keyboard(&key);
		
		if (status != -1) {
			//获取按键
			//读取控制键状态
			modi = gui_get_modifiers();
			
			if(key == GUI_KEY_F5) {
				//F5 刷新窗口，清屏
				window_clean_screen();

			} else if(key == GUI_KEY_UP) {
				//ctrl + UP
				if (modi&GUI_ACTIVE_CTRL) {
					//window_scroll(SCROLL_UP, 1, 0);
				}
				//UP
				//cursor_move(cursor.x, cursor.y-1);
			} else if(key == GUI_KEY_DOWN){
				//ctrl + DOWN
				if (modi&GUI_ACTIVE_CTRL) {
					//window_scroll(SCROLL_DOWN, 1, 0);
				}
				//DOWN
				//cursor_move(cursor.x, cursor.y+1);
			} else if(key == GUI_KEY_LEFT){

				//LEFT
				
				
				//cursor_move(cursor.x-1, cursor.y);

			} else if(key == GUI_KEY_RIGHT){
				
				//RIGHT
				
				//cursor_move(cursor.x+1, cursor.y);
				
			/*
			ASICLL键码
			*/
			} else if ((0 < key && key < 255) || key&FLAG_PAD) {
				//是一般字符就传输出去
				*buf = (char)key;
				
				break;
			}
			
			key = -1;
		}
		//读取按键时可以检测
		force_exit_check();
	}while (status == -1);
	
	return 1;
}

void force_exit_check()
{
	//如果线程取消
	if (thread_testcancel()) {
		pipe_close(pipe_id);
		//printf("close pipe %d\n", pipe_id);
		gui_window_close();
		//printf("close window\n");
		exit(0);
	}
}

/*
int read_key(char *buf)
{
	//从键盘获取按键
	char key;
	do{
		key = getchar();
	}while(key == -1);
	*buf = key;
	return 1;
}*/

/* 
将最上层路径名称解析出来
传入的路径不能有磁盘符c:
只能是/a/b...
*/
char* path_parse(char* pathname, char* name_store) 
{
   if (pathname[0] == '/') {   // 根目录不需要单独解析
    /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
       while(*(++pathname) == '/');
   }

   /* 开始一般的路径解析 */
   while (*pathname != '/' && *pathname != 0) {
      *name_store++ = *pathname++;
   }
   if (pathname[0] == 0) {   // 若路径字符串为空则返回NULL
      return NULL;
   }
   return pathname; 
}

void print_prompt()
{
	gui_printf("%s>", cwd_cache);
}

int wait(int *status)
{
	int pid;
	do{
		pid = _wait(status);
	}while(pid == -1);
	return pid;
}

void receive_child_message()
{
	//gui_printf("get message");
	memset(pipe_buf, 0, PIPE_BUF_LEN);
	//读取pipe
	if (pipe_read(pipe_id, pipe_buf) >= 0) {
		//获取信息成功，显示pipe消息
		gui_printf("%s", pipe_buf);
	} else {
		
	}

}


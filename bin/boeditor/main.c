#include <stdio.h>
#include <stdlib.h>
#include <graphic.h>
#include <string.h>


#define CHAR_WIDTH	8
#define CHAR_HEIGHT	16
/*
#define padInfo.char_lines	40
#define padInfo.char_culomns	12
*/
#define NAME_FILE_LEN	12
#define NAME_STATUS_LEN	12

#define MSG_ERR	1
#define MSG_TIP	2

#define MAX_ARGS_NR	16

struct Cursor
{
	int x,y;
	uint32_t color;
};

struct Cursor cursor;

struct FileInfo
{
	char name[NAME_FILE_LEN];
	uint8 *buffer;
	uint8 *text;
	int size;
};

static struct FileInfo fileInfo;

struct PadInfo
{
	uint8 *charBuffer;
	uint32_t charColor, bgColor;
	uint32_t existColor, enterColor;
	uint32_t statusColor,errorColor, tipColor;
	int charLength;
	char status[NAME_STATUS_LEN];
	char *argv[MAX_ARGS_NR];
	int char_lines, char_culomns;
	
};

#define WINDOW_NAME "boeditor"

#define BACK_COLOR ARGB(255,240,240,240)
#define FRONT_COLOR ARGB(255,0,0,0)

#define WIN_WIDTH 480
#define WIN_HEIGHT 360

/*
用一个窗口结构体保存所有窗体信息
*/
struct window_s
{
	int width, height;	//窗口的宽高
	uint32_t bcolor;	//背景颜色
	uint32_t fcolor;	//前景颜色
}window;


static struct PadInfo padInfo;

static int initAll();
static void myKeyboard(int key, int x, int y);

static void charWrite(int x, int y, uint8 ch);
static void charClear(int x, int y);
static uint8 charRead(int x, int y);
static void charShow();
static void makeFileBuf();
static int writeFileBuf();
static int readFileBuf();
static void makeCharBuf();
static void loadFram();
static void cleanCharBuf();
static void cleanFileBuf();
static void resetAll();
static void showStatus();
static int getCharLength();
static void putMessage(int type, char *msg);
static void cursorClear();
static void cursorDraw();
static void cursorSpecial(uint32_t color);
void force_exit_check();
int initWindow(int width, int height);

int screen_width, screen_height;
/*
F6:read
F7:write
F12:reset
*/

int main(int argc, char *argv[])
{
	//init_graphic();
	
	if (initAll() == -1) {
		return -1;
	}
	cursorDraw();
	showStatus();
	
	int key;
	while(1){
		force_exit_check();
		if(!gui_keyboard(&key)){
			myKeyboard(key, 0, 0);
		}
	}
	
	return 0;
}

static int initAll()
{
	cursor.x = 0;
	cursor.y = 0;
	cursor.color = ARGB(225,0xff,0xff,0xff);
	padInfo.charColor = ARGB(225,0xff,0xff,0xff);
	padInfo.bgColor = ARGB_POTATO_RED;
	padInfo.existColor  = ARGB(225,0,128,192);
	padInfo.enterColor  = ARGB(225,128,128,192);
	padInfo.statusColor  = ARGB(225,100,240,60);
	padInfo.errorColor  = ARGB(225,225,0,0);
	padInfo.tipColor  = ARGB(225, 0,225,0);
	
	//get_screen(&screen_width,&screen_height);
	if (initWindow(WIN_WIDTH, WIN_HEIGHT) == -1) {
		printf("beditor init window failed!\n");
		return -1;
	}
	padInfo.char_lines = window.width/CHAR_WIDTH-1;
	padInfo.char_culomns = window.height/CHAR_HEIGHT - 1;
	
	padInfo.charBuffer = malloc(padInfo.char_lines*padInfo.char_culomns);
	fileInfo.buffer = malloc(padInfo.char_lines*padInfo.char_culomns);
	int i;
	for(i = 0; i < MAX_ARGS_NR; i++){
		padInfo.argv[i] = NULL;
	}
	
	return 0;
}

int initWindow(int width, int height)
{
	window.width = width;
	window.height = height;
	
	//背景色为黑色
	window.bcolor = ARGB_POTATO_RED;
	//前景色为白色
	window.fcolor = FRONT_COLOR;
	
	if (gui_init() == -1) {
		return -1;
	}
	gui_window_size(window.width, window.height);
	gui_window_position(100, 24);
	if (gui_window_create(WINDOW_NAME) == -1) {
		return -1;
	}

	//清空背景
	gui_color(window.bcolor);
	gui_rect(0, 0, window.width, window.height);

	//全部显示出来
	gui_draw(0,0,window.width, window.height);

	return 0;
}

static void cursorClear()
{
	gui_color(padInfo.bgColor);
	gui_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
	gui_draw(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
}

static void cursorDraw()
{
	gui_color(cursor.color);
	gui_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT + 14, CHAR_WIDTH, 2);
	gui_draw(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT + 14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT + 16);
}

static void cursorSpecial(uint32_t color)
{
	gui_color(color);
	gui_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
	gui_draw(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT + 16);
}

static void myKeyboard(int key, int x, int y)
{
	if(key == GUI_KEY_F4){
		free(padInfo.charBuffer);
		free(fileInfo.buffer);
		graphic_exit();
		exit(1);
	}else if(key == GUI_KEY_UP){
		//擦去光标和字符
		cursorClear();
		
		//显示字符
		
		charShow();
		cursor.y--;
		
		if(cursor.y < 0){
			cursor.y = 0;
		}
		//擦去光标和字符
		cursorClear();
		
		//显示字符
		charShow();
		
		//显示光标
		cursorDraw();
	}else if(key == GUI_KEY_DOWN){
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		cursor.y++;
		if(cursor.y > padInfo.char_culomns-1){
			cursor.y = padInfo.char_culomns-1;
		}
		
		//擦去光标和字符
		cursorClear();
		
		//显示字符
		charShow();
		
		//显示光标
		cursorDraw();
	}else if(key == GUI_KEY_LEFT){
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		
		cursor.x--;
		if(cursor.x < 0){
			if(cursor.y > 0){
				cursor.y--;
				cursor.x = padInfo.char_lines-1;
			}else{
				cursor.x = 0;
			}
		}else{
			//擦去光标和字符
			cursorClear();
			//显示字符
			charShow();
		
		}
		
		//显示光标
		cursorDraw();
	}else if(key == GUI_KEY_RIGHT){
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		
		cursor.x++;
		if(cursor.x > padInfo.char_lines-1){
			cursor.x = 0;
			if(cursor.y < padInfo.char_culomns-1){
				cursor.y++;
			}else{
				cursor.y = padInfo.char_culomns-1;
				cursor.x = padInfo.char_lines;
			}
		}
		
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		
		//显示光标
		cursorDraw();
		
	}else if(key == GUI_KEY_HOME){
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		
		cursor.x = 0;
		
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		//显示光标
		cursorDraw();
	}else if(key == GUI_KEY_END){
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		
		cursor.x = padInfo.char_lines-1;
		
		//擦去光标和字符
		cursorClear();
		//显示字符
		charShow();
		//显示光标
		cursorDraw();
	}else if(key == GUI_KEY_F6){
		makeFileBuf();
		readFileBuf();
	}else if(key == GUI_KEY_F7){
		makeFileBuf();
		writeFileBuf();
	}else if(key == GUI_KEY_F12){	
		resetAll();
	}else if(key == '\n'){
		//擦去光标
		cursorSpecial(padInfo.enterColor);
		
		charWrite(cursor.x, cursor.y, key);
		charShow();
		if(cursor.y < padInfo.char_culomns-1){
			cursor.x = 0;
			cursor.y++;
		}
		//显示光标
		cursorDraw();
	
	}else if(key == '\b'){
		//擦去光标
		cursorClear();
		cursor.x--;
		if(cursor.x < 0 ){
			if(cursor.y > 0){
				cursor.y--;
				cursor.x = padInfo.char_lines-1;
			}else{
				cursor.x = 0;
			}
		}
		charClear(cursor.x, cursor.y);
		
		//擦除字符
		cursorClear();
		//显示光标
		cursorDraw();
	
	}else{
		//擦去光标
		cursorClear();
		
		charWrite(cursor.x, cursor.y, key);
		//显示字符
		charShow();
		cursor.x++;
		if(cursor.x > padInfo.char_lines-1){
			cursor.x = 0;
			if(cursor.y < padInfo.char_culomns-1){
				cursor.y++;
			}else{
				
				cursor.x = padInfo.char_lines-1;
			}
			
		}
		//显示光标
		cursorDraw();
	
	}
	showStatus();
}

static void charWrite(int x, int y, uint8 ch)
{
	
	padInfo.charBuffer[y*padInfo.char_lines+x] = ch;
}
static void charClear(int x, int y)
{
	
	padInfo.charBuffer[y*padInfo.char_lines+x] = 0;
}

static uint8 charRead(int x, int y)
{
	return padInfo.charBuffer[y*padInfo.char_lines+x];
}

static void charShow()
{
	char s[2];
	s[1] = '\0';

	s[0] = charRead(cursor.x, cursor.y);
	if(s[0] != '\0'){
		if(s[0] == '\n'){
			//是回车
			cursorSpecial(padInfo.enterColor);
		
		}else if(s[0] != '\0'){
			/*gui_color(padInfo.existColor);
			gui_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
			gui_draw(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
			*/
			//不是回车就显示一个存在
			cursorSpecial(padInfo.existColor);
		
		}
		if(s[0] != '\n' ){
			gui_color(padInfo.charColor);
			gui_text(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, s);
			gui_draw(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
			
		}
	}
}

static void makeFileBuf()
{
	int i,j;
	uint8 ch;
	i = 0;
	j = 0;
	
	cleanFileBuf();
	
	while(i < padInfo.char_culomns*padInfo.char_lines){
		ch = padInfo.charBuffer[i];
		if(ch != 0){
			fileInfo.buffer[j] = ch;
			j++;
		}
		i++;
	}
	
	//在最后加上0
	fileInfo.buffer[j] = 0;

	int pos = strpos((char *)fileInfo.buffer, '\n');
	memset(fileInfo.name, 0, NAME_FILE_LEN);
	strncpy((char *)fileInfo.name, (char *)fileInfo.buffer, pos);
	fileInfo.name[pos] = 0;
	
	//1 is '\n', we skip it
	fileInfo.text = fileInfo.buffer+pos+1;

	fileInfo.size = j - (pos+1);

	putMessage(MSG_TIP, "FILE BUF DONE");
	
}

static void makeCharBuf()
{
	//导入内容
	int i,j;
	uint8 ch;
	i = padInfo.char_lines;
	j = 0;
	int odd;
	while(i < padInfo.char_culomns*padInfo.char_lines && fileInfo.buffer[j]){
		//获取一个字符
		ch = fileInfo.buffer[j];
		padInfo.charBuffer[i] = ch;
		if(ch == '\n'){
			odd = i%padInfo.char_lines;
			i -= odd;
			i += padInfo.char_lines;
		}else{
			i++;
		}
		j++;
		
	}
	//文件内容指向读入的数据
	fileInfo.text = fileInfo.buffer;
	
	putMessage(MSG_TIP, "CHAR BUF DONE");
}

static int writeFileBuf()
{
	if(fileInfo.name[1] != ':'){
		putMessage(MSG_ERR, "NO DRIVE");
		return -1;
	}
	
	if(fileInfo.name[2] != '/' || fileInfo.name[3] == 0){
		putMessage(MSG_ERR, "NO NAME");
		return -1;
	}
	int fd = fopen(fileInfo.name, O_CREAT|O_RDWR);
	if(fd == -1){
		putMessage(MSG_ERR, "OPEN FAILED");
		return -1;
	}
	
	int write = fwrite(fd, fileInfo.text, fileInfo.size);
	if(write == -1){
		putMessage(MSG_ERR, "WRITE FAILED");
		fclose(fd);
		return -1;
	}
	fclose(fd);
	putMessage(MSG_TIP, "WRITE DONE");
	return 0;
}

static int readFileBuf()
{
	
	if(fileInfo.name[0] == '\0' || fileInfo.name[1] != ':'){
		putMessage(MSG_ERR, "NO DRIVE");
		return -1;
	}
	
	if(fileInfo.name[2] != '/' || fileInfo.name[3] == 0){
		putMessage(MSG_ERR, "NO NAME");
		return -1;
	}
	int fd = fopen(fileInfo.name, O_RDWR);
	
	if(fd == -1){
		
		putMessage(MSG_ERR, "OPEN FAILED");
		return -1;
	}
	
	struct stat stat;
	fstat(fileInfo.name, &stat);
	/*
	char s[40];
	sprintf(s,"file size:%d",stat.st_size);
	gluRectangle(0, padInfo.char_culomns*CHAR_HEIGHT+16, padInfo.char_lines*CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
	gluString(0, padInfo.char_culomns*CHAR_HEIGHT+16, s, COLOR_RED);
	*/
	//printf("read file size:%d\n", stat.st_size);
	
	cleanFileBuf();
	
	int read = fread(fd, fileInfo.buffer, stat.st_size);
	if(read == -1){
		putMessage(MSG_ERR, "RED FAILED");
		fclose(fd);
		return -1;
	}
	
	fileInfo.size = read;
	fileInfo.text = fileInfo.buffer;
	
	putMessage(MSG_TIP, "READ DONE");
	
	fclose(fd);
	//printf("file load sucess!\n");
	makeCharBuf();
	loadFram();
	return 0;
}

static void loadFram()
{
	//先清空
	gui_color(padInfo.bgColor);
	gui_rect(0, 0, padInfo.char_lines*CHAR_WIDTH, padInfo.char_culomns*CHAR_HEIGHT);
	gui_draw(0, 0, padInfo.char_lines*CHAR_WIDTH, padInfo.char_culomns*CHAR_HEIGHT);
	
	//导入内容
	char s[2];
	s[1] = '\0';
	int x,y;
	//加载的时候，没有改变光标位置，所以绘制图形只能自己实现
	for(y = 0; y < padInfo.char_culomns; y++){
		for(x = 0; x < padInfo.char_lines; x++){
			s[0] = charRead(x,y);
			if(s[0] == '\n'){
				gui_color(padInfo.enterColor);
				gui_rect(x*CHAR_WIDTH, y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
				gui_draw(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
				//cursorSpecial(padInfo.enterColor);
			}else if(s[0] != '\0'){
				gui_color(padInfo.existColor);
				gui_rect(x*CHAR_WIDTH, y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
				gui_draw(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
				//cursorSpecial(padInfo.existColor);
			}
			if(s[0] != '\n' && s[0] != '\0'){
				
				gui_color(padInfo.charColor);
				
				gui_text(x*CHAR_WIDTH, y*CHAR_HEIGHT, s);
				gui_draw(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
				
			}
		}
	}
	
	//显示光标
	cursorDraw();
	//刷新
	
}

static void cleanCharBuf()
{
	memset(padInfo.charBuffer, 0, padInfo.char_lines*padInfo.char_culomns);
}


static void cleanFileBuf()
{
	memset(fileInfo.buffer, 0, padInfo.char_lines*padInfo.char_culomns);
}

static void resetAll()
{
	cleanCharBuf();
	cleanFileBuf();
	cursor.x = 0;
	cursor.y = 0;
	fileInfo.text = fileInfo.buffer;
	fileInfo.size = 0;
	makeCharBuf();
	loadFram();
	
}

static void showStatus()
{
	gui_color(padInfo.bgColor);
	gui_rect(0, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120, CHAR_HEIGHT);
	gui_draw(0, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT+CHAR_HEIGHT);
	
	padInfo.charLength = getCharLength();
	
	char s[64];
	sprintf(s,"length:%d Ln:%d Col:%d",padInfo.charLength, cursor.y,cursor.x);
	gui_color(padInfo.statusColor);
	gui_text(0, padInfo.char_culomns*CHAR_HEIGHT, s);
	
	gui_draw(0, padInfo.char_culomns*CHAR_HEIGHT, CHAR_WIDTH*padInfo.char_lines,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
}

static int getCharLength()
{
	int i = 0, j = 0;
	while(i < padInfo.char_culomns*padInfo.char_lines){
		if(padInfo.charBuffer[i] != 0){
			j++;
		}
		i++;
	}
	return j;
} 

static void putMessage(int type, char *msg)
{
	int len = strlen(msg);
	gui_color(padInfo.bgColor);
	gui_rect(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, 120, CHAR_HEIGHT);
	if(type == MSG_ERR){
		gui_color(padInfo.errorColor);
		gui_text(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, msg);
		gui_draw(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120+len*CHAR_WIDTH,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
	
	}else if(type == MSG_TIP){
		gui_color(padInfo.tipColor);
		gui_text(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, msg);
		gui_draw(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120+len*CHAR_WIDTH,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
	}
}

void force_exit_check()
{
	//如果线程取消
	if (thread_testcancel()) {
		
		gui_window_close();
		
		exit(1);
	}
}

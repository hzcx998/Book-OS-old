/**
 * ===================NNOS-Devices============================
 *【蜂鸣音乐播放器】（支持MML文件）
 * -----------------------------------------------------------
 * ·日期：2019-3-22     改进者：年年软件 原作者：川合秀实
 * ===========================================================
**/

//#include "api.h" //【移植点：更改为BookOS的API头文件】,已兼容

//#include <string.h>	//strlen()，已兼容

int strtol(char *s, char **endp, int base);	//标准函数（stdlib.h）
void waittimer(int timer, int time);
void end(char *s);

void NNOSMain(){
	char winbuf[256 * 112], txtbuf[100 * 1024];
	char s[32], *p, *r;
	int win, timer, i, j, t = 120, l = 192 / 4, o = 4, q = 7, note_old = 0;

	/*
	音号与频率（mHz）对照表
	如，04A为440Hz（国际标准音），即440000
	第16八度的A为1802240Hz，即1802240000
	以下为第168度的列表（C~B）
	*/
	static int tonetable[12] = {
		1071618315, 1135340056, 1202850889, 1274376125, 1350154473, 1430438836,
		1515497155, 1605613306, 1701088041, 1802240000, 1909406767, 2022946002
	};
	static int notetable[7] = { +9, +11, +0 /* C */, +2, +4, +5, +7 };

	api_command(s, 30); //命令行解析
	for (p = s; *p > ' '; p++) { } //读到空格为止
	for (; *p == ' '; p++) { }	//跳过空格
	i = strlen(p);
	if (i > 12) {
file_error:
		end("file open error.\n");
	}
	if (i == 0) {
		end(0);
	}

	win = api_window(winbuf, 256, 112, -1, "Music Player"); //创建窗口 //【移植点：更改为BookOS的窗口创建API】，已兼容
	if(win == -1){
		printf("invader init window failed!\n");
		return -1;	
	}
	api_ascwin(win, 128, 32, 3, i, p);  //【移植点：更改为BookOS的字符串绘制API】
	api_boxwin(win, 8, 60, 247,  76, 17);  //【移植点：更改为BookOS的色块/矩形绘制API】
	api_boxwin(win, 10, 62, 245,  74, 23); //【移植点：更改为BookOS的色块/矩形绘制API】
	api_boxwin(win, 6, 86, 249, 105, 20); //【移植点：更改为BookOS的色块/矩形绘制API】

	i = api_fopen(p); //载入文件 //【移植点：更改为BookOS的文件打开API】
	if (i == 0) {
		goto file_error;
	}
	j = api_fsize(i, 0); //【移植点：更改为BookOS的文件大小获取API】
	if (j >= 100 * 1024) {
		j = 100 * 1024 - 1;
	}
	api_fread(txtbuf, j, i); //【移植点：更改为BookOS的文件读取API】
	api_fclose(i); //【移植点：更改为BookOS的文件关闭API】
	txtbuf[j] = 0;
	r = txtbuf;
	i = 0; //普通模式
	for (p = txtbuf; *p != 0; p++) {	//简化处理，去掉注释和空白
		if (i == 0 && *p > ' ') {	//非空格非换行
			if (*p == '/') {
				if (p[1] == '*') {
					i = 1;
				} else if (p[1] == '/') {
					i = 2;
				} else {
					*r = *p;
					if ('a' <= *p && *p <= 'z') {
						*r += 'A' - 'a';	//小写字母转为大写字母
					}
					r++;
				}
			} else if (*p == 0x22) {
				*r = *p;
				r++;
				i = 3;
			} else {
				*r = *p;
				r++;
			}
		} else if (i == 1 && *p == '*' && p[1] == '/') {	//段注释
			p++;
			i = 0;
		} else if (i == 2 && *p == 0x0a) {	//行注释
			i = 0;
		} else if (i == 3) {	//字符串
			*r = *p;
			r++;
			if (*p == 0x22) {
				i = 0;
			} else if (*p == '%') {
				p++;
				*r = *p;
				r++;
			}
		}
	}
	*r = 0;

	timer = api_alloctimer(); //初始化定时器 //【移植点：更改为BookOS的创建定时器API】
	api_inittimer(timer, 128); //【移植点：更改为BookOS的初始化定时器API】

	p = txtbuf; //程序主体
	for (;;) {
		if (('A' <= *p && *p <= 'G') || *p == 'R') { //音符、休止符
			if (*p == 'R') { //计算频率
				i = 0;
				s[0] = 0;
			} else {
				i = o * 12 + notetable[*p - 'A'] + 12;
				s[0] = 'O';
				s[1] = '0' + o;
				s[2] = *p;
				s[3] = ' ';
				s[4] = 0;
			}
			p++;
			if (*p == '+' || *p == '-' || *p == '#') {
				s[3] = *p;
				if (*p == '-') {
					i--;
				} else {
					i++;
				}
				p++;
			}
			if (i != note_old) {
				api_boxwin(win + 1, 32, 36, 63, 51, 8); //【移植点：更改为BookOS的色块/矩形绘制API】
				if (s[0] != 0) {
					api_ascwin(win + 1, 32, 36, 26, 4, s); //【移植点：更改为BookOS的字符串绘制API】
				}
				api_refreshwin(win, 32, 36, 64, 52); //【移植点：更改为BookOS的窗口刷新API】
				if (28 <= note_old && note_old <= 107) {
					//api_boxwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10,  76, 4);
					//api_boxwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10,  61, 17);
					//api_boxwin(win, (note_old - 28) * 3 + 8, 62, (note_old - 28) * 3 + 10,  74, 4);
					api_boxwin(win, (note_old - 28) * 3 + 8, 62, (note_old - 28) * 3 + 10,  74, *p); //【移植点：更改为BookOS的色块/矩形绘制API】
					//api_boxwin(win, (note_old - 28) * 3 + 8, 75, (note_old - 28) * 3 + 10,  76, 17);
				}
				if (28 <= i && i <= 107) {
					//api_boxwin(win, (i - 28) * 3 + 8, 60, (i - 28) * 3 + 10,  76, 1);
					api_boxwin(win, (i - 28) * 3 + 8, 62, (i - 28) * 3 + 10,  74, 1); //【移植点：更改为BookOS的色块/矩形绘制API】
				}
				if (s[0] != 0) {
					api_beep(tonetable[i % 12] >> (17 - i / 12)); //【移植点：更改为BookOS的创建蜂鸣发声器API】
				} else {
					api_beep(0); //【移植点：更改为BookOS的创建蜂鸣发声器API】
				}
				note_old = i;
			}
			if ('0' <= *p && *p <= '9') { //计算音长
				i = 192 / strtol(p, &p, 10);
			} else {
				i = l;
			}
			for (; *p == '.'; ) {
				p++;
				i += i / 2;
			}
			i *= (60 * 100 / 48);
			i /= t;
			if (s[0] != 0 && q < 8 && *p != '&') {
				j = i * q / 8;
				waittimer(timer, j);
				api_boxwin(win, 32, 36, 63, 51, 8); //【移植点：更改为BookOS的色块/矩形绘制API】
				if (28 <= note_old && note_old <= 107) {
					//api_boxwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10, 61, 17);
					//api_boxwin(win, (note_old - 28) * 3 + 8, 60, (note_old - 28) * 3 + 10,  76, 18);
					//api_boxwin(win, (note_old - 28) * 3 + 8, 62, (note_old - 28) * 3 + 10,  74, 18);
					api_boxwin(win, (note_old - 28) * 3 + 8, 62, (note_old - 28) * 3 + 10,  74,*p); //【移植点：更改为BookOS的色块/矩形绘制API】
					//api_boxwin(win, (note_old - 28) * 3 + 8, 75, (note_old - 28) * 3 + 10,  76, 17);
				}
				note_old = 0;
				api_beep(0); //【移植点：更改为BookOS的蜂鸣发声器创建API】
			} else {
				j = 0;
				if (*p == '&') {
					p++;
				}
			}
			waittimer(timer, i - j);
		} else if (*p == '<') {	//八度--
			p++;
			o--;
		} else if (*p == '>') {	//八度++
			p++;
			o++;
		} else if (*p == 'O') { //指定八度
			o = strtol(p + 1, &p, 10);
		} else if (*p == 'Q') {	 //指定Q参数
			q = strtol(p + 1, &p, 10);
		} else if (*p == 'L') { //指定音长
			l = strtol(p + 1, &p, 10);
			if (l == 0) {
				goto syntax_error;
			}
			l = 192 / l;
			for (; *p == '.'; ) {
				p++;
				l += l / 2;
			}
		} else if (*p == 'T') { //指定播放速度
			t = strtol(p + 1, &p, 10);
		} else if (*p == '$') {	//扩展命令
			if (p[1] == 'K') {	//卡拉OK命令
				p += 2;
				for (; *p != 0x22; p++) {
					if (*p == 0) {
						goto syntax_error;
					}
				}
				p++;
				for (i = 0; i < 32; i++) {
					if (*p == 0) {
						goto syntax_error;
					}
					if (*p == 0x22) {
						break;
					}
					if (*p == '%') {
						s[i] = p[1];
						p += 2;
					} else {
						s[i] = *p;
						p++;
					}
				}
				if (i > 30) {
					end("Karaoke too long.\n");
				}
				api_boxwin(win + 1, 8, 88, 247, 103, 20); //【移植点：更改为BookOS的色块/矩形绘制API】
				s[i] = 0;
				if (i != 0) {
					api_ascwin(win + 1, 128 - i * 4, 88, 3, i, s); //【移植点：更改为BookOS的字符串绘制API】
				}
				api_refreshwin(win, 8, 88, 248, 104); //【移植点：更改为BookOS的窗口刷新API】
			}
			for (; *p != ';'; p++) {
				if (*p == 0) {
					goto syntax_error;
				}
			}
			p++;
		} else if (*p == 0) {
			p = txtbuf;
		} else {
syntax_error:
			end("Music player syntax error.\n");
		}
	}
}

void waittimer(int timer, int time)
{
	int i;
	api_settimer(timer, time); //【移植点：更改为BookOS的定时器创建API】
	for (;;) {
		i = api_getkey(1); //【移植点：更改为BookOS的单字符输入API】
		if (i == 'Q' || i == 'q') {
			api_beep(0); //【移植点：更改为BookOS的蜂鸣发声器创建API】
			api_return(); //【移植点：更改为BookOS的应用程序返回API】
		}
		if (i == 128) {
			return;
		}
	}
}

void end(char *s)
{
	if (s != 0) {
		api_printl(s); //【移植点：更改为BookOS的字符串输出API】
	}
	api_beep(0); //【移植点：更改为BookOS的蜂鸣发声器创建API】
	api_return(); //【移植点：更改为BookOS的应用程序返回API】
}

//api兼容
int api_window(char *winbuf,int width,int height,int focus,char *title){
	init_window(int width, int height);
	gui_window_size(window.width,window.height);
	gui_window_position(100, 24);
	
	return gui_window_create(WINDOW_NAME);
}

int init_window(int width, int height)
{
	window.width = width;
	window.height = height;
	
	//背景色为黑色
	window.bcolor = BACK_COLOR;
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

void force_exit_check()
{
	//如果线程取消
	if (thread_testcancel()) {
		gui_window_close();

		exit(1);
	}
}

/*
File:		device/keyboard.c
Contains:	driver program for keyboard
Auther:		Hu Zicheng
Time:		2019/1/29
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_DEVIVE_KEYBOARD_

#include <sys/dev.h>
#include <sys/arch.h>
#include <sys/core.h>

#include <sys/gui.h>

uint8_t get_byte_from_kbuf() ;

static void kb_wait();
static void kb_ack();
static void set_leds();

void key_char_process(uint32_t key);
void put_key(uint32_t key);
void wait_KBC_sendready(void);

struct keyboard keyboard;

void init_keyboard()
{
	printk("> init keyboard start.\n");
	
	keyboard.code_with_E0 = 0;
	
	keyboard.shift_l	= keyboard.shift_r = 0;
	keyboard.alt_l	= keyboard.alt_r   = 0;
	keyboard.ctrl_l	= keyboard.ctrl_r  = 0;
	
	keyboard.caps_lock   = 0;
	keyboard.num_lock    = 1;
	keyboard.scroll_lock = 0;
	
	keyboard.key = -1;
	
	keyboard.ioqueue = create_ioqueue();
	ioqueue_init(keyboard.ioqueue);
	
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
	
	printk("< init keyboard done.\n");
}

void thread_keyboard(void *arg)
{
	//printk("running in keyboard\n");
	thread_bus.keyboard = 1;
	
	while(1){
		//对原始数据进行处理
		keyboard_analysis();

		//与gui的交互
		#ifdef _CONFIG_GUI_BOGUI_

			//读取按键，读取后把按键设置为NONE
			if (keyboard.key != -1) {
				//bogui_container_manager->current_container->keyboard_data = keyboard.key;
				//printk("%c ", keyboard.key);

				if (bogui_container_manager->current_container->keyboard_listen != NULL) {
					//执行监听内容
					bogui_container_manager->current_container->keyboard_listen(\
						bogui_container_manager->current_container, keyboard.key);
				}
				keyboard.key = -1;
				
			}
			
		#endif //_CONFIG_GUI_BOGUI_
	}
}

void keyboard_handler(int32_t irq)
{
	uint8_t scan_code = io_in8(KB_DATA);
	ioqueue_put(keyboard.ioqueue, scan_code);
	
}

void wait_KBC_sendready()
{
	/*等待键盘控制电路准备完毕*/
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}



/*
把数据交给keyboard处理
*/

void keyboard_analysis()
{
	uint8_t scan_code;
	int make;
	
	uint32_t key = 0;
	uint32_t* keyrow;

		keyboard.code_with_E0 = 0;

		scan_code = get_byte_from_kbuf();
		
		if(scan_code == 0xe1){
			int i;
			uint8_t pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			int is_pausebreak = 1;
			for(i=1;i<6;i++){
				if (get_byte_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		} else if(scan_code == 0xe0){
			scan_code = get_byte_from_kbuf();

			//PrintScreen 被按下
			if (scan_code == 0x2A) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			//PrintScreen 被释放
			if (scan_code == 0xB7) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			//不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. 
			if (key == 0) {
				keyboard.code_with_E0 = 1;
			}
		}if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			make = (scan_code & FLAG_BREAK ? 0 : 1);

			//先定位到 keymap 中的行 
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			
			keyboard.column = 0;
			int caps = keyboard.shift_l || keyboard.shift_r;
			if (keyboard.caps_lock) {
				if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
					caps = !caps;
				}
			}
			if (caps) {
				keyboard.column = 1;
			}

			if (keyboard.code_with_E0) {
				keyboard.column = 2;
			}
			
			key = keyrow[keyboard.column];
			
			switch(key) {
			case SHIFT_L:
				keyboard.shift_l = make;
				break;
			case SHIFT_R:
				keyboard.shift_r = make;
				break;
			case CTRL_L:
				keyboard.ctrl_l = make;
				break;
			case CTRL_R:
				keyboard.ctrl_r = make;
				break;
			case ALT_L:
				keyboard.alt_l = make;
				break;
			case ALT_R:
				keyboard.alt_l = make;
				break;
			case CAPS_LOCK:
				if (make) {
					keyboard.caps_lock   = !keyboard.caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					keyboard.num_lock    = !keyboard.num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					keyboard.scroll_lock = !keyboard.scroll_lock;
					set_leds();
				}
				break;	
			default:
				break;
			}
			
			if (make) { //忽略 Break Code
				int pad = 0;

				//首先处理小键盘
				if ((key >= PAD_SLASH) && (key <= PAD_9)) {
					pad = 1;
					switch(key) {
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (keyboard.num_lock &&
						    (key >= PAD_0) &&
						    (key <= PAD_9)) 
						{
							key = key - PAD_0 + '0';
						}else if (keyboard.num_lock &&
							(key == PAD_DOT)) 
						{
							key = '.';
						}else{
							switch(key) {
							case PAD_HOME:
								key = HOME;
								
								break;
							case PAD_END:
								key = END;
								
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}
				
				key |= keyboard.shift_l	? FLAG_SHIFT_L	: 0;
				key |= keyboard.shift_r	? FLAG_SHIFT_R	: 0;
				key |= keyboard.ctrl_l	? FLAG_CTRL_L	: 0;
				key |= keyboard.ctrl_r	? FLAG_CTRL_R	: 0;
				key |= keyboard.alt_l	? FLAG_ALT_L	: 0;
				key |= keyboard.alt_r	? FLAG_ALT_R	: 0;
				key |= pad      ? FLAG_PAD      : 0;
				key_char_process(key);
			}else{
				//key_char_process(0);
				//key_char_process(key);
			}
		}
}

void key_char_process(uint32_t key)
{
	//char output[2] = {'\0', '\0'};

	if (!(key & FLAG_EXT)) {
		
		put_key(key);
	} else {
		
		int raw_code = key & MASK_RAW;
		
		switch(raw_code) {
			case ENTER:
				put_key('\n');
				break;
			case BACKSPACE:
				put_key( '\b');
				break;
			case TAB:
				if(key & FLAG_SHIFT_L || key & FLAG_SHIFT_R){
					//在shell和应用程序之间切换
					struct video_buffer *vidbuf = get_next_vidbuf();
					switch_video_buffer(vidbuf);
					
				}else{
					put_key( '\t');
				}
				break;
			case F1:
				put_key( F1);
				/*if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					select_console(0);
				}*/
				break;
			case F2:
				put_key( F2);
				/*if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					select_console(1);
				}*/
				break;
				
			case F3:
				//shft +f4 关闭窗口程序
				put_key( F3);
				
				break;
			case F4:
				//alt +f4 关闭程序
				/*if(key & FLAG_SHIFT_L || key & FLAG_SHIFT_R){
					if(current_vidbuf != vidbuf_console){
						current_vidbuf->thread->status = THREAD_HANGING;
						
						thread_graphic_exit(current_vidbuf->thread);
					}
				}*/
				put_key( F4);
				break;	
			case F5:
				put_key( F5);

				if(key & FLAG_ALT_L || key & FLAG_ALT_R){
					
					bogui_container_t *container = bogui_container_manager->current_container;

					//容器是窗口
					if (container != NULL) {
						if (container->flags&BOGUI_CONTAINER_WINDOW) {
							//容器线程不为空
							if (container->thread != NULL) {
								thread_cancel(container->thread);

							}
						}
					}
				}

				break;	  
			case F6:
				put_key( F6);
				break;	   
			case F7:
				put_key( F7);
				break;	  
			case F8:
				put_key( F8);
				break;	  
			case F9:
				put_key( F9);
				break;	  
			case F10:
				put_key( F10);
				break;	  
			case F11:
				put_key( F11);
				break;	  
			case F12:  
				put_key( F12);
				break;
			case ESC:
				put_key(ESC);
				
				break;	
			case UP:
				put_key(UP);
				/*if(key & FLAG_CTRL_L || key & FLAG_CTRL_R){
					scroll_screen(current_console, SCREEN_UP);
				}else{
					put_key( UP);
				}*/
				break;
			case DOWN:
				put_key(DOWN);
				/*if(key & FLAG_CTRL_L || key & FLAG_CTRL_R){
					scroll_screen(current_console, SCREEN_DOWN);
				}else{
					put_key(DOWN);
				}*/
				break;
			case LEFT:
				
				put_key(LEFT);
				break;
			case RIGHT:
				put_key(RIGHT);
				break;
			case PAGEUP:
				put_key(PAGEUP);
				break;
			case PAGEDOWN:
				put_key(PAGEDOWN);
				break;	
			case HOME:
				put_key(HOME);
				break;	
			case END:
				put_key(END);
				break;	
			case INSERT:
				put_key(INSERT);
				break;	
			case DELETE:
				put_key(DELETE);
				break;	
			default:
				break;
		}
	}
}

void put_key(uint32_t key)
{
	/*if(current_console->status == CONSOLE_ACTIVE){
		current_console->keybord_data = key;
	}*/
	keyboard.key = key;
}

int sys_get_key()
{
	struct thread *cur = thread_current();
	
	int key = -1;
	if(cur->vidbuf == current_vidbuf || current_vidbuf == vidbuf_console){
		key = keyboard.key;
		keyboard.key = -1;
	}
	return key; 
}

//从键盘缓冲区中读取下一个字节
uint8_t get_byte_from_kbuf()       
{
	return (uint8_t )ioqueue_get(keyboard.ioqueue);
}
// 等待 8042 的输入缓冲区空
static void kb_wait()
{
	uint8_t kb_stat;

	do {
		kb_stat = io_in8(KB_CMD);
	} while (kb_stat & 0x02);
}

static void kb_ack()
{
	uint8_t kb_read;

	do {
		kb_read = io_in8(KB_DATA);
	} while ((kb_read =! KB_ACK));
	
}

static void set_leds()
{
	uint8_t leds = (keyboard.caps_lock << 2) | (keyboard.num_lock << 1) | keyboard.scroll_lock;

	kb_wait();
	io_out8(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	io_out8(KB_DATA, leds);
	kb_ack();
}


/* Keymap for US MF-2 keyboard. */
uint32_t keymap[NR_SCAN_CODES * MAP_COLS] = {

/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,		0,		0,
/* 0x01 - ESC		*/	ESC,		ESC,		0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	BACKSPACE,	BACKSPACE,	0,
/* 0x0F - TAB		*/	TAB,		TAB,		0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	ENTER,		ENTER,		PAD_ENTER,
/* 0x1D - l. Ctrl	*/	CTRL_L,		CTRL_L,		CTRL_R,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	SHIFT_L,	SHIFT_L,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		PAD_SLASH,
/* 0x36 - r. SHIFT	*/	SHIFT_R,	SHIFT_R,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	ALT_L,		ALT_L,  	ALT_R,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - CapsLock	*/	CAPS_LOCK,	CAPS_LOCK,	0,
/* 0x3B - F1		*/	F1,		F1,		0,
/* 0x3C - F2		*/	F2,		F2,		0,
/* 0x3D - F3		*/	F3,		F3,		0,
/* 0x3E - F4		*/	F4,		F4,		0,
/* 0x3F - F5		*/	F5,		F5,		0,
/* 0x40 - F6		*/	F6,		F6,		0,
/* 0x41 - F7		*/	F7,		F7,		0,
/* 0x42 - F8		*/	F8,		F8,		0,
/* 0x43 - F9		*/	F9,		F9,		0,
/* 0x44 - F10		*/	F10,		F10,		0,
/* 0x45 - NumLock	*/	NUM_LOCK,	NUM_LOCK,	0,
/* 0x46 - ScrLock	*/	SCROLL_LOCK,	SCROLL_LOCK,	0,
/* 0x47 - Home		*/	PAD_HOME,	'7',		HOME,
/* 0x48 - CurUp		*/	PAD_UP,		'8',		UP,
/* 0x49 - PgUp		*/	PAD_PAGEUP,	'9',		PAGEUP,
/* 0x4A - '-'		*/	PAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	PAD_LEFT,	'4',		LEFT,
/* 0x4C - MID		*/	PAD_MID,	'5',		0,
/* 0x4D - Right		*/	PAD_RIGHT,	'6',		RIGHT,
/* 0x4E - '+'		*/	PAD_PLUS,	'+',		0,
/* 0x4F - End		*/	PAD_END,	'1',		END,
/* 0x50 - Down		*/	PAD_DOWN,	'2',		DOWN,
/* 0x51 - PgDown	*/	PAD_PAGEDOWN,	'3',		PAGEDOWN,
/* 0x52 - Insert	*/	PAD_INS,	'0',		INSERT,
/* 0x53 - Delete	*/	PAD_DOT,	'.',		DELETE,
/* 0x54 - Enter		*/	0,		0,		0,
/* 0x55 - ???		*/	0,		0,		0,
/* 0x56 - ???		*/	0,		0,		0,
/* 0x57 - F11		*/	F11,		F11,		0,	
/* 0x58 - F12		*/	F12,		F12,		0,	
/* 0x59 - ???		*/	0,		0,		0,	
/* 0x5A - ???		*/	0,		0,		0,	
/* 0x5B - ???		*/	0,		0,		GUI_L,	
/* 0x5C - ???		*/	0,		0,		GUI_R,	
/* 0x5D - ???		*/	0,		0,		APPS,	
/* 0x5E - ???		*/	0,		0,		0,	
/* 0x5F - ???		*/	0,		0,		0,
/* 0x60 - ???		*/	0,		0,		0,
/* 0x61 - ???		*/	0,		0,		0,	
/* 0x62 - ???		*/	0,		0,		0,	
/* 0x63 - ???		*/	0,		0,		0,	
/* 0x64 - ???		*/	0,		0,		0,	
/* 0x65 - ???		*/	0,		0,		0,	
/* 0x66 - ???		*/	0,		0,		0,	
/* 0x67 - ???		*/	0,		0,		0,	
/* 0x68 - ???		*/	0,		0,		0,	
/* 0x69 - ???		*/	0,		0,		0,	
/* 0x6A - ???		*/	0,		0,		0,	
/* 0x6B - ???		*/	0,		0,		0,	
/* 0x6C - ???		*/	0,		0,		0,	
/* 0x6D - ???		*/	0,		0,		0,	
/* 0x6E - ???		*/	0,		0,		0,	
/* 0x6F - ???		*/	0,		0,		0,	
/* 0x70 - ???		*/	0,		0,		0,	
/* 0x71 - ???		*/	0,		0,		0,	
/* 0x72 - ???		*/	0,		0,		0,	
/* 0x73 - ???		*/	0,		0,		0,	
/* 0x74 - ???		*/	0,		0,		0,	
/* 0x75 - ???		*/	0,		0,		0,	
/* 0x76 - ???		*/	0,		0,		0,	
/* 0x77 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x7A - ???		*/	0,		0,		0,	
/* 0x7B - ???		*/	0,		0,		0,	
/* 0x7C - ???		*/	0,		0,		0,	
/* 0x7D - ???		*/	0,		0,		0,	
/* 0x7E - ???		*/	0,		0,		0,
/* 0x7F - ???		*/	0,		0,		0
};


/*
	回车键:	把光标移到第一列
	换行键:	把光标前进到下一行
*/


/*====================================================================================*
				Appendix: Scan code set 1
 *====================================================================================*

KEY	MAKE	BREAK	-----	KEY	MAKE	BREAK	-----	KEY	MAKE	BREAK
--------------------------------------------------------------------------------------
A	1E	9E		9	0A	8A		[	1A	9A
B	30	B0		`	29	89		INSERT	E0,52	E0,D2
C	2E	AE		-	0C	8C		HOME	E0,47	E0,C7
D	20	A0		=	0D	8D		PG UP	E0,49	E0,C9
E	12	92		\	2B	AB		DELETE	E0,53	E0,D3
F	21	A1		BKSP	0E	8E		END	E0,4F	E0,CF
G	22	A2		SPACE	39	B9		PG DN	E0,51	E0,D1
H	23	A3		TAB	0F	8F		U ARROW	E0,48	E0,C8
I	17	97		CAPS	3A	BA		L ARROW	E0,4B	E0,CB
J	24	A4		L SHFT	2A	AA		D ARROW	E0,50	E0,D0
K	25	A5		L CTRL	1D	9D		R ARROW	E0,4D	E0,CD
L	26	A6		L GUI	E0,5B	E0,DB		NUM	45	C5
M	32	B2		L ALT	38	B8		KP /	E0,35	E0,B5
N	31	B1		R SHFT	36	B6		KP *	37	B7
O	18	98		R CTRL	E0,1D	E0,9D		KP -	4A	CA
P	19	99		R GUI	E0,5C	E0,DC		KP +	4E	CE
Q	10	19		R ALT	E0,38	E0,B8		KP EN	E0,1C	E0,9C
R	13	93		APPS	E0,5D	E0,DD		KP .	53	D3
S	1F	9F		ENTER	1C	9C		KP 0	52	D2
T	14	94		ESC	01	81		KP 1	4F	CF
U	16	96		F1	3B	BB		KP 2	50	D0
V	2F	AF		F2	3C	BC		KP 3	51	D1
W	11	91		F3	3D	BD		KP 4	4B	CB
X	2D	AD		F4	3E	BE		KP 5	4C	CC
Y	15	95		F5	3F	BF		KP 6	4D	CD
Z	2C	AC		F6	40	C0		KP 7	47	C7
0	0B	8B		F7	41	C1		KP 8	48	C8
1	02	82		F8	42	C2		KP 9	49	C9
2	03	83		F9	43	C3		]	1B	9B
3	04	84		F10	44	C4		;	27	A7
4	05	85		F11	57	D7		'	28	A8
5	06	86		F12	58	D8		,	33	B3

6	07	87		PRTSCRN	E0,2A	E0,B7		.	34	B4
					E0,37	E0,AA

7	08	88		SCROLL	46	C6		/	35	B5

8	09	89		PAUSE E1,1D,45	-NONE-				
				      E1,9D,C5


-----------------
ACPI Scan Codes:
-------------------------------------------
Key		Make Code	Break Code
-------------------------------------------
Power		E0, 5E		E0, DE
Sleep		E0, 5F		E0, DF
Wake		E0, 63		E0, E3


-------------------------------
Windows Multimedia Scan Codes:
-------------------------------------------
Key		Make Code	Break Code
-------------------------------------------
Next Track	E0, 19		E0, 99
Previous Track	E0, 10		E0, 90
Stop		E0, 24		E0, A4
Play/Pause	E0, 22		E0, A2
Mute		E0, 20		E0, A0
Volume Up	E0, 30		E0, B0
Volume Down	E0, 2E		E0, AE
Media Select	E0, 6D		E0, ED
E-Mail		E0, 6C		E0, EC
Calculator	E0, 21		E0, A1
My Computer	E0, 6B		E0, EB
WWW Search	E0, 65		E0, E5
WWW Home	E0, 32		E0, B2
WWW Back	E0, 6A		E0, EA
WWW Forward	E0, 69		E0, E9
WWW Stop	E0, 68		E0, E8
WWW Refresh	E0, 67		E0, E7
WWW Favorites	E0, 66		E0, E6

*=====================================================================================*/


#endif //_CONFIG_DEVIVE_KEYBOARD_

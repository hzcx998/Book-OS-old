[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GUI_INIT EQU 51
_NR_GUI_WINDOW_SIZE EQU 52
_NR_GUI_WINDOW_POSITION EQU 53
_NR_GUI_WINDOW_CREATE EQU 54
_NR_GUI_WINDOW_CLOSE EQU 55

_NR_GUI_COLOR 	EQU	56
_NR_GUI_POINT 	EQU	57
_NR_GUI_LINE 	EQU	58
_NR_GUI_RECT 	EQU	59
_NR_GUI_TEXT 	EQU	60
_NR_GUI_BUFFER 	EQU	61
_NR_GUI_FRAME 	EQU	62
_NR_GUI_DRAW 	EQU	63

_NR_GUI_MOUSE_MOVE 	    EQU	64
_NR_GUI_MOUSE_CLICK 	EQU	65
_NR_GUI_KEYBOARD 	    EQU	66
_NR_GUI_GET_MODIFIERS	EQU 67

global gui_init
gui_init:	; void gui_init();
	mov eax, _NR_GUI_INIT
	int INT_VECTOR_SYS_CALL
	ret

global gui_window_size
gui_window_size:	; void gui_window_size(int width, int height);
	mov eax, _NR_GUI_WINDOW_SIZE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global gui_window_position
gui_window_position:	; void gui_window_position(int x, int y);
	mov eax, _NR_GUI_WINDOW_POSITION
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global gui_window_create
gui_window_create:	; void gui_window_create(const char *title);
	mov eax, _NR_GUI_WINDOW_CREATE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret


global gui_window_close
gui_window_close:	; int gui_window_close();
	mov eax, _NR_GUI_WINDOW_CLOSE
	int INT_VECTOR_SYS_CALL
	ret

global gui_color
gui_color:	; void gui_color(unsigned int color);
	mov eax, _NR_GUI_COLOR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global gui_point
gui_point:	; void gui_point(int x, int y);
	mov eax, _NR_GUI_POINT
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global gui_line
gui_line:	;void gui_line(int x1, int y1, int x2, int y2);
	mov eax, _NR_GUI_LINE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]
	int INT_VECTOR_SYS_CALL
	ret

global gui_rect
gui_rect:	; void gui_rect(int x, int y, int width, int height);
	mov eax, _NR_GUI_RECT
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]
	int INT_VECTOR_SYS_CALL
	ret

global gui_text
gui_text:	; void gui_text(int x, int y, char *text);
	mov eax, _NR_GUI_TEXT
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	int INT_VECTOR_SYS_CALL
	ret

global gui_buffer
gui_buffer:	; void gui_buffer(unsigned char *buffer);
	mov eax, _NR_GUI_BUFFER
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	

global gui_frame
gui_frame:	; void gui_frame(int x, int y, int width, int height);
	mov eax, _NR_GUI_FRAME
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]
	int INT_VECTOR_SYS_CALL
	ret

global gui_draw
gui_draw:	; void gui_draw(int x1, int y1, int x2, int y2);
	mov eax, _NR_GUI_DRAW
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]
	int INT_VECTOR_SYS_CALL
	ret

global gui_mouse_move
gui_mouse_move:	; int gui_mouse_move(int *x, int *y);
	mov eax, _NR_GUI_MOUSE_MOVE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global gui_mouse_click
gui_mouse_click:	; int gui_mouse_click(int *key, int *status, int *x, int *y);
	mov eax, _NR_GUI_MOUSE_CLICK
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	mov esi, [esp + 16]
	int INT_VECTOR_SYS_CALL
	ret	

global gui_keyboard
gui_keyboard:	; int gui_keyboard(int *key);
	mov eax, _NR_GUI_KEYBOARD
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global gui_get_modifiers
gui_get_modifiers:	; int gui_get_modifiers();
	mov eax, _NR_GUI_GET_MODIFIERS
	int INT_VECTOR_SYS_CALL
	ret


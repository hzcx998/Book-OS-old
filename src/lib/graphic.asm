[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_INIT_GRAPHIC equ 40
_NR_GRAPH_POINT equ 41
_NR_GRAPH_REFRESH equ 42
_NR_GRAPH_LINE equ 43
_NR_GRAPH_RECT equ 44
_NR_GRAPH_TEXT equ 45
_NR_GRAPHIC_EXIT equ 46
_NR_GRAP_BUFFER equ 47
_NR_GRAP_CHAR equ 48
_NR_GRAP_DRAW_BMP equ 49
_NR_GRAP_DRAW_JPEG equ 50

global init_graphic
init_graphic:
	mov eax, _NR_INIT_GRAPHIC
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global guipoint
guipoint:
	mov eax, _NR_GRAPH_POINT
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
global guirefresh
guirefresh:
	mov eax, _NR_GRAPH_REFRESH
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
global guiline
guiline:
	mov eax, _NR_GRAPH_LINE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
global guirect
guirect:
	mov eax, _NR_GRAPH_RECT
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	
global guitext
guitext:
	mov eax, _NR_GRAPH_TEXT	
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	
	
global graphic_exit
graphic_exit:
	mov eax, _NR_GRAPHIC_EXIT	
	int INT_VECTOR_SYS_CALL
	ret		
	
global guibuffer
guibuffer:
	mov eax, _NR_GRAP_BUFFER
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	
global guichar
guichar:
	mov eax, _NR_GRAP_CHAR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	
	
global guibmp
guibmp:
	mov eax, _NR_GRAP_DRAW_BMP
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret		

global guijpeg
guijpeg:
	mov eax, _NR_GRAP_DRAW_JPEG
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret		

[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_MOUSE_POS EQU 32
_NR_GET_MOUSE_BTN EQU 33

global get_mouse_pos
get_mouse_pos:
	mov eax, _NR_GET_MOUSE_POS
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global get_mouse_btn
get_mouse_btn:
	mov eax, _NR_GET_MOUSE_BTN
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

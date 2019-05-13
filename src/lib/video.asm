[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_SCREEN EQU 34

global get_screen
get_screen:
	mov eax, _NR_GET_SCREEN
	mov ebx, [esp + 4]		
	mov ecx, [esp + 8]		
	int INT_VECTOR_SYS_CALL
	ret
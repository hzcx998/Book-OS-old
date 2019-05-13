[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_READDIR EQU 19

global readdir
readdir:
	mov eax, _NR_READDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
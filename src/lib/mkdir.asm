[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_MKDIR EQU 21

global mkdir
mkdir:
	mov eax, _NR_MKDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
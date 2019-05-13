[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_CLOSEDIR EQU 18

global closedir
closedir:
	mov eax, _NR_CLOSEDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_CLEAR EQU 8

global clear
clear:
	mov eax, _NR_CLEAR
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_UNLINK EQU 16

global unlink
unlink:
	mov eax, _NR_UNLINK
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_OPENDIR EQU 17

global opendir
opendir:
	mov eax, _NR_OPENDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
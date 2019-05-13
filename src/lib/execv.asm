[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_EXECV EQU 6

global execv
execv:
	mov eax, _NR_EXECV
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
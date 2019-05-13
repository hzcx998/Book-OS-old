[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FREE EQU 2

global free
free:
	mov eax, _NR_FREE
	mov ebx, [esp + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_MALLOC EQU 1

global malloc
malloc:
	mov eax, _NR_MALLOC
	mov ebx, [esp + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	ret
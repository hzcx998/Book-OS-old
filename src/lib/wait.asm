[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_WAIT EQU 7

global _wait
_wait:
	mov eax, _NR_WAIT
	mov ebx, [esp + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	ret
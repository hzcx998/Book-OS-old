[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_EXIT EQU 3

global exit
exit:
	mov eax, _NR_EXIT
	mov ebx, [esp + 4]		;第一个参数
	int INT_VECTOR_SYS_CALL
	ret
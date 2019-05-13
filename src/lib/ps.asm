[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PS EQU 9

global ps
ps:
	mov eax, _NR_PS
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
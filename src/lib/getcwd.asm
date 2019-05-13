[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETCWD EQU 24

global getcwd
getcwd:
	mov eax, _NR_GETCWD
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
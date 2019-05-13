[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FOPEN EQU 10

global fopen
fopen:
	mov eax, _NR_FOPEN
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
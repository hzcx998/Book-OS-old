[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FSTAT EQU 15

global fstat
fstat:
	mov eax, _NR_FSTAT
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
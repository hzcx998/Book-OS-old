[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_LSDIR EQU 29

global lsdir
lsdir:
	mov eax, _NR_LSDIR
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
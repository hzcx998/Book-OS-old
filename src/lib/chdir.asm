[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_CHDIR EQU 23

global chdir
chdir:
	mov eax, _NR_CHDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
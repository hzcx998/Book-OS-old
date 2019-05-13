[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETTIME EQU 31

global gettime
gettime:
	mov eax, _NR_GETTIME
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
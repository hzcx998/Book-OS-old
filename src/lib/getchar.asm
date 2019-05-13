[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GETCHAR EQU 4

global getchar
getchar:
	mov eax, _NR_GETCHAR
	int INT_VECTOR_SYS_CALL
	ret
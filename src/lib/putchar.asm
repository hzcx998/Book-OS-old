[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PUTCHAR EQU 5

global putchar
putchar:
	mov eax, _NR_PUTCHAR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_LSDISK EQU 39

global lsdisk
lsdisk:
	mov eax, _NR_LSDISK
	int INT_VECTOR_SYS_CALL
	ret
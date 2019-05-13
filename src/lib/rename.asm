[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_RENAME EQU 25

global rename
rename:
	mov eax, _NR_RENAME
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
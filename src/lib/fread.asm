[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FREAD EQU 12

global fread
fread:
	mov eax, _NR_FREAD
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	int INT_VECTOR_SYS_CALL
	ret
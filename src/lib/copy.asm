[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_COPY EQU 26

global copy
copy:
	mov eax, _NR_COPY
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
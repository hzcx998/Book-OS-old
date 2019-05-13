[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_MOVE EQU 27

global move
move:
	mov eax, _NR_MOVE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
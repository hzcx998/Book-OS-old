[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_FCLOSE EQU 11

global fclose
fclose:
	mov eax, _NR_FCLOSE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
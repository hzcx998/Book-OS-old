[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_RMDIR EQU 22

global rmdir
rmdir:
	mov eax, _NR_RMDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_LSEEK EQU 14

global lseek
lseek:
	mov eax, _NR_LSEEK
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	int INT_VECTOR_SYS_CALL
	ret
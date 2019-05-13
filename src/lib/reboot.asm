[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_REBOOT EQU 30

global reboot
reboot:
	mov eax, _NR_REBOOT
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
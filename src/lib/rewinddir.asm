[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_REWINDIR EQU 20

global rewinddir
rewinddir:
	mov eax, _NR_REWINDIR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
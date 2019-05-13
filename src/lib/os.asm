[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_TICKS EQU 37
_NR_SLEEP EQU 38

global get_ticks
get_ticks:
	mov eax, _NR_GET_TICKS
	int INT_VECTOR_SYS_CALL
	ret


global sleep
sleep:
	mov eax, _NR_SLEEP
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret	
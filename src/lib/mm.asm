[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_MEMORY EQU 35
_NR_MM_REALLOC EQU 85

global get_memory
get_memory:
	mov eax, _NR_GET_MEMORY
	mov ebx, [esp + 4]		
	mov ecx, [esp + 8]		
	int INT_VECTOR_SYS_CALL
	ret

global realloc
realloc:	;void *realloc(void *mem_address, unsigned int newsize);
	mov eax, _NR_MM_REALLOC
	mov ebx, [esp + 4]		
	mov ecx, [esp + 8]		
	int INT_VECTOR_SYS_CALL
	ret	
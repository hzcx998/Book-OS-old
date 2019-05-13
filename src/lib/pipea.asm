[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_PIPE_CREATE		EQU	76
_NR_PIPE_CONNECT	EQU	77
_NR_PIPE_WRITE	    EQU	78
_NR_PIPE_READ	    EQU	79
_NR_PIPE_CANCEL	    EQU	80
_NR_PIPE_CLOSE	    EQU	81

global pipe_create
pipe_create:	; int32_t pipe_create(uint32_t size);
	mov eax, _NR_PIPE_CREATE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
	
global pipe_connect
pipe_connect:	; int32_t pipe_connect(uint32_t *size);
	mov eax, _NR_PIPE_CONNECT
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
	
global _pipe_write
_pipe_write:	; int32_t _pipe_write(uint32_t pipe_id,void* data,uint32_t size);
	mov eax, _NR_PIPE_WRITE
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	mov edx, [esp + 12]
	int INT_VECTOR_SYS_CALL
	ret
	
global pipe_read
pipe_read:	; int32_t pipe_read(uint32_t pipe_id,void* buffer);
	mov eax, _NR_PIPE_READ
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret
	
global _pipe_cancel
_pipe_cancel:	; int32_t _pipe_cancel(uint32_t pipe_id);
	mov eax, _NR_PIPE_CANCEL
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
	
global pipe_close
pipe_close:	; int32_t pipe_close(uint32_t pipe_id);
	mov eax, _NR_PIPE_CLOSE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
	
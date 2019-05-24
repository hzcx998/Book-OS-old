[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_TIMER_ALLOC		equ	90
_NR_TIMER_FREE		equ	91
_NR_TIMER_SETTM		equ	92
_NR_TIMER_CANCEL	equ	93
_NR_TIMER_OCCUR		equ	94

global talloc
talloc:	;timer_t *talloc(void);
	mov eax, _NR_TIMER_ALLOC
	int INT_VECTOR_SYS_CALL
	ret

global tfree
tfree:	;void tfree(timer_t *timer);
	mov eax, _NR_TIMER_FREE
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global settimer
settimer:	;int settimer(timer_t *timer, uint32 timeout);
	mov eax, _NR_TIMER_SETTM
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

global tcancel
tcancel:	;int tcancel(timer_t *timer);
	mov eax, _NR_TIMER_CANCEL
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global toccur
toccur:	;int toccur(timer_t *timer);
	mov eax, _NR_TIMER_OCCUR
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

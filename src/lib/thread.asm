[bits 32]
[section .text]

INT_VECTOR_SYS_CALL equ 0x80

_NR_GET_PID equ 36
_NR_THREAD_CONNECT equ 73
_NR_THREAD_GETINFO equ 74
_NR_THREAD_TESTCANCEL equ 75
_NR_GET_PPID	    equ 82
_NR_WAIT_CHILD_EXIT	equ 83
_NR_THREAD_KILL		equ 84
global get_pid
get_pid:
	mov eax, _NR_GET_PID
	int INT_VECTOR_SYS_CALL
	ret

global get_ppid
get_ppid:
	mov eax, _NR_GET_PPID
	int INT_VECTOR_SYS_CALL
	ret

global thread_connect
thread_connect:	;struct thread *thread_connect();
	mov eax, _NR_THREAD_CONNECT
	int INT_VECTOR_SYS_CALL
	ret

global thread_getinfo
thread_getinfo:	;struct thread *thread_getinfo(struct thread *ptr);
	mov eax, _NR_THREAD_GETINFO
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

global thread_testcancel
thread_testcancel:	;int thread_testcancel(void);
	mov eax, _NR_THREAD_TESTCANCEL
	int INT_VECTOR_SYS_CALL
	ret

global wait_child_exit
wait_child_exit:	;int wait_child_exit();
	mov eax, _NR_WAIT_CHILD_EXIT
	int INT_VECTOR_SYS_CALL
	ret
	
global thread_kill
thread_kill:	; int thread_kill(int pid);
	mov eax, _NR_THREAD_KILL
	mov ebx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret
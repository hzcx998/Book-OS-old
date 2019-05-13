;kernel/interrupt.asm
;description: interrupt
;auther: huzicheng
;time: 2019/2/20
;Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com


%include "sys/const.inc"

extern exception_handler
extern irq_table
extern sys_call_table
extern tss 

global switch_to
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error

global intr_exit_clock

CLOCK_IRQ equ 0
KEYBOARD_IRQ equ 1

IRQ_PORT9 equ 9
ETHERNET_IRQ equ 11
MOUSE_IRQ equ 12
PRIMARY_IRQ equ 14
SECONDARY_IRQ equ 15

[section .text]
[bits 32]
; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	push esp
	call	exception_handler
	add	esp, 4*3	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

global IRQ_clock
IRQ_clock:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	
	;mov esp, INTERRUPT_STACK_TOP
	cli
	
	mov	al, EOI
	out	INT_M_CTL, al
	
	push CLOCK_IRQ
	call [irq_table + CLOCK_IRQ*4]
	add esp, 4

	sti
	jmp intr_exit
	
global IRQ_keyboard	
IRQ_keyboard:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	
	;mov esp, INTERRUPT_STACK_TOP
	
	cli 
	mov	al, EOI
	out	INT_M_CTL, al
	
	push KEYBOARD_IRQ
	call [irq_table + KEYBOARD_IRQ*4]
	add esp, 4
	
	sti
	jmp intr_exit
	
global IRQ_mouse
IRQ_mouse:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	cli
	
	;intr over
	mov	al, EOI
	;master
	out	INT_M_CTL, al
	;slaver
	out	INT_S_CTL, al
	
	push MOUSE_IRQ
	call [irq_table + MOUSE_IRQ*4]
	add esp, 4
	sti
	
	jmp intr_exit

global IRQ_port9
IRQ_port9:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	
	cli 
	
	push IRQ_PORT9
	call [irq_table + IRQ_PORT9*4]
	add esp, 4
	
	;intr over
	mov	al, EOI
	;master
	out	INT_M_CTL, al
	;slaver
	out	INT_S_CTL, al
	
	sti
	jmp intr_exit

global IRQ_ethernet
IRQ_ethernet:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	
	cli 
	;intr over
	mov	al, EOI
	;master
	out	INT_M_CTL, al
	;slaver
	out	INT_S_CTL, al
	
	push ETHERNET_IRQ
	call [irq_table + ETHERNET_IRQ*4]
	add esp, 4
	
	
	sti
	jmp intr_exit

global IRQ_primary_channel
IRQ_primary_channel:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx
	
	;mov esp, INTERRUPT_STACK_TOP
	cli 
	;intr over
	mov	al, EOI
	;master
	out	INT_M_CTL, al
	;slaver
	out	INT_S_CTL, al
	
	push PRIMARY_IRQ
	call [irq_table + PRIMARY_IRQ*4]
	add esp, 4
	
	sti
	jmp intr_exit

global IRQ_secondary_channel
IRQ_secondary_channel:
	push ds
	push es
	push fs
	push gs
	pushad
	
	mov dx,ss
	mov ds, dx
	mov es, dx

	cli
	
	;intr over
	mov	al, EOI
	;master
	out	INT_M_CTL, al
	;slaver
	out	INT_S_CTL, al
	
	push SECONDARY_IRQ
	call [irq_table + SECONDARY_IRQ*4]
	add esp, 4
	
	sti
	jmp intr_exit
	
global intrrupt_sys_call
intrrupt_sys_call:
	push ds
	push es
	push fs
	push gs
	pushad
	
	cli
	
	mov edi, edx
	mov dx,ss
	mov ds, dx
	mov es, dx
	mov edx, edi
	;先把后面的参数压栈, 最多支持4个参数
	push esi
	push edx
	push ecx
	push ebx
	call [sys_call_table + eax*4]
	add esp, 4*4
	
	;keep eax in return stack
	mov [esp + 4*7], eax
	
	sti
	jmp intr_exit
	
global thread_intr_exit	
global intr_exit

thread_intr_exit:
	mov esp, [esp + 4]
	sti
intr_exit:
	popad
	pop gs
	pop fs	
	pop es	 
	pop ds
	iretd	

global intr_exit_clock
intr_exit_clock:
	;mov esp, [task_ready]
	;lea eax, [esp + STACKTOP]
	;mov dword [tss + TSS_ESP0], eax 
	pop gs
	pop fs
	pop es	 
	pop ds
	popad
	iretd
	
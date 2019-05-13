;kernel/x86.asm
;description: x86 operation
;auther: huzicheng
;time: 2016/11/26 20:58
;Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com

EOI EQU 0X20
INT_M_CTL	equ	0x20	; I/O port for interrupt controller         <Master>
INT_M_CTLMASK	equ	0x21	; setting bits in this port disables ints   <Master>
INT_S_CTL	equ	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	equ	0xA1	; setting bits in this port disables ints   <Slave>

global io_in8
global io_out8
global io_in16
global io_out16
global io_in32
global io_out32
global io_cli
global io_sti
global io_hlt
global io_stihlt
global load_tr
global	read_cr3
global	write_cr3
global	read_cr0
global	write_cr0
global	store_gdtr
global	load_gdtr
global	store_idtr
global load_idtr
global run_kernel_high
global enable_irq
global disable_irq
global io_load_eflags
global io_store_eflags
global port_read
global port_write
global io_ud2
[section .text]
[bits 32]

P_STACKBASE	equ	0
GSREG		equ	P_STACKBASE
FSREG		equ	GSREG		+ 4
ESREG		equ	FSREG		+ 4
DSREG		equ	ESREG		+ 4
EDIREG		equ	DSREG		+ 4
ESIREG		equ	EDIREG		+ 4
EBPREG		equ	ESIREG		+ 4
KERNELESPREG	equ	EBPREG		+ 4
EBXREG		equ	KERNELESPREG	+ 4
EDXREG		equ	EBXREG		+ 4
ECXREG		equ	EDXREG		+ 4
EAXREG		equ	ECXREG		+ 4
EIPREG		equ	EAXREG		+ 4
CSREG		equ	EIPREG		+ 4
EFLAGSREG	equ	CSREG		+ 4
ESPREG		equ	EFLAGSREG	+ 4
SSREG		equ	ESPREG		+ 4
P_STACKTOP	equ	SSREG		+ 4

io_in8:	;int io_in8(int port);
	mov		edx,[esp+4]
	xor		eax,eax
	in		al,dx
	ret

io_in16:	;int io_in16(int port);
	mov		edx,[esp+4]
	xor		eax,eax
	in		ax,dx
	ret
io_in32:	;int io_in32(int port);
	mov		edx,[esp+4]
	in		eax,dx
	ret

io_out8:	; void io_out8(int port, int data);
	mov		edx,[esp+4]
	mov		al,[esp+8]
	out		dx,al
	ret

io_out16:	; void io_out16(int port, int data);
	mov		edx,[esp+4]
	mov		ax,[esp+8]
	out		dx,ax
	ret	

io_out32:	; void io_out32(int port, int data);
	mov		edx,[esp+4]
	mov		eax,[esp+8]
	out		dx,eax
	ret	
io_cli:	; void io_cli(void);
	cli
	ret

io_sti:	; void io_sti(void);
	sti
	ret
	
io_hlt: ;void io_hlt(void);
	hlt
	ret
io_stihlt:
	sti
	hlt
	ret
load_tr:		; void load_tr(int tr);
	ltr	[esp+4]			; tr
	ret
	
read_cr3:
	mov eax,cr3
	ret

write_cr3:
	mov eax,[esp+4]
	mov cr3,eax
	ret
read_cr0:
	mov eax,cr0
	ret

write_cr0:
	mov eax,[esp+4]
	mov cr0,eax
	ret	
	
store_gdtr:
	mov eax, [esp + 4]
	sgdt [eax]
	ret

load_gdtr:	;void load_gdtr(int limit, int addr);
	mov ax, [esp + 4]
	mov	[esp+6],ax		
	lgdt [esp+6]
	
	jmp dword 0x08: .l
	
.l:
	mov ax, 0x10
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov ss, ax 
	mov gs, ax 
	ret

store_idtr:
	mov eax, [esp + 4]
	sidt [eax]
	ret

load_idtr:	;void load_idtr(int limit, int addr);
	mov		ax,[esp+4]
	mov		[esp+6],ax
	lidt	[esp+6]
	ret

; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;	}
disable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, 1
        rol     ah, cl                  ; ah = (1 << (irq % 8))
        cmp     cl, 8
        jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
        in      al, INT_M_CTLMASK
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_M_CTLMASK, al       ; set bit at master 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
disable_8:
        in      al, INT_S_CTLMASK
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_S_CTLMASK, al       ; set bit at slave 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
dis_already:
        popf
        xor     eax, eax                ; already disabled
        ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8){
;               out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;       }
;       else{
;               out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;       }
;
enable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, ~1
        rol     ah, cl                  ; ah = ~(1 << (irq % 8))
        cmp     cl, 8
        jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
        in      al, INT_M_CTLMASK
        and     al, ah
        out     INT_M_CTLMASK, al       ; clear bit at master 8259
        popf
        ret
enable_8:
        in      al, INT_S_CTLMASK
        and     al, ah
        out     INT_S_CTLMASK, al       ; clear bit at slave 8259
        popf
        ret


io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS
		POP		EAX
		RET

io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS
		RET

; ========================================================================
;                  void port_read(u16 port, void* buf, int n);
; ========================================================================
port_read:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1		;ecx/2
	cld
	rep	insw
	ret

; ========================================================================
;                  void port_write(u16 port, void* buf, int n);
; ========================================================================
port_write:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1	;ecx/2
	cld
	rep	outsw
	ret

io_ud2:
	ud2
	ret
	
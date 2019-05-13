;const.inc

;;;;;interrupt
;中断栈寄存器的偏移
STACKBASE	equ	0
GSREG		equ	STACKBASE
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
STACKTOP	equ	SSREG		+ 4
;tss相关寄存器的偏移
TSSBASE EQU 0
TSS_ESP0 EQU TSSBASE + 4
TSS_SS0 EQU TSS_ESP0 + 4
TSS_ESP1 EQU TSS_SS0 + 4
TSS_SS1 EQU TSS_ESP1 + 4
TSS_ESP2 EQU TSS_SS1 + 4
TSS_SS2 EQU TSS_ESP2 + 4
TSS_CR3 EQU TSS_SS2 + 4
;中断
EOI EQU 0X20 ;中断结束
INT_M_CTL EQU 0X20;主控制
INT_M_CTLMASK EQU 0X21 ;主控制数据
INT_S_CTL	equ	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	equ	0xA1	; setting bits in this port disables ints   <Slave>

;默认内核栈顶
KERNEL_STACK_TOP EQU 0x009fc00

INTERRUPT_STACK_TOP EQU 0x009a000

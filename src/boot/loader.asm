;boot/loader.asm
;description: loade kernel and set basic info
;auther: Hu Zicheng
;time: 2018/1/23
;copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com

%include "const.inc"

org 0x90000
[bits 16]
align 16
	;进入loader程序会从这里开始执行
	;跳到LoaderStart
	jmp LoaderStart
;Global Descriptor Table,GDT
;gdt[0] always null
;1个gdt描述符的内容是8字节，可以根据那个描述得结构来分析这个结构体里面的内容
;一个描述符的格式如下
;低32位
;|0~15			|16~31			|
;|段界限0~15	|段地址0~15		|
;高32位
;|0~7			|8~11|12|13~14	|15|16~19 		|20 |21|22	|23|24~31		|
;|段地址16~23	|TYPE|S	|DPL	|P |段界限16~19 |AVL|L |D/B	|G |段地址24~31	|
;接下来分析一下代码段和数据段的格式吧
GDT:
	;0:void
	dd		0x00000000
	dd		0x00000000
	;1:4GB(flat-mode) code segment 0
	;低32位
	;|0~15			|16~31			|
	;|段界限0~15	|段地址0~15		|
	;|0xffff		|0x0000			|
	;高32位
	;|0~7			|8~11 	|12 |13~14	|15	|16~19 			|20 |21|22	|23|24~31		|
	;|段地址16~23	|TYPE	|S	|DPL	|P 		|段界限16~19 	|AVL|L |D/B	|G |段地址24~31	|
	;|0x00			|1010b	|1b |00b	|1b		|1111b 			|0  |0 |1	|1 |0x0000		|
	;=================
	;通过对数据的解析，我们可以知道他的段地址0~31位都是0，段界限0~19位都是1，并且发现G是1，
	;也就是说他是一个基地址为0x00000000,段界限为0xffffffff(因为G为1，所以粒度是4KB，也就是
	;段界限值0xfffff*4kb = 0xffffffff)的一个段。
	;S为1，说明是一个非系统段（不是说不能给系统用，只是说系统有自己特别类型的段，已经存在的）。
	;TYPE类型是1010b，也就是说他是一个可执行的、一致性代码段
	;DPL是0，也就是说他是一个特权级为0的段，正好，我们的系统的特权级就是0。
	;P表示是否存在，这个地方是1，表示存在
	;AVL ，AVaiLable，可用的。不过这是对用户来说的，也就是操作系统可以随意用此位。
	;对硬件来说，它没有专门的用途
	;L 为 1 表示 64 位代码段，否则表示 32位代码段。我们是32位操作系统，所以它是0
	;D/B用来指定是16位指令还是32位指令，为1是32位指令，为0是16位指令，这个地方是1
	;=================
	;根据解析，我们可以知道他是一个32位的，可执行的，基地址为0，界限为0xffffffff的代码段
	dd		0x0000ffff
	dd		0x00cf9A00
	;2:4GB(flat-mode) data segment 0
	;低32位
	;|0~15			|16~31			|
	;|段界限0~15	|段地址0~15		|
	;|0xffff		|0x0000			|
	;高32位
	;|0~7			|8~11 	|12 |13~14	|15	|16~19 			|20 |21|22	|23|24~31		|
	;|段地址16~23	|TYPE	|S	|DPL	|P 		|段界限16~19 	|AVL|L |D/B	|G |段地址24~31	|
	;|0x00			|0010b	|1b |00b	|1b		|1111b 			|0  |0 |1	|1 |0x0000		|
	;=================
	;发现这个描述符的和上一个的差别就是类型不一样。
	;TYPE类型是0010b，通过查表可知，也就是说他是可读可写的数据段
	;=================
	;根据解析，我们可以知道他是一个32位的，可执行的，基地址为0，界限为0xffffffff的数据段
	dd		0x0000ffff
	dd		0x00cf9200
	
GdtLen equ $ - GDT	
Gdt48:
	dw	(GdtLen-1)
	dd	GDT

LoaderStart:
	;初始化段和栈
	;由于从boot跳过来的时候用的jmp LOADER_SEG(0x9000):0
	;所以这个地方的cs是0x9000，其它的段也是这个值
	mov ax, cs
	mov ds, ax 
	mov ss, ax
	mov sp, 0
	mov ax, 0xb800
	mov es, ax
	
	;show 'LOADER'
	mov byte [es:160+0],'L'
	mov byte [es:160+1],0x07
	mov byte [es:160+2],'O'
	mov byte [es:160+3],0x07
	mov byte [es:160+4],'A'
	mov byte [es:160+5],0x07
	mov byte [es:160+6],'D'
	mov byte [es:160+7],0x07
	mov byte [es:160+8],'E'
	mov byte [es:160+9],0x07
	mov byte [es:160+10],'R'
	mov byte [es:160+11],0x07
;在这个地方把elf格式的内核加载到一个内存，elf文件不能从头执行，
;必须把它的代码和数据部分解析出来，这个操作是进入保护模式之后进行的
LoadeKernel:
	;loade kernel
	;first block 128 sectors
	;把内核文件加载到段为KERNEL_SEG（0x1000）的地方，也就是物理内存
	;为0x10000的地方，一次性加载BLOCK_SIZE（128）个扇区
	;写入参数
	mov ax, KERNEL_SEG
	mov si, KERNEL_OFF
	mov cx, BLOCK_SIZE
	;调用读取一整个块的扇区数据函数，其实也就是循环读取128个扇区，只是
	;把它做成函数，方便调用
	call LoadeBlock
	
	;second block 128 sectors
	;当读取完128个扇区后，我们的缓冲区位置要改变，也就是增加128*512=0x10000
	;的空间，由于ax会给es，所以这个地方用改变段的位置，所以就是0x1000,
	;扇区的位置是保留在si中的，上一次调用后，si递增了128，所以这里我们不对
	;si操作
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	;third block 128 sectors
	;这个地方和上面同理
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	jmp LoadOver
;在这个地方把file加载到一个内存
LoadeFile:
	;loade file
	;first block 128 sectors
	;把内核文件加载到段为FILE_SEG（0x4200）的地方，也就是物理内存
	;为0x42000的地方，一次性加载BLOCK_SIZE（128）个扇区
	;写入参数
	mov ax, FILE_SEG
	mov si, FILE_OFF
	mov cx, BLOCK_SIZE
	;调用读取一整个块的扇区数据函数，其实也就是循环读取128个扇区，只是
	;把它做成函数，方便调用
	call LoadeBlock
	
	;second block 128 sectors
	;当读取完128个扇区后，我们的缓冲区位置要改变，也就是增加128*512=0x10000
	;的空间，由于ax会给es，所以这个地方用改变段的位置，所以就是0x1000,
	;扇区的位置是保留在si中的，上一次调用后，si递增了128，所以这里我们不对
	;si操作
	add ax, 0x1000
	mov cx, BLOCK_SIZE
	call LoadeBlock
	
	;third block 128 sectors
	;这个地方和上面同理
	;add ax, 0x1000
	;mov cx, BLOCK_SIZE
	;call LoadeBlock
		
	;128*2 = 256 sector = 128 kb

LoadOver:

	;我们不再使用软盘，所以这里关闭软盘驱动
	call KillMotor
	
;检测内存
;这个方法就是获取内存信息结构体ARDS, 获取所有的信息后就跳到初始化图形
CheckMemory:
	xor ebx, ebx 
	mov edx, 0x534d4150
	mov di, 0
	mov ax, ARDS_SEG 
	mov es, ax
	mov word [es:ARDS_ZONE_SIZE-4], 0
.E820MemGetLoop:
	mov eax, 0x0000e820
	mov ecx, 20
	int 0x15
	jc .E820CheckFailed
	add di, cx 
	inc word [es:ARDS_ZONE_SIZE-4]
	cmp ebx, 0
	jnz .E820MemGetLoop
	jmp VideoGraphciSet
.E820CheckFailed:
	jmp $
;初始化图形界面的基本信息以及切换到图形模式
VideoGraphciSet:
	;检查VBE是否存在
	;缓冲区 0x90000开始
	mov	ax,VBEINFO_SEG	
	mov	es,ax
	mov	di,0
	mov	ax,0x4f00	;检查VBE存在功能，指定ax=0x4f00
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 存在
	jne	.VideoError
	
	;检查VBE版本，必须是VBE 2.0及其以上
	mov	ax,[es:di+4]
	cmp	ax,0x0200
	jb	.VideoError			; if (ax < 0x0200) goto screen_default

	;获取画面信息， 256字节
	;cx=输入检查的模式
	;[es:di+0x00]	模式属性	bit7是1就能加上0x4000，便于操作
	;[es:di+0x12]	x的分辨率	宽度
	;[es:di+0x14]	y的分辨率	高度
	;[es:di+0x19]	颜色数		8位，16位，24位，32位
	;[es:di+0x1b]	颜色的指定方法 	调色板等
	;[es:di+0x28]	VRAM 地址
	
	mov ax, VBEMODE_SEG
	mov es, ax
	
	mov	cx,VBEMODE	;cx=模式号
	mov	ax,0x4f01	;获取画面模式功能，指定ax=0x4f01
	int	0x10
	cmp	ax,0x004f	;ax=0x004f 指定的这种模式可以使用
	jne	.VideoError

	;切换到指定的模式
	mov	BX,VBEMODE+0x4000	;bx=模式号
	mov	ax,0x4f02	;切换模式模式功能，指定ax=0x4f01
	int	0x10
	
	mov ax, VIDEO_INFO_SEG
	mov ds, ax
	
	xor ax, ax
	mov	al,[es:di+0x19]
	mov	[BITS_PER_PIXE],ax ;保存颜色位数
	mov	ax,[es:di+0x12]
	mov	[VIDEO_WIDTH],ax		;保存x分辨率 宽度
	mov	ax,[es:di+0x14]
	mov	[VIDEO_HEIGHT],ax		;保存y分辨率 高度
	mov	eax,[es:di+0x28]
	mov	[VIDEO_RAM],eax		;保存VRAM地址
	
	;reset ds
	;由于初始化图形模式改变了ds的值，这里设置和cs一样
	mov ax, cs
	mov ds, ax
	;进入保护模式设置
	JMP	SetProtectMode

	;切换失败切换到默认的模式
.VideoError:
	jmp $
;保护模式设置的步骤为
;1.关闭中断，防止中间发生中断，因为保护模式中断和实模式中断的方式不一样
;2.加载gdt，保护模式进行内存访问需要用到gdt里面定义的数据结构
;3.打开A20总线，使得可以访问1MB以上的内存空间
;4.设置cr0的最低1位位1，就是切换成保护模式
;5.执行一个远跳转，清空cpu流水线
SetProtectMode:
	;close the interruption
	cli
	;load GDTR
	lgdt	[Gdt48]
	
	;enable A20 line
	in		al,0x92
	or		al,2
	out		0x92,al
	;set CR0 bit PE
	mov		eax,cr0
	or		eax,1
	mov		cr0,eax
	
	;far jump:to clean the cs
	;这个地方的0x08是选择子，不是段
	;选择子的格式是这样的
	;|0~1|2 |3~15		|
	;|RPL|TI|描述符索引	|
	;0x08的解析是
	;|00b|0b|1			|
	;也及时说RPL为0，及要访问的段的特权级为0
	;TI为0，也就是说在GDT中获取描述符，TI为1时，是在IDT中获取描述符。
	;索引为1，也就是第二个描述符，第一个是NULL的，根据GDT可知，他是一个代码段
	;=============
	;也就是说，我们使用了代码段
	jmp	dword 0x08:Flush
	
;si=LBA address, from 0
;cx=sectors
;es:dx=buffer address	
;this function was borrowed from internet
ReadSectors:
	push ax 
	push cx 
	push dx 
	push bx 
	
	mov ax, si 
	xor dx, dx 
	mov bx, 18
	
	div bx 
	inc dx 
	mov cl, dl 
	xor dx, dx 
	mov bx, 2
	
	div bx 
	
	mov dh, dl
	xor dl, dl 
	mov ch, al 
	pop bx 
.RP:
	mov al, 0x01
	mov ah, 0x02 
	int 0x13 
	jc .RP 
	pop dx
	pop cx 
	pop ax
	ret

;ax = 写入的段偏移
;si = 扇区LBA地址
;cx = 扇区数
LoadeBlock:
	mov es, ax
	xor bx, bx 
.loop:
	call ReadSectors
	add bx, 512
	inc si 
	loop .loop
	ret	

;don't use floppy from now on
KillMotor:
	push dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al
	pop	dx
	ret
	
[bits 32]
align 32
;切换到保护模式后会跳转到这里
Flush:
	;init all segment registeres
	;初始化保护模式下的段描述符，此时访问内存方式已经改变
	
	;这个地方的0x10是选择子，不是段
	;选择子的格式是这样的
	;|0~1|2 |3~15		|
	;|RPL|TI|描述符索引	|
	;0x08的解析是
	;|00b|0b|2			|
	;也及时说RPL为0，及要访问的段的特权级为0
	;TI为0，也就是说在GDT中获取描述符，TI为1时，是在IDT中获取描述符。
	;索引为2，也就是第三个描述符，根据GDT可知，他是一个数据段
	;=============
	;也就是说，我们使用了数据段段
	mov ax, 0x10	;the data 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 
	mov esp, LOADER_STACK_TOP
	
	;put 'P'
	mov byte [0xb8000+160*2+0], 'P'
	mov byte [0xb8000+160*2+1], 0X07
	mov byte [0xb8000+160*2+2], 'R'
	mov byte [0xb8000+160*2+3], 0X07
	mov byte [0xb8000+160*2+4], 'O'
	mov byte [0xb8000+160*2+5], 0X07
	mov byte [0xb8000+160*2+6], 'T'
	mov byte [0xb8000+160*2+7], 0X07
	mov byte [0xb8000+160*2+8], 'E'
	mov byte [0xb8000+160*2+9], 0X07
	mov byte [0xb8000+160*2+10], 'C'
	mov byte [0xb8000+160*2+11], 0X07
	mov byte [0xb8000+160*2+12], 'T'
	mov byte [0xb8000+160*2+13], 0X07
	
	;从elf内核文件中读取内核的代码段和数据段到1M的位置
	call ReadKernel
	;这个时候，我们就可以跳转到1M这个地方去执行我们的内核了
	;由于在makefile中我们制定了-e _start,所以，就会把kernel/_start.asm中的
	;_start标签最为整个内核程序的入口地址，也就是说_start实在代码的最前面，
	;这个地方跳转过去，就是相当于跳转到_start执行
	jmp 0X08:KERNEL_START_ADDR
	
	push eax
	jmp $

; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
;把内核的代码段和数据段从elf文件中读取到1个对应的内存地址中
ReadKernel:
	xor	esi, esi
	mov	cx, word [KERNEL_PHY_ADDR + 2Ch]; ┓ ecx <- pELFHdr->e_phnum
	movzx	ecx, cx					;
	mov	esi, [KERNEL_PHY_ADDR + 1Ch]	; esi <- pELFHdr->e_phoff
	add	esi, KERNEL_PHY_ADDR		; esi <- OffsetOfKernel + pELFHdr->e_phoff
.begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				; PT_NULL
	jz	.unaction
	push	dword [esi + 010h]		; size	┓
	mov	eax, [esi + 04h]		;	┃
	add	eax, KERNEL_PHY_ADDR	;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax				; src	┃		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	┃		pPHdr->p_filesz;
	call	memcpy				;	┃
	add	esp, 12				;	┛
.unaction:
	add	esi, 020h			; esi += pELFHdr->e_phentsize
	dec	ecx
	jnz	.begin
	ret
	
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回	
	
;fill it with 1kb
times (4096-($-$$)) db 0

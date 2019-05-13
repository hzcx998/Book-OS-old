;path: kernel/switch_to.asm
;auther: huzicheng
;time: 2019/5/2
;copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.

[section .text]
[bits 32]

global switch_to
switch_to:
	push esi
	push edi
	push ebx 
	push ebp
	
	mov eax, [esp + 20]
	mov [eax], esp
	
	mov eax, [esp + 24]
	mov esp, [eax]
	
	pop ebp
	pop ebx
	pop edi
	pop esi
	ret
	
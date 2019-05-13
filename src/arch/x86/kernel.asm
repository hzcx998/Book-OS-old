;path: arch/x86/kernel.asm
;auther: huzicheng
;time: 2019/5/2
;copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.

[section .text]
[bits 32]
global x86_cpuid
x86_cpuid:	; void x86_cpuid(int id_eax, int id_ecx, int *eax, int *ebx, int *ecx, int *edx);
	pushad
	mov		eax,[esp+36]		; id_eax
	mov		ecx,[esp+40]		; id_ecx
	db		0x0F, 0xA2			; CPUID
	mov edi,[esp+44]
	mov [edi], eax
	mov edi,[esp+48]
	mov dword [edi], ebx
	mov edi,[esp+52]
	mov dword [edi], ecx
	mov edi,[esp+56]
	mov dword [edi], edx
	popad
	ret
	
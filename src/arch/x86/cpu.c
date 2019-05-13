/*
File:		kernel/cpu.c
Contains:	get cpu info 
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>
#include <sys/core.h>

#include <string.h>

struct cpu cpu;

void init_cpu()
{
	printk("> init cpu start.\n");
	int i;
	
	int eax, ebx, ecx, edx;
	
	//vendor ID
	x86_cpuid(0x00000000,0x00, &eax, &ebx, &ecx, &edx);
	cpu.max_cpuid = eax;
	
	memcpy(cpu.vendor_string    , &ebx, 4);
	memcpy(cpu.vendor_string + 4, &edx, 4);
	memcpy(cpu.vendor_string + 8, &ecx, 4);
	cpu.vendor_string[12] = '\0';

	//Processor Signiture and Feature
	x86_cpuid(0x00000001, 0x0, &eax, &ebx, &ecx, &edx);
	cpu.family   = (((eax >> 20) & 0xFF) << 4)
             + ((eax >> 8) & 0xF);
	cpu.model    = (((eax >> 16) & 0xF) << 4)
             + ((eax >> 4) & 0xF);
	cpu.stepping = (eax >> 0) & 0xF;
	
	//Extern
	x86_cpuid(0x80000000, 0x0, &eax, &ebx, &ecx, &edx);
	cpu.max_cpuid_ex = eax;
	if(cpu.max_cpuid_ex >= 0x80000001){
		x86_cpuid(0x80000001, 0x0, &eax, &ebx, &ecx, &edx);
		cpu.family_ex   = (((eax >> 20) & 0xFF) << 4)
                    + ((eax >> 8) & 0xF);
		cpu.model_ex    = (((eax >> 16) & 0xF) << 4)
                    + ((eax >> 4) & 0xF);
		cpu.stepping_ex = (eax >> 0) & 0xF;
		}
	if(cpu.max_cpuid_ex >= 0x80000004){
		x86_cpuid(0x80000002, 0x0, &eax, &ebx, &ecx, &edx);
		memcpy(cpu.name_string      , &eax, 4);
		memcpy(cpu.name_string  +  4, &ebx, 4);
		memcpy(cpu.name_string  +  8, &ecx, 4);
		memcpy(cpu.name_string  + 12, &edx, 4);
		x86_cpuid(0x80000003, 0x0, &eax, &ebx, &ecx, &edx);
		memcpy(cpu.name_string  + 16, &eax, 4);
		memcpy(cpu.name_string  + 20, &ebx, 4);
		memcpy(cpu.name_string  + 24, &ecx, 4);
		memcpy(cpu.name_string  + 28, &edx, 4);
		x86_cpuid(0x80000004, 0x0, &eax, &ebx, &ecx, &edx);
		memcpy(cpu.name_string  + 32, &eax, 4);
		memcpy(cpu.name_string  + 36, &ebx, 4);
		memcpy(cpu.name_string  + 40, &ecx, 4);
		memcpy(cpu.name_string  + 44, &edx, 4);
		cpu.name_string[49] = '\0';
		for (i = 0; i < 49; i++){
			if (cpu.name_string[i] > 0x20) {
				break;
			}
		}
	}
	print_cpu();
	printk("< init cpu done.\n");
	
}

void print_cpu()
{
	#ifdef _DEBUG_CPU_
		printk("=====CPU info=====\n");
		printk("vendor:%s \n", cpu.vendor_string);
		printk("name:%s \n", cpu.name_string);
		printk("family:%d model:%d stepping:%d\n", cpu.family, cpu.model, cpu.stepping);
		printk("family_ex:%d model_ex:%d stepping_ex:%d\n", cpu.family_ex, cpu.model_ex, cpu.stepping_ex);
		printk("max_cpuid:%x max_cpuid_ex:%x\n", cpu.max_cpuid, cpu.max_cpuid_ex);
	#endif
}

#endif //_CONFIG_ARCH_X86_


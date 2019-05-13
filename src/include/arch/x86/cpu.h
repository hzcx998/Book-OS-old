#ifndef _CPU_H_
#define _CPU_H_
#include <types.h>

/*
cpu结构体
*/
struct cpu 
{
	char vendor_string[16];
	char name_string[50];
	int family, model, stepping;
	int family_ex, model_ex, stepping_ex;
	int max_cpuid, max_cpuid_ex;
};

extern struct cpu cpu;

void init_cpu();
void print_cpu();

void x86_cpuid(int id_eax, int id_ecx, int *eax, int *ebx, int *ecx, int *edx);

#endif


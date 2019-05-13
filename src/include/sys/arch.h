#ifndef _ARCH_H
#define _ARCH_H

#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_
   #include <arch/x86/8259a.h>
   #include <arch/x86/ards.h>
   #include <arch/x86/cmos.h>
   #include <arch/x86/cpu.h>
   #include <arch/x86/descriptor.h>
   #include <arch/x86/page.h>
   #include <arch/x86/pci.h>
   #include <arch/x86/tss.h>
   #include <arch/x86/x86.h>

#endif



#endif   //_ARCH_H

#ifndef _SYSTEM_H_
#define	_SYSTEM_H_

#include <stdint.h>

#define BOOT_KBD 1
#define BOOT_BIOS 2		/*实验失败*/
#define BOOT_CF9 3
#define BOOT_TRIPLE 4

void do_reboot(int reboot_type);
int32_t sys_reboot(int reboot_type);

#endif	/*_SYSTEM_H_*/
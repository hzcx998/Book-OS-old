/*
File:		kernel/8259a.c
Contains:	init 8259a
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>

void init_8259a(void)
{
	io_out8(PIC0_IMR,  0xff  );
	io_out8(PIC1_IMR,  0xff  );

	io_out8(PIC0_ICW1, 0x11  );
	io_out8(PIC0_ICW2, 0x20  );
	io_out8(PIC0_ICW3, 1 << 2);
	io_out8(PIC0_ICW4, 0x01  );

	io_out8(PIC1_ICW1, 0x11  );
	io_out8(PIC1_ICW2, 0x28  );
	io_out8(PIC1_ICW3, 2     );
	io_out8(PIC1_ICW4, 0x01  );

	io_out8(PIC0_IMR,  0xff  );
	io_out8(PIC1_IMR,  0xff  );
	//put_str(">init 8259a.\n");
	return;
}


#endif //_CONFIG_ARCH_X86_


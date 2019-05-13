#ifndef _CORE_H
#define _CORE_H

#include <sys/config.h>
#include <stdint.h>

#ifdef _CONFIG_CORE_
	#include <core/bitmap.h>
	#include <core/console.h>
	#include <core/ioqueue.h>
	#include <core/process.h>
	#include <core/sync.h>
	#include <core/syscall.h>
	#include <core/system.h>
	#include <core/thread.h>
	#include <core/timer.h>
	#include <core/print.h>
	#include <core/pipe.h>
	
#endif //_CONFIG_CORE_

#endif //_CORE_H
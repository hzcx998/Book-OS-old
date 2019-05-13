#ifndef _DEV_H
#define _DEV_H

#include <sys/config.h>
#include <stdint.h>

#ifdef _CONFIG_DEVIVE_

	#include <dev/device.h>

	#ifdef _CONFIG_DEVIVE_HARDDISK_
		#include <dev/harddisk.h>
	#endif

	#ifdef _CONFIG_DEVIVE_RAMDISK_
		#include <dev/ramdisk.h>
	#endif

	#ifdef _CONFIG_DEVIVE_MOUSE_
		#include <dev/mouse.h>
	#endif

	#ifdef _CONFIG_DEVIVE_KEYBOARD_
		#include <dev/keyboard.h>
		
	#endif

	#ifdef _CONFIG_DEVIVE_VGA_
		#include <dev/vga.h>
	#endif

	#ifdef _CONFIG_DEVIVE_VIDEO_
		#include <dev/video.h>
	#endif
	
	#ifdef _CONFIG_DEVIVE_CLOCK_
		#include <dev/clock.h>
	#endif

#endif //_CONFIG_DEVIVE_



#endif //_DEV_H
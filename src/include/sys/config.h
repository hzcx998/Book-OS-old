#ifndef _CONFIG_H_
#define _CONFIG_H_

/*-----
- arch config 
-----*/

#define _CONFIG_ARCH_X86_ 
//#define _CONFIG_ARCH_ARM_ 


/*-----
- dev config 
-----*/
#define _CONFIG_DEVIVE_ 

	#ifdef _CONFIG_DEVIVE_

		#define _CONFIG_DEVIVE_HARDDISK_
		#define _CONFIG_DEVIVE_RAMDISK_
		#define _CONFIG_DEVIVE_MOUSE_
		#define _CONFIG_DEVIVE_KEYBOARD_
		#define _CONFIG_DEVIVE_VGA_
		#define _CONFIG_DEVIVE_VIDEO_
		#define _CONFIG_DEVIVE_CLOCK_
		
			#ifdef _CONFIG_DEVIVE_VIDEO_
				#define _VIDEO_16_MODE_
				//#define _VIDEO_24_MODE_
			
			#endif
			
	#endif

/*-----
- core config 
-----*/
#define _CONFIG_CORE_		

/*-----
- fs config 
-----*/
#define _CONFIG_FS_BOFS_		// Bofs	file system

/*-----
- gui config 
-----*/
#define _CONFIG_GUI_BOGUI_		// Bogui graphic
#define _CONFIG_GUI_BASIC_		// Bogui graphic

/*-----
- mm config 
-----*/
#define _CONFIG_MM_BASIC_
/*-----
- net config 
-----*/
#define _CONFIG_NET_ETHERNET_	//ethernet
	
//#define _CONFIG_NET_RTL8139_	//rtl8139 network card*/

//#define _CONFIG_NET_AM79C973_	//am79c973 network card
		
//#define _CONFIG_NET_AM79C970A_	//am79c970a network card

/*-----
- sys config 
-----*/
#define _CONFIG_BYTE_ORDER_LE_		//LE little-endian
//#define _CONFIG_BYTE_ORDER_BE_		//BE big-endian


/*-----
- debug config 
-----*/
//debug config
#define _SYS_DEBUG_ 
	#ifdef _SYS_DEBUG_

		#define _DEBUG_CORE_
		#define _DEBUG_FS_
		#define _DEBUG_MM_
		#define _DEBUG_NET_
		#define _DEBUG_LIB_
		#define _DEBUG_DEVICE_
		#define _DEBUG_GUI_

		#ifdef _DEBUG_CORE_
			//#define _DEBUG_CPU_

		#endif

		#ifdef _DEBUG_FS_
			//#define _DEBUG_BOFS_

		#endif

		#ifdef _DEBUG_DEVICE_
			//#define _DEBUG_PCI_
			//#define _DEBUG_HD_
			//#define _DEBUG_RAMDISK_
			//#define _DEBUG_KEYBOARD_
			//#define _DEBUG_MOUSE_
			
		#endif

		#ifdef _DEBUG_NET_
			//#define _DEBUG_AM79C973_
			//#define _DEBUG_RTL8139_
			#define _DEBUG_AM79C970A_

		#endif

	#endif


#endif

;boot/const.inc
;description: boot&loader's const var
;auther: Hu Zicheng
;time: 2018/1/23
;Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
;E-mail: 2323168280@qq.com

;----boot const----
;we read loader.bin from disk sector 2
LOADER_OFF EQU 2	

;we assume loader.bin size is 4kb(8 sectors)	
LOADER_CNTS EQU 8		

;we read loader.bin to memory 0x90000
;so the segment is 0x9000, we are in real mode now.
LOADER_SEG EQU 0x9000	

;----loader const----
;we read kernel.elf from disk sector 10 
KERNEL_OFF EQU 10	

;----we assume that kernel max is 200 kb (0x32000 byte)----

;we read kernel.elf to memory 0x10000
;so the segment is 0x1000, we are in real mode now.
KERNEL_SEG EQU 0x1000

;kernel.elf phy addr is KERNEL_SEG*16, 
;0x1000*16 = 0x10000
KERNEL_PHY_ADDR EQU KERNEL_SEG*16

;----we assume that file max is 312 kb (0x4e000 byte)----
;we read kernel.elf from disk sector 10 
FILE_OFF EQU 500

FILE_SEG EQU 0x4200


;we will read kernel from kerne.elf to 1M after we jmp to protect mode,
;so this is our kernel finally addr
KERNEL_START_ADDR EQU 0x100000

;loader stack top is 0x90000
LOADER_STACK_TOP EQU 0x90000

;we can read sector less than 128 sector. so I set block size,
;every time we load 128 sectors
BLOCK_SIZE EQU 128

;memory manage, load memory info in ards, we will use it when
;we init memory
;ards zone(0x500~0x600)
ARDS_SEG	EQU		0x50	;ards start segment
ARDS_ZONE_SIZE	EQU		0x100	;ards zone

;vbe info is for graphic
VBEINFO_SEG	EQU		0x60	
VBEINFO_ZONE_SIZE	EQU		0x100	

VBEMODE_SEG	EQU		0x70	
VBEMODE_ZONE_SIZE	EQU		0x100	

VIDEO_INFO_SEG	EQU		0x80	
VIDEO_INFO_ZONE_SIZE	EQU		0x100	
;16 bits
MODE_640_480_16 EQU 0x111
MODE_800_600_16 EQU 0x114
MODE_1024_768_16 EQU 0x117
MODE_1280_1024_16 EQU 0x11A

;24 bits
;24bit 不能再vmware中使用，不知道为何，可能是不支持
MODE_640_480_24 EQU 0x112
MODE_800_600_24 EQU 0x115
MODE_1024_768_24 EQU 0x118
MODE_1280_1024_24 EQU 0x11B
MODE_VMWARE_800_600_24	EQU	0x4140

VBEMODE	EQU	MODE_800_600_16

BITS_PER_PIXE	EQU		0		; 每个颜色占用多少像素
VIDEO_WIDTH	EQU		2			; x的分辨率	宽度
VIDEO_HEIGHT	EQU		4			; y的分辨率	高度
VIDEO_RAM	EQU		6			; VRAM 地址

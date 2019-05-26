#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd

#The file path
SYS_DIR = ../../src/include/
LIB_DIR = ../../src/lib/

CLI_DIR = cli/
COMPILER_DIR = compiler/
GC_DIR = gc/
OBJECT_DIR = object/
PARSER_DIR = parser/
VM_DIR = vm/
INCLUDE_DIR = include/

BIN = bin

BIN_OFF = 0 
BIN_CNTS = 600

#img dir
IMG_DIR = ../../img/

DISK_IMG = d.img

DISK_VHD = d.vhd

HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)

#flags
ASM_FLAGS	= -I $(SYS_DIR) -f elf

LDFLAGS		= -e _start -Ttext 0x80000000

#CFLAGS = -g -DDEBUG -lm -Wall -I object -I vm -I compiler -I parser -I include -I cli -I gc -W -Wstrict-prototypes -Wmissing-prototypes -Wsystem-headers
C_FLAGS = -c -fno-builtin -std=c99 -g -lm  \
	-I $(SYS_DIR) -I $(CLI_DIR) -I $(COMPILER_DIR) -I $(GC_DIR) \
	-I $(OBJECT_DIR) -I $(PARSER_DIR) -I $(VM_DIR) -I $(INCLUDE_DIR) \
	-Wall -Wunused

#DIRS =  object include cli compiler parser vm gc  
#CFILES = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.c))

MAIN_OBJS =  _start.o \
		main.o

CLI_OBJS =   $(CLI_DIR)cli.o \

COMPILER_OBJS =   $(COMPILER_DIR)compiler.o \
		$(COMPILER_DIR)debug.o 

GC_OBJS =   $(GC_DIR)gc.o \

OBJECT_OBJS =   $(OBJECT_DIR)class.o \
		$(OBJECT_DIR)header_obj.o \
		$(OBJECT_DIR)meta_obj.o \
		$(OBJECT_DIR)obj_fn.o \
		$(OBJECT_DIR)obj_list.o \
		$(OBJECT_DIR)obj_map.o \
		$(OBJECT_DIR)obj_range.o \
		$(OBJECT_DIR)obj_string.o \
		$(OBJECT_DIR)obj_thread.o

PARSER_OBJS =   $(PARSER_DIR)parser.o \
		
VM_OBJS =   $(VM_DIR)core.o \
		$(VM_DIR)vm.o

INCLUDE_OBJS =   $(INCLUDE_DIR)unicodeUtf8.o \
		$(INCLUDE_DIR)utils.o

LD_OBJS = $(LIB_DIR)printf.o\
		$(LIB_DIR)malloc.o\
		$(LIB_DIR)free.o\
		$(LIB_DIR)exit.o\
		$(LIB_DIR)getchar.o\
		$(LIB_DIR)putchar.o\
		$(LIB_DIR)write.o\
		$(LIB_DIR)vsprintf.o\
		$(LIB_DIR)string.o\
		$(LIB_DIR)fopen.o\
		$(LIB_DIR)fclose.o\
		$(LIB_DIR)fread.o\
		$(LIB_DIR)fstat.o\
		$(LIB_DIR)lseek.o\
		$(LIB_DIR)gettime.o\
		$(LIB_DIR)mm.o\
		$(LIB_DIR)fsa.o\
		$(LIB_DIR)thread.o\
		$(LIB_DIR)pipea.o\
		$(LIB_DIR)pipec.o \
		$(LIB_DIR)ctype.o \
		$(LIB_DIR)math.o
		
OBJS =  $(MAIN_OBJS) $(LD_OBJS) $(CLI_OBJS) $(COMPILER_OBJS) $(GC_OBJS) \
		$(OBJECT_OBJS) $(PARSER_OBJS) $(VM_OBJS) $(INCLUDE_OBJS)

#OBJS = $(patsubst %.c,%.o,$(CFILES)) 
#$(OBJS):$(CFILES)
#	$(CC) $(CFLAGS) -o $@ $<

#First read here
.PHONY: all

all: compile link img

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS)

img:
	$(DD) if=$(BIN) of=$(HD_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
vhd:
	$(DD) if=$(BIN) of=$(HD_VHD) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	
qemu: compile link img
	
vbox: compile link vhd

#Clean temporary files
clean:
	-rm bin
	-$(RM) $(OBJS)

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<

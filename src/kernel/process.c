/*
File:		kernel/process.c
Contains:	user process
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/

#include <string.h>
#include <sys/core.h>
#include <sys/mm.h>
#include <sys/debug.h>
#include <sys/fs.h>
#include <math.h>
#include <sys/arch.h>

typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* 32位elf头 */
struct Elf32_Ehdr {
   unsigned char e_ident[16];
   Elf32_Half    e_type;
   Elf32_Half    e_machine;
   Elf32_Word    e_version;
   Elf32_Addr    e_entry;
   Elf32_Off     e_phoff;
   Elf32_Off     e_shoff;
   Elf32_Word    e_flags;
   Elf32_Half    e_ehsize;
   Elf32_Half    e_phentsize;
   Elf32_Half    e_phnum;
   Elf32_Half    e_shentsize;
   Elf32_Half    e_shnum;
   Elf32_Half    e_shstrndx;
};

/* 程序头表Program header.就是段描述头 */
struct Elf32_Phdr {
   Elf32_Word p_type;		 // 见下面的enum segment_type
   Elf32_Off  p_offset;
   Elf32_Addr p_vaddr;
   Elf32_Addr p_paddr;
   Elf32_Word p_filesz;
   Elf32_Word p_memsz;
   Elf32_Word p_flags;
   Elf32_Word p_align;
};

/* 段类型 */
enum segment_type {
   PT_NULL,            // 忽略
   PT_LOAD,            // 可加载程序段
   PT_DYNAMIC,         // 动态加载信息 
   PT_INTERP,          // 动态加载器名称
   PT_NOTE,            // 一些辅助信息
   PT_SHLIB,           // 保留
   PT_PHDR             // 程序头表
};

extern void thread_intr_exit(struct intr_stack *proc_stack);
void process_use_bitmap(struct thread *thread, int count);

void start_process(void *filename)
{
	void *function = filename;
	struct thread *cur = thread_current();
	cur->self_stack += sizeof(struct thread_stack);
	struct intr_stack *proc_stack = (struct intr_stack *)cur->self_stack;
	proc_stack->edi = proc_stack->esi = \
	proc_stack->ebp = proc_stack->esp_dummy = 0;
	
	proc_stack->ebx = proc_stack->edx = \
	proc_stack->ecx = proc_stack->eax = 0;
	
	proc_stack->gs = proc_stack->ds = \
	proc_stack->es = proc_stack->fs = USER_DATA_SEL;
	
	proc_stack->cs = USER_CODE_SEL;
	proc_stack->eip = function;
	
	proc_stack->eflags = (EFLAGS_MBS|EFLAGS_IF_1|EFLAGS_IOPL_0);
	
	proc_stack->esp = (void *)((uint32)thread_use_vaddr(USER_STACK3_ADDR) + PAGE_SIZE);
	//proc_stack->esp = (void *)(mm.malloc(PAGE_SIZE) + PAGE_SIZE);
	
	proc_stack->ss = USER_STACK_SEL;
	
	thread_intr_exit(proc_stack);
}

void page_dir_activate(struct thread *thread)
{
	uint32 pagedir_phy_addr = PAGE_DIR_PHY_ADDR;
	
	if(thread->pdir != NULL){
		pagedir_phy_addr = addr_v2p((uint32) thread->pdir);
	}
	write_cr3(pagedir_phy_addr);
}

void process_activate(struct thread *thread)
{
	assert(thread != NULL);
	page_dir_activate(thread);
	
	if(thread->pdir){
		update_tss_esp(thread);
	}
}

uint32 *create_page_dir()
{
	uint32 *page_dir_vaddr = (uint32 *)mm.malloc(PAGE_SIZE);
	if(page_dir_vaddr == NULL){
		printk("create_page_dir: mm.malloc failed!\n");
		return NULL;
	}
	memcpy((void *)page_dir_vaddr, (void *)PAGE_DIR_VIR_ADDR, 2048);
	
	uint32 new_page_dir_phy_addr = addr_v2p((uint32) page_dir_vaddr);
	page_dir_vaddr[1023] = new_page_dir_phy_addr|0x07;
	
	return page_dir_vaddr;
}
/*
运行用户态进程
*/
int process_execute(void *filename, char *name)
{
	struct thread *cur = thread_current();
	struct thread *thread = alloc_thread();
	thread_init(thread, name, THREAD_DEFAULT_PRO);
	//user vir bitmap
	thread_init_bitmap(thread);
	//父进程id设置
	thread->parent_pid = cur->pid;

	thread_create(thread, start_process, filename);
	thread->pdir = create_page_dir();
	init_thread_memory_manage(thread);
	
	int old_status = io_load_eflags();
	io_cli();
	
	thread_add(thread);
	
	io_store_eflags(old_status);
	return thread->pid;
}

/* 将文件描述符fd指向的文件中,偏移为offset,大小为filesz的段加载到虚拟地址为vaddr的内存 */
static bool segment_load(int32_t fd, uint32_t offset, uint32_t filesz, uint32_t vaddr) 
{
	uint32_t vaddr_first_page = vaddr & 0xfffff000;    // vaddr地址所在的页框
	uint32_t size_in_first_page = PAGE_SIZE - (vaddr & 0x00000fff);     // 加载到内存后,文件在第一个页框中占用的字节大小
	uint32_t occupy_pages = 0;
 	/* 若一个页框容不下该段 */
	//统计一共要占用多大的页
	if (filesz > size_in_first_page) {
		uint32_t left_size = filesz - size_in_first_page;
    	occupy_pages = DIV_ROUND_UP(left_size, PAGE_SIZE) + 1;	     // 1是指vaddr_first_page
   	} else {
    	occupy_pages = 1;
   	}

   	/* 为进程分配内存 */
	map_pages(vaddr_first_page,occupy_pages);

	//读取数据进入内存
   	fs.lseek(fd, offset, SEEK_SET);
   	if (fs.read(fd, (void*)vaddr, filesz) != filesz) {
		printk("read elf segment failed!\n");
		return false;
   	}
   	return true;
}

/* 从文件系统上加载用户程序pathname,成功则返回程序的起始地址,否则返回-1 */
static int32_t load(const char* pathname) 
{
   	int32_t ret = -1;
   	struct Elf32_Ehdr elf_header;
   	struct Elf32_Phdr prog_header;
   	memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));

	//打开文件
   	int32_t fd = fs.open(pathname, O_RDONLY);
   	if (fd == -1) {
		printk("file open failed!\n");
    	return -1;
   	}

	//读取elf header
   	if (fs.read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) != sizeof(struct Elf32_Ehdr)) {
    	ret = -1;
		printk("elf header size error!\n");
    	goto done;
   	}

   	/* 校验elf头 */
   	if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) \
    	|| elf_header.e_type != 2 \
     	|| elf_header.e_machine != 3 \
      	|| elf_header.e_version != 1 \
      	|| elf_header.e_phnum > 1024 \
      	|| elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
      	ret = -1;
	  	printk("elf header error!\n");
      	goto done;
   	}

   	Elf32_Off prog_header_offset = elf_header.e_phoff; 
   	Elf32_Half prog_header_size = elf_header.e_phentsize;

   	/* 遍历所有程序头 */
   	uint32_t prog_idx = 0;
   	while (prog_idx < elf_header.e_phnum) {
      	memset(&prog_header, 0, prog_header_size);
      
      	/* 将文件的指针定位到程序头 */
      	fs.lseek(fd, prog_header_offset, SEEK_SET);

     	/* 只获取程序头 */
      	if (fs.read(fd, &prog_header, prog_header_size) != prog_header_size) {
	 		ret = -1;
	 		printk("program header size error!\n");
	 		goto done;
      	}

      	/* 如果是可加载段就调用segment_load加载到内存 */
      	if (PT_LOAD == prog_header.p_type) {
			if (!segment_load(fd, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr)) {
				ret = -1;
				printk("load segment failed!\n");
				goto done;
			}
		}

      	/* 更新下一个程序头的偏移 */
      	prog_header_offset += elf_header.e_phentsize;
      	prog_idx++;
   	}
   	ret = elf_header.e_entry;
done:
   	fs.close(fd);
   	return ret;
}


/* 检测是否为ELF格式的文件 */
static int32_t ELF_file_access(const char* pathname)
{
   	int32_t ret = -1;
   	struct Elf32_Ehdr elf_header;
   	memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));

	//打开文件
   	int32_t fd = fs.open(pathname, O_RDONLY);
   	if (fd == -1) {
		printk("file open failed!\n");
    	return -1;
   	}

	//读取elf header
   	if (fs.read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) != sizeof(struct Elf32_Ehdr)) {
    	ret = -1;
		//printk("elf header size error!\n");
    	goto done;
   	}

   	/* 校验elf头 */
   	if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) \
    	|| elf_header.e_type != 2 \
     	|| elf_header.e_machine != 3 \
      	|| elf_header.e_version != 1 \
      	|| elf_header.e_phnum > 1024 \
      	|| elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
      	ret = -1;
	  	//printk("Not ELF format file!\n");
      	goto done;
   	}
	ret = 0;   
done:
   	fs.close(fd);
   	return ret;
}


int sys_execv(char *path, char *argv[])
{
	//printk("start execv.\n");
	struct thread *parent_thread = thread_current();

	struct thread *thread = alloc_thread();
	
	char name[MAX_FILE_NAME_LEN];
	memset(name,0,MAX_FILE_NAME_LEN);
	fs.path_to_name(path+2, name);
	//printk("path:%s name:%s\n", path, name);
	/*检测是否是后台程序*/
	int daemon = 0;
	int arg_idx = 0;
	if(argv != NULL){
		while(argv[arg_idx] != NULL){
			if(!strcmp(argv[arg_idx], "&")){
				daemon = 1;
			}
			arg_idx++;
		}
	}
	
	/*检测文件是否存在，是否是可执行文件*/

	if(fs.access(path, F_OK) == -1){
		printk("exec: path:%s file %s not found!\n", path, name);
		return -1;
	}
	if(fs.access(path, X_OK) == -1){
		printk("exec: path:%s file %s can't execute!\n", path, name);
		return -1;
	}

	if(ELF_file_access(path)){
		printk("exec: path:%s file %s not elf file!\n", path, name);
		return -1;
	}

	//printk("file check done!\n");
	
	thread_init(thread, name, THREAD_DEFAULT_PRO);
	//user vir bitmap
	thread_init_bitmap(thread);
	
	//做成包，然后传入进去
	thread->bag = mm.malloc(PAGE_SIZE);
	if(thread->bag == NULL){
		return -1;
	}
	
	*((int *)thread->bag) = make_arguments(thread->bag+4, argv);
	
	strcpy(thread->bag+(4096-128), path);
	
	thread_create(thread, load_process, NULL);
	thread->pdir = create_page_dir();
	
	if(!daemon){
		//不是后台程序
		thread->parent_pid = parent_thread->pid;
	}else{
		//后台程序，父进程不管他（弃婴-.-）
		thread->parent_pid = -1;
	}
	
	init_thread_memory_manage(thread);
	
	int old_status = io_load_eflags();
	io_cli();
	//printk("ready to run!\n");
	thread_add(thread);
	
	io_store_eflags(old_status);
	
	return thread->pid;
}

void load_process(void *path)
{
	struct thread *cur = thread_current();

	cur->self_stack += sizeof(struct thread_stack);
	struct intr_stack *proc_stack = (struct intr_stack *)cur->self_stack;
	proc_stack->edi = proc_stack->esi = \
	proc_stack->ebp = proc_stack->esp_dummy = 0;
	
	proc_stack->ebx = proc_stack->edx = \
	proc_stack->ecx = proc_stack->eax = 0;
	
	proc_stack->gs = proc_stack->ds = \
	proc_stack->es = proc_stack->fs = USER_DATA_SEL;
	
	proc_stack->cs = USER_CODE_SEL;
	
	proc_stack->eflags = (EFLAGS_MBS|EFLAGS_IF_1|EFLAGS_IOPL_0);
	
	proc_stack->esp = (void *)((uint32)thread_use_vaddr(USER_STACK3_ADDR) + PAGE_SIZE);
	//proc_stack->esp = (void *)(mm.malloc(PAGE_SIZE*2) + PAGE_SIZE*2);
	
	proc_stack->ss = USER_STACK_SEL;
	//解析包
	//load file 
	char *pathname = (char *)cur->bag+(4096-128);
	//printk("load file %s \n", pathname);
	
	int32_t fd = fs.open(pathname, O_RDONLY);
	if (fd == -1) {
		printk("execute: open %s faild!\n",pathname);
		//结束任务
		sys_exit(-1);
		
	}
	
	/*get file size*/
	struct stat fstat;
	fs.stat(pathname, &fstat);
	
	//5.link pages with parent vir memory.
	//how many pages should we occupy.
	uint32_t pages = DIV_ROUND_UP(fstat.st_size, PAGE_SIZE);

	process_use_bitmap(cur, pages);
	
	int32_t entry_point = load(pathname);

	if (entry_point == -1) {
		printk("load elf file failed!\n");
		//结束任务
		sys_exit(-1);
		
	}

	proc_stack->eip = (void *)entry_point;
	proc_stack->ebx = (uint32 )(cur->bag+4);
	proc_stack->ecx = *((int *)cur->bag);
	
	//while(1);
	thread_intr_exit(proc_stack);
}

void process_use_bitmap(struct thread *thread, int count)
{
	int i;
	int idx = bitmap_scan(&thread->vir_mem_bitmap, count);
	for(i = 0; i < count; i++){
		bitmap_set(&thread->vir_mem_bitmap, idx + i, 1);
	}
}

int32_t make_arguments(char *buf, char **argv)
{
	//11.make arguments
	uint32_t argc = 0;
	//把参数放到栈中去
	char *arg_stack = (char *)buf;
	if(argv != NULL){
		
		//先复制下来
		while (argv[argc]) {  
			argc++;
		}
		//printk("argc %d\n", argc);
		//构建整个栈
		
		//临时字符串
		int str_stack[STACK_ARGC_MAX/4];
		
		int stack_len = 0;
		//先预留出字符串的指针
		int i;
		for(i = 0; i < argc; i++){
			stack_len += 4;
		}
		//printk("stack_len %d\n", stack_len);
		int idx, len;
		
		for(idx = 0; idx < argc; idx++){
			len = strlen(argv[idx]);
			memcpy(arg_stack + stack_len, argv[idx], len);
			str_stack[idx] = (int )(arg_stack + stack_len);
			//printk("str ptr %x\n",str_stack[idx]);
			arg_stack[stack_len + len + 1] = 0;
			stack_len += len + 1;
			//printk("len %d pos %d\n",len, stack_len);
		}
		//转换成int指针   
		int *p = (int *)arg_stack;
		//重新填写地址
		for(idx = 0; idx < argc; idx++){
			p[idx] = str_stack[idx];
			//printk("ptr %x\n",p[idx]);
		}
		//p[argc] = NULL;
		//指向地址
		//char **argv2 = (char **)arg_stack;
		
		//argv2[argc] = NULL;
		//打印参数
		for (i = 0; i < argc; i++) {  
			//printk("args:%s\n",argv2[i]);
		}
		
	}
	return argc;
}

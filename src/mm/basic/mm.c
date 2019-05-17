/*
File:		mm/basic/main.c
Contains:	basic mm
Auther:		Hu Zicheng
Time:		2019/5/1
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_MM_BASIC_

#include <sys/arch.h>
#include <string.h>
#include <math.h>
#include <sys/mm.h>
#include <sys/core.h>
#include <sys/debug.h>

uint32_t memory_total_size;
int memory_used_size;
extern struct bitmap phy_mem_bitmap;
extern struct bitmap vir_mem_bitmap;

//�ڴ�������ڴ��У�ҳ�з��䣬�����ں���ռ�ռ�
struct memory_manage *memory_manage;

//static void memory_test();
void memory_init()
{
	memory_total_size = 0;
	//��ʼ��ȡ
	init_ards();
	
	if(memory_total_size > 0XFFFFFFF0){
		memory_total_size = 0XFFFFFFF0;
	}
	/*�ܹ�-�̶�ռ�� = ���д�С*/
	int page_bytes = (memory_total_size-PHY_MEM_BASE_ADDR)/(PAGE_SIZE*8);
	
	phy_mem_bitmap.bits = (uint8_t*)PHY_MEM_BITMAP;
	phy_mem_bitmap.btmp_bytes_len = page_bytes;
	//b1
	
	vir_mem_bitmap.bits = (uint8_t*)VIR_MEM_BITMAP;
	vir_mem_bitmap.btmp_bytes_len = PHY_MEM_BITMAP_SIZE-PHY_MEM_BASE_ADDR/(PAGE_SIZE*8);
	//b2
	
	//memset(vir_mem_bitmap.bits, 0, phy_mem_bitmap.btmp_bytes_len);
	
	bitmap_init(&phy_mem_bitmap);
	//while(1);
	bitmap_init(&vir_mem_bitmap);
	//b3
	init_memory_manage( );
	
	
	
	
	//clean low 2G
	//memset((void *)(PAGE_DIR_VIR_ADDR+4),0,2044);
	//b4
	//
	
	printk(">Init memory.\n");
	
	//memory_test();
	

}

void sys_get_memory(int *size, int *free)
{
	*size = memory_total_size;
	*free = get_free_memory();
}


/*
static void memory_test()
{
	
	char *addr = kernel_alloc_page(1);
	printk("addr:%x\n", addr);
	
	
	
	uint32_t *a;
	a = kmalloc(32);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(32);
	printk("%x\n", a);
	a = kmalloc(1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024);
	printk("%x\n", a);
	a = kmalloc(4096);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(4096);
	printk("%x\n", a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(1024*1024);
	printk("%x\n", a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
	kfree(a);
	a = kmalloc(10*1024*1024);
	printk("%x\n", a);
}
*/
void init_memory_manage(void )
{
	int i;
	uint32_t memory_manage_pages = DIV_ROUND_UP(sizeof(struct memory_manage),PAGE_SIZE);
	/*Ϊ�ṹ�����ռ�*/
	memory_manage = (struct memory_manage *)kernel_alloc_page(memory_manage_pages);
	if(memory_manage == NULL){
		panic("memory manage failed!");
	}
	memset(memory_manage, 0, memory_manage_pages*PAGE_SIZE);
	for(i = 0; i < MEMORY_BLOCKS; i++){	
		memory_manage->free_blocks[i].size = 0;	//��С��ҳ������
		memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
	}
}

void *kernel_malloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//Ҫ�����ʲô��С
	uint32_t break_cnt;//Ҫ����ɼ���
	void *new_address;
	
	//����1024�ֽھ���ҳ
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//һ��ռ���ٸ�ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = (uint32_t )kernel_alloc_page(pages);	//����ҳ
				memory_manage->free_blocks[i].address = address;	
				memory_manage->free_blocks[i].size = pages;	//��С��ҳ������
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		//�����жϣ�Ҫ��ɢ�ɶ��
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		//��һ��Ѱ�ң�����ڿ���û���ҵ����ʹ�ɢһ��ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
		//�����û���ҵ�������һ��ҳ��Ȼ���ɢ
		//����һ��ҳ����������ɢ
		new_address = kernel_alloc_page(1);
		break_cnt = PAGE_SIZE/break_size;
		
		//��ɢ��break_cnt��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�һ�����Ա�ʹ�õ�
				//��ַ����
				
				//�������յ�ַ
				memory_manage->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//����size
				memory_manage->free_blocks[i].size = break_size;
				//����Ϊ���Է���
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//����ΪС��ģʽ
				memory_manage->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//��ɢ���Ѱ��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(memory_manage->free_blocks[i].size == break_size && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = memory_manage->free_blocks[i].address;
				memory_manage->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("Alloc:%x idx:%d\n", address,i);
				return (void *)address;
			}
		}
	}
	//size=0����û���ҵ�
	return NULL;	//ʧ��
}

int kernel_free(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(memory_manage->free_blocks[i].address == addr && memory_manage->free_blocks[i].flags == MEMORY_BLOCK_USING){	//�ҵ�
			if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				kernel_free_page(memory_manage->free_blocks[i].address, memory_manage->free_blocks[i].size);
				memory_manage->free_blocks[i].size = 0;		//ֻ�д�����Ҫ��������size
			}else if(memory_manage->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//С���ڴ����վ�����
				memset((void *)memory_manage->free_blocks[i].address, 0, memory_manage->free_blocks[i].size);
				//����һ��������Ǿ������б���ɢ���ڴ涼���ͷź󣬿�����Ҫ�ͷ��Ǹ�ҳ��Ŀǰ��û�п�����
				//С�鲻��Ҫ���ô�С����Ϊ���Ǵ�ɢ�˵Ŀ�
			}
			memory_manage->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	
	return -1;	//ʧ��
}
struct memory_block *get_memory_block(struct memory_manage *manager, void *address)
{
	int i;
	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(manager->free_blocks[i].address == (uint32_t)address && \
			manager->free_blocks[i].flags == MEMORY_BLOCK_USING){	//如果地址相等并且还是使用中的

			return &manager->free_blocks[i];
		}
	}
	return NULL;
}

void *kernel_realloc(void *mem_address, unsigned int newsize)
{
	//如果地址是NULL，就相当于malloc
	if (mem_address == NULL) {
		//printk("addr is NULL\n");
		return kernel_malloc(newsize);
	}

	//如果大小是0，就相当于free
	if (newsize == 0) {
		//printk("size is 0\n");
		kernel_free(mem_address);
		return NULL;
	}

	struct memory_block *mem_block;

	mem_block = get_memory_block(memory_manage, mem_address);

	//如果没有找到对应的内存块
	if (mem_block == NULL) {
		//printk("realloc invalid pointer\n");
		return NULL;
	}

	//如果新的大小和原来的一样发或者小
	if (newsize <= mem_block->size ) {
		//设定新的大小
		mem_block->size = newsize;

		//让数据丢失
		//printk("size <= old size.\n");
		//返回原来的地址
		return (void *)mem_block->address;
	}

	//新的大小比原来的大

	/*
	mem_adrr 所在内存是否有newsize的连续空间检测，由于算法原因，我们默认就没有
	*/

	//分配一个对应大小的内存
	void *address = kernel_malloc(newsize);
	if (address == NULL) {
		//分配失败
		return NULL;
	}

	//把旧空间的数据拷贝到新空间
	memcpy(address, (void *)mem_block->address, mem_block->size);

	//释放掉旧的空间
	kernel_free((void *)mem_block->address);

	//返回新的地址
	return address;
}


void *user_malloc(uint32_t size)
{
	int i;
	uint32_t address;
	uint32_t break_size;//Ҫ�����ʲô��С
	uint32_t break_cnt;//Ҫ����ɼ���
	void *new_address;
	struct thread *thread = thread_current();
	struct memory_manage *thread_mm = thread->memory_manager;

	//����1024�ֽھ���ҳ
	if(size >= 2048){
		int pages = DIV_ROUND_UP(size, PAGE_SIZE);	//һ��ռ���ٸ�ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = (uint32_t )thread_alloc_page(thread, pages);	//����ҳ
				thread_mm->free_blocks[i].address = address;	
				thread_mm->free_blocks[i].size = pages;	//��С��ҳ������
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				thread_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_BIG;
				//printk("Found pages ");
				//printk("1 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}else if(0 < size &&size <= 2048){	//size <= 2048
		
		//�����жϣ�Ҫ��ɢ�ɶ��
		if(0 < size && size <= 32){
			break_size = 32;
		}else if(32 < size && size <= 64){
			break_size = 64;
		}else if(64 < size && size <= 128){
			break_size = 128;
		}else if(128 < size && size <= 256){
			break_size = 256;
		}else if(256 < size && size <= 512){
			break_size = 512;
		}else if(512 < size && size <= 1024){
			break_size = 1024;
		}else if(1024 < size && size <= 2048){
			break_size = 2048;
		}
		
		//��һ��Ѱ�ң�����ڿ���û���ҵ����ʹ�ɢһ��ҳ
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].size == break_size && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = thread_mm->free_blocks[i].address;
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found broken ");
				//printk("2 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
		//�����û���ҵ�������һ��ҳ��Ȼ���ɢ
		//����һ��ҳ����������ɢ
		new_address = thread_alloc_page(thread, 1);
		break_cnt = PAGE_SIZE/break_size;
		//printk("*addr %x broken into %d parts\n", new_address, break_cnt);
		//��ɢ��break_cnt��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			
			if(thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�һ�����Ա�ʹ�õ�
				//��ַ����
				//�������յ�ַ
				thread_mm->free_blocks[i].address = (uint32_t)new_address;
				new_address += break_size;
				//����size
				thread_mm->free_blocks[i].size = break_size;
				//����Ϊ���Է���
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
				//����ΪС��ģʽ
				thread_mm->free_blocks[i].mode = MEMORY_BLOCK_MODE_SMALL;
				break_cnt--;
				if(break_cnt <= 0){
					break;
				}
			}
		}
		//��ɢ���Ѱ��
		for(i = 0; i < MEMORY_BLOCKS; i++){
			if(thread_mm->free_blocks[i].size == break_size && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_FREE){	//�ҵ�
				address = thread_mm->free_blocks[i].address;
				thread_mm->free_blocks[i].flags = MEMORY_BLOCK_USING;
				//printk("Found new broken ");
				//printk("3 Alloc:%x idx:%d size:%d size:%d \n", address,i, thread_mm->free_blocks[i].size, size);
				return (void *)address;
			}
		}
	}
	//size=0����û���ҵ�
	return NULL;	//ʧ��
}

int user_free(void *address)
{
	int i;
	uint32_t addr = (uint32_t )address;
	struct thread *thread = thread_current();
	struct memory_manage *thread_mm = thread->memory_manager;

	for(i = 0; i < MEMORY_BLOCKS; i++){
		if(thread_mm->free_blocks[i].address == addr && thread_mm->free_blocks[i].flags == MEMORY_BLOCK_USING){	//�ҵ�
			if(thread_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_BIG){
				thread_free_page(thread, thread_mm->free_blocks[i].address, thread_mm->free_blocks[i].size);
				thread_mm->free_blocks[i].size = 0;		//ֻ�д�����Ҫ��������size
			}else if(thread_mm->free_blocks[i].mode == MEMORY_BLOCK_MODE_SMALL){
				//С���ڴ����վ�����
				memset((void *)thread_mm->free_blocks[i].address, 0, thread_mm->free_blocks[i].size);
				//����һ��������Ǿ������б���ɢ���ڴ涼���ͷź󣬿�����Ҫ�ͷ��Ǹ�ҳ��Ŀǰ��û�п�����
				//С�鲻��Ҫ���ô�С����Ϊ���Ǵ�ɢ�˵Ŀ�
			}
			thread_mm->free_blocks[i].flags = MEMORY_BLOCK_FREE;
			
			//printk("Free:%x idx:%d\n", address,i);
			return 0;
		}
	}
	//printk("not found that address %x\n", address);
	return -1;	//ʧ��
}

void *user_realloc(void *mem_address, unsigned int newsize)
{
	struct thread *thread = thread_current();

	//如果地址是NULL，就相当于malloc
	if (mem_address == NULL) {
		//printk("addr is NULL\n");
		return user_malloc(newsize);
	}

	//如果大小是0，就相当于free
	if (newsize == 0) {
		//printk("size is 0\n");
		user_free(mem_address);
		return NULL;
	}

	struct memory_block *mem_block;

	mem_block = get_memory_block(thread->memory_manager, mem_address);

	//如果没有找到对应的内存块
	if (mem_block == NULL) {
		//printk("realloc invalid pointer\n");
		return NULL;
	}

	//如果新的大小和原来的一样发或者小
	if (newsize <= mem_block->size ) {
		//设定新的大小
		mem_block->size = newsize;

		//让数据丢失
		//printk("size <= old size.\n");
		//返回原来的地址
		return (void *)mem_block->address;
	}

	//新的大小比原来的大

	/*
	mem_adrr 所在内存是否有newsize的连续空间检测，由于算法原因，我们默认就没有
	*/

	//分配一个对应大小的内存
	void *address = user_malloc(newsize);
	if (address == NULL) {
		//分配失败
		return NULL;
	}

	//把旧空间的数据拷贝到新空间
	memcpy(address, (void *)mem_block->address, mem_block->size);

	//释放掉旧的空间
	user_free((void *)mem_block->address);

	//返回新的地址
	return address;
}



#endif //_CONFIG_GUI_BASIC_

/*
File:		kernel/page.c
Contains:	page model, manage page
Auther:		Hu Zicheng
Time:		2019/2/20
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_ARCH_X86_

#include <sys/arch.h>
#include <sys/core.h>
#include <sys/mm.h>
#include <sys/dev.h>
#include <math.h>
#include <string.h>
#include <sys/debug.h>
#include <sys/dev.h>

struct bitmap phy_mem_bitmap;
struct bitmap vir_mem_bitmap;

void init_page()
{
	
	u32 *pdt = (u32 *)PAGE_DIR_PHY_ADDR;
	memset((void *)PAGE_DIR_PHY_ADDR,0,PAGE_SIZE);	//Clean pdt
	//Set 0 pde and 512 pde for pt
	pdt[0] = PAGE_TBL_PHY_ADDR|0x07;
	//pdt[512] = PAGE_TBL_PHY_ADDR|0x07;
	//Set myself in 1023 pde
	pdt[1023] = PAGE_DIR_PHY_ADDR|0x07;
	
	int i;
	u32 *pt = (u32 *)PAGE_TBL_PHY_ADDR;
	u32 page_addr = 0x00|0x07;
	//Set low 4M physical address to pt
	for(i = 0; i < 1024; i++){
		pt[i] = page_addr;
		page_addr += PAGE_SIZE;
	}
	
	//vram buffer map
	uint32_t *vram_addr = (uint32_t *)(VIDEO_INFO_ADDR+6);
	//printk(">vram:%x\n", (int)*vram_addr);
	
	//PDIR
	pdt[1] = VRAM_PT_PHY_ADDR|0x07;
	//PT
	pt = (u32 *)VRAM_PT_PHY_ADDR;
	page_addr = *vram_addr|0x07;
	
	for(i = 0; i < 1024; i++){
		pt[i] = page_addr;
		page_addr += PAGE_SIZE;
	}
	
	//Set page dir to cr3
	write_cr3(PAGE_DIR_PHY_ADDR);
	//Read cr0
	u32 cr0 = read_cr0();
	cr0 |= 0x80000000;
	//Write cr0
	write_cr0(cr0);
	//printk(">init page\n");
}

/**���ҳ�ķ���״��*/
int pages_status()
{
	int i , j = 0;
	for(i = 0; i < phy_mem_bitmap.btmp_bytes_len*8; i++){
		if(bitmap_scan_test(&phy_mem_bitmap, i) == 0)j++;
	}
	return j;
}

/**����û��ʹ�õ��ڴ���*/
int get_free_memory()
{
	int pages = pages_status();
	int free_size = pages*PAGE_SIZE;
	return free_size;
}

/*
�������ַת����������ַ
*/
uint32_t addr_v2p(uint32_t vaddr)
{
	//print_vai(vaddr);
	uint32_t *pte = pte_ptr(vaddr);
	uint32_t idx_in_page = vaddr & 0x00000fff;
	uint32_t phy_addr = *pte + idx_in_page;	//ҳ���е����ݣ�����ҳ��+ ҳ�е�ƫ��-����ŵ���������ַ���������������ԣ������㶮�ã���
	phy_addr = phy_addr&0xfffff000;	//ȥ������
	//printk(" %x->%x\n",vaddr, phy_addr);
	return phy_addr;
}

void vir_link_with_phy(uint32_t vir_addr, uint32_t phy_addr)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_addr);
	if(((*pde)&0x00000001) != 0x00000001){	//������ҳ��
		uint32_t pt = alloc_mem_page();	//����ҳ����ַ
		pt |= 0x00000007;
		*pde = pt;		//��дҳĿ¼��Ϊҳ���ĵ�ַ
	}
	pte = pte_ptr(vir_addr);
	phy_addr |= 0x00000007;
	*pte = phy_addr;	//��дҳ����Ϊҳ�ĵ�ַ
}

void *copy_kernel_pdt()
{
	void *vir_kernel_pdt_addr = kernel_alloc_page(1);	//����
	memcpy((void *)(vir_kernel_pdt_addr), (void *)(PAGE_DIR_VIR_ADDR), 4096);	//����
	
	//memset(vir_kernel_pdt_addr , 0 , 2048);	//����
	return vir_kernel_pdt_addr;	//����
}

uint32_t *pte_ptr(uint32_t vaddr)
{
	uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
	return pte;
}

uint32_t *pde_ptr(uint32_t vaddr)
{
	uint32_t *pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
	return pde;
}

void map_pages(uint32_t vir_address,int pages)
{
	int i;
	for(i = 0; i < pages; i++){
		fill_vir_page_talbe(vir_address + i*PAGE_SIZE);
	}
}


void upmap_pages(uint32_t vir_address,int pages)
{
	int i;
	for(i = 0; i < pages; i++){
		clean_vir_page_table(vir_address + i*PAGE_SIZE);
	}
}



void fill_vir_page_talbe(uint32_t vir_address)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_address);
	if(((*pde)&0x00000001) != 0x00000001){	//������ҳ��
		uint32_t pt = alloc_mem_page();	//����ҳ����ַ
		pt |= 0x00000007;
		*pde = pt;		//��дҳĿ¼��Ϊҳ���ĵ�ַ
	}
	pte = pte_ptr(vir_address);
	if(((*pte)&0x00000001) != 0x00000001){	//������ҳ��
		uint32_t page = alloc_mem_page();	//����ҳ��ַ
		page |= 0x00000007;
		*pte = page;	//��дҳ����Ϊҳ�ĵ�ַ
	}
}

uint32_t fill_vir_page_talbe_directly(uint32_t vir_address)
{
	uint32_t *pde, *pte;
	pde = pde_ptr(vir_address);
	//����ҳ����ַ
	uint32_t pt = alloc_mem_page();
	pt |= 0x00000007;
	*pde = pt;		//��дҳĿ¼��Ϊҳ���ĵ�ַ
	
	pte = pte_ptr(vir_address);
	//����ҳ��ַ
	uint32_t page = alloc_mem_page();	
	page |= 0x00000007;
	*pte = page;	//��дҳ����Ϊҳ�ĵ�ַ
	return pt;
}

void clean_vir_page_table(uint32_t vir_address)
{
	uint32_t *pde, *pte;
	uint32_t page_phy_addr;
	pde = pde_ptr(vir_address);
	if(((*pde)&0x00000001) != 0x00000001){	//������ҳ��,ֱ�Ӳ���ҳ����
		//panic("clean vir page failed.");
	}
	pte = pte_ptr(vir_address);
	if(((*pte)&0x00000001) != 0x00000001){	//������ҳ��,ֱ�Ӳ���ҳ����
		//panic("clean vir page failed.");
	}
	page_phy_addr = *pte;	//���ҳ������ҳ��������ַ
	*pte = 0;	//���ҳ����
	page_phy_addr &= 0xfffff000;	//������22λ������
	free_mem_page(page_phy_addr);	//�ͷŶ�Ӧ������ҳ
}

int alloc_mem_page()
{
	int idx;
	uint32_t mem_addr;
	idx = bitmap_scan(&phy_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&phy_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	mem_addr = idx*0x1000 + PHY_MEM_BASE_ADDR;

	return mem_addr;
}

int free_mem_page(uint32_t address)
{
	int addr = address;
	uint32_t idx;

	idx = (addr-PHY_MEM_BASE_ADDR)/0x1000;
	if(bitmap_scan_test(&phy_mem_bitmap, idx)){
		bitmap_set(&phy_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

uint32_t* create_page_dir2(void)
{
	uint32_t* page_dir_vaddr = (uint32_t* )kernel_alloc_page(1);
	if (page_dir_vaddr == NULL) {
		printk("create_page_dir: mm.malloc failed!\n");
		return NULL;
	}
	memset(page_dir_vaddr,0,PAGE_SIZE);
	memcpy((void *)page_dir_vaddr, (void *)(PAGE_DIR_VIR_ADDR), PAGE_SIZE);	//����
	//memset(page_dir_vaddr,0,512);
	page_dir_vaddr[1023] = (uint32_t)(addr_v2p((int )page_dir_vaddr)|PAGE_P_1|PAGE_RW_W|PAGE_US_U);	//������ַ+����
   return page_dir_vaddr;
}

//�������Ǻ������ַ��ص�
int alloc_vir_page()
{
	int idx;
	uint32_t vir_addr;
	idx = bitmap_scan(&vir_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&vir_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	vir_addr = idx*0x1000 +  VIR_MEM_BASE_ADDR;
	return vir_addr;
}

int free_vir_page(uint32_t address)
{
	uint32_t addr = address;
	uint32_t idx;

	idx = (addr-VIR_MEM_BASE_ADDR)/0x1000;

	if(bitmap_scan_test(&vir_mem_bitmap, idx)){
		bitmap_set(&vir_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

void *kernel_alloc_page(uint32_t pages)
{
	int i;
	uint32_t vir_page_addr, vir_page_addr_more;
	
	uint32_t old_status = io_load_eflags();
	io_cli();
	
	vir_page_addr = alloc_vir_page();	//����һ�������ַ��ҳ
	
	fill_vir_page_talbe(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
	
	if(pages == 1){	//���ֻ��һ��ҳ
		memset((void *)vir_page_addr,0,PAGE_SIZE);
		
		io_store_eflags(old_status);
		return (void *)vir_page_addr;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr_more = alloc_vir_page();	//����һ�������ַ��ҳ
			fill_vir_page_talbe(vir_page_addr_more);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��	
		}
		memset((void *)vir_page_addr,0,PAGE_SIZE*pages);
		io_store_eflags(old_status);
		
		return (void *)vir_page_addr;
	}else{
		return NULL;
	}
	//Ӧ�ò��ᵽ������
	return NULL;
}

void kernel_free_page(uint32_t vaddr, uint32_t pages)
{
	
	int i;
	uint32_t vir_page_addr = vaddr;
	
	uint32_t old_status = io_load_eflags();
	io_cli();
	
	free_vir_page(vir_page_addr);
	clean_vir_page_table(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
	if(pages == 1){	//���ֻ��һ��ҳ
		io_store_eflags(old_status);
		
		return;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr += PAGE_SIZE;
			free_vir_page(vir_page_addr);
			clean_vir_page_table(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
		}
		io_store_eflags(old_status);
		
		return;
	}
}


//���Ǹ����ں˵�����д�û�̬��
void *thread_use_vaddr(uint32 vaddr)
{
	struct thread *cur = thread_current();
	
	int idx = -1;
	idx = (vaddr - USER_VIR_MEM_BASE_ADDR)/PAGE_SIZE;
	
	assert(idx > 0);
	
	bitmap_set(&cur->vir_mem_bitmap, idx, 1);
	
	fill_vir_page_talbe(vaddr);
	memset((void *)vaddr,0,PAGE_SIZE);
	return (void *)vaddr;
}

//���Ǹ����ں˵�����д�û�̬��
int thread_alloc_vir_page(struct thread *thread)
{
	int idx;
	uint32_t vir_addr;
	idx = bitmap_scan(&thread->vir_mem_bitmap, 1);
	if(idx != -1){
		bitmap_set(&thread->vir_mem_bitmap, idx, 1);
	}else{
		return -1;
	}
	vir_addr = idx*0x1000 +  USER_VIR_MEM_BASE_ADDR;
	return vir_addr;
}

int thread_free_vir_page(struct thread *thread,uint32_t address)
{
	uint32_t addr = address;
	uint32_t idx;

	idx = (addr-USER_VIR_MEM_BASE_ADDR)/0x1000;

	if(bitmap_scan_test(&thread->vir_mem_bitmap, idx)){
		bitmap_set(&thread->vir_mem_bitmap, idx, 0);
	}else{
		return -1;
	}
	return 0;
}

void *thread_alloc_page(struct thread *thread, uint32_t pages)
{
	int i;
	uint32_t vir_page_addr, vir_page_addr_more;

	vir_page_addr = thread_alloc_vir_page(thread);	//����һ�������ַ��ҳ
	
	fill_vir_page_talbe(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
	
	if(pages == 1){	//���ֻ��һ��ҳ
		memset((void *)vir_page_addr,0,PAGE_SIZE);
		return (void *)vir_page_addr;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr_more = thread_alloc_vir_page(thread);	//����һ�������ַ��ҳ
			fill_vir_page_talbe(vir_page_addr_more);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��	
		}
		memset((void *)vir_page_addr,0,PAGE_SIZE*pages);
		return (void *)vir_page_addr;
	}else{
		return NULL;
	}
	//Ӧ�ò��ᵽ������
	return NULL;
}

void thread_free_page(struct thread *thread, uint32_t vaddr, uint32_t pages)
{
	int i;
	uint32_t vir_page_addr = vaddr;
	thread_free_vir_page(thread, vir_page_addr);
	clean_vir_page_table(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
	if(pages == 1){	//���ֻ��һ��ҳ
		return;
	}else if(pages > 1){
		for(i = 1; i < pages; i++){
			vir_page_addr += PAGE_SIZE;
			thread_free_vir_page(thread, vir_page_addr);
			clean_vir_page_table(vir_page_addr);		//��ҳ���ӵ���ǰҳĿ¼��ϵͳ�У�ʹ�����Ա�ʹ��
		}
		return;
	}
}

#endif //_CONFIG_ARCH_X86_


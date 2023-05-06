#include "screen.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"
#include "shell.h"
#include "rtl8139.h"
#include "ps2.h"
#include "network.h"
#include "vfs.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

uint32_t kernel_size=0;

void deinit_special_regions()
{

	//! deinit the region the kernel is in as its in use
	pmmngr_deinit_region(0x100000, kernel_size*512 + 0x1000);

	//! deinit the region the pmm bitmap is in as its in use
	pmmngr_deinit_region(0x100000 + kernel_size*512 + 0x1000, pmmngr_get_block_count()/PMMNGR_BLOCKS_PER_BYTE + 0x1000);

	//! deinit the region the vfs is in as its in use
	pmmngr_deinit_region(VFS_BASE, VFS_CEILING-VFS_BASE);

	// deinit first 2mb for safety reasons
	pmmngr_deinit_region(0, 0x200000);

}

void kmain(uint32_t _, multiboot_info* bootinfo, uint32_t _kernel_size) {

	kernel_size = _kernel_size;

	idt_install();

	//! get memory size in KB (1st mb + 1-16mb memory + 16+ memory)
	uint32_t mem_size = 1024 + bootinfo->m_memoryLo + bootinfo->m_memoryHi*64; 
	pmmngr_init(mem_size, (uint32_t *)(0x100000 + kernel_size*512 + 0x1000));

	pmmngr_init_memory_regions(0x1000 + 512*5);

	deinit_special_regions();

	vmmngr_initialize();
	
	irq_install();
	timer_install();
	//display_logo();
	//install_nic();
	ps2_init();
	keyboard_install();
	mouse_install();
	init_screen();
	init_vfs();

	shell_main(); // start terminal

	return;
}

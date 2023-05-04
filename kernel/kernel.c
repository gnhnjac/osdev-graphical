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
#include "heap.h"

void kmain(uint32_t _, multiboot_info* bootinfo) {

	uint32_t kernel_size=100;

	//! get memory size in KB (1st mb + 1-16mb memory + 16+ memory)
	uint32_t mem_size = 1024 + bootinfo->m_memoryLo + bootinfo->m_memoryHi*64; 
	pmmngr_init(mem_size, (uint32_t *)(0x10000 + kernel_size*512+0x1000));

	pmmngr_init_memory_regions(0x1000 + 512*5);

	//! deinit the region the kernel is in as its in use
	pmmngr_deinit_region(0x10000, kernel_size*512+0x1000);

	//! deinit the region the vfs is in as its in use
	pmmngr_deinit_region(VFS_BASE, VFS_CEILING-VFS_BASE);

	//! deinit the region the heap is in as its in use
	pmmngr_deinit_region(HEAP_ADDR, HEAP_CEILING-HEAP_ADDR);

	idt_install();
	irq_install();
	timer_install();
	//display_logo();
	#ifdef QEMU
			//install_nic();
	#endif
	ps2_init();
	keyboard_install();
	#ifdef BOCHS
		mouse_install(); // mouse has issues in qemu.
	#endif
	init_vfs();
	char tmp[2];
	getchar(-1,-1,tmp);
	while(is_taking_char())
		continue;
	init_screen();
	shell_main(); // start terminal

	return;
}

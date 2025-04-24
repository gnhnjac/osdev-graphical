#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"
#include "shell.h"
#include "rtl8139.h"
#include "network.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "floppy.h"
#include "memory.h"
#include "tmpfsys.h"
#include "fat12fsys.h"
#include "vfs.h"
#include "tss.h"
#include "low_level.h"
#include "sysapi.h"
#include "user.h"
#include "process.h"
#include "scheduler.h"
#include "graphics.h"
#include "window_sys.h"
#include "debug.h"

uint32_t kernel_size=0;

void deinit_special_regions()
{

	//! deinit the region the kernel is in as its in use
	pmmngr_deinit_region(0x100000, kernel_size*512);
	//pmmngr_deinit_region(0xC0000000, kernel_size*512);

	//! deinit the region the pmm bitmap is in as its in use
	pmmngr_deinit_region(0x100000 + kernel_size*512, pmmngr_get_block_count()/PMMNGR_BLOCKS_PER_BYTE);
	//pmmngr_deinit_region(0xC0000000 + kernel_size*512, pmmngr_get_block_count()/PMMNGR_BLOCKS_PER_BYTE);

	//! deinit the region the vfs is in as its in use
	pmmngr_deinit_region(TFSYS_BASE-0xC0000000+0x100000, TFSYS_CEILING-TFSYS_BASE);
	//pmmngr_deinit_region(TFSYS_BASE, TFSYS_CEILING-TFSYS_BASE);

	//! deinit the region the kernel heap is in as its in use
	pmmngr_deinit_region(HEAP_BASE-0xC0000000+0x100000, HEAP_CEILING-HEAP_BASE);
	//pmmngr_deinit_region(HEAP_BASE, HEAP_CEILING-HEAP_BASE);

	// deinit first 1mb for safety reasons (stack is there, bios is there)
	pmmngr_deinit_region(0, 0x100000);

}

void kmain(uint32_t _, multiboot_info* bootinfo, uint32_t _kernel_size) {

	kernel_size = _kernel_size;

	gdt_install();
	idt_install();
	irq_install();

	//! get memory size in KB (1st mb + 1-16mb memory + 16+ memory)
	uint32_t mem_size = 1024 + bootinfo->m_memoryLo + bootinfo->m_memoryHi*64; 
	pmmngr_init(mem_size, (uint32_t *)(0xC0000000 + kernel_size*512));

	pmmngr_init_memory_regions(0x3000 + 512*5);

	deinit_special_regions();

	vmmngr_initialize();

	heap_init();

	//! set drive 0 as current drive
	flpydsk_set_working_drive (0);

	//! install floppy disk to IR 38, uses IRQ 6
	flpydsk_install ();

	// initialize the fat12 file system driver
	fat12fsys_init ();

	// initialize the temp file system driver
	tfsys_init();

	graphics_init();

	winsys_init();

	init_screen();

	//install_nic();

	timer_install();

	keyboard_install();

	mouse_install();

	// initialize the system call api
	install_syscalls();

	//! initialize TSS
	install_tss (5,0x10,0);

	scheduler_initialize();

	execute_idle();

	return;
}

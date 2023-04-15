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

void kmain();

void entry()
{

	kmain();

}

void kmain() {
	idt_install();
	irq_install();
	timer_install();
	//display_logo();
	init_screen();
	#ifdef QEMU
			//install_nic();
	#endif
	ps2_init();
	keyboard_install();
	#ifdef BOCHS
		mouse_install(); // mouse has issues in qemu.
	#endif
	init_vfs();

	shell_main(); // start terminal

	return;
}

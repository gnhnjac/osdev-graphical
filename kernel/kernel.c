#include "screen.h"
#include "idt.h"
#include "irq.h"
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"
#include "shell.h"
#include "heap.h"
#include "rtl8139.h"

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
			install_nic();
	#endif
	keyboard_install();
	#ifdef BOCHS
		mouse_install(); // mouse has issues in qemu.
	#endif

	// for (int i = 0; i < 80; i++)
	// {	

	// 	char lol = i%26;
	// 	if (i % 3 == 0)
	// 	{
	// 		lol += 'A';

	// 	}
	// 	else
	// 	{

	// 		lol += 'a';

	// 	}
	// 	int lolly = i;
	// 	printf("Hello, world! character: %c, iteration: 0x%x\n", lol, lolly);
	// 	//wait_milliseconds(100);

	// } 
	
	//shell_main(); // start terminal

	return;
}

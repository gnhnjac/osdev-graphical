#include "sysapi.h"
#include "idt.h"
#include "screen.h"
#include "process.h"
#include "scheduler.h"
#include "graphics.h"

extern void syscall_stub(void);

void* _syscalls[] = {
	print,
	terminateProcess,
	fork,
	winsys_create_win_user,
	winsys_remove_window_user,
	winsys_display_window_section_user,
	load_font_to_buffer,
	thread_sleep,
	winsys_dequeue_from_event_handler_user,
	inc_proc_brk
};

void install_syscalls()
{

	idt_set_gate(0x80, (void *)syscall_stub, 0x8E|0x60); // present|32 bit interrupt gate|dpl 3

}
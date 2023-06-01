#include "sysapi.h"
#include "idt.h"
#include "screen.h"

extern void syscall_stub(void);

void* _syscalls[] = {
	printf
};

void install_syscalls()
{

	idt_set_gate(0x80, (void *)syscall_stub, 0x8E|0x60); // present|32 bit interrupt gate|dpl 3

}

void syscall_handler()
{

	static int idx=0;
	__asm__ ("" : "=a" ( idx ));

	printf("%d",idx);

	//! bounds check
	if (idx>=MAX_SYSCALL)
		return;

	//! get service
	static void (*fnct)();
	fnct = _syscalls[idx];

	if (fnct)
		fnct();

}
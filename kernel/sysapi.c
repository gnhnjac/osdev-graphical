#include "sysapi.h"
#include "idt.h"
#include "screen.h"
#include "process.h"

extern void syscall_stub(void);

void* _syscalls[] = {
	print,
	terminateProcess
};

void install_syscalls()
{

	idt_set_gate(0x80, (void *)syscall_stub, 0x8E|0x60); // present|32 bit interrupt gate|dpl 3

}
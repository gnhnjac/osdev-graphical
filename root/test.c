
static char *hello = "hello, data";

char bss[10];

__attribute__((force_align_arg_pointer))
void _start()
{

	__asm__("mov $0, %eax");
	__asm__("mov %0, %%ebx" : : "m" (hello));
	__asm__("int $0x80");

	for (int i = 0; i < 10; i++)
	{

		char *tmp = bss+i;

		__asm__("mov $0, %eax");
		__asm__("mov %0, %%ebx" : : "m" (tmp));
		__asm__("int $0x80");

	}
	
	__asm__("mov $1, %eax");
	__asm__("int $0x80");
	__builtin_unreachable();
}

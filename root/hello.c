
__attribute__((force_align_arg_pointer))
void _start()
{
	
	char *hello = "hello world!\n";

	__asm__("mov $0, %eax");
	__asm__("mov %0, %%ebx" : : "m" (hello));
	__asm__("int $0x80");

	__asm__("mov $1, %eax");
	__asm__("int $0x80");
	__builtin_unreachable();
}

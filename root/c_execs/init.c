
__attribute__((force_align_arg_pointer))
void _start()
{
	
	__asm__("mov $1, %eax");
	__asm__("int $0x80");
	__builtin_unreachable();
}

__attribute__((force_align_arg_pointer))
void _start()
{

	while(1) __asm__("pause");
	__builtin_unreachable();
}
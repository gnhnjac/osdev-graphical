__attribute__((force_align_arg_pointer))
void _main()
{

	char *a = 0;
	*a = 'x';
	__builtin_unreachable();
}

void int_to_str(int n, int base);
int fib(int n);

__attribute__((force_align_arg_pointer))
void _start()
{
	
	char *new_line = "\r\n";

	for (int i = 0; i < 20; i++)
	{

		int n = fib(i);
		int_to_str(n, 10);

		__asm__("mov $0, %eax");
		__asm__("mov %0, %%ebx" : : "m" (new_line));
		__asm__("int $0x80");
	}

	__asm__("mov $1, %eax");
	__asm__("int $0x80");
	__builtin_unreachable();
}

void int_to_str(int n, int base)
{
	char digits[] = {'0',0,'1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0,'8',0,'9',0,'A',0,'B',0,'C',0,'D',0,'E',0,'F',0};

	int temp_n = n;
	int n_of_digits = 0;
	do
	{

		n_of_digits++;
		temp_n /= base;

	} while (temp_n);

	int i = 0;
	do
	{	
		char *digit = &digits[2*(n%base)];
		__asm__("mov $0, %eax");
		__asm__("mov %0, %%ebx" : : "m" (digit));
		__asm__("int $0x80");
		i++;
		n /= base;

	} while(n);

}

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

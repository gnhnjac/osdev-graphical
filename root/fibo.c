
void int_to_str(int n, char *buffer, int base);
int fib(int n);

__attribute__((force_align_arg_pointer))
void _start()
{
	
	char *dig_buf = "xxxxxxxxxxxxxxxxxxxxxxxxxx";
	char *new_line = "\r\n";

	for (int i = 0; i < 20; i++)
	{

		int n = fib(i);
		int_to_str(n, dig_buf, 10);

		__asm__("mov $0, %eax");
		__asm__("mov %0, %%ebx" : : "m" (dig_buf));
		__asm__("int $0x80");

		__asm__("mov $0, %eax");
		__asm__("mov %0, %%ebx" : : "m" (new_line));
		__asm__("int $0x80");
	}

	__asm__("mov $1, %eax");
	__asm__("int $0x80");
	__builtin_unreachable();
}

void int_to_str_padding(int n, char *buffer, int base, int padding)
{
	char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int neg = 0;
	if (n < 0)
	{
		n = -n;
		neg = 1;
	}
	int temp_n = n;
	int n_of_digits = 0;
	do
	{

		n_of_digits++;
		temp_n /= base;

	} while (temp_n);

	if (neg)
	{
		n_of_digits += 1;
		buffer[0] = '-';
	}

	int i = neg;
	padding -= n_of_digits;
	if(padding > 0)
	{
		while(padding > 0)
		{
			buffer[i++] = '0';
			n_of_digits++;
			padding--;
		}
	}
	i = 0;
	do
	{	

		int digit = digits[n%base];
		*(buffer + n_of_digits - 1 - i) = digit;
		i++;
		n /= base;

	} while(n);

	buffer[n_of_digits] = 0;

}

void int_to_str(int n, char *buffer, int base)
{

	int_to_str_padding(n,buffer,base,0);

}

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

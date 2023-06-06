#include <stdlib.h>

void print(char *str)
{

	__asm__("mov $0, %eax");
	__asm__("mov %0, %%ebx" : : "m" (str));
	__asm__("int $0x80");

}

void printf(const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);

	const char *orig = fmt;

	while (*fmt)
	{

		char buff[30];

		if (*fmt == '%' && ((*(fmt-1) != '\\' && fmt != orig) || fmt == orig))
		{

			switch(*++fmt)
			{

				case 'd':

					itoa(va_arg(valist, int), buff, 10);
					print(buff);
					break;

				case 'u':

					uitoa(va_arg(valist, int), buff, 10);
					print(buff);
					break;

				case 'U':

					uitoa(va_arg(valist, int), buff, 16);
					print(buff);
					break;

				case 'c':
					buff[0] = (char)va_arg(valist, int);
					buff[1] = 0;
					print(buff);
					break;

				case 's':
					print((char *)va_arg(valist, int));
					break;

				case 'x':
					itoa(va_arg(valist, int), buff, 16);
					print(buff);
					break;

				case 'b':
					itoa(va_arg(valist, int), buff, 2);
					print(buff);
					break;



				default:

					printf("Unknown format type \\%%c", fmt);
					return;
			}

		}
		else if(*fmt == '\\')
		{	
			fmt++;
			continue;
		}
		else
		{

			buff[0] = *fmt;
			buff[1] = 0;
			print(buff);

		}

		fmt++;

	}

	return;

}
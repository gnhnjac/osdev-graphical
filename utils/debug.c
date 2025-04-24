#include "debug.h"
#include "low_level.h"
#include "strings.h"

int debug_print(const char *str)
{

	while (*str)
	{

		outb(DEBUG_PORT, *str);

		str++;

	}

	return 0;

}

int debug_putchar(const char c)
{

	outb(DEBUG_PORT, c);

	return 0;

}

int debug_printf(const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);
	debug_vprintf(fmt,valist);

}

int debug_vprintf(const char *fmt, va_list valist)
{

	const char *orig = fmt;

	while (*fmt)
	{

		if (*fmt == '%' && ((*(fmt-1) != '\\' && fmt != orig) || fmt == orig))
		{

			char buff[30];

			switch(*++fmt)
			{

				case 'd':

					int_to_str(va_arg(valist, int), buff, 10);
					debug_print(buff);
					break;

				case 'u':

					uint_to_str(va_arg(valist, int), buff, 10);
					debug_print(buff);
					break;

				case 'U':

					uint_to_str(va_arg(valist, int), buff, 16);
					debug_print(buff);
					break;

				case 'c':

					debug_putchar((char)va_arg(valist, int));
					break;

				case 's':
					debug_print((char *)va_arg(valist, int));
					break;

				case 'x':
					int_to_str(va_arg(valist, int), buff, 16);
					debug_print(buff);
					break;

				case 'X':

					byte_to_str((uint8_t)va_arg(valist, int), buff, 16);
					debug_print(buff);
					break;

				case 'b':
					int_to_str(va_arg(valist, int), buff, 2);
					debug_print(buff);
					break;



				default:

					debug_printf("Unknown format type \\%%c", fmt);
					return -1;
			}

		}
		else if(*fmt == '\\' && *(fmt+1) == '%')
		{	
			fmt++;
			continue;
		}
		else
		{

			debug_putchar(*fmt);

		}

		fmt++;

	}

	return 0;

}
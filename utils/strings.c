#include "strings.h"
#include <stdarg.h>
#include "memory.h"
#include "heap.h"
#include "std.h"
#include <stdint.h>
#include "screen.h"

void int_to_str(int n, char *buffer, int base)
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

	int i = 0;
	if (neg)
	{
		n_of_digits += 1;
		buffer[0] = '-';
	}
	do
	{	

		int digit = digits[n%base];
		*(buffer + n_of_digits - 1 - i) = digit;
		i++;
		n /= base;

	} while(n);

	buffer[n_of_digits] = 0;

}

void byte_to_str(unsigned char n, char *buffer, int base)
{
	char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int n_of_digits = 2;
	buffer[0] = '0';

	int i = 0;
	do
	{	

		int digit = digits[n%base];
		*(buffer + n_of_digits - 1 - i) = digit;
		i++;
		n /= base;

	} while(n);

	buffer[n_of_digits] = 0;

}

int strlen(char *str)
{
	int len = 0;
	while (*str++)
	{

		len++;
	}

	return len;

}

void to_lower(char *str)
{

	for(int i = 0; i < strlen(str); i++)
	{

		if ('A' <= str[i] && str[i] <= 'Z')
			str[i] += 'a'-'A';

	}

}

void strip_character(char *str, char character)
{

	int len = strlen(str);

	for(int i = 0; i < len; i++)
	{

		if (str[i] == character)
		{

			for (int j = i; j < len; j++)
			{

				str[j] = str[j+1];

			}

			len--;
			i--;

		}

	}

}

void strip_from_start(char *str, char character)
{

	int len = strlen(str);

	for(int i = 0; i < len; i++)
	{

		if (str[i] == character)
		{

			for (int j = i; j < len; j++)
			{

				str[j] = str[j+1];

			}

			len--;
			i--;

		}
		else
		{

			return;

		}

	}

}

// seperate string by seperator and take the {index} numbered substring. return is malloced ***DONT FORGET TO FREE***.
// index starts from 0.
char *seperate_and_take(char* str, char seperator, int index)
{
	int curr_index = 0;
	int len = 0;
	char *str_start = str;
	for (int i = 0; i < strlen(str); i++)
	{

		if (str[i] == seperator)
		{

			if (curr_index == index)
			{
				char *buff = (char *)malloc();
				buff[len] = 0;
				memcpy(buff,str_start,len);
				return buff;

			}
			str_start = str+i+1;
			len = 0;
			curr_index++;

		}
		len++;

	}

	if (curr_index == index)
	{
		char *buff = (char *)malloc();
		buff[len] = 0;
		memcpy(buff,str_start,len);
		return buff;

	}
	else
	{
		char *buff = (char *)malloc();
		memcpy(buff,str,strlen(str)+1); // if didn't find seperator copy the whole thing.
		return buff;
	}
	
}

// counts how many substrings the string has when seperated with seperator
int count_substrings(char *str, char seperator)
{

	int count = 1;

	for (int i = 0; i < strlen(str); i++)
	{

		if (str[i] == seperator)
			count++;

	}

	return count;

}


void strip_from_end(char *str, char character)
{

	int len = strlen(str);

	for(int i = len-1; i >= 0; i--)
	{

		if (str[i] == character)
		{

			for (int j = i; j < len; j++)
			{

				str[j] = str[j+1];

			}

			i--;

		}
		else
		{

			return;

		}

	}

}


bool strcmp(char *s1, char *s2)
{

	if (strlen(s1) != strlen(s2))
		return false;

	for(int i = 0; i < strlen(s1); i++)
	{

		if (s1[i] != s2[i])
			return false;

	}

	return true;

}

int sprintf(char *str, const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);

	const char *orig = fmt;

	while (*fmt)
	{

		if (*fmt == '%' && ((*(fmt-1) != '\\' && fmt != orig) || fmt == orig))
		{

			char buff[30];
			char *to_cpy;
			int len;

			switch(*++fmt)
			{

				case 'd':

					int_to_str(va_arg(valist, int), buff, 10);
					len = strlen(buff);
					memcpy(str,buff,len);
					str+=len;
					break;

				case 'c':

					*str = (char)va_arg(valist, int);
					str++;
					break;

				case 's':
					to_cpy = (char *)va_arg(valist, int);
					len = strlen(to_cpy);
					memcpy(str,to_cpy,len);
					str+=len;
					break;

				case 'x':

					int_to_str(va_arg(valist, int), buff, 16);
					len = strlen(buff);
					memcpy(str,buff,len);
					str+=len;
					break;

				case 'b':
					int_to_str(va_arg(valist, int), buff, 2);
					len = strlen(buff);
					memcpy(str,buff,len);
					str+=len;
					break;

				default:

					printf("Unknown format type \\%%c", fmt);
					return STDERR;
			}

		}
		else if(*fmt == '\\')
		{	
			fmt++;
			continue;
		}
		else
		{
			*str = *fmt;
			str++;
		}

		fmt++;

	}
	*str = 0; // put end of string character

	return STDOK;

}
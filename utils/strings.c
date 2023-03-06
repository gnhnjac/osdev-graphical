#include "strings.h"
#include "memory.h"

void num_to_str(int n, char *buffer, int base)
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

// seperate string by seperator and take the {index} numbered substring. buff is dest. buff must be the size of the original str.
// index starts from 0.
void seperate_and_take(char* str, char *buff, char seperator, int index)
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

				buff[len] = 0;
				memcpy(buff,str_start,len);
				return;

			}
			str_start = str+i+1;
			curr_index++;

		}
		len++;

	}
	memcpy(buff,str,strlen(str)+1); // if didn't find seperator copy the whole thing.
	return;
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
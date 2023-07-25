#include "string.h"

unsigned int strlen(const char *str)
{

	if (!str)
		return 0;

	int len = 0;
	while (*str++)
	{

		len++;
	}

	return len;

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

void strcpy(char *dest, const char *source)
{

   while(*source)
   {

         *dest=*source;
         source++;
         dest++;
   }
   *dest = '\0';

}
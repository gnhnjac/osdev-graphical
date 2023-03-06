#include "memory.h"

/* Copy bytes from one place to another . */
void memcpy (char* dest , const char* source , int count) {

	for (int i =0; i < count; i++) {
		*(dest + i) = *(source + i);
	}
}

char *memset(char *dest, unsigned char val, int count)
{
    /* set 'count' bytes in 'dest' to 'val'. */

    for (int i = 0; i < count; i++)
    {

    	*(dest + i) = val;

    }
}

short *memsetw(short *dest, unsigned short val, int count)
{
   for (int i = 0; i < count; i++)
    {

    	*(dest + i) = val;

    }
}
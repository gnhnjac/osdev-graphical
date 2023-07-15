#include "memory.h"
#include "screen.h"
#include "process.h"
#include "math.h"

/* Copy bytes from one place to another . */
void memcpy (char* dest , const char* source , int count) {

	for (int i =0; i < count; i++) {
		*(dest + i) = *(source + i);
	}
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

bool memcmp(char *m1, char *m2, uint32_t len)
{

   for (int i = 0; i < len; i++)
   {

      if (*(m1+i) != *(m2+i))
         return false;

   }

   return true;

}

void hexdump(void *mem, int count)
{

   static const int INTS_PER_ROW = 4;

   for (int i = 0; i < count; i += 4*INTS_PER_ROW)
   {

      printf("%x: ", mem+i);

      for (int k = 0; k < INTS_PER_ROW; k++)
      {

         int printed = 0;
         for(int j = 0; j < min(count-i-k*4, 4); j++)
         {

            printf("%X",*(unsigned char *)(mem+i+j+4*k));
            printed++;

         }

         for(int j = 0; j < 4-printed; j++)
         {

            print("  ");

         }

         if(k < INTS_PER_ROW-1)
            printf(" ");
      }

      print(" | ");

      for (int k = 0; k < INTS_PER_ROW; k++)
      {

         for(int j = 0; j < min(count-i-k*4, 4); j++)
         {

            printf("%c",*(unsigned char *)(mem+i+j+4*k));

         }

         for(int j = 0; j < 4-min(count-i-k*4, 4); j++)
         {

            print(" ");

         }

         if(k < INTS_PER_ROW-1)
            printf(" ");
      }

      print("\n");

   }

}
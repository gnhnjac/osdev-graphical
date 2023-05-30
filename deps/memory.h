#include <stdbool.h>
#include <stdint.h>
//refs
void memcpy (char* dest , const char* source , int count);
void strcpy(char *dest, const char *source);
char *memset(char *dest, unsigned char val, int count);
short *memsetw(short *dest, unsigned short val, int count);
bool memcmp(char *m1, char *m2, uint32_t len);
void hexdump(void *mem, int count);
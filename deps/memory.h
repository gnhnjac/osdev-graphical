#include <stdbool.h>
#include <stdint.h>
//refs
void memcpy (char* dest , const char* source , int count);
void memcpy_dword (uint32_t* dest , const uint32_t* source , int count);
void strcpy(char *dest, const char *source);
void memset_dword (uint32_t* dest , uint32_t val , int count);
char *memset(char *dest, unsigned char val, int count);
short *memsetw(short *dest, unsigned short val, int count);
bool memcmp(char *m1, char *m2, uint32_t len);
void hexdump(void *mem, int count);
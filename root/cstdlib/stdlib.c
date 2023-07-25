#include "syscalls.h"
#include "stdio.h"
#include "heap.h"
#include "string.h"

unsigned int atoi(const char *str)
{

	unsigned int result = 0;
	unsigned int power = 1;

	for(unsigned int i = strlen(str)-1; i >= 0; i--)
	{

		result += (str[i]-'0')*power;

		power *= 10;

	}

	return result;

}

void itoa(int n, char *buf, int base)
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
		buf[0] = '-';
	}

	unsigned int i = neg;
	i = 0;
	do
	{	

		int digit = digits[n%base];
		*(buf + n_of_digits - 1 - i) = digit;
		i++;
		n /= base;

	} while(n);

	buf[n_of_digits] = 0;

}

void uitoa(unsigned int n, char *buffer, int base)
{
	char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	unsigned int u_n = (unsigned int)n; 
	int temp_n = u_n;
	int n_of_digits = 0;
	do
	{

		n_of_digits++;
		temp_n /= base;

	} while (temp_n);

	unsigned int i = 0;
	do
	{	

		int digit = digits[u_n%base];
		*(buffer + n_of_digits - 1 - i) = digit;
		i++;
		u_n /= base;

	} while(u_n);

	buffer[n_of_digits] = 0;

}

unsigned int abs(int x)
{

	return (x > 0) ? x : -x;

}

void *malloc(uint32_t size)
{
	
	size += BLOCK_AMT_DESC_SIZE;

	if (size % HEAP_BLOCK_SIZE != 0)
		size += HEAP_BLOCK_SIZE - size % HEAP_BLOCK_SIZE;

	uint32_t block_amt = size/HEAP_BLOCK_SIZE;

	void *addr = malloc_alloc_blocks(block_amt);

	if (!addr)
		return 0;

	*((uint32_t *)addr) = block_amt;


	return addr+BLOCK_AMT_DESC_SIZE;

}

void *calloc(uint32_t size)
{

	void *ptr = malloc(size);

	memset(ptr,0,size);

	return ptr;

}

void free(void *addr)
{
	if (addr)
	{
		uint32_t block_amt = *((uint32_t *)((uint32_t)addr-BLOCK_AMT_DESC_SIZE));

		malloc_free_blocks(addr-BLOCK_AMT_DESC_SIZE, (uint32_t)block_amt);
	}
}
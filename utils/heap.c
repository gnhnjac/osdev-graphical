#include "heap.h"
void *malloc(unsigned int size)
{
	void *heap_head = (void *)HEAP_ADDR;
	void *addr = 0;
	while(!addr)
	{

		if (*(unsigned int *)heap_head == 0)
		{
			*(unsigned int *)heap_head = size;
			addr = heap_head + sizeof(unsigned int);

		}

		heap_head += *(unsigned int *)heap_head + sizeof(unsigned int);

	}

	return addr;

}

void free(void *addr)
{

	*(unsigned int *)(addr-sizeof(unsigned int)) = 0;

}
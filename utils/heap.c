#include "heap.h"

// NOTE: CURRENTLY EACH MALLOC IS LIMITED TO 4096 BYTES

void *malloc(unsigned int size)
{
	void *heap_head = (void *)HEAP_ADDR;
	void *addr = 0;
	while(!addr)
	{

		if (*(unsigned char *)heap_head == 0)
		{
			*(unsigned char *)heap_head = 1; // set to occupied
			addr = heap_head + sizeof(unsigned char);

		}

		heap_head +=  sizeof(unsigned char) + HEAP_CHUNK_SIZE;

	}

	return addr;

}

void free(void *addr)
{

	*(unsigned char *)(addr-sizeof(unsigned char)) = 0;

}
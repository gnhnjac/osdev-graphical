#include <stdint.h>

#define HEAP_CHUNK_SIZE 4096
#define	BLOCK_AMT_DESC_SIZE 1

//refs
void *kmalloc(uint32_t size);
void kfree(void *phys_addr);
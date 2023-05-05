#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "screen.h"

void *kmalloc(uint32_t size)
{
	
	uint32_t block_amt = (size+BLOCK_AMT_DESC_SIZE+(HEAP_CHUNK_SIZE-1))/HEAP_CHUNK_SIZE;

	void *phys_addr = pmmngr_alloc_blocks(block_amt);

	if (!phys_addr)
		return 0;

	*((uint8_t *)phys_addr) = block_amt;


	return phys_addr+BLOCK_AMT_DESC_SIZE;

}

void kfree(void *phys_addr)
{
	uint8_t block_amt = *((uint8_t *)((uint32_t)phys_addr-BLOCK_AMT_DESC_SIZE));

	pmmngr_free_blocks(phys_addr, (uint32_t)block_amt);

}
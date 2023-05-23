#include "heap.h"
#include "memory.h"
#include "screen.h"

//! size of physical memory
static const uint32_t	_alloc_memory_size=HEAP_CEILING-HEAP_BASE;
 
//! number of blocks currently in use
static uint32_t	_alloc_used_blocks=0;
 
//! maximum number of available memory blocks
static const uint32_t	_alloc_max_blocks=(HEAP_CEILING-HEAP_BASE)/HEAP_BLOCK_SIZE;
 
//! memory map bit array. Each bit represents a memory block
static uint32_t _alloc_memory_map[((HEAP_CEILING-HEAP_BASE)/HEAP_BLOCK_SIZE)/32];

void *kmalloc(uint32_t size)
{
	
	size += BLOCK_AMT_DESC_SIZE;

	if (size % HEAP_BLOCK_SIZE != 0)
		size += HEAP_BLOCK_SIZE - size % HEAP_BLOCK_SIZE;

	uint32_t block_amt = size/HEAP_BLOCK_SIZE;

	void *phys_addr = kmalloc_alloc_blocks(block_amt);

	if (!phys_addr)
		return 0;

	*((uint32_t *)phys_addr) = block_amt;


	return phys_addr+BLOCK_AMT_DESC_SIZE;

}

void kfree(void *phys_addr)
{
	uint32_t block_amt = *((uint32_t *)((uint32_t)phys_addr-BLOCK_AMT_DESC_SIZE));

	kmalloc_free_blocks(phys_addr-BLOCK_AMT_DESC_SIZE, (uint32_t)block_amt);

}

// occupy a block
static inline void _alloc_mmap_set(int bit) {

  _alloc_memory_map[bit / 32] |= (1 << (bit % 32));
}


// unoccupy a block
static inline void _alloc_mmap_unset(int bit) {
 
  _alloc_memory_map[bit / 32] &= ~ (1 << (bit % 32));
}

// check if a block is occupied
static inline bool _alloc_mmap_test(int bit) {
 
 return _alloc_memory_map[bit / 32] &  (1 << (bit % 32));
}

// returns the first free block we can use
int _alloc_mmap_first_free() {
 
	//! find the first free bit
	for (uint32_t i=0; i< alloc_get_block_count() / 32; i++) {
		// if not all blocks in this int sized buffer are full then check for the empty one.
		if (_alloc_memory_map[i] != 0xffffffff)
		{
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (_alloc_memory_map[i] & bit) )
					return i*4*8+j; // bit number, 4*8 is just int size.
			}
 		}
 	}
	return -1;
}

// returns the first free block sequence we can use
int _alloc_mmap_first_free_s(int seq_len) {
 	
 	int current_seq_len = 0;
 	int current_set_first_index = 0;
	//! find the first free bit
	for (uint32_t i=0; i< alloc_get_block_count() / 32; i++)
	{
		// if not all blocks in this int sized buffer are full then check for the empty one.
		if (_alloc_memory_map[i] != 0xffffffff)
		{
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (_alloc_memory_map[i] & bit))
				{
					if (!current_seq_len)
						current_set_first_index = i*4*8+j; // bit number, 4*8 is just int size.
					current_seq_len++;
				}
				else
					current_seq_len = 0;

				if (current_seq_len == seq_len)
				{

					return current_set_first_index;

				}

			}
		}
		else
			current_seq_len = 0;

 	}
	return -1;
}

int alloc_get_block_count()
{
	return _alloc_max_blocks;
}

int alloc_get_used_blocks()
{
	return _alloc_used_blocks;
}

int alloc_get_free_block_count()
{
	return _alloc_max_blocks-_alloc_used_blocks;
}

// allocate a block of memory and get it's address
void* kmalloc_alloc_block() {
 
	if (alloc_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = _alloc_mmap_first_free();
 
	if (frame == -1)
		return 0;	//out of memory
 
	_alloc_mmap_set (frame);
 
	uint32_t addr = HEAP_BASE + frame * HEAP_BLOCK_SIZE;
	_alloc_used_blocks++;
 
	return (void*)addr;
}

// allocate a block of memory and get it's address
void* kmalloc_alloc_blocks(int seq_len) {
 
	if (alloc_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = _alloc_mmap_first_free_s(seq_len);
 
	if (frame == -1)
		return 0;	//out of memory
 
	uint32_t addr = HEAP_BASE + frame * HEAP_BLOCK_SIZE;

	for(; seq_len > 0; seq_len--)
	{
		_alloc_mmap_set (frame++);
		_alloc_used_blocks++;
	}
 
	return (void*)addr;
}


void kmalloc_free_block(void* p) {
 
	uint32_t addr = (uint32_t)p;
	int frame = (addr-HEAP_BASE) / HEAP_BLOCK_SIZE;
 
	_alloc_mmap_unset (frame);
 
	_alloc_used_blocks--;
}

void kmalloc_free_blocks(void* p, int seq_len) {
 
	uint32_t addr = (uint32_t)p;
	int frame = (addr-HEAP_BASE) / HEAP_BLOCK_SIZE;
 
	for(; seq_len > 0; seq_len--)
	{
		_alloc_mmap_unset (frame++);
		_alloc_used_blocks--;
	}
 
}

void heap_init()
{

	memset((char *)_alloc_memory_map, 0, (((HEAP_CEILING-HEAP_BASE)/HEAP_BLOCK_SIZE)/32));

}

void print_heap_stats()
{

	printf("available blocks: %d\nused blocks: %d\n",_alloc_max_blocks,_alloc_used_blocks);
	print("heap image:\n");

	for (int i = 0; i < alloc_get_block_count()/128; i++)
	{

			if (i % 50 == 0)
				print("\n");

			uint32_t avg = 0;
			for(int j = 0; j < 128; j++)
			{
				avg += _alloc_mmap_test(i*128+j);
			}

			putchar((avg > 128) ? 'O' : 'F');


	}

}
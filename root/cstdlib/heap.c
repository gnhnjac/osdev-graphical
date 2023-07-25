#include "syscalls.h"
#include "stdio.h"
#include "heap.h"
#include <stdint.h>

//! number of blocks currently in use
static uint32_t	_alloc_used_blocks=0;
 
//! maximum number of available memory blocks
static const uint32_t	_alloc_max_blocks=HEAP_LIM/HEAP_BLOCK_SIZE;
 
//! memory map bit array. Each bit represents a memory block
static uint32_t *_alloc_memory_map;
	
static uintptr_t _sbrk_top = 0;

static uintptr_t HEAP_BASE = 0;

static bool heap_initialized = false;

static int alloc_get_block_count()
{
	return _alloc_max_blocks;
}

static int alloc_get_used_blocks()
{
	return _alloc_used_blocks;
}

static int alloc_get_free_block_count()
{
	return _alloc_max_blocks-_alloc_used_blocks;
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
static int _alloc_mmap_first_free() {
 
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
static int _alloc_mmap_first_free_s(int seq_len) {
 	
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

static void heap_init()
{

	_alloc_memory_map = (uint32_t *)sbrk((HEAP_LIM/HEAP_BLOCK_SIZE)/8);

	memset((char *)_alloc_memory_map, 0, ((HEAP_LIM/HEAP_BLOCK_SIZE)/8));

	HEAP_BASE = (uintptr_t)(((uint32_t)_alloc_memory_map)+(HEAP_LIM/HEAP_BLOCK_SIZE)/8);
	_sbrk_top = HEAP_BASE;

	heap_initialized = true;


}

// allocate a block of memory and get it's address
void* malloc_alloc_block() {
 	
	if (!heap_initialized)
		heap_init();

	if (alloc_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = _alloc_mmap_first_free();
 
	if (frame == -1)
		return 0;	//out of memory
 
	_alloc_mmap_set (frame);
 
	uint32_t addr = HEAP_BASE + frame * HEAP_BLOCK_SIZE;
	if (addr+HEAP_BLOCK_SIZE > _sbrk_top)
	{
		sbrk(HEAP_BLOCK_SIZE);
		_sbrk_top += HEAP_BLOCK_SIZE;
	}
	_alloc_used_blocks++;
 
	return (void*)addr;
}

// allocate a block of memory and get it's address
void* malloc_alloc_blocks(int seq_len) {

	if (!heap_initialized)
		heap_init();

	if (alloc_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = _alloc_mmap_first_free_s(seq_len);
 
	if (frame == -1)
		return 0;	//out of memory
 
	uint32_t addr = HEAP_BASE + frame * HEAP_BLOCK_SIZE;

	if (addr+seq_len*HEAP_BLOCK_SIZE > _sbrk_top)
	{
		uint32_t inc = addr+seq_len*HEAP_BLOCK_SIZE-_sbrk_top;
		sbrk(inc);
		_sbrk_top += inc;
	}

	for(; seq_len > 0; seq_len--)
	{
		_alloc_mmap_set (frame++);
		_alloc_used_blocks++;
	}
 
	return (void*)addr;
}


void malloc_free_block(void* p) {
 
	uint32_t addr = (uint32_t)p;
	int frame = (addr-HEAP_BASE) / HEAP_BLOCK_SIZE;
 
	_alloc_mmap_unset (frame);
 
	_alloc_used_blocks--;
}

void malloc_free_blocks(void* p, int seq_len) {
 
	uint32_t addr = (uint32_t)p;
	int frame = (addr-HEAP_BASE) / HEAP_BLOCK_SIZE;
 
	for(; seq_len > 0; seq_len--)
	{
		_alloc_mmap_unset (frame++);
		_alloc_used_blocks--;
	}
 
}
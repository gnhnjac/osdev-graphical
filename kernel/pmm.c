#include "pmm.h"
#include "memory.h"
#include "screen.h"

//! different memory regions (in memory_region.type)
char* strMemoryTypes[] = {

	"Available",			//memory_region.type==0
	"Reserved",			//memory_region.type==1
	"ACPI Reclaim",		//memory_region.type==2
	"ACPI NVS Memory"		//memory_region.type==3
};

//! size of physical memory
static uint32_t	_mmngr_memory_size=0;
 
//! number of blocks currently in use
static uint32_t	_mmngr_used_blocks=0;
 
//! maximum number of available memory blocks
static uint32_t	_mmngr_max_blocks=0;
 
//! memory map bit array. Each bit represents a memory block
static uint32_t* _mmngr_memory_map= 0;


// occupy a block
static inline void mmap_set(int bit) {

  _mmngr_memory_map[bit / 32] |= (1 << (bit % 32));
}


// unoccupy a block
static inline void mmap_unset(int bit) {
 
  _mmngr_memory_map[bit / 32] &= ~ (1 << (bit % 32));
}

// check if a block is occupied
static inline bool mmap_test(int bit) {
 
 return _mmngr_memory_map[bit / 32] &  (1 << (bit % 32));
}

// returns the first free block we can use
int mmap_first_free() {
 
	//! find the first free bit
	for (uint32_t i=0; i< pmmngr_get_block_count() / 32; i++) {
		// if not all blocks in this int sized buffer are full then check for the empty one.
		if (_mmngr_memory_map[i] != 0xffffffff)
		{
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (_mmngr_memory_map[i] & bit) )
					return i*4*8+j; // bit number, 4*8 is just int size.
			}
 		}
 	}
	return -1;
}

// returns the first free block sequence we can use
int mmap_first_free_s(int seq_len) {
 	
 	int current_seq_len = 0;
 	int current_set_first_index = 0;
	//! find the first free bit
	for (uint32_t i=0; i< pmmngr_get_block_count() / 32; i++)
	{
		// if not all blocks in this int sized buffer are full then check for the empty one.
		if (_mmngr_memory_map[i] != 0xffffffff)
		{
			for (int j=0; j<32; j++) {		//! test each bit in the dword
 
				int bit = 1 << j;
				if (! (_mmngr_memory_map[i] & bit))
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

int pmmngr_get_block_count()
{
	return _mmngr_max_blocks;
}

// get memory size in kilobytes (1024 bytes)
int pmmngr_get_memory_size()
{
	return _mmngr_memory_size;
}

int pmmngr_get_used_blocks()
{
	return _mmngr_used_blocks;
}

int pmmngr_get_free_block_count()
{
	return _mmngr_max_blocks-_mmngr_used_blocks;
}

void pmmngr_init(uint32_t mem_size, uint32_t *bitmap) {
 
	_mmngr_memory_size	=	mem_size;
	_mmngr_memory_map	=	bitmap;
	_mmngr_max_blocks	=	(pmmngr_get_memory_size()*1024) / PMMNGR_BLOCK_SIZE; // how many blocks are available
	_mmngr_used_blocks	=	pmmngr_get_block_count(); // the start point is that all the blocks are used
 
	//! By default, all of memory is in use
	memset((char *)_mmngr_memory_map, 0xff, pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE);

	printf("pmm initialized with %d KB of physical memory\n\n", mem_size);
}

void pmmngr_init_memory_regions(uint32_t mem_regions_addr)
{

	memory_region* region = (memory_region*)mem_regions_addr;

	for (int i=0; i<15; ++i) {

		//! sanity check; if type is > 4 mark it reserved
		if (region[i].type>4)
			region[i].type=1;

		//! if start address is 0, there is no more entries, break out
		if (i>0 && region[i].startLo==0)
			break;

		//! display entry
		printf("region %d: start: 0x%x%x length (bytes): 0x%x%x type: %d (%s)\n", i, 
		region[i].startHi, region[i].startLo,
		region[i].sizeHi,region[i].sizeLo,
		region[i].type, strMemoryTypes[region[i].type-1]);

		//! if region is avilable memory, initialize the region for use
		if (region[i].type==1)
			pmmngr_init_region(region[i].startLo, region[i].sizeLo);
	}

}

// clean certain region from occupation for use.
void pmmngr_init_region(uint32_t base, uint32_t size) {
 
	int align = base / PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;
 
	for (; blocks>0; blocks--) {
		mmap_unset (align++);
		_mmngr_used_blocks--;
	}
 
	mmap_set (0);	//first block is always set. This insures allocs cant be 0
}

void pmmngr_deinit_region(uint32_t base, uint32_t size) {
 
	int align = base / PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;
 
	for (; blocks>0; blocks--) {
		mmap_set (align++);
		_mmngr_used_blocks++;
	}
}

// allocate a block of memory and get it's address
void* pmmngr_alloc_block() {
 
	if (pmmngr_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = mmap_first_free();
 
	if (frame == -1)
		return 0;	//out of memory
 
	mmap_set (frame);
 
	uint32_t addr = frame * PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks++;
 
	return (void*)addr;
}

// allocate a block of memory and get it's address
void* pmmngr_alloc_blocks(int seq_len) {
 
	if (pmmngr_get_free_block_count() <= 0)
		return 0;	//out of memory
 
	int frame = mmap_first_free_s(seq_len);
 
	if (frame == -1)
		return 0;	//out of memory
 
	uint32_t addr = frame * PMMNGR_BLOCK_SIZE;

	for(; seq_len > 0; seq_len--)
	{
		mmap_set (frame++);
		_mmngr_used_blocks++;
	}
 
	return (void*)addr;
}


void pmmngr_free_block(void* p) {
 
	uint32_t addr = (uint32_t)p;
	int frame = addr / PMMNGR_BLOCK_SIZE;
 
	mmap_unset (frame);
 
	_mmngr_used_blocks--;
}

void pmmngr_free_blocks(void* p, int seq_len) {
 
	uint32_t addr = (uint32_t)p;
	int frame = addr / PMMNGR_BLOCK_SIZE;
 
	for(; seq_len > 0; seq_len--)
	{
		mmap_unset (frame++);
		_mmngr_used_blocks--;
	}
 
}

extern void pmmngr_paging_enable (bool b);

extern void pmmngr_load_PDBR (void *addr);

extern void *pmmngr_get_PDBR ();

void print_mem_map()
{

	for(int i = 0; i < pmmngr_get_block_count()/32; i++)
	{

		if (i % 50 == 0)
		{
			printf("\n0x%x: ",i*32*PMMNGR_BLOCK_SIZE);
		}

		uint32_t avg = 0;
		for(int j = 0; j < 32; j++)
		{
			avg += mmap_test(i*32+j);
		}

		printf("%c", (avg > 16) ? 'O' : 'F');

	}

}
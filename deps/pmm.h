#include <stdint.h>
#include <stdbool.h>

//! 8 blocks per byte
#define PMMNGR_BLOCKS_PER_BYTE 8
 
//! block size (4k)
#define PMMNGR_BLOCK_SIZE	4096
 
//! block alignment
#define PMMNGR_BLOCK_ALIGN	PMMNGR_BLOCK_SIZE

//! format of a memory region
typedef struct {

	uint32_t	startLo;
	uint32_t	startHi;
	uint32_t	sizeLo;
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
} memory_region;

//refs
//! different memory regions (in memory_region.type);
static inline void mmap_set(int bit);
static inline void mmap_unset(int bit);
static inline bool mmap_test(int bit);
int mmap_first_free();
int mmap_first_free_s(int seq_len);
int pmmngr_get_block_count();
// get memory size in kilobytes (1024 bytes);
int pmmngr_get_memory_size();
int pmmngr_get_free_block_count();
void pmmngr_init(uint32_t mem_size, uint32_t *bitmap);
void pmmngr_init_memory_regions(uint32_t mem_regions_addr);
void pmmngr_init_region(uint32_t base, uint32_t size);
void pmmngr_deinit_region(uint32_t base, uint32_t size);
void* pmmngr_alloc_block();
void* pmmngr_alloc_blocks(int seq_len);
void pmmngr_free_block(void* p);
void pmmngr_free_blocks(void* p, int seq_len);
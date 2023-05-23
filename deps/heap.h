#include <stdint.h>
#include <stdbool.h>

#define HEAP_BLOCK_SIZE 8

#define HEAP_BASE 0x300000
#define HEAP_CEILING 0x400000

#define HEAP_POOL_SIZE HEAP_CEILING-HEAP_BASE

#define BLOCK_AMT_DESC_SIZE 4

//refs
void *kmalloc(uint32_t size);
void kfree(void *phys_addr);
static inline void _alloc_mmap_set(int bit);
static inline void _alloc_mmap_unset(int bit);
static inline bool _alloc_mmap_test(int bit);
int _alloc_mmap_first_free();
int _alloc_mmap_first_free_s(int seq_len);
int alloc_get_block_count();
int alloc_get_used_blocks();
int alloc_get_free_block_count();
void* kmalloc_alloc_block();
void* kmalloc_alloc_blocks(int seq_len);
void kmalloc_free_block(void* p);
void kmalloc_free_blocks(void* p, int seq_len);
void heap_init();
void print_heap_stats();
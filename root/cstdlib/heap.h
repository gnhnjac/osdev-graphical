#pragma once

#define HEAP_LIM 0x400000
#define HEAP_BLOCK_SIZE 8
#define BLOCK_AMT_DESC_SIZE 4

void* malloc_alloc_block();
void* malloc_alloc_blocks(int seq_len);
void malloc_free_block(void* p);
void malloc_free_blocks(void* p, int seq_len);
#define HEAP_ADDR 0x200000
#define HEAP_CEILING 0x400000
#define HEAP_CHUNK_SIZE 4096

//refs
void *malloc();
void free(void *addr);
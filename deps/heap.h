#define HEAP_ADDR 0x100000
#define HEAP_CEILING 0x300000
#define HEAP_CHUNK_SIZE 4096

//refs
void *malloc();
void free(void *addr);
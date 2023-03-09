#define HEAP_ADDR 0x100000
#define HEAP_CHUNK_SIZE 4096

//refs
void *malloc(unsigned int size);
void free(void *addr);
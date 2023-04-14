#include <stdbool.h>
#include <stdint.h>

#define sizeof(type) (char *)(&type+1)-(char*)(&type)

#define VFS_BASE 0x200000
#define BLOCK_SIZE 4096

typedef enum{File,SubFile,Dir} block_type; // add symbolic link maybe

typedef struct b_metadata block_metadata; // this is a typedef and a declaration of the struct

struct b_metadata
{
	block_metadata *parent_block;
	block_metadata *child_block;
	block_type type;
	uint32_t fid; // file id -> unique to directories and files
	uint32_t bid; // block id -> unique to all types
	bool occupied;
	char name[10];
};

typedef struct d_record dir_record; // directory record of file/directory/...

struct d_record
{
	uint32_t fid;
	char name[10];
	uint32_t size;
	bool occupied;
	dir_record *next_record;

}

#define META_SIZE sizeof(block_metadata);
#define RECORD_SIZE sizeof(d_record);

//refs
uint32_t get_next_fid();
block_metadata *get_faddr_by_id(uint32_t fid);
block_metadata *create_block(block_metadata *parent_block, block_type type, char *name);
void add_file_to_dir(block_metadata *f_meta, block_metadata *p_data);
// get size of file in KiB (1024 bytes);
uint32_t get_fsize(uint32_t fid);
void remove_file_from_dir(int fid);
block_metadata *create_base_dir(char *name);
int create_dir(int parent_fid, char *name);
int create_file(int parent_fid, char *name);
void extend_block(int fid);
bool free_block(int bid);
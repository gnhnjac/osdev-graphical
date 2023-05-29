#include <stdbool.h>
#include <stdint.h>

#define sizeof(type) (char *)(&type+1) - (char*)(&type)

#define TFSYS_BASE 0x400000
#define TFSYS_CEILING 0x600000
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
	uint32_t data_pointer; // pointer to next free space in data part of file starting from the start of the data part.
	bool occupied;
	char name[10];
};


typedef struct d_record dir_record; // directory record of file/directory/...

struct d_record
{
	uint32_t fid;
	char name[10];
	bool occupied;
	dir_record *next_record;

};


//refs
void tmpfsys_init();
void set_tfsys_screen_stats();
void ls(uint32_t fid);
void pwd(uint32_t fid);
void mkdir(char *name, uint32_t parent_fid);
void touch(char *name, uint32_t parent_fid);
void concat(char *name, uint32_t fid1, uint32_t fid2, uint32_t pid);
uint32_t get_fid_by_name(char *name, uint32_t fid);
void reset_file(uint32_t fid);
void write(uint32_t fid, char *chr, int raw);
void cat(uint32_t fid);
char get_nth_char(uint32_t fid, uint32_t n);
uint32_t size(uint32_t fid);
uint32_t get_next_fid();
block_metadata *get_faddr_by_id(uint32_t fid);
uint32_t get_bid_by_faddr(block_metadata *base);
block_metadata *create_block(block_metadata *parent_block, block_type type, char *name);
void add_record_to_dir(block_metadata *f_meta, block_metadata *p_data);
// get size of file in KiB (1024 bytes);
uint32_t get_fsize(uint32_t fid);
void remove_record_from_dir(int r_id, int dir_id);
block_metadata *create_base_dir(char *name);
uint32_t create_dir(uint32_t parent_fid, char *name);
uint32_t create_file(uint32_t parent_fid, char *name);
void extend_file(uint32_t fid);
bool free_block(uint32_t bid);
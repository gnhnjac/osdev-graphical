#include <vfs.h>
#include <math.h>
#include <strings.h>
#include <memory.h>

void *largest_addr = (void *)VFS_BASE;

uint32_t get_next_fid()
{

	block_metadata *base = (block_metadata *)VFS_BASE;

	uint32_t fid = 0;

	while(base->occupied)
	{

		if(base->type == Dir || base->type == File)
			fid++;
		base += BLOCK_SIZE;
	}

	return fid;

}

block_metadata *get_faddr_by_id(uint32_t fid)
{

	block_metadata *base = (block_metadata *)VFS_BASE;

	while(base->fid!=fid)
		base += BLOCK_SIZE;

	return base;

}

uint32_t get_bid_by_faddr(block_metadata *base)
{

	return ((uint32_t)base-VFS_BASE)/BLOCK_SIZE;

}

block_metadata *create_block(block_metadata *parent_block, block_type type, char *name)
{

	block_metadata *metadata;

	metadata->parent_block = parent_block;

	metadata->child_block = 0;

	metadata->type = type;

	metadata->occupied = true;

	memcpy(metadata->name,name,min(10,strlen(name)));

	if(type == Dir || type == File)
		metadata->fid = get_next_fid();
	else if(type == SubFile)
		metadata->fid = parent_block->fid;

	block_metadata *base = (block_metadata *)VFS_BASE;

	while(base->occupied)
		base += BLOCK_SIZE;

	metadata->bid = get_bid_by_faddr(base);

	memcpy((char *)base,(char *)metadata,META_SIZE);

	if (type == File)
		add_record_to_dir(base, parent_block);
	else if(type == Dir)
	{

		block_metadata *false_dir;
		block_metadata *false_parent;

		memcpy((char *)false_dir,(char *)metadata,META_SIZE);
		memcpy((char *)false_parent,(char *)parent_block,META_SIZE);

		false_dir->name = ".";
		false_parent->name = "..";

		add_record_to_dir(false_dir,base);
		add_record_to_dir(false_parent,base);
	}

	return base;
}

void add_record_to_dir(block_metadata *f_meta, block_metadata *p_data)
{

	dir_record *record;

	record->fid = f_meta->fid;
	memcpy(record->name,f_meta->name,10);
	record->size = get_fsize(f_meta->fid);
	record->occupied = true;

	dir_record *base = (dir_record *)((char *)p_data + META_SIZE);

	dir_record *pivot = base;

	while(pivot->occupied)
		pivot += RECORD_SIZE;

	record->next_record = base->next_record;

	base->next_record = pivot;

	memcpy((char *)pivot,(char *)record,RECORD_SIZE);

}

// get size of file in KiB (1024 bytes)
uint32_t get_fsize(uint32_t fid)
{

	block_metadata *fbase = get_faddr_by_id(fid);
	uint32_t size = 0;
	while (fbase)
	{
		fbase = fbase->child_block;
		size += BLOCK_SIZE/1024;
	}

	return size;

}

void remove_record_from_dir(int r_id, int dir_id)
{
	dir_record *base = (dir_record *)((char *)get_faddr_by_id(dir_id) + META_SIZE);

	dir_record *prev;

	while(base->fid!=r_id)
	{
		prev = base;
		base = base->next_record;
	}

	prev->next_record = base->next_record;

	memset((char *)base,0,RECORD_SIZE);

}

block_metadata *create_base_dir(char *name)
{

	block_metadata *metadata;

	metadata->child_block = 0;

	metadata->type = Dir;

	metadata->occupied = true;

	memcpy(metadata->name,name,min(10,strlen(name)));

	metadata->fid = 0;

	metadata->bid = 0;

	block_metadata *base = (block_metadata *)VFS_BASE;

	metadata->parent_block = base;

	memcpy((char *)base,(char *)metadata,META_SIZE);

	return base;

}

// returns directory id
int create_dir(int parent_fid, char *name)
{

	return create_block(get_faddr_by_id(parent_fid),Dir,name)->fid;

}

// returns file id
int create_file(int parent_fid, char *name)
{

	return create_block(get_faddr_by_id(parent_fid),File,name)->fid;

}

void extend_file(int fid)
{

	block_metadata *f_base = get_faddr_by_id(fid);

	while(f_base->child_block)
		f_base = f_base->child_block;

	f_base->child_block = create_block(f_base,SubFile,"");

}

bool free_block(int bid)
{

	if (bid == 0) // can't free base directory
		return false;

	block_metadata *block = (block_metadata *)(VFS_BASE + bid*BLOCK_SIZE);

	if (block->type == File || block->type == Dir)
	{

		remove_record_from_dir(block->fid,block->parent_block->fid);

	}

	if (block->type == SubFile)
	{

		block_metadata *parent_block = block->parent_block;

		block_metadata *child_block = block->child_block;

		parent_block->child_block = child_block;

		if (child_block)
			child_block->parent_block = parent_block;
	}

	memset((char *)block,0,BLOCK_SIZE);

	return true;

}
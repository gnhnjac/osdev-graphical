#include <vfs.h>
#include <math.h>
#include <strings.h>
#include <memory.h>
#include <heap.h>
#include <screen.h>
#include <keyboard.h>

block_metadata a;
uint32_t META_SIZE = (char *)(&a+1) - (char*)(&a);
dir_record b;
uint32_t RECORD_SIZE = (char *)(&b+1) - (char*)(&b);

void init_vfs()
{
	char *base = (char *)VFS_BASE;
	while ((uint32_t)base < VFS_CEILING)
	{
		((block_metadata *)base)->occupied = 0;

		base += BLOCK_SIZE;
	}
	create_base_dir("root");

}

void set_vfs_screen_stats()
{

	char *base = (char *)VFS_BASE;

	uint32_t blocks = 0;
	uint32_t files = 0;
	uint32_t directories = 0;

	while ((uint32_t)base < VFS_CEILING)
	{

		if (((block_metadata *)base)->occupied)
		{

			blocks++;

			if (((block_metadata *)base)->type == Dir)
				directories++;
			else if(((block_metadata *)base)->type == File)
				files++;

		}

		base += BLOCK_SIZE;

	}

	int cursor_coords = get_cursor();
	set_cursor_coords(0,VFS_OFF);
	printf("VFS: BLKS=%d, FLS=%d, DIRS=%d   ",blocks,files,directories);
	set_cursor(cursor_coords);

}

void ls(uint32_t fid)
{

	dir_record *cur_record = (dir_record *)((char *)get_faddr_by_id(fid)+META_SIZE);

	while (cur_record)
	{
		if(get_faddr_by_id(cur_record->fid)->type == File)
			print("F ");
		else
			print("D ");

		printf("%s, %dKiB\n",cur_record->name,cur_record->size);
		cur_record = cur_record->next_record;
	}

}

void pwd(uint32_t fid)
{

	printf("%s",get_faddr_by_id(fid)->name);

}

void mkdir(char *name, uint32_t parent_fid)
{

	create_dir(parent_fid,name);

}

void touch(char *name, uint32_t parent_fid)
{

	create_file(parent_fid,name);

}

uint32_t get_fid_by_name(char *name, uint32_t fid)
{

	dir_record *cur_record = (dir_record *)((char *)get_faddr_by_id(fid)+META_SIZE);

	uint32_t new_fid = fid;

	while (cur_record)
	{
		if(strcmp(cur_record->name,name))
		{
			new_fid = cur_record->fid;
			break;
		}
		cur_record = cur_record->next_record;
	}

	return new_fid;

}

void write(uint32_t fid, char *str)
{

	block_metadata *meta = get_faddr_by_id(fid);
	char *ptr = (char *)((char *)meta+META_SIZE);
	uint32_t written = 0;

	while(*str)
	{

		if (written == BLOCK_SIZE-META_SIZE)
		{
			meta->data_pointer = written;
			extend_file(meta->fid);
			meta = meta->child_block;
			ptr = (char *)((char *)meta+META_SIZE);
			written = 0;
		}

		*ptr = *str;

		*ptr++;
		*str++;
		written++;

	}

	meta->data_pointer = written;

}

void cat_through_keyboard(uint32_t fid)
{
	block_metadata *meta = get_faddr_by_id(fid);
	char *ptr = (char *)((char *)meta+META_SIZE);
	uint32_t read = 0;
	while(meta)
	{
		if(*ptr)
			virtual_keyboard_input(*ptr);

		*ptr++;
		read++;
		if (read == BLOCK_SIZE-META_SIZE)
		{

			meta = meta->child_block;
			ptr = (char *)((char *)meta+META_SIZE);

		}

	}

}

void cat(uint32_t fid)
{
	block_metadata *meta = get_faddr_by_id(fid);
	char *ptr = (char *)((char *)meta+META_SIZE);
	uint32_t read = 0;
	while(meta && read < meta->data_pointer)
	{
		if(*ptr)
			printf("%c",*ptr);

		*ptr++;
		read++;
		if (read == BLOCK_SIZE-META_SIZE)
		{
			read = 0;
			meta = meta->child_block;
			ptr = (char *)((char *)meta+META_SIZE);

		}

	}

}

uint32_t get_next_fid()
{

	char *base = (char *)VFS_BASE;

	uint32_t fid = 0;

	while(((block_metadata *)base)->occupied)
	{

		if(((block_metadata *)base)->type == Dir || ((block_metadata *)base)->type == File)
			fid++;
		base += BLOCK_SIZE;
	}

	return fid;

}

block_metadata *get_faddr_by_id(uint32_t fid)
{
	char *base = (char *)VFS_BASE;

	while(((block_metadata *)base)->fid!=fid)
		base += BLOCK_SIZE;

	return (block_metadata *)base;

}

uint32_t get_bid_by_faddr(block_metadata *base)
{

	return ((uint32_t)base-VFS_BASE)/BLOCK_SIZE;

}

block_metadata *create_block(block_metadata *parent_block, block_type type, char *name)
{

	block_metadata *metadata = (block_metadata *)malloc();

	metadata->parent_block = parent_block;

	metadata->child_block = 0;

	metadata->type = type;

	metadata->occupied = true;

	metadata->data_pointer = 0;

	memcpy(metadata->name,name,min(9,strlen(name)));
	metadata->name[min(9,strlen(name))] = '\0';

	if(type == Dir || type == File)
		metadata->fid = get_next_fid();
	else if(type == SubFile)
		metadata->fid = parent_block->fid;

	char *base_ptr = (char *)VFS_BASE;

	while(((block_metadata *)base_ptr)->occupied)
		base_ptr += BLOCK_SIZE;

	metadata->bid = get_bid_by_faddr((block_metadata *)base_ptr);

	block_metadata *base = (block_metadata *)base_ptr;

	memcpy((char *)base,(char *)metadata,META_SIZE);

	add_record_to_dir(base, parent_block);

	if(type == Dir)
	{
		block_metadata *false_dir = (block_metadata *)malloc();
		block_metadata *false_parent = (block_metadata *)malloc();
		memcpy((char *)false_dir,(char *)metadata,META_SIZE);
		memcpy((char *)false_parent,(char *)parent_block,META_SIZE);
		strcpy(false_dir->name,".");
		strcpy(false_parent->name,"..");
		add_record_to_dir(false_dir,base);
		add_record_to_dir(false_parent,base);
		free(false_dir);
		free(false_parent);

	}

	free(metadata);

	set_vfs_screen_stats();

	return base;
}

void add_record_to_dir(block_metadata *f_meta, block_metadata *p_data)
{
	dir_record *record = (dir_record *)malloc();
	record->fid = f_meta->fid;
	memcpy(record->name,f_meta->name,10);
	record->size = get_fsize(f_meta->fid);
	record->occupied = true;

	dir_record *base = (dir_record *)((char *)p_data + META_SIZE);

	char *pivot = (char *)base;

	while(((dir_record *)pivot)->occupied)
		pivot += RECORD_SIZE;

	record->next_record = base->next_record;

	base->next_record = (dir_record *)pivot;

	memcpy((char *)pivot,(char *)record,RECORD_SIZE);

	free(record);

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

	block_metadata *metadata = (block_metadata *)malloc();

	metadata->child_block = 0;

	metadata->type = Dir;

	metadata->occupied = true;

	metadata->data_pointer = 0;

	memcpy(metadata->name,name,min(9,strlen(name)));
	metadata->name[min(9,strlen(name))] = '\0';

	metadata->fid = 0;

	metadata->bid = 0;

	block_metadata *base = (block_metadata *)VFS_BASE;

	metadata->parent_block = base;

	memcpy((char *)base,(char *)metadata,META_SIZE);

	block_metadata *false_dir = (block_metadata *)malloc();
	block_metadata *false_parent = (block_metadata *)malloc();

	memcpy((char *)false_dir,(char *)metadata,META_SIZE);
	memcpy((char *)false_parent,(char *)metadata->parent_block,META_SIZE);
	strcpy(false_dir->name,".");
	strcpy(false_parent->name,"..");

	add_record_to_dir(false_dir,base);
	add_record_to_dir(false_parent,base);

	free(metadata);
	free(false_dir);
	free(false_parent);

	set_vfs_screen_stats();

	return base;

}

// returns directory id
uint32_t create_dir(uint32_t parent_fid, char *name)
{

	return create_block(get_faddr_by_id(parent_fid),Dir,name)->fid;

}

// returns file id
uint32_t create_file(uint32_t parent_fid, char *name)
{

	return create_block(get_faddr_by_id(parent_fid),File,name)->fid;

}

void extend_file(uint32_t fid)
{

	block_metadata *f_base = get_faddr_by_id(fid);

	while(f_base->child_block)
		f_base = f_base->child_block;

	f_base->child_block = create_block(f_base,SubFile,"");

}

bool free_block(uint32_t bid)
{

	if (bid == 0) // can't free base directory
		return false;

	block_metadata *block = (block_metadata *)(VFS_BASE + bid*BLOCK_SIZE);

	if (block->type == File || block->type == Dir)
	{

		remove_record_from_dir(block->fid,block->parent_block->fid);

		if (block->type == File)
		{

			block_metadata *prev = block->child_block;
			block_metadata *child;
			while(prev)
			{
				child = prev->child_block;
				free_block(prev->bid);
				prev = child;

			}

		}
		else if(block->type == Dir)
		{

			//todo recursive removal of directory

		}

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

	set_vfs_screen_stats();

	return true;

}
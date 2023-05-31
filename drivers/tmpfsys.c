#include <tmpfsys.h>
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

FILESYSTEM _FSysTmp;

void tfsys_init()
{
	char *base = (char *)TFSYS_BASE;
	while ((uint32_t)base < TFSYS_CEILING)
	{
		((block_metadata *)base)->occupied = 0;

		base += BLOCK_SIZE;
	}
	create_base_dir("\\");

	//! initialize filesystem struct
	strcpy (_FSysTmp.Name, "TMPFSYS");
	_FSysTmp.Open      = tfsys_open;
	_FSysTmp.Read      = tfsys_read;
	_FSysTmp.Close     = tfsys_close;
	_FSysTmp.OpenDir   = tfsys_open_dir;
	_FSysTmp.Create    = tfsys_create;

	//! register ourself to volume manager
	volRegisterFileSystem ( &_FSysTmp, 't'-'a' );

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

void concat(char *name, uint32_t fid1, uint32_t fid2, uint32_t pid)
{

	touch(name, pid);

	uint32_t new_fid = get_fid_by_name(name,pid);

	for(int i = 0; i < size(fid1); i++)
	{
		char c = get_nth_char(fid1, i);
		write(new_fid, &c, 1);
	}

	for(int i = 0; i < size(fid2); i++)
	{
		char c = get_nth_char(fid2, i);
		write(new_fid, &c, 1);
	}

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

void reset_file(uint32_t fid)
{
	block_metadata *meta = get_faddr_by_id(fid);
	while(meta->child_block)
	{
		free_block(meta->child_block->bid);
	}
	meta->data_pointer = 0;
}

// raw refers to whether to take note to escape characters and backspaces.
void write(uint32_t fid, char *chr, int raw)
{

	if (*chr == 27 && !raw) // esc
		return;


	block_metadata *meta = get_faddr_by_id(fid);
	char *ptr = (char *)((char *)meta+META_SIZE);

	while (meta->data_pointer == BLOCK_SIZE-META_SIZE)
	{
		if(!meta->child_block)
			extend_file(meta->fid);
		meta = meta->child_block;
		ptr = (char *)((char *)meta+META_SIZE);
	}

	if(*chr == '\b' && !raw) // backspace
	{

		if (meta->data_pointer == 0)
		{
			if(meta->type == File)
				return;
			if(meta->parent_block->type == File || meta->parent_block->type == SubFile)
			{

				meta->parent_block->data_pointer -= 1;
				free_block(meta->bid);
			}
		} 
		else
		{

			meta->data_pointer-=1;
			return;

		}

	}

	*(ptr + meta->data_pointer) = *chr;

	meta->data_pointer += 1;

}

void tfsys_read(PFILE file, unsigned char* Buffer, unsigned int Length )
{
	block_metadata *meta = get_faddr_by_id(file->id);
	char *ptr = (char *)((char *)meta+META_SIZE);
	uint32_t read = 0;
	while(meta && read < meta->data_pointer && Length > 0)
	{
		if (read >= file->position)
		{
			sprintf(Buffer, "%c",*ptr);
			Buffer++;
			Length--;
			file->position++;
		}

		ptr++;
		read++;
		if (read == BLOCK_SIZE-META_SIZE)
		{
			read = 0;
			meta = meta->child_block;
			ptr = (char *)((char *)meta+META_SIZE);

		}

	}

	if (!meta)
		file->eof = 1;

}

char get_nth_char(uint32_t fid, uint32_t n)
{

	block_metadata *meta = get_faddr_by_id(fid);
	char *ptr = (char *)((char *)meta+META_SIZE);
	uint32_t read = 0;

	while(meta && read < meta->data_pointer && n > 0)
	{

		ptr+=min(n,BLOCK_SIZE-META_SIZE);
		read+=min(n,BLOCK_SIZE-META_SIZE);
		n-=min(n,BLOCK_SIZE-META_SIZE);
		if (read == BLOCK_SIZE-META_SIZE)
		{
			read = 0;
			meta = meta->child_block;
			ptr = (char *)((char *)meta+META_SIZE);

		}

	}

	return *ptr;

}

// get file size in bytes
uint32_t size(uint32_t fid)
{
	block_metadata *meta = get_faddr_by_id(fid);
	uint32_t size = 0;
	while(meta)
	{
		size += meta->data_pointer;
		meta = meta->child_block;
	}

	return size;

}

uint32_t get_next_fid()
{

	char *base = (char *)TFSYS_BASE;
	uint32_t fid = 0;

	while(((block_metadata *)base)->occupied && base < (char *)TFSYS_CEILING)
	{
		if(((block_metadata *)base)->type == Dir || ((block_metadata *)base)->type == File)
			fid++;
		base += BLOCK_SIZE;
	}

	return fid;

}

block_metadata *get_faddr_by_id(uint32_t fid)
{
	char *base = (char *)TFSYS_BASE;

	while(((block_metadata *)base)->fid!=fid || ((block_metadata *)base)->type == SubFile)
		base += BLOCK_SIZE;

	return (block_metadata *)base;

}

uint32_t get_bid_by_faddr(block_metadata *base)
{

	return ((uint32_t)base-TFSYS_BASE)/BLOCK_SIZE;

}

block_metadata *create_block(block_metadata *parent_block, block_type type, char *name)
{	

	block_metadata *metadata = (block_metadata *)kmalloc(100);

	metadata->parent_block = parent_block;

	metadata->child_block = 0;

	metadata->type = type;

	metadata->occupied = true;

	metadata->data_pointer = 0;

	memcpy(metadata->name,name,min(31,strlen(name)));
	metadata->name[min(31,strlen(name))] = '\0';

	if(type == Dir || type == File)
		metadata->fid = get_next_fid();
	else if(type == SubFile)
		metadata->fid = parent_block->fid;

	char *base_ptr = (char *)TFSYS_BASE;

	while(((block_metadata *)base_ptr)->occupied)
		base_ptr += BLOCK_SIZE;

	metadata->bid = get_bid_by_faddr((block_metadata *)base_ptr);

	block_metadata *base = (block_metadata *)base_ptr;
	memset((char *)base,0,BLOCK_SIZE);
	memcpy((char *)base,(char *)metadata,META_SIZE);

	if(type == File || type == Dir)
		add_record_to_dir(base, parent_block);

	if(type == Dir)
	{
		block_metadata *false_dir = (block_metadata *)kmalloc(100);
		block_metadata *false_parent = (block_metadata *)kmalloc(100);
		memcpy((char *)false_dir,(char *)metadata,META_SIZE);
		memcpy((char *)false_parent,(char *)parent_block,META_SIZE);
		strcpy(false_dir->name,".");
		strcpy(false_parent->name,"..");

		add_record_to_dir(false_dir,base);
		add_record_to_dir(false_parent,base);
		kfree(false_dir);
		kfree(false_parent);

	}

	kfree(metadata);

	return base;
}

void add_record_to_dir(block_metadata *f_meta, block_metadata *p_data)
{
	dir_record *record = (dir_record *)kmalloc(100);
	record->fid = f_meta->fid;
	memcpy(record->name,f_meta->name,10);
	record->occupied = true;

	dir_record *base = (dir_record *)((char *)p_data + META_SIZE);

	char *pivot = (char *)base;

	while(((dir_record *)pivot)->occupied)
		pivot += RECORD_SIZE;

	record->next_record = base->next_record;

	base->next_record = (dir_record *)pivot;

	memcpy((char *)pivot,(char *)record,RECORD_SIZE);

	kfree(record);

}

// get size of file in KiB (1024 bytes)
uint32_t get_fsize(uint32_t fid)
{
	block_metadata *fbase = get_faddr_by_id(fid);
	uint32_t size = 0;
	while(fbase)
	{
		size += BLOCK_SIZE/1024;
		fbase = fbase->child_block;
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

	block_metadata *metadata = (block_metadata *)kmalloc(100);

	metadata->child_block = 0;

	metadata->type = Dir;

	metadata->occupied = true;

	metadata->data_pointer = 0;

	memcpy(metadata->name,name,min(31,strlen(name)));
	metadata->name[min(31,strlen(name))] = '\0';

	metadata->fid = 0;

	metadata->bid = 0;

	block_metadata *base = (block_metadata *)TFSYS_BASE;

	metadata->parent_block = base;

	memcpy((char *)base,(char *)metadata,META_SIZE);

	block_metadata *false_dir = (block_metadata *)kmalloc(100);
	block_metadata *false_parent = (block_metadata *)kmalloc(100);

	memcpy((char *)false_dir,(char *)metadata,META_SIZE);
	memcpy((char *)false_parent,(char *)metadata->parent_block,META_SIZE);
	strcpy(false_dir->name,".");
	strcpy(false_parent->name,"..");

	add_record_to_dir(false_dir,base);
	add_record_to_dir(false_parent,base);

	kfree(metadata);
	kfree(false_dir);
	kfree(false_parent);

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

	block_metadata *block = (block_metadata *)(TFSYS_BASE + bid*BLOCK_SIZE);

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

			dir_record *record_base = (dir_record *)((char *)block+META_SIZE);


			while(record_base)
			{

				if(!strcmp(record_base->name,".") && !strcmp(record_base->name,"..")) // skip . and .. records
				{

					free_block(get_faddr_by_id(record_base->fid)->bid);

				}

				record_base = record_base->next_record;

			}

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

	return true;

}

FILE tfsys_open (char *path)
{

	if (strcmp(path,"\\") || strcmp(path, ""))
	{

		FILE base_dir;
		base_dir.id = 0;
		base_dir.flags = FS_DIRECTORY;
		base_dir.name[0] = '\\';
		base_dir.name[1] = 0;
		return base_dir;

	}

	// path looks like this: \folder\folder\file.txt

	int path_substrings = count_substrings(path, '\\'); // \ is our seperator

	FILE curr_file;

	for (int i = 0; i < path_substrings; i++) // iterate over path
	{

		char *curr_filename = seperate_and_take(path,'\\',i + 1);

		if (i == 0) // the first parameter is in the root folder
		{
			curr_file = tfsys_root_open(curr_filename);
		}
		else
		{
			curr_file = tfsys_subdir_open(curr_file, curr_filename); // our current file is a subdirectory
		}

		kfree(curr_filename);

		if (curr_file.flags == FS_INVALID)
			break; // an error has occured, do not continue

	}

	return curr_file;


}

FILE tfsys_root_open (char* fname) {
	
	FILE file;

	dir_record *cur_record = (dir_record *)((char *)get_faddr_by_id(0)+META_SIZE);

	while (cur_record)
	{
		if(strcmp(cur_record->name,fname))
		{
			strcpy(file.name,cur_record->name);
			file.eof = 0;
			file.position = 0;
			file.fileLength = size(cur_record->fid);
			file.DateCreated = 0;
			file.TimeCreated = 0;
			file.flags = (get_faddr_by_id(cur_record->fid)->type == File) ? FS_FILE : FS_DIRECTORY;
			file.id = cur_record->fid;
			return file;
		}
		cur_record = cur_record->next_record;
	}

	file.flags = FS_INVALID;
	return file;
	
}

FILE tfsys_subdir_open (FILE dir, char *fname)
{

	FILE file;

	if (dir.flags != FS_DIRECTORY)
	{
		file.flags = FS_INVALID;
		return file;
	}

	dir_record *cur_record = (dir_record *)((char *)get_faddr_by_id(dir.id)+META_SIZE);

	while (cur_record)
	{
		if(strcmp(cur_record->name,fname))
		{
			strcpy(file.name,cur_record->name);
			file.eof = 0;
			file.position = 0;
			file.fileLength = size(cur_record->fid);
			file.DateCreated = 0;
			file.TimeCreated = 0;
			file.flags = (get_faddr_by_id(cur_record->fid)->type == File) ? FS_FILE : FS_DIRECTORY;
			file.id = cur_record->fid;
			return file;
		}
		cur_record = cur_record->next_record;
	}

	file.flags = FS_INVALID;
	return file;

}

PFILELIST tfsys_open_dir (char *path)
{
	FILE dir = tfsys_open(path);

	if (dir.flags != FS_DIRECTORY)
		return 0;

	PFILELIST lst = 0;
	PFILELIST head = 0;

	dir_record *cur_record = (dir_record *)((char *)get_faddr_by_id(dir.id)+META_SIZE);
	while (cur_record)
	{
		FILE file;
		strcpy(file.name, cur_record->name);
		file.eof = 0;
		file.position = 0;
		file.fileLength = size(cur_record->fid);
		file.DateCreated = 0;
		file.TimeCreated = 0;
		file.flags = (get_faddr_by_id(cur_record->fid)->type == File) ? FS_FILE : FS_DIRECTORY;

		PFILELIST tmp = (PFILELIST)kmalloc(sizeof(FILELIST));

		tmp->f = file;
		tmp->next = 0;

		if (lst)
		{
			lst->next = tmp;
			lst = tmp;
		}
		else
		{
			lst = tmp;
			head = lst;
		}

		cur_record = cur_record->next_record;
	}

	return head;

}

void tfsys_close (PFILE file) {

	if (file)
		file->flags = FS_INVALID;
}

void tfsys_create (FILE file, char *fname, uint32_t flags)
{

	if (file.flags == FS_DIRECTORY)
	{
		if (flags == FS_DIRECTORY)
			mkdir(fname, file.id);
		else if(flags == FS_FILE)
			touch(fname, file.id);
	}

}
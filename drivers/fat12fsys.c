#include "fat12fsys.h"
#include "floppy.h"
#include "memory.h"
#include "strings.h"
#include "heap.h"
#include "screen.h"

//! FAT FileSystem
FILESYSTEM _FSysFat;

static MOUNT_INFO _MountInfo;

//! File Allocation Table (FAT)
uint8_t FAT [SECTOR_SIZE*2];

FILELIST a;
uint32_t filelist_sz = (char *)(&a+1) - (char*)(&a);

FILE fat12fsys_open (char *path)
{

	if (strcmp(path,"\\"))
	{

		FILE base_dir;
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
			curr_file = fat12fsys_root_open(curr_filename);
		}
		else
		{
			curr_file = fat12fsys_subdir_open(curr_file, curr_filename); // our current file is a subdirectory
		}

		kfree(curr_filename);

		if (curr_file.flags == FS_INVALID)
			break; // an error has occured, do not continue

	}

	return curr_file;


}

void ToDosFileName(char *filename, char DosFileName[11 + 1])
{

	char *Filename;

	if (filename[0] == '.')
		Filename = 0;
	else
		Filename = seperate_and_take(filename, '.', 0);

	char *Ext;

	if (count_substrings(filename,'.') > 1)
		Ext = seperate_and_take(filename, '.', 1);
	else
		Ext = 0;

	if (strlen(Filename) > 8 || strlen(Ext) > 3)
	{
		printf("ToDosFileName: Error, filename or ext too large");
		return;
	}

	to_upper(Filename);
	to_upper(Ext);

	memset(DosFileName, ' ',11);
	if (Filename)
		memcpy(DosFileName,Filename,strlen(Filename));
	if (Ext)
		memcpy(DosFileName+11-strlen(Ext),Ext,strlen(Ext));
	if (strcmp(filename,"."))
		DosFileName[0] = '.';

	DosFileName[11] = 0; // null terminator

	kfree(Filename);
	kfree(Ext);

}

FILE fat12fsys_root_open (char* FileName) {

	FILE file;
	unsigned char* buf;
	PDIRECTORY directory;

	//! get 8.3 directory name
	char DosFileName[11 + 1];
	ToDosFileName (FileName, DosFileName);

	for (int sector=0; sector<14; sector++) {

		//! read in sector
		buf = (unsigned char*) flpydsk_read_sector ( _MountInfo.rootOffset + sector );

		//! get directory info
		directory = (PDIRECTORY) buf;

		//! 16 entries per sector
		for (int i=0; i<16; i++) {

			//! get current filename
			char name[11 + 1];
			memcpy (name, directory->Filename, 11);
			name[11]=0;

			//! find a match?
			if (strcmp (DosFileName, name)) {
				//! found it, set up file info
				strcpy (file.name, FileName);
				file.id             = 0;
				file.currentCluster = directory->FirstCluster;
				file.eof            = 0;
				file.position = 0;
				file.fileLength     = directory->FileSize;

				//! set file type
				if (directory->Attrib == 0x10)
					file.flags = FS_DIRECTORY;
				else
					file.flags = FS_FILE;


				//! return file
				return file;
			}
			//! go to next directory
			directory++;
		}
	}

	//! unable to find file
	file.flags = FS_INVALID;
	return file;



}

void fat12fsys_read (PFILE file, unsigned char* Buffer, unsigned int Length) {

	if (file && Length) {

		//! starting physical sector
		unsigned int physSector = 32 + (file->currentCluster - 1);

		//! read in sector
		unsigned char* sector = (unsigned char*) flpydsk_read_sector ( physSector );

		//! copy block of memory
		memcpy (Buffer, sector+file->position, (Length >= SECTOR_SIZE-file->position) ? SECTOR_SIZE-file->position : Length);

		if (Length < SECTOR_SIZE-file->position)
		{

			file->position += Length;

			if (file->position < SECTOR_SIZE)
				return;

		}

		unsigned int FAT_Offset = file->currentCluster + (file->currentCluster / 2); //multiply by 1.5
		unsigned int FAT_Sector = 1 + (FAT_Offset / SECTOR_SIZE);
		unsigned int entryOffset = FAT_Offset % SECTOR_SIZE;

		//! read 1st FAT sector
		sector = (unsigned char*) flpydsk_read_sector ( FAT_Sector );
		memcpy (FAT, sector, 512);

		//! read 2nd FAT sector because fat entry doesn't divide in a single sector
		sector = (unsigned char*) flpydsk_read_sector ( FAT_Sector + 1 );
		memcpy (FAT + SECTOR_SIZE, sector, 512);

		//! read entry for next cluster
		uint16_t nextCluster = *( uint16_t*) &FAT [entryOffset];

		//! test if entry is odd or even because we are reading a 12 bit value into 16 bit registers which means overlap.
		if( file->currentCluster & 0x0001 )
			nextCluster >>= 4;      //grab high 12 bits
		else
			nextCluster &= 0x0FFF;   //grab low 12 bits

		//! test for end of file
		if ( nextCluster >= 0xff8) {

			file->eof = 1;
			return;
		}

		//! test for file corruption
		if ( nextCluster == 0 ) {

			file->eof = 1;
			return;
		}

		//! set next cluster
		file->currentCluster = nextCluster;

		uint32_t old_pos = file->position;

		file->position = 0;

		fat12fsys_read(file,Buffer+(SECTOR_SIZE-old_pos),Length-(SECTOR_SIZE-old_pos));

	}
}

// kfile is the subdirectory
FILE fat12fsys_subdir_open (FILE kFile, char* filename) 
{

	FILE file;

	//! get 8.3 directory name
	char DosFileName[11 + 1];
	ToDosFileName (filename, DosFileName);

	//! read directory
	while (! kFile.eof ) {

		//! read directory
		unsigned char buf[512];
		fat12fsys_read (&kFile, buf, 512);

		//! set directory
		PDIRECTORY pkDir = (PDIRECTORY) buf;

		//! 16 entries in buffer (per sector of the directory)
		for (unsigned int i = 0; i < 16; i++) {

			//! get current filename
			char name[11 + 1];
			memcpy (name, pkDir->Filename, 11);
			name[11]=0;

			//! match?
			if (strcmp (name, DosFileName)) {
				//! found it, set up file info
				strcpy (file.name, filename);
				file.id             = 0;
				file.currentCluster = pkDir->FirstCluster;
				file.fileLength     = pkDir->FileSize;
				file.eof            = 0;
				file.position =     0;

				//! set file type
				if (pkDir->Attrib == 0x10)
					file.flags = FS_DIRECTORY;
				else
					file.flags = FS_FILE;

				//! return file
				return file;
			}
			//! go to next entry
			pkDir++;
		}
	}

	//! unable to find file
	file.flags = FS_INVALID;
	return file;
}

PFILELIST fat12fsys_open_dir (char *path)
{

	PFILELIST lst = 0;

	PFILELIST head = 0;

	bool is_root_dir = strcmp(path,"\\");

	FILE kFile;

	if (!is_root_dir)
	{
		kFile = fat12fsys_open(path);
		if (kFile.flags == FS_INVALID || kFile.flags == FS_FILE)
			return 0;
	}

	int sector = 0;

	//! read directory
	while ( true ) {

		//! read directory
		unsigned char *buf;

		if (is_root_dir)
		{
			if (sector == 14)
				break;
			//! read in sector
			buf = (unsigned char*) flpydsk_read_sector ( _MountInfo.rootOffset + sector );
			sector ++;
		}
		else
		{
			if (kFile.eof)
				break;
			buf = kmalloc(512);
			fat12fsys_read (&kFile, buf, 512);
		}

		//! set directory
		PDIRECTORY pkDir = (PDIRECTORY) buf;

		//! 16 entries in buffer (per sector of the directory)
		for (unsigned int i = 0; i < 16; i++) {

			if (pkDir->FirstCluster != 0)
			{

				FILE file;

				//! get current filename
				char name[11 + 1];
				memcpy (name, pkDir->Filename, 11);
				name[11]=0;
				//! found it, set up file info
				strcpy (file.name, name);
				file.id             = 0;
				file.currentCluster = pkDir->FirstCluster;
				file.fileLength     = pkDir->FileSize;
				file.eof            = 0;
				file.position = 0;

				//! set file type
				if (pkDir->Attrib == 0x10)
					file.flags = FS_DIRECTORY;
				else
					file.flags = FS_FILE;

				PFILELIST tmp = (PFILELIST)kmalloc(filelist_sz);

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

			}
			
			//! go to next entry
			pkDir++;
		}


		if (!is_root_dir)
			kfree(buf);
	}

	return head;

}

/**
*	Closes file
*/
void fat12fsys_close (PFILE file) {

	if (file)
		file->flags = FS_INVALID;
}

/**
*	Mounts the filesystem
*/
void fat12fsys_mount () {

	//! Boot sector info
	PBOOTSECTOR bootsector;

	//! read boot sector
	bootsector = (PBOOTSECTOR) flpydsk_read_sector (0);

	//! store mount info
	_MountInfo.numSectors     = bootsector->Bpb.NumSectors;
	_MountInfo.fatOffset      = 1;
	_MountInfo.fatSize        = bootsector->Bpb.SectorsPerFat;
	_MountInfo.fatEntrySize   = 8;
	_MountInfo.numRootEntries = bootsector->Bpb.NumDirEntries;
	_MountInfo.rootOffset     = (bootsector->Bpb.NumberOfFats * bootsector->Bpb.SectorsPerFat) + 1;
	_MountInfo.rootSize       = ( bootsector->Bpb.NumDirEntries * 32 ) / bootsector->Bpb.BytesPerSector;
 
}

/**
*	Initialize filesystem
*/
void fat12fsys_init () {

	//! initialize filesystem struct
	strcpy (_FSysFat.Name, "FAT12");
	_FSysFat.Mount     = fat12fsys_mount;
	_FSysFat.Open      = fat12fsys_open;
	_FSysFat.Read      = fat12fsys_read;
	_FSysFat.Close     = fat12fsys_close;
	_FSysFat.OpenDir   = fat12fsys_open_dir;

	//! register ourself to volume manager
	volRegisterFileSystem ( &_FSysFat, 0 );

	//! mount filesystem
	fat12fsys_mount ();
}

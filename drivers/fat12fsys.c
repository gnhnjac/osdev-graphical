#include "fat12fsys.h"
#include "vfs.h"
#include "floppy.h"
#include "memory.h"
#include "strings.h"

static MOUNT_INFO _MountInfo;

void fat12fsys_init()
{

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

FILE fat12fsys_open (const char* FileName) {

	FILE file;
	unsigned char* buf;
	PDIRECTORY directory;

	//! get 8.3 directory name
	char DosFileName[11 + 1];
	ToDosFileName (FileName, DosFileName);
	DosFileName[11 + 1]=0;

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
			name[11 + 1]=0;

			//! find a match?
			if (strcmp (DosFileName, name) == 0) {
				//! found it, set up file info
				strcpy (file.name, FileName);
				file.id             = 0;
				file.currentCluster = directory->FirstCluster;
				file.eof            = 0;
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

	if (file) {

		//! starting physical sector
		unsigned int physSector = 32 + (file->currentCluster - 1);

		//! read in sector
		unsigned char* sector = (unsigned char*) flpydsk_read_sector ( physSector );

		//! copy block of memory
		memcpy (Buffer, sector, 512);
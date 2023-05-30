#include <stdint.h>
#include "vfs.h"
#include "floppy.h"

#define SECTOR_SIZE 512

#pragma pack(1)

typedef struct _BIOS_PARAMATER_BLOCK {

	uint16_t		BytesPerSector;
	uint8_t			SectorsPerCluster;
	uint16_t		ReservedSectors;
	uint8_t			NumberOfFats;
	uint16_t		NumDirEntries;
	uint16_t		NumSectors;
	uint8_t			Media;
	uint16_t		SectorsPerFat;
	uint16_t		SectorsPerTrack;
	uint16_t		HeadsPerCyl;
	uint32_t		HiddenSectors;
	uint32_t		LongSectors;

} BIOSPARAMATERBLOCK, *PBIOSPARAMATERBLOCK;

typedef struct _BIOS_PARAMATER_BLOCK_EXT {

	uint8_t		DriveNumber;   //physical drive number
	uint8_t		Flags;             //flags
	uint8_t		ExBootSignature;
	uint32_t	SerialNumber; 
	uint8_t	VolLabel[11];
	uint8_t	FsType[8];

} BIOSPARAMATERBLOCKEXT, *PBIOSPARAMATERBLOCKEXT;

typedef struct _BOOT_SECTOR {

	uint8_t			Ignore[3];		//first 3 bytes are ignored (our jmp instruction)
	uint8_t			OEMName[8];
	BIOSPARAMATERBLOCK	Bpb;			//BPB structure
	BIOSPARAMATERBLOCKEXT	BpbExt;			//extended BPB info
	uint8_t			Filler[512-51-8-3];		//needed to make struct 512 bytes

} BOOTSECTOR, *PBOOTSECTOR;

// *** directory entry
typedef struct _DIRECTORY {

	char   Filename[8];           //filename
	char   Ext[3];                //extension (8.3 filename format)
	uint8_t   Attrib;                //file attributes
	uint8_t   Reserved;
	uint8_t   TimeCreatedMs;         //creation time
	uint16_t  TimeCreated;
	uint16_t  DateCreated;           //creation date
	uint16_t  DateLastAccessed;
	uint16_t  FirstClusterHiBytes;
	uint16_t  LastModTime;           //last modification date/time
	uint16_t  LastModDate;
	uint16_t  FirstCluster;          //first cluster of file data
	uint32_t  FileSize;              //size in bytes

}DIRECTORY, *PDIRECTORY;

#pragma pack()

typedef struct _MOUNT_INFO {

	uint16_t numSectors;
	uint32_t fatOffset;
	uint16_t numRootEntries;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint16_t fatSize;
	uint32_t fatEntrySize;

}MOUNT_INFO, *PMOUNT_INFO;


// attrib format:
// Read only: 1
// Hidden: 2
// System: 4
// Volume Lable: 8
// Subdirectory: 0x10
// Archive: 0x20
// Device: 0x60

// date format:
// Bits 0-4: Day (0-31)
// Bits 5-8: Month (0-12)
// Bits 9-15: Year

// time format:
// Bits 0-4: Second
// Bits 5-10: Minute
// Bits 11-15: Hour

//refs
//! File Allocation Table (FAT);
FILE fat12fsys_open (char *path);
bool ToDosFileName(char *filename, char DosFileName[11 + 1]);
FILE fat12fsys_root_open (char* FileName);
void fat12fsys_create(FILE kFile, char* FileName, uint32_t flags);
unsigned int fat12fsys_occupy_free_cluster(unsigned int chain_cluster);
void fat12fsys_read(PFILE file, unsigned char* Buffer, unsigned int Length);
void fat12fsys_write(PFILE file, unsigned char* Buffer, unsigned int Length);
void fat12fsys_rw (PFILE file, unsigned char* Buffer, unsigned int Length, floppy_dir dir);
FILE fat12fsys_subdir_open (FILE kFile, char* filename);
PFILELIST fat12fsys_open_dir (char *path);
void fat12fsys_close (PFILE file);
void fat12fsys_mount ();
void fat12fsys_init ();
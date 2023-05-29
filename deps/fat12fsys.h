#include <stdint.h>

typedef struct _BIOS_PARAMATER_BLOCK {

	uint8_t			OEMName[8];
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

}BIOSPARAMATERBLOCK, *PBIOSPARAMATERBLOCK;

typedef struct _BIOS_PARAMATER_BLOCK_EXT {

	uint32_t			SectorsPerFat32;   //sectors per FAT
	uint16_t			Flags;             //flags
	uint16_t			Version;           //version
	uint32_t			RootCluster;       //starting root directory
	uint16_t			InfoCluster;
	uint16_t			BackupBoot;        //location of bootsector copy
	uint16_t			Reserved[6];

}BIOSPARAMATERBLOCKEXT, *PBIOSPARAMATERBLOCKEXT;

typedef struct _BOOT_SECTOR {

	uint8_t			Ignore[3];		//first 3 bytes are ignored (our jmp instruction)
	BIOSPARAMATERBLOCK	Bpb;			//BPB structure
	BIOSPARAMATERBLOCKEXT	BpbExt;			//extended BPB info
	uint8_t			Filler[448];		//needed to make struct 512 bytes

}BOOTSECTOR, *PBOOTSECTOR;

typedef struct _MOUNT_INFO {

	uint32_t numSectors;
	uint32_t fatOffset;
	uint32_t numRootEntries;
	uint32_t rootOffset;
	uint32_t rootSize;
	uint32_t fatSize;
	uint32_t fatEntrySize;

}MOUNT_INFO, *PMOUNT_INFO;


// *** directory entry
typedef struct _DIRECTORY {

	uint8_t   Filename[8];           //filename
	uint8_t   Ext[3];                //extension (8.3 filename format)
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
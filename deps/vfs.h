#pragma once
#include <stdint.h>

typedef struct _FILE {

	char        name[32];
	uint32_t    flags;
	uint32_t    fileLength;
	uint32_t    id;
	uint32_t    eof;
	uint32_t    position;
	uint32_t    currentCluster;
	uint32_t    deviceID;
	uint16_t    TimeCreated;
	uint16_t    DateCreated;

}FILE, *PFILE;

// date format:
// Bits 0-4: Day (0-31)
// Bits 5-8: Month (0-12)
// Bits 9-15: Year

// time format:
// Bits 0-4: Second
// Bits 5-10: Minute
// Bits 11-15: Hour

typedef struct _FILELIST *PFILELIST;

typedef struct _FILELIST {

	FILE f;
	PFILELIST next;

}FILELIST;

#define FS_FILE       0
#define FS_DIRECTORY  1
#define FS_INVALID    2
#define DEVICE_MAX 26 // letters a-z

typedef struct _FILE_SYSTEM {

	char Name [8];
	void	           (*Mount)      ();
	void               (*Read)       (PFILE file, unsigned char* Buffer, unsigned int Length);
	void               (*Write)      (PFILE file, unsigned char* Buffer, unsigned int Length);
	void	           (*Close)      (PFILE);
	FILE               (*Open)       (char* FileName);
	PFILELIST          (*OpenDir)    (char* FileName);
	void               (*Create)     (FILE directory, char *FileName, uint32_t flags);

}FILESYSTEM, *PFILESYSTEM;

//refs
FILE volOpenFile (const char* fname);
PFILELIST volOpenDir (const char* fname);
void volCloseFile (PFILE file);
void volReadFile (PFILE file, unsigned char* Buffer, unsigned int Length);
void volWriteFile (PFILE file, unsigned char* Buffer, unsigned int Length);
void volCreateFile (PFILE file, char *fname, uint32_t flags);
void volRegisterFileSystem (PFILESYSTEM fsys, unsigned int deviceID);
void volUnregisterFileSystem (PFILESYSTEM fsys);
void volUnregisterFileSystemByID (unsigned int deviceID);
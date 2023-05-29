#include <stdint.h>

typedef struct _FILE {

	char        name[32];
	uint32_t    flags;
	uint32_t    fileLength;
	uint32_t    id;
	uint32_t    eof;
	uint32_t    position;
	uint32_t    currentCluster;
	uint32_t    device;

}FILE, *PFILE;

#define FS_FILE       0
#define FS_DIRECTORY  1
#define FS_INVALID    2
#define DEVICE_MAX 26 // letters a-z

typedef struct _FILE_SYSTEM {

	char Name [8];
	FILE               (*Directory)  (const char* DirectoryName);
	void	           (*Mount)      ();
	void               (*Read)       (PFILE file, unsigned char* Buffer, unsigned int Length);
	void	           (*Close)      (PFILE);
	FILE               (*Open)       (const char* FileName);

}FILESYSTEM, *PFILESYSTEM;

//refs
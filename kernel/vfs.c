#include "vfs.h"
#include "lock.h"

//! File system list
PFILESYSTEM _FileSystems[DEVICE_MAX];

// NEED TO ADD SPINLOCK TO REST OF API

FILE volOpenFile (const char* fname) {

	if (fname) {

		//! default to device 'a' or 0
		unsigned char device = 0;

		//! filename
		char* filename = (char*) fname;

		//! in all cases, if fname[1]==':' then the first character must be device letter
		if (fname[1]==':') {

			device = fname[0] - 'a';
			filename += 2; //strip it from pathname
		}

		//! call filesystem
		if (_FileSystems [device]) {

			//! set volume specific information and return file

			static lock_t vfs_open_spinlock = ATOMIC_LOCK_INIT;

			acquireLock(&vfs_open_spinlock);

			FILE file = _FileSystems[device]->Open (filename);

			releaseLock(&vfs_open_spinlock);

			file.deviceID = device;
			return file;
		}
	}

	FILE file;
	file.flags = FS_INVALID;
	return file;
}

PFILELIST volOpenDir (const char* fname) {

	PFILELIST head = 0;

	if (fname) {

		//! default to device 'a' or 0
		unsigned char device = 0;

		//! filename
		char* filename = (char*) fname;

		//! in all cases, if fname[1]==':' then the first character must be device letter
		if (fname[1]==':') {

			device = fname[0] - 'a';
			filename += 2; //strip it from pathname
		}

		//! call filesystem
		if (_FileSystems [device]) {

			//! set volume specific information and return file
			head = _FileSystems[device]->OpenDir (filename);
			PFILELIST lst = head;
			while (lst)
			{
				lst->f.deviceID = device;
				lst = lst->next;
			}
			return head;
		}
	}

	return head;
}

void volCloseFile (PFILE file) {

	if (file)
	{
		if (_FileSystems [file->deviceID])
			_FileSystems[file->deviceID]->Close (file);
	}
}

void volReadFile (PFILE file, unsigned char* Buffer, unsigned int Length) {

	if (file)
	{
		if (_FileSystems [file->deviceID])
		{
			static lock_t vfs_read_spinlock = ATOMIC_LOCK_INIT;

			acquireLock(&vfs_read_spinlock);
			_FileSystems[file->deviceID]->Read (file,Buffer,Length);
			releaseLock(&vfs_read_spinlock);
		}
	}
}

void volWriteFile (PFILE file, unsigned char* Buffer, unsigned int Length) {

	if (file)
	{
		if (_FileSystems [file->deviceID])
		{
			static lock_t vfs_write_spinlock = ATOMIC_LOCK_INIT;

			acquireLock(&vfs_write_spinlock);
			_FileSystems[file->deviceID]->Write(file,Buffer,Length);
			releaseLock(&vfs_write_spinlock);
		}
	}
}

void volCreateFile (PFILE file, char *fname, uint32_t flags) {

	if (file)
	{
		if (_FileSystems [file->deviceID])
			_FileSystems[file->deviceID]->Create (*file,fname,flags);
	}
}

void volRegisterFileSystem (PFILESYSTEM fsys, unsigned int deviceID) {

	static int i=0; // is in the data segment

	if (deviceID < DEVICE_MAX && i < DEVICE_MAX){
		if (fsys)
		{
			_FileSystems[ deviceID ] = fsys;
			i++;
		}
	}
}

void volUnregisterFileSystem (PFILESYSTEM fsys) {

	for (int i=0;i < DEVICE_MAX; i++)
		if (_FileSystems[i]==fsys)
			_FileSystems[i]=0;
}

void volUnregisterFileSystemByID (unsigned int deviceID) {

	if (deviceID < DEVICE_MAX)
		_FileSystems [deviceID] = 0;
}
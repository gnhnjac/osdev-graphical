#include "vfs.h"

//! File system list
PFILESYSTEM _FileSystems[DEVICE_MAX];

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
			FILE file = _FileSystems[device]->Open (filename);
			file.deviceID = device;
			return file;
		}
	}

	FILE file;
	file.flags = FS_INVALID;
	return file;
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
			_FileSystems[file->deviceID]->Read (file,Buffer,Length);
	}
}

void volRegisterFileSystem (PFILESYSTEM fsys, unsigned int deviceID) {

	if (deviceID < DEVICE_MAX){
		if (fsys)
			_FileSystems[ deviceID ] = fsys;
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
#include "loader.h"
#include "image.h"
#include "vfs.h"
#include "heap.h"

// loads executable into virtual memory and returns entry point address
PImageInfo load_executable(pdirectory *pdir, char *path)
{

	FILE exec = volOpenFile(path);

	if (exec.flags != FS_FILE)
       	return 0; 

    IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER*)kmalloc(sizeof(IMAGE_DOS_HEADER));

    volReadFile(&exec,(void *)dosHeader,sizeof(IMAGE_DOS_HEADER));

    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
    		volCloseFile(&exec);
    		kfree(dosHeader);
            return 0;
    }

	uint32_t peHeaderRVA = dosHeader->e_lfanew;

	void *dosStub = kmalloc(peHeaderRVA-sizeof(IMAGE_DOS_HEADER));

	kfree(dosHeader);

	volReadFile(&exec,(void *)dosStub,peHeaderRVA-sizeof(IMAGE_DOS_HEADER));

	kfree(dosStub);

	IMAGE_NT_HEADERS *peHeader = (IMAGE_NT_HEADERS *)kmalloc(sizeof(IMAGE_NT_HEADERS));

	volReadFile(&exec,(void *)peHeader,sizeof(IMAGE_NT_HEADERS));

	if  (  (peHeader->Signature != IMAGE_NT_SIGNATURE)
	    || (peHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
	    || (!(peHeader->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
	    || ((peHeader->OptionalHeader.ImageBase < 0x400000) || (peHeader->OptionalHeader.ImageBase > 0x80000000))
	    || (peHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	    )
	{
		volCloseFile(&exec);
    	kfree(dosHeader);
        return 0;
	}

	// //! get image base and entry point address from optional header
	uint32_t imageBase = peHeader->OptionalHeader.ImageBase;
	uint32_t imageSize = peHeader->OptionalHeader.SizeOfImage;
	uint32_t entryPoint = peHeader->OptionalHeader.AddressOfEntryPoint;

	PIMAGE_DATA_DIRECTORY dataDirectory = peHeader->OptionalHeader.DataDirectory;
	PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY) (dataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + imageBase);

	kfree(peHeader);

	uint32_t fileSize = imageSize;
	if (fileSize % 4096 != 0)
		fileSize += 4096 - fileSize % 4096;

	for  (uint32_t i = 0; i < fileSize; i+=4096)
		vmmngr_alloc_virt(pdir, (void *)(imageBase+i), I86_PDE_WRITABLE|I86_PDE_USER, I86_PTE_WRITABLE|I86_PTE_USER);

	volReadFile(&exec,(void *)imageBase+peHeaderRVA+sizeof(IMAGE_NT_HEADERS),exec.fileLength); // load the file into memory.

	volCloseFile(&exec);

	PImageInfo imageInfo = (PImageInfo)kmalloc(sizeof(ImageInfo));

	imageInfo->ImageBase = imageBase;
	imageInfo->ImageSize = imageSize;
	imageInfo->EntryPointRVA = entryPoint;

	return imageInfo;

}
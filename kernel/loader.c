#include "loader.h"
#include "image.h"
#include "vfs.h"
#include "heap.h"
#include "memory.h"

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
	uint32_t numOfSections = peHeader->FileHeader.NumberOfSections;
	uint32_t fileAlignment = peHeader->OptionalHeader.FileAlignment;

	PIMAGE_DATA_DIRECTORY dataDirectory = peHeader->OptionalHeader.DataDirectory;
	PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY) (dataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + imageBase);

	kfree(peHeader);

	PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)kmalloc(numOfSections*IMAGE_SIZEOF_SECTION_HEADER);

	void *tmpSectionHeader = (void *)sectionHeader;

	volReadFile(&exec,(void *)sectionHeader,numOfSections*IMAGE_SIZEOF_SECTION_HEADER); // load the headers into memory.

	uint32_t unalignedSectionBase = sizeof(IMAGE_NT_HEADERS) + peHeaderRVA + numOfSections*IMAGE_SIZEOF_SECTION_HEADER;
	uint32_t alignmentBytes = fileAlignment - unalignedSectionBase % fileAlignment;

	void *tmpBuff = kmalloc(sizeof(alignmentBytes));
	volReadFile(&exec,tmpBuff,alignmentBytes); // load the alignment
	kfree(tmpBuff);

	// load the sections into memory and allocate virtual memory for them

	for (int i = 0; i < numOfSections; i++)
	{
		uint32_t sectionPageBase = imageBase+sectionHeader->VirtualAddress;
		if (sectionPageBase % 4096 != 0)
			sectionPageBase -= 4096 - sectionPageBase % 4096;

		uint32_t sectionPageTop = imageBase+sectionHeader->VirtualAddress+sectionHeader->SizeOfRawData;
		if (IMAGE_SCN_CNT_UNINITIALIZED_DATA&sectionHeader->Characteristics) // if section is bss then virtual size is used
			sectionPageTop += sectionHeader->Misc.VirtualSize;
		if (sectionPageTop % 4096 != 0)
			sectionPageTop += 4096 - sectionPageTop % 4096;

		uint32_t pdeFlags = I86_PDE_USER;
		uint32_t pteFlags = I86_PTE_USER;

		if (IMAGE_SCN_MEM_WRITE&sectionHeader->Characteristics) // if section is writable add the writable flag to the page
		{
			pdeFlags |= I86_PDE_WRITABLE;
			pteFlags |= I86_PTE_WRITABLE;
		}

		for  (uint32_t virt = sectionPageBase; virt < sectionPageTop; virt+=4096)
			vmmngr_alloc_virt(pdir, (void *)virt, pdeFlags, pteFlags);

		if (!(IMAGE_SCN_CNT_UNINITIALIZED_DATA&sectionHeader->Characteristics)) // if section doesn't contain uninitialized data
		{
			volReadFile(&exec,(void *)(imageBase+sectionHeader->VirtualAddress),sectionHeader->SizeOfRawData); // load the section into memory.
		}
		else
		{
			// zero out the bss section
			memset((char *)(imageBase+sectionHeader->VirtualAddress),0,sectionHeader->Misc.VirtualSize);
		}
		sectionHeader++;

	}

	kfree(tmpSectionHeader);

	volCloseFile(&exec);

	PImageInfo imageInfo = (PImageInfo)kmalloc(sizeof(ImageInfo));

	imageInfo->ImageBase = imageBase;
	imageInfo->ImageSize = imageSize;
	imageInfo->EntryPointRVA = entryPoint;

	return imageInfo;

}
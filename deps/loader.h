#include <stdint.h>
#include "vmm.h"

typedef struct _ImageInfo
{

	uint32_t ImageBase;
	uint32_t ImageSize;
	uint32_t EntryPointRVA;

} ImageInfo, *PImageInfo;

//refs
PImageInfo load_executable(pdirectory *pdir, char *path);
PImageInfo load_mz_executable(pdirectory *pdir, char *path);
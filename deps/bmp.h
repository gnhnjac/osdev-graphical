#include <stdint.h>

#define BMP_SIGNATURE 0x4D42 // BM

#pragma pack(1)

typedef struct _BITMAPFILEHEADER
{

	uint16_t Signature;
	uint32_t FileSize;
	uint32_t Reserved;
	uint32_t DataOffset;

} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER
{

	uint32_t Size; // 40 (size of info header)
	uint32_t Width;
	uint32_t Height;
	uint16_t Planes;
	uint16_t BPP;
	uint32_t Compression;
	uint32_t Imagesize;
	uint32_t XpixelsPerM;
	uint32_t YpixelsPerM;
	uint32_t ColorsUsed;
	uint32_t ImportantColors;

} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()
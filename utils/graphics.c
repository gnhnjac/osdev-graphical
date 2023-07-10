#include "vfs.h"
#include "graphics.h"
#include "screen.h"
#include "heap.h"

uint32_t mask_table[256][2];
uint8_t *font_buff;

static uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void init_psf1_8x16()
{

	font_buff = kmalloc(256*16);

	for (int i = 0; i < 256; i++)
	{

		uint8_t bot_nibble = i & 0xF;
		uint8_t top_nibble = (i & 0xF0)>>4;

		uint32_t bot_double = ((bot_nibble&1)?0xFF:0)|((bot_nibble&2)?0xFF00:0)|((bot_nibble&4)?0xFF0000:0)|((bot_nibble&8)?0xFF000000:0);
		uint32_t top_double = ((top_nibble&1)?0xFF:0)|((top_nibble&2)?0xFF00:0)|((top_nibble&4)?0xFF0000:0)|((top_nibble&8)?0xFF000000:0);

		mask_table[i][0] = bot_double;
		mask_table[i][1] = top_double;

	}

}

int load_psf1_8x16(char *path)
{

	FILE psf = volOpenFile(path);
	
	PSF1_Header *psf_header = (PSF1_Header*)kmalloc(sizeof(PSF1_Header));

    volReadFile(&psf,(void *)psf_header,sizeof(PSF1_Header));

    if (psf_header->magic != PSF1_FONT_MAGIC || psf_header->characterSize != 16 || psf_header->fontMode & PSF1_MODE512)
    	return 0;

    kfree(psf_header);

    volReadFile(&psf, font_buff, 256*16);

    for (int i = 0; i < 256; i++)
    {

    	for (int j = 0; j < 16; j++)
    	{

    		font_buff[i*16 + j] = reverse(font_buff[i*16 + j]);

    	}

    }

    volCloseFile(&psf);

    return 1;

}

void display_psf1_8x16_char(char c, int x, int y, int fgcolor)
{

	uint32_t *dest32;
	uint8_t *dest;
	unsigned char *src;
	int row;
	uint32_t fgcolor32;
 
	fgcolor32 = fgcolor | (fgcolor << 8) | (fgcolor << 16) | (fgcolor << 24);
	src = font_buff + c * 16;
	dest = (uint8_t *)(VIDEO_ADDRESS + y * PIXEL_WIDTH + x);
	for(row = 0; row < 16; row++) {
		if(*src != 0) {
			uint32_t mask_low = mask_table[*src][0];
			uint32_t mask_high = mask_table[*src][1];
			dest32 = (uint32_t *)dest;
			dest32[0] = (dest32[0] & ~mask_low) | (fgcolor32 & mask_low);
			dest32[1] = (dest32[1] & ~mask_high) | (fgcolor32 & mask_high);
		}
		src++;
		dest += PIXEL_WIDTH;
	}

}

void display_psf1_8x16_char_bg(char c, int x, int y, int bgcolor, int fgcolor)
{

	uint32_t *dest32;
	uint8_t *dest;
	unsigned char *src;
	int row;
	uint32_t fgcolor32;
	uint32_t bgcolor32;
 
	fgcolor32 = fgcolor | (fgcolor << 8) | (fgcolor << 16) | (fgcolor << 24);
	bgcolor32 = bgcolor | (bgcolor << 8) | (bgcolor << 16) | (bgcolor << 24);
	src = font_buff + c * 16;
	dest = (uint32_t *)(VIDEO_ADDRESS + y * PIXEL_WIDTH + x);
	for(row = 0; row < 16; row++) {
		if(*src != 0) {
			uint32_t mask_low = mask_table[*src][0];
			uint32_t mask_high = mask_table[*src][1];
			dest32 = (uint32_t *)dest;
			dest32[0] = (bgcolor32 & ~mask_low) | (fgcolor32 & mask_low);
			dest32[1] = (bgcolor32 & ~mask_high) | (fgcolor32 & mask_high);
		}
		else
		{
			dest32 = (uint32_t *)dest;
			dest32[0] = bgcolor32;
			dest32[1] = bgcolor32;
		}
		src++;
		dest += PIXEL_WIDTH;
	}

}

void fill_rect(int x, int y, int width, int height, uint8_t color)
{

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + y * PIXEL_WIDTH + x;

	for (int i = 0; i < width; i++)
	{

		for (int j = 0; j < height; j++)
		{

			vram[j*PIXEL_WIDTH] = color;

		}

		vram += 1;

	}

}

void set_pixel (int x, int y, uint8_t color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + y * PIXEL_WIDTH + x;

	*vram = color;

}

uint8_t get_pixel (int x, int y)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return 0;

	return *(uint8_t *)((uint32_t)VIDEO_ADDRESS + y * PIXEL_WIDTH + x);

}
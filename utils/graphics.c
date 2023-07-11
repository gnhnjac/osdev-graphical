#include "vfs.h"
#include "graphics.h"
#include "screen.h"
#include "heap.h"
#include "low_level.h"
#include "math.h"

//uint32_t mask_table[256][2];

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

	// for (int i = 0; i < 256; i++)
	// {

	// 	uint8_t bot_nibble = i & 0xF;
	// 	uint8_t top_nibble = (i & 0xF0)>>4;

	// 	uint32_t bot_double = ((bot_nibble&1)?0xFF:0)|((bot_nibble&2)?0xFF00:0)|((bot_nibble&4)?0xFF0000:0)|((bot_nibble&8)?0xFF000000:0);
	// 	uint32_t top_double = ((top_nibble&1)?0xFF:0)|((top_nibble&2)?0xFF00:0)|((top_nibble&4)?0xFF0000:0)|((top_nibble&8)?0xFF000000:0);

	// 	mask_table[i][0] = bot_double;
	// 	mask_table[i][1] = top_double;

	// }

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

void display_psf1_8x16_char_linear(char c, int x, int y, int fgcolor)
{

	// uint32_t *dest32;
	// uint8_t *dest;
	// unsigned char *src;
	// int row;
	// uint32_t fgcolor32;
 
	// fgcolor32 = fgcolor | (fgcolor << 8) | (fgcolor << 16) | (fgcolor << 24);
	// src = font_buff + c * 16;
	// dest = (uint8_t *)(VIDEO_ADDRESS + y * PIXEL_WIDTH + x);
	// for(row = 0; row < 16; row++) {
	// 	if(*src != 0) {
	// 		uint32_t mask_low = mask_table[*src][0];
	// 		uint32_t mask_high = mask_table[*src][1];
	// 		dest32 = (uint32_t *)dest;
	// 		dest32[0] = (dest32[0] & ~mask_low) | (fgcolor32 & mask_low);
	// 		dest32[1] = (dest32[1] & ~mask_high) | (fgcolor32 & mask_high);
	// 	}
	// 	src++;
	// 	dest += PIXEL_WIDTH;
	// }

}

void display_psf1_8x16_char_bg_linear(char c, int x, int y, int bgcolor, int fgcolor)
{

	// uint32_t *dest32;
	// uint8_t *dest;
	// unsigned char *src;
	// int row;
	// uint32_t fgcolor32;
	// uint32_t bgcolor32;
 
	// fgcolor32 = fgcolor | (fgcolor << 8) | (fgcolor << 16) | (fgcolor << 24);
	// bgcolor32 = bgcolor | (bgcolor << 8) | (bgcolor << 16) | (bgcolor << 24);
	// src = font_buff + c * 16;
	// dest = (uint8_t *)(VIDEO_ADDRESS + y * PIXEL_WIDTH + x);
	// for(row = 0; row < 16; row++) {
	// 	if(*src != 0) {
	// 		uint32_t mask_low = mask_table[*src][0];
	// 		uint32_t mask_high = mask_table[*src][1];
	// 		dest32 = (uint32_t *)dest;
	// 		dest32[0] = (bgcolor32 & ~mask_low) | (fgcolor32 & mask_low);
	// 		dest32[1] = (bgcolor32 & ~mask_high) | (fgcolor32 & mask_high);
	// 	}
	// 	else
	// 	{
	// 		dest32 = (uint32_t *)dest;
	// 		dest32[0] = bgcolor32;
	// 		dest32[1] = bgcolor32;
	// 	}
	// 	src++;
	// 	dest += PIXEL_WIDTH;
	// }

}

void display_psf1_8x16_char(char c, int x, int y, uint8_t fgcolor)
{


	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)VIDEO_ADDRESS + (y * PIXEL_WIDTH + x)/PIXELS_PER_BYTE;

	uint8_t mask_lower = ~(1<<(x % 8)-1);
	uint8_t mask_upper = (1<<(x % 8)-1);

	outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
	for(int row = 0; row < 16; row++) {

		uint8_t mask_lower = reverse((*src)<<(x%8));
		uint8_t mask_upper = reverse((*src)>>(8-x%8));

		outb(0x3C5, 0xF);
		dest[0] &= ~mask_lower;
		dest[1] &= ~mask_upper;
		outb(0x3C5, fgcolor);
		dest[0] |= mask_lower;
		dest[1] |= mask_upper;

		src++;
		dest += PIXEL_WIDTH/PIXELS_PER_BYTE;
	}

}

void display_psf1_8x16_char_bg(char c, int x, int y, int bgcolor, int fgcolor)
{

	uint8_t *src = font_buff + c * 16;
	for(int row = 0; row < 16; row++) {

		uint8_t tmp = *src;

		for(int j = 0; j < 8; j++)
		{

			set_pixel(x+j,y+row,(tmp%2)?fgcolor:bgcolor);

			tmp /= 2;

		}
		
		src++;
	}

}

void fill_rect_linear(int x, int y, int width, int height, uint8_t color)
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

void fill_rect(int x, int y, int width, int height, uint8_t color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return;

	if (x + width >= PIXEL_WIDTH)
		width = PIXEL_WIDTH - x;
	if (y + height >= PIXEL_HEIGHT)
		height = PIXEL_HEIGHT - y;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + y*PIXEL_WIDTH/PIXELS_PER_BYTE;
	outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
	for (int i = 0; i < height; i++)
	{

		for (int j = 0; j < width; j++)
		{
			outb(0x3C5, 0xF);
			vram[(x+j)/PIXELS_PER_BYTE] &= ~(0x80>>((x+j)%PIXELS_PER_BYTE));
			outb(0x3C5, color);
			vram[(x+j)/PIXELS_PER_BYTE] |= (0x80>>((x+j)%PIXELS_PER_BYTE));

		}

		vram += PIXEL_WIDTH/PIXELS_PER_BYTE;

	}
	
}

void outline_circle(int mx, int my, int rad, uint8_t color)
{

	float increment = 1/(float)rad;
	float b = -2*my;
	float c_const = mx*mx+my*my-rad*rad;

	for(float x = mx-rad; x < mx+rad; x+=increment)
	{

		float c = x*x-2*mx*x+c_const;

		float sqrt = root(b*b-4*c);

		if (!sqrt)
			continue;

		float y1 = (-b+sqrt)/2;
		float y2 = (-b-sqrt)/2;

		set_pixel(x,y1,color);
		set_pixel(x,y2,color);

	}

	set_pixel(mx-rad,my,color);
	set_pixel(mx+rad,my,color);

}

void set_pixel (int x, int y, uint8_t color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return;

	outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
	outb(0x3C5, 0xF);

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (y * PIXEL_WIDTH + x)/PIXELS_PER_BYTE;

	*vram &= ~(0x80>>(x%PIXELS_PER_BYTE));

	outb(0x3C5, color);

	*vram |= 0x80>>(x%PIXELS_PER_BYTE);

}

uint8_t is_planar_bit_activated(int x, int y, uint8_t plane)
{

	outb(0x3CE, READ_MAP_SELECT);
	outb(0x3CF, plane);

	uint8_t byte = *(uint8_t *)((uint8_t *)VIDEO_ADDRESS + (y * PIXEL_WIDTH + x)/PIXELS_PER_BYTE);
	return byte & (0x80>>(x%PIXELS_PER_BYTE));

}

uint8_t is_planar_bit_activated_offset(uint8_t *byteoff, uint8_t bitmask, uint8_t plane)
{
	outb(0x3CE, READ_MAP_SELECT);
	outb(0x3CF, plane);

	return (*byteoff) & bitmask;

}

uint8_t get_pixel (int x, int y)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return 0;

	uint8_t color = 0;

	if (is_planar_bit_activated(x,y,0))
		color |= 1;
	if (is_planar_bit_activated(x,y,1))
		color |= 2;
	if (is_planar_bit_activated(x,y,2))
		color |= 4;
	if (is_planar_bit_activated(x,y,3))
		color |= 8;

	return color;

}
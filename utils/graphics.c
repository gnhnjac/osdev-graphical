#include "vfs.h"
#include "graphics.h"
#include "screen.h"
#include "heap.h"
#include "low_level.h"
#include "math.h"
#include "memory.h"

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

void graphics_init()
{

	init_psf1_8x16();
	if (!load_psf1_8x16("a:\\font.psf"))
		return;

}

uint8_t *get_font_buffer()
{

	return font_buff;

}

void load_font_to_buffer(void *buff)
{

	memcpy((char *)buff,font_buff,256*16);

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

void display_psf1_8x16_char(char c, int x, int y, uint32_t fgcolor)
{

	if (x+CHAR_WIDTH > PIXEL_WIDTH || y+CHAR_HEIGHT > PIXEL_HEIGHT || x < 0 || y < 0)
		return;

	uint8_t *src = font_buff + c * 16;
	for(int row = 0; row < 16; row++) {

		uint8_t tmp = *src;

		for(int j = 0; j < 8; j++)
		{

			if (tmp%2)
				set_pixel(x+j,y+row,fgcolor);

			tmp /= 2;

		}
		
		src++;
	}

}

void display_psf1_8x16_char_bg(char c, int x, int y, uint32_t bgcolor, uint32_t fgcolor)
{

	if (x+CHAR_WIDTH > PIXEL_WIDTH || y+CHAR_HEIGHT > PIXEL_HEIGHT || x < 0 || y < 0)
		return;

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

void outline_rect(int x, int y, int width, int height, int size, uint32_t color)
{

	fill_rect(x-size,y-size,width+size*2,size,color);
	fill_rect(x-size,y+height,width+size*2,size,color);

	fill_rect(x-size,y,size,height,color);
	fill_rect(x+width,y,size,height,color);

}

void fill_gradient(int x, int y, int width, int height, uint32_t start_color, uint32_t end_color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return;

	if (x < 0)
	{
		width += x;
		x = 0;
	}

	if (y < 0)
	{
		height += y;
		y = 0;
	}

	if (x + width >= PIXEL_WIDTH)
		width = PIXEL_WIDTH - x;
	if (y + height >= PIXEL_HEIGHT)
		height = PIXEL_HEIGHT - y;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (y*PIXEL_WIDTH + x)*3;

	int32_t sr = start_color&0xff;
	int32_t sg = (start_color>>8)&0xff;
   int32_t sb = (start_color>>16)&0xff;

   int32_t er = end_color&0xff;
	int32_t eg = (end_color>>8)&0xff;
   int32_t eb = (end_color>>16)&0xff;

	for (int i = 0; i < height; i++)
	{

		for (int j = 0; j < width; j++)
		{

			int32_t r = sr + (er - sr) * j / width;
			int32_t g = sg + (eg - sg) * j / width;
			int32_t b = sb + (eb - sb) * j / width;

			uint32_t color = r | (g << 8) | (b << 16);

			*(uint32_t *)(vram + j*3) &= 0xFF000000;

			*(uint32_t *)(vram + j*3) |= color&0xffffff;

		}

		vram += PIXEL_WIDTH*3;

	}

}

void fill_rect(int x, int y, int width, int height, uint32_t color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
		return;

	if (x < 0)
	{
		width += x;
		x = 0;
	}

	if (y < 0)
	{
		height += y;
		y = 0;
	}

	if (x + width >= PIXEL_WIDTH)
		width = PIXEL_WIDTH - x;
	if (y + height >= PIXEL_HEIGHT)
		height = PIXEL_HEIGHT - y;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (y*PIXEL_WIDTH + x)*3;

	for (int i = 0; i < height; i++)
	{

		uint8_t *tmp = vram;

		for (int j = 0; j < width; j++)
		{

			*(uint32_t *)vram &= 0xFF000000;

			*(uint32_t *)vram |= color&0xffffff;

			vram += 3;

		}

		vram = tmp + PIXEL_WIDTH*3;

	}
	
}

void outline_circle(int mx, int my, int rad, uint32_t color)
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

void set_pixel (int x, int y, uint32_t color)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT || x < 0 || y < 0)
		return;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (y * PIXEL_WIDTH + x)*3;
	
	*(uint32_t *)vram &= 0xFF000000;

	*(uint32_t *)vram |= color&0xffffff;

}

uint32_t get_pixel (int x, int y)
{

	if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT || x < 0 || y < 0)
		return 0;
	
	return *(uint32_t *)(VIDEO_ADDRESS + (y * PIXEL_WIDTH + x)*3);

}
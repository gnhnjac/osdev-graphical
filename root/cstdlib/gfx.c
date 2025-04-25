 #include "gfx.h"
#include "syscalls.h"
#include "stdlib.h"
#include "stdio.h"

void gfx_paint_char(PWINDOW win, char c, int x, int y, uint32_t fgcolor, void *font_buff)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height || x < 0 || y < 0)
		return;

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)*3;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{
			if(col%2)
			{

				*(uint32_t *)(dest + bit*3) &= 0xFF000000;

				*(uint32_t *)(dest + bit*3) |= fgcolor&0xffffff;

			}
				
			col /= 2;

		}

		src++;
		dest += win->width*3;
	}

}

void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint32_t bgcolor, uint32_t fgcolor, void *font_buff)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height || x < 0 || y < 0)
		return;

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)*3;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{

			uint32_t color = (col % 2) ? fgcolor : bgcolor;

			*(uint32_t *)(dest + bit*3) &= 0xFF000000;

			*(uint32_t *)(dest + bit*3) |= color&0xffffff;
				
			col /= 2;

		}

		src++;
		dest += win->width*3;
	}

}

void gfx_set_pixel(PWINDOW win,int x, int y,uint32_t color)
{

	if (x >= win->width || y >= win->height || x < 0 || y < 0 || !win)
		return;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + (y*win->width + x)*3);

	*(uint32_t *)buff &= 0xFF000000;

	*(uint32_t *)buff |= color&0xffffff;

}

void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint32_t color)
{

	if (x >= win->width || y >= win->height || (!win))
		return;

	if (x + width >= win->width)
		width = win->width - x;
	if (y + height >= win->height)
		height = win->height - y;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + (y*win->width + x)*3);

	for (int i = 0; i < height; i++)
	{

		if (y + i >= win->height || y + i < 0)
				continue;

		for (int j = 0; j < width; j++)
		{

			if (x + j >= win->width || x + j < 0)
				continue;

			*(uint32_t *)(buff + j*3) &= 0xFF000000;

			*(uint32_t *)(buff + j*3) |= color&0xffffff;

		}

		buff += win->width*3;

	}

}

void gfx_clear_win(PWINDOW win)
{

	gfx_fill_rect(win,0,0,win->width,win->height,0);

}

uint32_t vga_palette[16] = {
    0x00000000, // 0: Black
    0x000000AA, // 1: Blue
    0x0000AA00, // 2: Green
    0x0000AAAA, // 3: Cyan
    0x00AA0000, // 4: Red
    0x00AA00AA, // 5: Magenta
    0x00AA5500, // 6: Brown / Dark Yellow
    0x00AAAAAA, // 7: Light Gray
    0x00555555, // 8: Dark Gray
    0x005555FF, // 9: Bright Blue
    0x0055FF55, // 10: Bright Green
    0x0055FFFF, // 11: Bright Cyan
    0x00FF5555, // 12: Bright Red
    0x00FF55FF, // 13: Bright Magenta
    0x00FFFF55, // 14: Yellow
    0x00FFFFFF  // 15: White
};

void gfx_paint_bmp16(PWINDOW win, char *path, int x, int y)
{

	if (!win || x >= win->width || y >= win->height)
		return;

	uint32_t fd = fopen(path);

	BITMAPFILEHEADER bmp_header;

	fread(fd,(char *)&bmp_header,sizeof(BITMAPFILEHEADER));

	if (bmp_header.Signature != BMP_SIGNATURE)
	{
		printf("File is not a bmp.\n");
		fclose(fd);
		return;
	}

	BITMAPINFOHEADER bmp_info;

	fread(fd,(char *)&bmp_info,sizeof(BITMAPINFOHEADER));

	if (bmp_info.BPP != 4)
	{
		printf("File is not a 16 color bmp.\n");
		fclose(fd);
		return;
	}

	uint32_t width = bmp_info.Width;
	uint32_t height = bmp_info.Height;

	uint32_t color_buff;

	// read 16 color palette
	for (int i = 0; i < 16; i++)
		fread(fd,(char *)&color_buff,4);

	uint8_t two_pixels;
	uint8_t padding_bytes = 4-(width/2+width%2)%4;
	if (padding_bytes == 4)
		padding_bytes = 0;
	
	uint32_t img_buffer_size = width*height/2;
	img_buffer_size += padding_bytes*height + img_buffer_size%2;
	char *img_buffer = malloc(img_buffer_size);
	for (int i = 0; i < 4; i++)
		fread(fd,img_buffer+i*(img_buffer_size/4),img_buffer_size/4);
	uint32_t img_buffer_ptr = 0;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + ((height-1)*win->width)*3);
	for (int i = 0; i < height; i++)
	{
		int pix_count = 0;
		for (int j = 0; j < width; j++)
		{

			if (pix_count % 2 == 0)
			{
				two_pixels = img_buffer[img_buffer_ptr++];
				//volReadFile(&bmp,(char *)&two_pixels,1);
			}
			else
				two_pixels >>= 4;

			uint32_t color = vga_palette[two_pixels & 0xF];

			*(uint32_t *)(buff + j*3) &= 0xFF000000;

			*(uint32_t *)(buff + j*3) |= color&0xffffff;
			//gfx_set_pixel(win,j,height-i-1,color);

			pix_count++;

		}

		img_buffer_ptr += padding_bytes;
		//volReadFile(&bmp,(char *)&padding_buff,padding_bytes);

		buff -= win->width*3;

	}

	free(img_buffer);

	fclose(fd);

}
#include "gfx.h"
#include "syscalls.h"
#include "stdlib.h"
#include "stdio.h"

void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor, void *font_buff)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height)
		return;

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)/2;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{
			if(col%2)
			{
				uint8_t color_mask = fgcolor;
				uint8_t cancel_mask = 0xF0;

				if (bit % 2)
				{
					color_mask <<= 4;
					cancel_mask >>= 4;
				}

				dest[bit/2] &= cancel_mask;
				dest[bit/2] |= color_mask;
			}
				
			col /= 2;

		}

		src++;
		dest += win->width/2;
	}

}

void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor, void *font_buff)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height)
		return;

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)/2;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{
			uint8_t color_mask;
			uint8_t cancel_mask = 0xF0;

			if(col%2)
				color_mask = fgcolor;
			else
				color_mask = bgcolor;

			if (bit % 2)
			{
				color_mask <<= 4;
				cancel_mask >>= 4;
			}

			dest[bit/2] &= cancel_mask;
			dest[bit/2] |= color_mask;
				
			col /= 2;

		}

		src++;
		dest += win->width/2;
	}

}

void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color)
{

	if (x >= win->width || y >= win->height || !win)
		return;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + (y*win->width + x)/2);

	uint8_t color_mask = color;
	uint8_t cancel_mask = 0xF0;

	if (x % 2)
	{
		color_mask <<= 4;
		cancel_mask >>= 4;
	}

	*buff &= cancel_mask;
	*buff |= color_mask;

}

void gfx_set_pixel_at_linear_off(PWINDOW win,int x, int y, uint8_t *off, uint8_t color)
{

	if (x >= win->width || y >= win->height || !win)
		return;

	uint8_t *buff = off;

	uint8_t color_mask = color;
	uint8_t cancel_mask = 0xF0;

	if (x % 2)
	{
		color_mask <<= 4;
		cancel_mask >>= 4;
	}

	*buff &= cancel_mask;
	*buff |= color_mask;

}

void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color)
{

	if (x >= win->width || y >= win->height || (!win))
		return;

	if (x + width >= win->width)
		width = win->width - x;
	if (y + height >= win->height)
		height = win->height - y;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + (y*win->width + x)/2);

	for (int i = 0; i < height; i++)
	{

		for (int j = 0; j < width; j++)
		{

			uint8_t color_mask = color;
			uint8_t cancel_mask = 0xF0;

			if (j % 2)
			{
				color_mask <<= 4;
				cancel_mask >>= 4;
			}

			buff[j/2] &= cancel_mask;
			buff[j/2] |= color_mask;

		}

		buff += win->width/2;

	}

}

void gfx_clear_win(PWINDOW win)
{

	gfx_fill_rect(win,0,0,win->width,win->height,0);

}

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

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + ((height-1)*win->width)/2);
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

			uint8_t color = two_pixels & 0xF;

			gfx_set_pixel_at_linear_off(win,j,height-i-1,buff+j/2,color);
			//gfx_set_pixel(win,j,height-i-1,color);

			pix_count++;

		}

		img_buffer_ptr += padding_bytes;
		//volReadFile(&bmp,(char *)&padding_buff,padding_bytes);

		buff -= win->width/2;

	}

	free(img_buffer);

	fclose(fd);

}
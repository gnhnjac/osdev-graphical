#include "gfx.h"
#include "syscalls.h"

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
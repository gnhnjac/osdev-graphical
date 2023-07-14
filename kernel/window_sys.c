#include "window_sys.h"
#include "graphics.h"
#include "heap.h"
#include "memory.h"
#include "strings.h"
#include "screen.h"
#include <stdarg.h>

static PWINDOW win_list;
static PWINDOW working_window;

PWINDOW winsys_get_working_window()
{

	return working_window;

}

int winsys_get_free_id()
{

	int id = 1;

    PWINDOW tmp = win_list;

    while (tmp)
    {

            if (tmp->id >= id)
                id=tmp->id+1;

            tmp = tmp->next;

    }

    return id;

}

void winsys_init()
{

	win_list = 0;
	working_window = 0;

}

PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name, bool is_closable)
{

	PWINDOW win = (PWINDOW)kcalloc(sizeof(WINDOW));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->closable = is_closable;

	win->w_buffer = kcalloc(width*height/2);
	win->w_name = (char *)kcalloc(strlen(w_name)+1);
	strcpy(win->w_name,w_name);

	win->id = winsys_get_free_id();

	win->next = 0;

	working_window = win;

	if (win_list)
	{
		PWINDOW tmp = win_list;

		while(tmp->next)
			tmp = tmp->next;

		tmp->next = win;
	}
	else
		win_list = win;

	winsys_display_window(win);

	return win;

}

int winsys_set_working_window(int wid)
{

	PWINDOW tmp = win_list;

	while (tmp)
	{

		if (tmp->id == wid)
		{
			working_window = tmp;
			return 1;
		}
		tmp = tmp->next;
	}

	return 0;

}

void winsys_paint_window_frame(PWINDOW win)
{

	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,TITLE_BAR_HEIGHT,WIN_FRAME_COLOR);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y,WIN_FRAME_SIZE,win->height,WIN_FRAME_COLOR);
	fill_rect(win->x+win->width,win->y,WIN_FRAME_SIZE,win->height,WIN_FRAME_COLOR);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y+win->height,win->width+WIN_FRAME_SIZE*2,WIN_FRAME_SIZE,WIN_FRAME_COLOR);

	char *tmp = win->w_name;

	uint32_t name_y = win->y-(TITLE_BAR_HEIGHT+CHAR_HEIGHT)/2;
	uint32_t name_x = win->x;

	while (*tmp)
	{
		display_psf1_8x16_char(*tmp, name_x, name_y, TITLE_NAME_COLOR);

		name_x += CHAR_WIDTH;

		tmp++;
	}

	if (win->closable)
		display_psf1_8x16_char('x', win->x+win->width-CHAR_WIDTH, name_y, TITLE_NAME_COLOR);

}

void winsys_paint_window(PWINDOW win)
{

	if (!win)
		return;

	uint8_t *buff = (uint8_t *)win->w_buffer;

	for (int i = 0; i < win->height; i++)
	{

		for (int j = 0; j < win->width; j++)
		{

			uint8_t color = buff[j/2];

			if (j % 2)
				color >>= 4;
			else
				color &= 0xF;

			set_pixel(win->x+j,win->y+i,color);

		}

		buff += win->width/2;

	}

	winsys_paint_window_frame(win);

}

void winsys_display_window(PWINDOW win)
{

	if (!win)
		return;
	winsys_paint_window(win);
	winsys_display_window(win->next);

}

void winsys_clear_window(PWINDOW win)
{

	fill_rect(win->x,win->y,win->width,win->height,0);

}

void winsys_clear_window_frame(PWINDOW win)
{

	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,TITLE_BAR_HEIGHT,0);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y,WIN_FRAME_SIZE,win->height,0);
	fill_rect(win->x+win->width,win->y,WIN_FRAME_SIZE,win->height,0);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y+win->height,win->width+WIN_FRAME_SIZE*2,WIN_FRAME_SIZE,0);

}

void winsys_clear_whole_window(PWINDOW win)
{

	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,0);

}

bool winsys_check_collide(PWINDOW w1, PWINDOW w2)
{

	uint32_t w1_x = w1->x-WIN_FRAME_SIZE;
	uint32_t w1_y = w1->y-TITLE_BAR_HEIGHT;
	uint32_t w1_w = w1->width + WIN_FRAME_SIZE*2;
	uint32_t w1_h = w1->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	uint32_t w2_x = w2->x-WIN_FRAME_SIZE;
	uint32_t w2_y = w2->y-TITLE_BAR_HEIGHT;
	uint32_t w2_w = w2->width + WIN_FRAME_SIZE*2;
	uint32_t w2_h = w2->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	return w1_x < w2_x + w2_w &&
    w1_x + w1_w > w2_x &&
    w1_y < w2_y + w2_h &&
    w1_y + w1_h > w2_y;

}

bool winsys_check_title_collide(PWINDOW w, int x, int y)
{

	uint32_t w_x = w->x-WIN_FRAME_SIZE;
	uint32_t w_y = w->y-(TITLE_BAR_HEIGHT+CHAR_HEIGHT)/2;
	uint32_t w_w = w->width + WIN_FRAME_SIZE*2;
	uint32_t w_h = CHAR_HEIGHT;

	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

bool winsys_check_close_collide(PWINDOW w, int x, int y)
{

	if (!w->closable)
		return false;

	uint32_t w_x = w->x+w->width-CHAR_WIDTH;
	uint32_t w_y = w->y-TITLE_BAR_HEIGHT;
	uint32_t w_w = CHAR_WIDTH;
	uint32_t w_h = TITLE_BAR_HEIGHT;

	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

PWINDOW winsys_get_window_from_title_collision(int x, int y)
{

	PWINDOW tmp = win_list;

	if (!tmp)
		return 0;

	while(!winsys_check_title_collide(tmp,x,y) && tmp)
		tmp = tmp->next;

	if (winsys_check_close_collide(tmp,x,y))
	{
		winsys_remove_window(tmp);
		return 0;
	}

	return tmp;

}

void winsys_move_window(PWINDOW win, int x, int y)
{

	winsys_clear_whole_window(win);

	win->x = x;
	win->y = y;

	winsys_display_window(win_list);

}

void winsys_remove_window(PWINDOW win)
{

	winsys_clear_whole_window(win);

	PWINDOW tmp = win_list;

	if (tmp->id == win->id)
		win_list = win->next;
	else
	{
		while(tmp->next->id != win->id)
			tmp = tmp->next;

		tmp->next = win->next;
	}

	kfree(win->w_buffer);
	kfree(win->w_name);
	kfree(win);

	winsys_display_window(win_list);

}

// needs to be in a seperate graphics libraray

void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height)
		return;

	uint8_t *font_buff = get_font_buffer();

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)/2;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{
			if(col%2)
			{
				uint8_t color_mask = fgcolor;

				if (bit % 2)
					color_mask <<= 4;

				dest[bit/2] |= color_mask;
			}
				
			col /= 2;

		}

		src++;
		dest += win->width/2;
	}

}

void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor)
{

	if (x+CHAR_WIDTH > win->width || y+CHAR_HEIGHT > win->height)
		return;

	uint8_t *font_buff = get_font_buffer();

	uint8_t *src = font_buff + c * 16;
	uint8_t *dest = (uint8_t *)win->w_buffer + (y * win->width + x)/2;

	for(int row = 0; row < 16; row++) {

		uint8_t col = *src;

		for (int bit = 0; bit < 8; bit++)
		{
			uint8_t color_mask;

			if(col%2)
				color_mask = fgcolor;
			else
				color_mask = bgcolor;

			if (bit % 2)
				color_mask <<= 4;

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

	if (x % 2)
		color_mask <<= 4;

	*buff |= color_mask;

}

void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color)
{

	if (x >= win->width || y >= win->height || !win)
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

			if (j % 2)
				color_mask <<= 4;

			buff[j/2] |= color_mask;

		}

		buff += win->width/2;

	}

}

void gfx_clear_win(PWINDOW win)
{

	fill_rect(0,0,win->width,win->height,0);

}

int gfx_get_win_x(int col)
{

	return col * CHAR_WIDTH;

}

int gfx_get_win_y(int row)
{

	return row * CHAR_HEIGHT;

}

void gfx_putchar(PWINDOW win, PINPINFO inp_info, char c)
{

	gfx_print_char(win,inp_info,c,-1,-1,0xF);

}

void gfx_print(PWINDOW win, PINPINFO inp_info, char *s)
{

	gfx_print_color(win,inp_info,s,0xF);

}

void gfx_print_color(PWINDOW win, PINPINFO inp_info, char *s, uint8_t color)
{

	while(*s)
	{

		gfx_print_char(win,inp_info,*s,-1,-1,color);

		s++;

	}

}

/* Print a char on the window at col, row, or at cursor position */
void gfx_print_char(PWINDOW win, PINPINFO inp_info, const char character, int row, int col, char color) 
{

	if(character == 27 || character == '\r') // don't print escape character
		return;

	if (!color)
		color = 0xF;

	/* Get the video memory offset for the screen location */
	int offset_x = 0;
	int offset_y = 0;
	/* If col and row are non-negative, use them for offset. */
	if ( col >= 0 && row >= 0) {
		offset_x = gfx_get_win_x(col);
		offset_y = gfx_get_win_y(row);
		/* Otherwise, use the current cursor position. */
	} else {
		offset_x = inp_info->cursor_offset_x;
		offset_y = inp_info->cursor_offset_y;
	}

	if (character == '\b')
	{

		// handle backspace
		offset_x-=1;
		if (offset_x < 0)
		{
			offset_y -= 1;
			offset_x = MAX_COLS-1;
		}
		if (offset_y < TOP) // can't be lower than start of screen.
		{

			offset_x = 0;
			offset_y = TOP;

		}
		gfx_fill_rect(win,gfx_get_win_x(offset_x),gfx_get_win_y(offset_y),CHAR_WIDTH,CHAR_HEIGHT,0);
		inp_info->cursor_offset_x = offset_x;
		inp_info->cursor_offset_y = offset_y;
		return;
	}
	else if (character == '\t')
	{

		// handle tab
		gfx_putchar(win,inp_info,' ');
		gfx_putchar(win,inp_info,' ');
		gfx_putchar(win,inp_info,' ');
		gfx_putchar(win,inp_info,' ');
		return;

	}

	// Make scrolling adjustment, for when we reach the bottom
	// of the screen.
	//offset_y = handle_scrolling(offset_y);

	// If we see a newline character, set offset to the end of
	// current row, so it will be advanced to the first col
	// of the next row.
	if (character == '\n') {
		offset_x = 0;
		offset_y += 1;
	} else {
		gfx_fill_rect(win,gfx_get_win_x(offset_x),gfx_get_win_y(offset_y),CHAR_WIDTH,CHAR_HEIGHT,0);
		gfx_paint_char(win,character,gfx_get_win_x(offset_x),gfx_get_win_y(offset_y),color);
		offset_x += 1;
	}

	//if it reached the scroll bar set it to the end of the line so the next character will be in the new line.
	if (offset_x >= MAX_COLS)
	{
		offset_x = 0;
		offset_y += 1;
	}
	// Update the cursor position on the window device.
	inp_info->cursor_offset_x = offset_x;
	inp_info->cursor_offset_y = offset_y;
}

void gfx_printf(PWINDOW win, PINPINFO inp_info,const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);

	const char *orig = fmt;

	while (*fmt)
	{

		if (*fmt == '%' && ((*(fmt-1) != '\\' && fmt != orig) || fmt == orig))
		{

			char buff[30];

			switch(*++fmt)
			{

				case 'd':

					int_to_str(va_arg(valist, int), buff, 10);
					gfx_print(win,inp_info,buff);
					break;

				case 'u':

					uint_to_str(va_arg(valist, int), buff, 10);
					gfx_print(win,inp_info,buff);
					break;

				case 'U':

					uint_to_str(va_arg(valist, int), buff, 16);
					gfx_print(win,inp_info,buff);
					break;

				case 'c':

					gfx_putchar(win,inp_info,(char)va_arg(valist, int));
					break;

				case 's':
					gfx_printf(win,inp_info,(char *)va_arg(valist, int));
					break;

				case 'x':
					int_to_str(va_arg(valist, int), buff, 16);
					gfx_print(win,inp_info,buff);
					break;

				case 'X':

					byte_to_str((uint8_t)va_arg(valist, int), buff, 16);
					gfx_print(win,inp_info,buff);
					break;

				case 'b':
					int_to_str(va_arg(valist, int), buff, 2);
					gfx_print(win,inp_info,buff);
					break;



				default:

					gfx_printf(win,inp_info,"Unknown format type \\%%c", fmt);
					return;
			}

		}
		else if(*fmt == '\\')
		{	
			fmt++;
			continue;
		}
		else
		{

			gfx_putchar(win,inp_info,*fmt);

		}

		fmt++;

	}


}

// int handle_scrolling(int offset_y)
// {

// 	if (offset_y < MAX_ROWS)
// 	{
// 		return offset_y;
// 	}

// 	disable_mouse();


// 	uint32_t char_line_size = PIXEL_WIDTH*CHAR_HEIGHT/PIXELS_PER_BYTE;
// 	uint32_t scroll_diff_size = SCROLL_ROWS*char_line_size;
// 	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + char_line_size;

// 	uint8_t prev_mask = 0;
	
// 	outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
// 	outb(0x3C5, 0xF);

// 	for (int i = TOP+SCROLL_ROWS; i < MAX_ROWS; i++)
// 	{

// 		uint8_t *tmp = vram;

// 		for (int j = 0; j < CHAR_HEIGHT; j++)
// 		{

// 			for (int k = 0; k < PIXEL_WIDTH; k++)
// 			{

// 				uint8_t mask = 0x80>>(k%PIXELS_PER_BYTE);
// 				uint32_t off = k/PIXELS_PER_BYTE;
// 				uint8_t *color_byte = (uint8_t *)(tmp+off+scroll_diff_size);
				
// 				if (k%PIXELS_PER_BYTE == 0)
// 				{
// 					outb(0x3CE, READ_MAP_SELECT);
// 					outb(0x3CF, 0);
// 					if (*color_byte)
// 						goto not_totally_black;
// 					outb(0x3CF, 1);
// 					if (*color_byte)
// 						goto not_totally_black;
// 					outb(0x3CF, 2);
// 					if (*color_byte)
// 						goto not_totally_black;
// 					outb(0x3CF, 3);
// 					if (*color_byte)
// 						goto not_totally_black;

// 					k += PIXELS_PER_BYTE-1;

// 					if (prev_mask != 0xF)
// 					{
// 						outb(0x3C5, 0xF);
// 						prev_mask = 0xF;
// 					}
// 					outb(0x3CE,0x8);
// 					outb(0x3CF,0xFF);
// 					tmp[off] = 0;

// 					continue;
					
// 				}

// 				not_totally_black:

// 				if (prev_mask != 0xF)
// 				{
// 					outb(0x3C5, 0xF);
// 					prev_mask = 0xF;
// 				}
// 				outb(0x3CE,0x8);
// 				outb(0x3CF,mask);

// 				tmp[off] &= ~mask;

// 				uint8_t color = 0;

// 				outb(0x3CE, READ_MAP_SELECT);
// 				outb(0x3CF, 0);
// 				if (*color_byte & mask)
// 					color |= 1;
// 				outb(0x3CF, 1);
// 				if (*color_byte & mask)
// 					color |= 2;
// 				outb(0x3CF, 2);
// 				if (*color_byte & mask)
// 					color |= 4;
// 				outb(0x3CF, 3);
// 				if (*color_byte & mask)
// 					color |= 8;

// 				if (prev_mask != color)
// 				{
// 					outb(0x3C5, color);
// 					prev_mask = color;
// 				}

// 				tmp[off] |= mask;

// 				//set_pixel(j,get_screen_y(i-1)+k,get_pixel(j,get_screen_y(i)+k));

// 			}

// 			tmp += PIXEL_WIDTH / PIXELS_PER_BYTE;

// 		}

// 		vram += char_line_size;

// 	}

// 	fill_rect(0,PIXEL_HEIGHT-CHAR_HEIGHT*SCROLL_ROWS,PIXEL_WIDTH,CHAR_HEIGHT*SCROLL_ROWS,0);

// 	offset_y-=SCROLL_ROWS;

// 	enable_mouse();
// 	cursor_input_row -= SCROLL_ROWS;

// 	return offset_y;

// }
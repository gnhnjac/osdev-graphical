#include "window_sys.h"
#include "graphics.h"
#include "heap.h"
#include "memory.h"
#include "strings.h"
#include "screen.h"
#include "low_level.h"
#include "math.h"
#include "vfs.h"
#include "bmp.h"
#include "process.h"

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

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE; i++)
	{

		win->event_handler.events[i].event_type = EVENT_INVALID;

	}

	// win->event_handler.event_thread = *get_current_task();

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

			PWINDOW tmp2 = win_list;

			if (tmp2->next == 0)
				return 1;

			if (tmp2->id == wid)
			{

				tmp2 = tmp2->next;
				win_list = tmp2;

			}
			else
			{
				while (tmp2->next->id != wid)
					tmp2 = tmp2->next;

				tmp2->next = tmp->next;
			}
			while(tmp2->next)
				tmp2 = tmp2->next;

			tmp2->next = tmp;
			tmp->next = 0;

			winsys_display_window(working_window);

			return 1;
		}
		tmp = tmp->next;
	}

	return 0;

}

void winsys_paint_window_frame(PWINDOW win)
{

	if (!win)
		return;

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
	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + win->y * PIXEL_WIDTH / PIXELS_PER_BYTE;

	for (int i = 0; i < win->height; i++)
	{

		for (int j = 0; j < win->width; j++)
		{

			uint32_t x = win->x+j;
			uint32_t y = win->y+i;

			if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
				continue;

			uint8_t color = buff[j/2];

			if (j % 2)
				color >>= 4;
			else
				color &= 0xF;

			uint8_t mask = (0x80>>(x%PIXELS_PER_BYTE));

			outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
			outb(0x3CE,0x8);
			outb(0x3CF,mask);

			outb(0x3C5, 0xF);

			vram[x/PIXELS_PER_BYTE] &= ~mask;

			outb(0x3C5, color);

			vram[x/PIXELS_PER_BYTE] |= mask;

		}

		buff += win->width/2;
		vram += PIXEL_WIDTH/PIXELS_PER_BYTE;

	}

	winsys_paint_window_frame(win);

}

void winsys_paint_window_section(PWINDOW win, int x, int y, int width, int height)
{


	uint32_t min_height = min(win->height,height);
	uint32_t min_width = min(win->width,width);

	int max_x = max(x,0);
	int max_y = max(y,0);

	uint8_t *buff = (uint8_t *)win->w_buffer + max_y*win->width/2;
	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (win->y+max_y) * PIXEL_WIDTH / PIXELS_PER_BYTE;

	for (int i = max_y; i < max_y+min_height; i++)
	{

		for (int j = max_x; j < max_x+min_width; j++)
		{

			uint32_t x = win->x+j;
			uint32_t y = win->y+i;

			if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
				continue;

			uint8_t color = buff[j/2];

			if (j % 2)
				color >>= 4;
			else
				color &= 0xF;

			uint8_t mask = (0x80>>(x%PIXELS_PER_BYTE));

			outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
			outb(0x3CE,0x8);
			outb(0x3CF,mask);

			outb(0x3C5, 0xF);

			vram[x/PIXELS_PER_BYTE] &= ~mask;

			outb(0x3C5, color);

			vram[x/PIXELS_PER_BYTE] |= mask;

		}

		buff += win->width/2;
		vram += PIXEL_WIDTH/PIXELS_PER_BYTE;

	}

	winsys_paint_window_frame(win);

}

void winsys_display_window_section(PWINDOW win, int x, int y, int width, int height)
{

	if (!win)
		return;
	winsys_paint_window_section(win, x, y, width, height);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (winsys_check_collide(win,tmp))
		{
			uint32_t overlap_x = max(win->x, tmp->x);

			uint32_t overlap_y = max(win->y, tmp->y);

			uint32_t overlap_w = min(win->x+win->width,tmp->x+tmp->width)-overlap_x;

			uint32_t overlap_h = min(win->y+win->height, tmp->y+tmp->height)-overlap_y;

			winsys_display_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);

		}

		tmp = tmp->next;

	}

}

void winsys_display_window_section_exclude_original(PWINDOW win, PWINDOW orig, int x, int y, int width, int height)
{

	if (!win)
		return;
	winsys_paint_window_section(win, x, y, width, height);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (winsys_check_collide(win,tmp) && tmp != orig)
		{
			uint32_t overlap_x = max(win->x, tmp->x);

			uint32_t overlap_y = max(win->y, tmp->y);

			uint32_t overlap_w = min(win->x+win->width,tmp->x+tmp->width)-overlap_x;

			uint32_t overlap_h = min(win->y+win->height, tmp->y+tmp->height)-overlap_y;

			winsys_display_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);

		}

		tmp = tmp->next;

	}

}

void winsys_display_window(PWINDOW win)
{

	if (!win)
		return;
	winsys_paint_window(win);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (winsys_check_collide(win,tmp))
		{
			uint32_t overlap_x = max(win->x, tmp->x);

			uint32_t overlap_y = max(win->y, tmp->y);

			uint32_t overlap_w = min(win->x+win->width,tmp->x+tmp->width)-overlap_x;

			uint32_t overlap_h = min(win->y+win->height, tmp->y+tmp->height)-overlap_y;

			winsys_display_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);

		}

		tmp = tmp->next;

	}

}

void winsys_display_window_exclude_original(PWINDOW win, PWINDOW orig)
{

	if (!win)
		return;
	winsys_paint_window(win);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (tmp != orig && winsys_check_collide(win,tmp))
		{
			uint32_t overlap_x = max(win->x, tmp->x);

			uint32_t overlap_y = max(win->y, tmp->y);

			uint32_t overlap_w = min(win->x+win->width,tmp->x+tmp->width)-overlap_x;

			uint32_t overlap_h = min(win->y+win->height, tmp->y+tmp->height)-overlap_y;

			winsys_display_window_section_exclude_original(tmp,orig,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);
		}

		tmp = tmp->next;

	}

}

void winsys_display_collided_windows(PWINDOW win)
{

	if (!win)
		return;


	PWINDOW tmp = win_list;

	while(tmp)
	{	
		if (tmp != win && winsys_check_collide(win,tmp))
			winsys_display_window_exclude_original(tmp,win);

		tmp = tmp->next;

	}

}


void winsys_clear_window(PWINDOW win)
{

	fill_rect(win->x,win->y,win->width,win->height,BG_COLOR);

}

void winsys_clear_window_frame(PWINDOW win)
{

	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,TITLE_BAR_HEIGHT,BG_COLOR);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y,WIN_FRAME_SIZE,win->height,BG_COLOR);
	fill_rect(win->x+win->width,win->y,WIN_FRAME_SIZE,win->height,BG_COLOR);
	fill_rect(win->x-WIN_FRAME_SIZE,win->y+win->height,win->width+WIN_FRAME_SIZE*2,WIN_FRAME_SIZE,BG_COLOR);

}

void winsys_clear_whole_window(PWINDOW win)
{

	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,BG_COLOR);

}

bool winsys_check_collide(PWINDOW w1, PWINDOW w2)
{

	if (!w1 || !w2)
		return false;

	int w1_x = w1->x-WIN_FRAME_SIZE;
	int w1_y = w1->y-TITLE_BAR_HEIGHT;
	int w1_w = w1->width + WIN_FRAME_SIZE*2;
	int w1_h = w1->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	int w2_x = w2->x-WIN_FRAME_SIZE;
	int w2_y = w2->y-TITLE_BAR_HEIGHT;
	int w2_w = w2->width + WIN_FRAME_SIZE*2;
	int w2_h = w2->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	return w2_x + w2_w > w1_x &&
		   w2_y + w2_h > w1_y &&
		   w1_x + w1_w > w2_x &&
		   w1_y + w1_h > w2_y;

}

bool winsys_check_collide_coords(PWINDOW w, int x, int y)
{

	int w_x = w->x-WIN_FRAME_SIZE;
	int w_y = w->y-TITLE_BAR_HEIGHT;
	uint32_t w_w = w->width + WIN_FRAME_SIZE*2;
	uint32_t w_h = w->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

bool winsys_check_title_collide(PWINDOW w, int x, int y)
{

	if (!w)
		return false;

	int w_x = w->x-WIN_FRAME_SIZE;
	int w_y = w->y-TITLE_BAR_HEIGHT;
	uint32_t w_w = w->width + WIN_FRAME_SIZE*2;
	uint32_t w_h = TITLE_BAR_HEIGHT;
	
	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

bool winsys_check_close_collide(PWINDOW w, int x, int y)
{

	if (!w->closable)
		return false;

	int w_x = w->x+w->width-CHAR_WIDTH;
	int w_y = w->y-(TITLE_BAR_HEIGHT+CHAR_HEIGHT)/2;
	uint32_t w_w = CHAR_WIDTH;
	uint32_t w_h = CHAR_HEIGHT;

	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

PWINDOW winsys_get_window_from_collision(int x, int y)
{

	PWINDOW tmp = win_list;

	if (!tmp)
		return 0;

	while(!winsys_check_collide_coords(tmp,x,y) && tmp)
		tmp = tmp->next;

	return tmp;

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
	winsys_display_collided_windows(win);

	win->x = x;
	win->y = y;
	winsys_set_working_window(win->id);

	winsys_display_window(win);

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

	tmp = win_list;
	
	while(tmp->next)
		tmp = tmp->next;

	if (working_window->id == win->id)
	{
		working_window = 0;
		winsys_set_working_window(tmp->id);
	}

	winsys_display_collided_windows(win);

	kfree(win->w_buffer);
	kfree(win->w_name);
	kfree(win);

}

void winsys_enqueue_to_event_handler(PEVENTHAND handler, EVENT e)
{

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE; i++)
	{

		if (handler->events[i].event_type & EVENT_INVALID)
		{
			handler->events[i] = e;
			return;
		}
	}

}

EVENT winsys_dequeue_from_event_handler(PEVENTHAND handler)
{
	EVENT e = handler->events[0];

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE-1; i++)
	{

		handler->events[i] = handler->events[i+1];

	}

	handler->events[EVENT_HANDLER_QUEUE_SIZE-1].event_type = EVENT_INVALID;

	return e;

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

int gfx_get_logical_row(int y)
{

	return y/CHAR_HEIGHT;

}

int gfx_get_logical_col(int x)
{

	return x/CHAR_WIDTH;

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
			offset_x = gfx_get_logical_col(win->width)-1;
		}
		if (offset_y < 0) // can't be lower than start of screen.
		{

			offset_x = 0;
			offset_y = 0;

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
	offset_y = gfx_handle_scrolling(win, inp_info, offset_y);

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
	if (offset_x >= gfx_get_logical_col(win->width))
	{
		offset_x = 0;
		offset_y += 1;
	}
	// Update the cursor position on the window device.
	inp_info->cursor_offset_x = offset_x;
	inp_info->cursor_offset_y = offset_y;
}

void gfx_vprintf(PWINDOW win, PINPINFO inp_info,const char *fmt, va_list valist)
{

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
					gfx_print(win,inp_info,(char *)va_arg(valist, int));
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
		else if(*fmt == '\\' && *(fmt+1) == '%')
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

void gfx_printf(PWINDOW win, PINPINFO inp_info,const char *fmt, ...)
{

	va_list valist;
	va_start(valist,fmt);

	gfx_vprintf(win,inp_info,fmt,valist);

}

void gfx_keyboard_input(PINPINFO inp_info, int col, int row, char *buffer, int bf_size)
{

  if(row < 0)
    row = inp_info->cursor_offset_y;
  if(col < 0)
    col = inp_info->cursor_offset_x;

  inp_info->cursor_input_row = row;
  inp_info->cursor_input_col = col;

  inp_info->input_buffer_index = 0;

  inp_info->input_buffer_limit = bf_size;
  inp_info->input_buffer = buffer;

  inp_info->is_taking_input = true;


}

int gfx_keyboard_input_character(PINPINFO inp_info, char character)
{
  inp_info->cursor_offset_x = inp_info->cursor_input_col;
  inp_info->cursor_offset_y = inp_info->cursor_input_row;

  if (character == '\b') // handle backspace
  {

    if (inp_info->input_buffer_index == 0)
      return -1;

    inp_info->input_buffer_index--;
    return 0;

  }

  else if (character == '\n') // handle newline (submit string)
  {
    inp_info->input_buffer[inp_info->input_buffer_index] = 0; // end of string
    inp_info->is_taking_input = false;
    return 1;
  }

  if (inp_info->input_buffer_index == inp_info->input_buffer_limit-1) // only if character is \n let it pass and finish taking the input
    return -1;

  inp_info->input_buffer[inp_info->input_buffer_index] = character;
  inp_info->input_buffer_index++;

  return 0;

}

int gfx_handle_scrolling(PWINDOW win, PINPINFO inp_info, int offset_y)
{
	if (offset_y < gfx_get_logical_row(win->height))
	{
		return offset_y;
	}

	uint32_t char_line_size = win->width*CHAR_HEIGHT/2;
	uint32_t scroll_diff_size = SCROLL_ROWS*char_line_size;
	uint8_t *buff = (uint8_t *)win->w_buffer;

	for (int i = SCROLL_ROWS; i < gfx_get_logical_row(win->height); i++)
	{

		memcpy(buff,buff+scroll_diff_size,char_line_size);

		buff += char_line_size;

	}

	gfx_fill_rect(win,0,win->height-CHAR_HEIGHT*SCROLL_ROWS,win->width,CHAR_HEIGHT*SCROLL_ROWS,0);

	offset_y-=SCROLL_ROWS;

	inp_info->cursor_input_row -= SCROLL_ROWS;

	inp_info->did_scroll = true;

	return offset_y;

}

void gfx_open_bmp16(char *path, int wx, int wy)
{

	FILE bmp = volOpenFile(path);

	BITMAPFILEHEADER bmp_header;

	volReadFile(&bmp,(char *)&bmp_header,sizeof(BITMAPFILEHEADER));

	if (bmp_header.Signature != BMP_SIGNATURE)
	{
		printf("File is not a bmp.\n");
		volCloseFile(&bmp);
		return;
	}

	BITMAPINFOHEADER bmp_info;

	volReadFile(&bmp,(char *)&bmp_info,sizeof(BITMAPINFOHEADER));

	if (bmp_info.BPP != 4)
	{
		printf("File is not a 16 color bmp.\n");
		volCloseFile(&bmp);
		return;
	}

	uint32_t width = bmp_info.Width;
	uint32_t height = bmp_info.Height;

	PWINDOW win = winsys_create_win(wx,wy,width+width%2,height,bmp.name,true);

	uint32_t color_buff;

	// read 16 color palette
	for (int i = 0; i < 16; i++)
		volReadFile(&bmp,(char *)&color_buff,4);

	uint8_t two_pixels;
	uint8_t padding_bytes = 4-(width/2+width%2)%4;
	uint32_t padding_buff;

	for (int i = 0; i < height; i++)
	{
		int pix_count = 0;
		for (int j = 0; j < width; j++)
		{

			if (pix_count % 2 == 0)
				volReadFile(&bmp,(char *)&two_pixels,1);
			else
				two_pixels >>= 4;

			uint8_t color = two_pixels & 0xF;

			gfx_set_pixel(win,j,height-i-1,color);

			pix_count++;

		}

		if (padding_bytes != 4)
		{
			volReadFile(&bmp,(char *)&padding_buff,padding_bytes);
		}

	}
	winsys_display_window(win);
	volCloseFile(&bmp);

}

void gfx_paint_bmp16(PWINDOW win, char *path, int x, int y)
{

	if (x >= win->width || y >= win->height || !win)
		return;

	FILE bmp = volOpenFile(path);

	BITMAPFILEHEADER bmp_header;

	volReadFile(&bmp,(char *)&bmp_header,sizeof(BITMAPFILEHEADER));

	if (bmp_header.Signature != BMP_SIGNATURE)
	{
		printf("File is not a bmp.\n");
		volCloseFile(&bmp);
		return;
	}

	BITMAPINFOHEADER bmp_info;

	volReadFile(&bmp,(char *)&bmp_info,sizeof(BITMAPINFOHEADER));

	if (bmp_info.BPP != 4)
	{
		printf("File is not a 16 color bmp.\n");
		volCloseFile(&bmp);
		return;
	}

	uint32_t width = bmp_info.Width;
	uint32_t height = bmp_info.Height;

	uint32_t color_buff;

	// read 16 color palette
	for (int i = 0; i < 16; i++)
		volReadFile(&bmp,(char *)&color_buff,4);

	uint8_t two_pixels;
	uint8_t padding_bytes = 4-(width/2+width%2)%4;
	uint32_t padding_buff;

	for (int i = 0; i < height; i++)
	{
		int pix_count = 0;
		for (int j = 0; j < width; j++)
		{

			if (pix_count % 2 == 0)
				volReadFile(&bmp,(char *)&two_pixels,1);
			else
				two_pixels >>= 4;

			uint8_t color = two_pixels & 0xF;

			gfx_set_pixel(win,x+j,y+height-i-1,color);

			pix_count++;

		}

		if (padding_bytes != 4)
		{
			volReadFile(&bmp,(char *)&padding_buff,padding_bytes);
		}

	}

	volCloseFile(&bmp);

}
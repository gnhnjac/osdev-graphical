#include "window_sys.h"
#include "graphics.h"
#include "heap.h"
#include "memory.h"
#include "strings.h"
#include "screen.h"

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

PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name)
{

	PWINDOW win = (PWINDOW)kcalloc(sizeof(WINDOW));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;

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
	uint32_t w_y = w->y-TITLE_BAR_HEIGHT;
	uint32_t w_w = w->width + WIN_FRAME_SIZE*2;
	uint32_t w_h = TITLE_BAR_HEIGHT;

	return x < w_x + w_w &&
    x > w_x &&
    y < w_y + w_h &&
    y > w_y;

}

bool winsys_check_close_collide(PWINDOW w, int x, int y)
{

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

void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color)
{

	if (x >= win->width || y >= win->height)
		return;

	uint8_t *buff = (uint8_t *)((uint32_t)win->w_buffer + (y*win->width + x)/2);

	uint8_t color_mask = color;

	if (x % 2)
		color_mask <<= 4;

	*buff |= color_mask;

}

void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color)
{

	if (x >= win->width || y >= win->height)
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
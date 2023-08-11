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
#include "vmm.h"
#include "scheduler.h"
#include "mouse.h"
#include "lock.h"

// ** NEED TO MAKE IT IT'S OWN PROCESS WITH A DISPLAY STACK THEN THERE WILL BE NO DRAWING ARTIFACTS

static PWINDOW win_list = 0;
static PWINDOW working_window = 0;
static WINSYSOP winsys_queue[WINSYS_QUEUE_SIZE] = {0};
static int winsys_queue_index = 0;
static lock_t winsys_queue_last_lock = ATOMIC_LOCK_INIT;
static lock_t winsys_queue_lock = ATOMIC_LOCK_INIT;
static lock_t winsys_win_list_lock = ATOMIC_LOCK_INIT;

static int winsys_mousex = 0;
static int winsys_mousey = 0;

static bool winsys_initialized = false;

// 18x10 cursor bitmap
static uint8_t mouse_bitmap[18][10] = {
{ 1,0,0,0,0,0,0,0,0,0, },
{ 1,1,0,0,0,0,0,0,0,0, },
{ 1,2,1,0,0,0,0,0,0,0, },
{ 1,2,2,1,0,0,0,0,0,0, },
{ 1,2,2,2,1,0,0,0,0,0, },
{ 1,2,2,2,2,1,0,0,0,0, },
{ 1,2,2,2,2,2,1,0,0,0, },
{ 1,2,2,2,2,2,2,1,0,0, },
{ 1,2,2,2,2,2,2,2,1,0, },
{ 1,2,2,2,2,2,1,1,1,1, },
{ 1,2,2,1,2,2,1,0,0,0, },
{ 1,2,1,1,2,2,1,0,0,0, },
{ 1,1,0,0,1,2,1,0,0,0, },
{ 1,0,0,0,0,1,2,1,0,0, },
{ 0,0,0,0,0,1,2,1,0,0, },
{ 0,0,0,0,0,0,1,2,1,0, },
{ 0,0,0,0,0,0,1,2,1,0, },
{ 0,0,0,0,0,0,0,1,0,0, }
};

static uint8_t mouse_placeholder_buffer[18][10];

static int winsys_pid = 0;

void winsys_disable_mouse()
{	

	disable_mouse();

	winsys_clear_mouse();

}

void winsys_save_to_mouse_buffer()
{
	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j])
				mouse_placeholder_buffer[i][j] = get_pixel(winsys_mousex+j,winsys_mousey+i);

		}

	}

}

void winsys_print_mouse()
{

	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j] == 1)
				set_pixel(winsys_mousex+j,winsys_mousey+i,0xf);
			else if(mouse_bitmap[i][j] == 2)
				set_pixel(winsys_mousex+j,winsys_mousey+i,0);

		}

	}

}

void winsys_clear_mouse()
{

	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j])
				set_pixel(winsys_mousex+j,winsys_mousey+i,mouse_placeholder_buffer[i][j]);

		}

	}

}

void winsys_enable_mouse()
{
	winsys_save_to_mouse_buffer();
	winsys_print_mouse();
	enable_mouse();
}

void winsys_move_mouse_operation(int x, int y)
{

	winsys_clear_mouse();

	winsys_mousex = x;
	winsys_mousey = y;

	winsys_save_to_mouse_buffer();
	winsys_print_mouse();

}

void winsys_do_operation()
{

	WINSYSOP op_handle = winsys_dequeue_from_winsys_listener();

	if (op_handle.op == WINSYS_MOVE_MOUSE)
	{
	
		winsys_move_mouse_operation(op_handle.x,op_handle.y);

	}
	else if (op_handle.op == WINSYS_DISPLAY_WINDOW)
	{

		winsys_display_window_operation(winsys_get_window_by_id(op_handle.wid));

	}
	else if (op_handle.op == WINSYS_DISPLAY_WINDOW_SECTION)
	{
		winsys_display_window_section_operation(winsys_get_window_by_id(op_handle.wid),op_handle.x,op_handle.y,op_handle.w,op_handle.h);

	}
	else if (op_handle.op == WINSYS_MOVE_WINDOW)
	{

		winsys_move_window_operation(winsys_get_window_by_id(op_handle.wid), op_handle.x, op_handle.y);

	}
	else if (op_handle.op == WINSYS_SET_WORKING_WINDOW)
	{

		winsys_set_working_window_operation(op_handle.wid);

	}
	else if (op_handle.op == WINSYS_REMOVE_WINDOW)
	{

		winsys_remove_window_operation(winsys_get_window_by_id(op_handle.wid));

	}

}

void winsys_listener()
{

	winsys_pid = get_running_process()->id;

	while(1)
	{


		while(winsys_queue_index == 0)
			__asm__("pause");

		winsys_do_operation();

	}

}

static bool mid_operation = false;

WINSYSOP winsys_dequeue_from_winsys_listener()
{
	mid_operation = true;
	acquireLock(&winsys_queue_lock);

	WINSYSOP op = winsys_queue[0];

	for (int i = 0; i < WINSYS_QUEUE_SIZE-1; i++)
	{

		winsys_queue[i] = winsys_queue[i+1];

	}

	//winsys_queue[WINSYS_QUEUE_SIZE-1].op = WINSYS_EMPTY;

	winsys_queue_index--;

	releaseLock(&winsys_queue_lock);

	releaseLock(&winsys_queue_last_lock);
	mid_operation = false;

	return op;

}

void winsys_enqueue_to_winsys_listener(WINSYSOP operation)
{

	if (!winsys_initialized)
		return;

	process *running_proc = get_running_process();

	if (mid_operation && running_proc && running_proc->id == winsys_pid)
		releaseLock(&winsys_queue_lock);

	mid_operation = true;
	acquireLock(&winsys_queue_lock);

	if (winsys_queue_index >= WINSYS_QUEUE_SIZE-1)
	{
		releaseLock(&winsys_queue_lock);

		if (winsys_queue_index == WINSYS_QUEUE_SIZE && running_proc->id == winsys_pid)
			winsys_do_operation();

		acquireLock(&winsys_queue_last_lock);
		acquireLock(&winsys_queue_lock);
	}

	winsys_queue[winsys_queue_index] = operation;
	winsys_queue_index++;
	releaseLock(&winsys_queue_lock);
	mid_operation = false;

}

void winsys_enqueue_to_winsys_listener_if_possible(WINSYSOP operation)
{

	if (!winsys_initialized)
		return;

	process *running_proc = get_running_process();

	if (mid_operation && running_proc && running_proc->id == winsys_pid)
		releaseLock(&winsys_queue_lock);

	mid_operation = true;
	acquireLock(&winsys_queue_lock);

	if (winsys_queue_index < WINSYS_QUEUE_SIZE)
	{
		winsys_queue[winsys_queue_index] = operation;
		winsys_queue_index++;
	}

	releaseLock(&winsys_queue_lock);
	mid_operation = false;

}

void winsys_enqueue_to_winsys_listener_if_possible_lockless(WINSYSOP operation)
{

	if (!winsys_initialized || winsys_queue_lock)
		return;

	if (winsys_queue_index < WINSYS_QUEUE_SIZE-1)
	{
		winsys_queue[winsys_queue_index] = operation;
		winsys_queue_index++;
	}

}

void winsys_move_mouse(int new_x, int new_y)
{

	WINSYSOP mouse_move_operation = {

		.op = WINSYS_MOVE_MOUSE,
		.x = new_x,
		.y = new_y

	};

	winsys_enqueue_to_winsys_listener_if_possible(mouse_move_operation);

}

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

PWINDOW winsys_get_window_by_id(int wid)
{

	PWINDOW tmp = win_list;

    while (tmp)
    {

            if (tmp->id == wid)
                return tmp;

            tmp = tmp->next;

    }

    return 0;

}

void winsys_init()
{

	win_list = 0;
	working_window = 0;

	for (int i = 0; i < WINSYS_QUEUE_SIZE; i++)
	{

		winsys_queue[i].op = WINSYS_EMPTY;

	}

	winsys_initialized = true;

}

uint32_t get_win_page_amt(PWINDOW win)
{

	if (!win)
		return 0;

	uint32_t framebuffer_size_in_bytes = win->width*win->height/2;
	uint32_t framebuffer_page_amt = framebuffer_size_in_bytes/PAGE_SIZE;

    if (framebuffer_size_in_bytes % PAGE_SIZE != 0)
        framebuffer_page_amt += 1;

    return framebuffer_page_amt;

}

PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name, bool is_closable)
{

	PWINDOW win = (PWINDOW)kcalloc(sizeof(WINDOW));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->closable = is_closable;
	if (get_running_process())
		win->parent_pid = get_running_process()->id;
	win->is_user = false;
	win->has_frame = true;

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE; i++)
	{

		win->event_handler.events[i].event_type = EVENT_INVALID;

	}

	win->w_buffer = kcalloc(width*height/2);
	win->w_name = (char *)kcalloc(strlen(w_name)+1);
	strcpy(win->w_name,w_name);

	win->id = winsys_get_free_id();

	win->next = 0;

	acquireLock(&winsys_win_list_lock);

	if (win_list)
	{
		PWINDOW tmp = win_list;

		while(tmp->next)
			tmp = tmp->next;

		tmp->next = win;
	}
	else
		win_list = win;

	releaseLock(&winsys_win_list_lock);

	winsys_set_working_window(win->id);

	return win;

}

void winsys_create_win_user(PWINDOW local_win, int x, int y, int width, int height)
{

	process *parent_proc = get_running_process();

	PWINDOW win = (PWINDOW)kcalloc(sizeof(WINDOW));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->closable = true;
	win->parent_pid = parent_proc->id;
	win->is_user = true;
	win->has_frame = true;
	win->event_handler.event_mask = local_win->event_handler.event_mask;

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE; i++)
	{

		win->event_handler.events[i].event_type = EVENT_INVALID;

	}


	win->w_buffer = allocate_user_space_pages(get_win_page_amt(win));
	memset(win->w_buffer,0,width*height/2);

	win->id = winsys_get_free_id();

	win->next = 0;

	acquireLock(&winsys_win_list_lock);

	if (win_list)
	{
		PWINDOW tmp = win_list;

		while(tmp->next)
			tmp = tmp->next;

		tmp->next = win;
	}
	else
		win_list = win;

	releaseLock(&winsys_win_list_lock);

	win->w_name = (char *)kcalloc(strlen(local_win->w_name)+1);
	strcpy(win->w_name,local_win->w_name);

	winsys_set_working_window(win->id);

	memcpy((char *)local_win,(char *)win,sizeof(WINDOW));
}

void winsys_set_working_window(int wid)
{

	WINSYSOP set_working_window_operation = {
		.op = WINSYS_SET_WORKING_WINDOW,
		.wid = wid
	};

	winsys_enqueue_to_winsys_listener(set_working_window_operation);

}

int winsys_set_working_window_operation(int wid)
{

	acquireLock(&winsys_win_list_lock);

	PWINDOW tmp = win_list;

	while (tmp)
	{

		if (tmp->id == wid)
		{

			PWINDOW old_working = working_window;

			working_window = tmp;

			if (working_window == old_working)
			{
				releaseLock(&winsys_win_list_lock);
				return 1;
			}

			PWINDOW tmp2 = win_list;

			if (tmp2->next == 0)
			{
				releaseLock(&winsys_win_list_lock);
				return 1;
			}

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

			if (old_working && winsys_get_window_by_id(old_working->id))
				winsys_display_window_operation(old_working);

			winsys_display_window_operation(working_window);

			releaseLock(&winsys_win_list_lock);

			return 1;
		}

		tmp = tmp->next;
	}

	releaseLock(&winsys_win_list_lock);

	return 0;

}

void winsys_paint_window_frame(PWINDOW win)
{

	if (!win)
		return;

	if (win == working_window)
	{
		fill_rect(win->x,win->y-TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,win->width,TITLE_BAR_HEIGHT-WIN_FRAME_SIZE,WORKING_TITLE_COLOR);
		fill_rect(win->x,win->y-WIN_FRAME_SIZE,win->width,WIN_FRAME_SIZE,WIN_FRAME_COLOR);
		fill_rect(win->x,win->y-TITLE_BAR_HEIGHT,win->width,WIN_FRAME_SIZE,WIN_FRAME_COLOR);
		fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,WIN_FRAME_SIZE,win->height+TITLE_BAR_HEIGHT,WIN_FRAME_COLOR);
		fill_rect(win->x+win->width,win->y-TITLE_BAR_HEIGHT,WIN_FRAME_SIZE,win->height+TITLE_BAR_HEIGHT,WIN_FRAME_COLOR);
	}
	else
	{
		fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,TITLE_BAR_HEIGHT,WIN_FRAME_COLOR);
		fill_rect(win->x-WIN_FRAME_SIZE,win->y,WIN_FRAME_SIZE,win->height,WIN_FRAME_COLOR);
		fill_rect(win->x+win->width,win->y,WIN_FRAME_SIZE,win->height,WIN_FRAME_COLOR);
	}

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

uint32_t xor_masks[16] = {

	0x0,
	0x11111111,
	0x22222222,
	0x33333333,
	0x44444444,
	0x55555555,
	0x66666666,
	0x77777777,
	0x88888888,
	0x99999999,
	0xAAAAAAAA,
	0xBBBBBBBB,
	0xCCCCCCCC,
	0xDDDDDDDD,
	0xEEEEEEEE,
	0xFFFFFFFF,

};

void winsys_paint_window(PWINDOW win)
{

	if (!win)
		return;

	winsys_paint_window_section(win, 0, 0, win->width, win->height);

	winsys_disable_mouse();
	if (win->has_frame)
		winsys_paint_window_frame(win);
	winsys_enable_mouse();

}

static int winsys_current_painting_process_pid = 0;

void winsys_paint_window_section(PWINDOW win, int x, int y, int width, int height)
{

	if (!win || x+width > win->width || y+height > win->height)
		return;

	pdirectory *winDir;
	void *tmp_win_buf_base;

	if (win->is_user){
		winsys_current_painting_process_pid = win->parent_pid;
		winDir = getProcessByID(win->parent_pid)->pageDirectory;
		#define TMP_WIN_BUF_VIRT 0xB0000000
		tmp_win_buf_base = find_user_space_pages((void *)TMP_WIN_BUF_VIRT,get_win_page_amt(win));
		for (int i = 0; i < get_win_page_amt(win); i++)
			vmmngr_mmap_virt2virt(winDir, vmmngr_get_directory(), win->w_buffer + i*PAGE_SIZE, (void *)((uint32_t)tmp_win_buf_base + i*PAGE_SIZE), I86_PDE_WRITABLE, I86_PTE_WRITABLE);
	}

	uint32_t min_height = min(win->height,height);
	uint32_t min_width = min(win->width,width);

	int max_x = max(x,0);
	int max_y = max(y,0);

	uint32_t lim_x = min(x+win->width,max_x+min_width);
	uint32_t lim_y = min(y+win->height,max_y+min_height);

	uint8_t *buff;
	if (win->is_user)
		buff = (uint8_t *)TMP_WIN_BUF_VIRT;
	else
		buff = (uint8_t *)win->w_buffer;

	buff += max_y*win->width/2;

	uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + (win->y+max_y) * PIXEL_WIDTH / PIXELS_PER_BYTE;

	bool hide_mouse = winsys_check_collide_rect_rect(win->x+max_x,win->y+max_y,min_width,min_height,winsys_mousex,winsys_mousey,MOUSE_WIDTH,MOUSE_HEIGHT);

	if (hide_mouse)
		winsys_disable_mouse();

	outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
	outb(0x3CE,0x8);
	int prev_color_mask = -1;
	int prev_pixel_mask = -1;
	for (int i = max_y; i < lim_y; i++)
	{

		for (int j = max_x; j < lim_x; j++)
		{

			uint32_t x = win->x+j;
			uint32_t y = win->y+i;

			if (x >= PIXEL_WIDTH || y >= PIXEL_HEIGHT)
				continue;

			uint32_t vram_off = x/PIXELS_PER_BYTE;

			if (x % 8 == 0 && j < max_x+min_width-8 && x < PIXEL_WIDTH-8)
			{

				uint32_t eight_colors = *(uint32_t *)(buff+j/2);

				if (j%2 != 0)
					eight_colors = ((eight_colors&0xFFFFFFF0)>>4)|((*(buff+j/2+4)&0xF)<<28);

				for (uint8_t color = 0; color < 16; color++)
				{

					uint32_t color_mask = eight_colors^(~xor_masks[color]);

					if (color_mask == 0xFFFFFFFF || color_mask == 0)
					{

						if (color_mask == 0)
							color = 15-color;

						if (prev_pixel_mask != 0xFF)
						{
							outb(0x3CF,0xFF);
							prev_pixel_mask = 0xFF;
						}

						if (color != 0xF)
						{
							if (prev_color_mask != 0xF)
							{
								outb(0x3C5, 0xF);
								prev_color_mask = 0xF;
							}

							vram[vram_off] = 0;
						}
						if (color != 0)
						{
							if (prev_color_mask != color)
							{
								outb(0x3C5, color);
								prev_color_mask = color;
							}
							vram[vram_off] = 0xFF;
						}
						break;

					}

					uint8_t mask = 0;
					uint8_t mask_mask = 0x80;

					while (color_mask > 0)
					{
						if ((color_mask&0xF) == 0xF)
							mask |= mask_mask;
						mask_mask >>= 1;
						color_mask >>= 4;

					}

					if (mask == 0)
						continue;

					if (prev_pixel_mask != mask)
					{
						outb(0x3CF,mask);
						prev_pixel_mask = mask;
					}

					if (color != 0xF)
					{
						if (prev_color_mask != 0xF)
						{
							outb(0x3C5, 0xF);
							prev_color_mask = 0xF;
						}

						vram[vram_off] &= ~mask;
					}
					if (color != 0)
					{
						if (prev_color_mask != color)
						{
							outb(0x3C5, color);
							prev_color_mask = color;
						}

						vram[vram_off] |= mask;
					}

				}

				j += 7;
				continue;
			}

			uint8_t color = buff[j/2];

			if (j % 2)
				color >>= 4;
			else
				color &= 0xF;

			uint8_t mask = (0x80>>(x%PIXELS_PER_BYTE));

			if (prev_pixel_mask != mask)
			{
				outb(0x3CF,mask);
				prev_pixel_mask = mask;
			}

			if (color != 0xF)
			{
				if (prev_color_mask != 0xF)
				{
					outb(0x3C5, 0xF);
					prev_color_mask = 0xF;
				}

				vram[vram_off] &= ~mask;
			}
			if (color != 0)
			{
				if (prev_color_mask != color)
				{
					outb(0x3C5, color);
					prev_color_mask = color;
				}

				vram[vram_off] |= mask;
			}

		}

		buff += win->width/2;
		vram += PIXEL_WIDTH/PIXELS_PER_BYTE;

	}

	if (hide_mouse)
		winsys_enable_mouse();

	if (win->is_user)
	{
		for (int i = 0; i < get_win_page_amt(win); i++)
			vmmngr_unmap_virt(vmmngr_get_directory(), (void *) tmp_win_buf_base + i*PAGE_SIZE); // MEMORY WILL LEAK SINCE WE ARE NOT FREEING THE PAGE TABLE!! thats fine though as it's only if its called from a kernel proc
		winsys_current_painting_process_pid = 0;
	}

	//winsys_paint_window_frame(win);

}

void winsys_display_window_section(PWINDOW win, int x, int y, int width, int height)
{

	if (!win)
		return;

	WINSYSOP display_window_section_operation = {

		.op = WINSYS_DISPLAY_WINDOW_SECTION,
		.wid = win->id,
		.x = x,
		.y = y,
		.w = width,
		.h = height

	};

	winsys_enqueue_to_winsys_listener(display_window_section_operation);

}

void winsys_display_window_section_operation(PWINDOW win, int x, int y, int width, int height)
{

	if (!win)
		return;

	winsys_paint_window_section(win, x, y, width, height);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (winsys_check_collide(win,tmp))
		{
			int overlap_x = max(win->x, tmp->x);
			int overlap_y = max(win->y, tmp->y);
			int overlap_w = min(win->x+win->width,tmp->x+tmp->width)-overlap_x;
			int overlap_h = min(win->y+win->height, tmp->y+tmp->height)-overlap_y;

			if (overlap_w > 0 && overlap_h > 0)
			{

				int sect_x = win->x+max(x,0);
				int sect_y = win->y+max(y,0);
				int sect_w = min(win->width,width);
				int sect_h = min(win->height,height);

				int overlap_sect_x = max(overlap_x,sect_x);
				int overlap_sect_y = max(overlap_y,sect_y);
				int overlap_sect_w = min(overlap_x+overlap_w,sect_x+sect_w)-overlap_sect_x;
				int overlap_sect_h = min(overlap_y+overlap_h, sect_y+sect_h)-overlap_sect_y;

				if (overlap_sect_w > 0 && overlap_sect_h > 0)
					winsys_paint_window_section(tmp,overlap_sect_x-tmp->x,overlap_sect_y-tmp->y,overlap_sect_w,overlap_sect_h);

			}

			bool is_title_colliding = winsys_check_collide_rect_rect(win->x,win->y,win->width,win->height,tmp->x-WIN_FRAME_SIZE,tmp->y-TITLE_BAR_HEIGHT, tmp->width + WIN_FRAME_SIZE*2, TITLE_BAR_HEIGHT);

			if (is_title_colliding && tmp->has_frame)
				winsys_paint_window_frame(tmp);

		}

		tmp = tmp->next;

	}

}

void winsys_display_window(PWINDOW win)
{

	if (!win)
		return;

	WINSYSOP display_window_operation = {

		.op = WINSYS_DISPLAY_WINDOW,
		.wid = win->id

	};

	winsys_enqueue_to_winsys_listener(display_window_operation);

}

void winsys_display_window_if_possible(PWINDOW win)
{

	if (!win)
		return;

	WINSYSOP display_window_operation = {

		.op = WINSYS_DISPLAY_WINDOW,
		.wid = win->id

	};

	winsys_enqueue_to_winsys_listener_if_possible(display_window_operation);

}

void winsys_display_window_if_possible_lockless(PWINDOW win)
{

	if (!win)
		return;

	WINSYSOP display_window_operation = {

		.op = WINSYS_DISPLAY_WINDOW,
		.wid = win->id

	};

	winsys_enqueue_to_winsys_listener_if_possible_lockless(display_window_operation);

}

void winsys_display_window_operation(PWINDOW win)
{

	if (!win)
		return;
	winsys_paint_window(win);

	PWINDOW tmp = win->next;

	while(tmp)
	{
		if (winsys_check_collide(win,tmp))
		{
			int overlap_x = max(win->x-WIN_FRAME_SIZE, tmp->x);

			int overlap_y = max(win->y-TITLE_BAR_HEIGHT, tmp->y);

			int overlap_w = min(win->x+win->width+WIN_FRAME_SIZE*2,tmp->x+tmp->width)-overlap_x;

			int overlap_h = min(win->y+win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE, tmp->y+tmp->height)-overlap_y;

			if (overlap_w > 0 && overlap_h > 0)
				winsys_paint_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);

		}

		bool is_title_colliding = winsys_check_collide_rect_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,tmp->x-WIN_FRAME_SIZE,tmp->y-TITLE_BAR_HEIGHT, tmp->width + WIN_FRAME_SIZE*2, TITLE_BAR_HEIGHT);

		if (is_title_colliding && tmp->has_frame && win->has_frame) // doing win->has_frame only because of the top bar windows, not correct
			winsys_paint_window_frame(tmp);

		tmp = tmp->next;

	}

}

void winsys_display_collided_windows(PWINDOW win)
{

	if (!win)
		return;

	//acquireLock(&winsys_win_list_lock);

	PWINDOW tmp = win_list;
	PWINDOW prev = 0;

	while(tmp)
	{	
		if (tmp != win && winsys_check_collide(win,tmp))
		{

			//winsys_paint_window(tmp);

			int overlap_x = max(win->x-WIN_FRAME_SIZE, tmp->x);

			int overlap_y = max(win->y-TITLE_BAR_HEIGHT, tmp->y);

			int overlap_w = min(win->x+win->width+WIN_FRAME_SIZE*2,tmp->x+tmp->width)-overlap_x;

			int overlap_h = min(win->y+win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE, tmp->y+tmp->height)-overlap_y;
			if (overlap_w > 0 && overlap_h > 0)
				winsys_paint_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);
			
			//bool is_title_colliding = winsys_check_collide_rect_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,tmp->x-WIN_FRAME_SIZE,tmp->y-TITLE_BAR_HEIGHT, tmp->width + WIN_FRAME_SIZE*2, TITLE_BAR_HEIGHT);

			//if (is_title_colliding && tmp->has_frame)
			if (tmp->has_frame)
				winsys_paint_window_frame(tmp);
		}

		if (tmp != win && prev && prev->has_frame)
		{

			int overlap_x = max(prev->x-WIN_FRAME_SIZE, tmp->x);

			int overlap_y = max(prev->y-TITLE_BAR_HEIGHT, tmp->y);

			int overlap_w = min(prev->x+prev->width+WIN_FRAME_SIZE*2,tmp->x+tmp->width)-overlap_x;

			int overlap_h = min(prev->y+prev->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE, tmp->y+tmp->height)-overlap_y;

			winsys_paint_window_section(tmp,overlap_x-tmp->x,overlap_y-tmp->y,overlap_w,overlap_h);
		}

		prev = tmp;

		tmp = tmp->next;
	}

	//releaseLock(&winsys_win_list_lock);

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
	winsys_disable_mouse();
	fill_rect(win->x-WIN_FRAME_SIZE,win->y-TITLE_BAR_HEIGHT,win->width+WIN_FRAME_SIZE*2,win->height+TITLE_BAR_HEIGHT+WIN_FRAME_SIZE,BG_COLOR);
	winsys_enable_mouse();
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

bool winsys_check_collide_win_rect(PWINDOW w1, int w2_x, int w2_y, int w2_w, int w2_h)
{

	if (!w1)
		return false;

	int w1_x = w1->x-WIN_FRAME_SIZE;
	int w1_y = w1->y-TITLE_BAR_HEIGHT;
	int w1_w = w1->width + WIN_FRAME_SIZE*2;
	int w1_h = w1->height + TITLE_BAR_HEIGHT+WIN_FRAME_SIZE;

	return w2_x + w2_w > w1_x &&
		   w2_y + w2_h > w1_y &&
		   w1_x + w1_w > w2_x &&
		   w1_y + w1_h > w2_y;

}

bool winsys_check_collide_rect_rect(int w1_x, int w1_y, int w1_w, int w1_h, int w2_x, int w2_y, int w2_w, int w2_h)
{

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
	PWINDOW top_collision = 0;

	if (!tmp)
		return 0;

	while(tmp)
	{
		while(tmp && !winsys_check_collide_coords(tmp,x,y))
			tmp = tmp->next;
		if (!tmp)
			break;
		top_collision = tmp;
		tmp = tmp->next;
	}

	return top_collision;

}

PWINDOW winsys_get_window_from_title_collision(int x, int y)
{

	PWINDOW tmp = win_list;
	PWINDOW top_collision = 0;

	if (!tmp)
		return 0;

	while(tmp)
	{
		while(!winsys_check_title_collide(tmp,x,y) && tmp)
			tmp = tmp->next;
		if (!tmp)
			break;
		top_collision = tmp;
		tmp = tmp->next;
	}

	if (!top_collision)
		return 0;

	if (winsys_check_close_collide(top_collision,x,y))
	{
		if (top_collision->is_user)
			terminateProcessById(top_collision->parent_pid);
		else if (top_collision->parent_pid)
			terminateKernelProcessById(top_collision->parent_pid);
		else
			winsys_remove_window(top_collision);
		return 0;
	}

	return top_collision;

}

void winsys_move_window(PWINDOW win, int x, int y)
{

	if (!win)
		return;

	WINSYSOP move_window_operation = {

		.op = WINSYS_MOVE_WINDOW,
		.wid = win->id,
		.x = x,
		.y = y

	};

	winsys_enqueue_to_winsys_listener(move_window_operation);

}

void winsys_move_window_through_mouse(PWINDOW win, int x, int y)
{

	if (!win)
		return;

	WINSYSOP move_window_operation = {

		.op = WINSYS_MOVE_WINDOW,
		.wid = win->id,
		.x = x,
		.y = y

	};

	winsys_enqueue_to_winsys_listener_if_possible(move_window_operation);

}

void winsys_move_window_operation(PWINDOW win, int x, int y)
{

	if (!win)
		return;

	winsys_clear_whole_window(win);
	winsys_display_collided_windows(win);

	win->x = x;
	win->y = y;
	if (working_window != win)
		winsys_set_working_window_operation(win->id);
	else
		winsys_display_window_operation(win);

}

void winsys_remove_window(PWINDOW win)
{

	if (!win)
		return;

	WINSYSOP remove_window_operation = {

		.op = WINSYS_REMOVE_WINDOW,
		.wid = win->id,

	};

	winsys_enqueue_to_winsys_listener(remove_window_operation);

}

void winsys_remove_window_operation(PWINDOW win)
{

	if (!win)
		return;

	if (win->is_user)
	{
		winsys_remove_window_user(win);
		return;
	}

	winsys_clear_whole_window(win);

	acquireLock(&winsys_win_list_lock);

	PWINDOW tmp = win_list;

	if (tmp->id == win->id)
		win_list = win->next;
	else
	{
		while(tmp->next->id != win->id)
			tmp = tmp->next;

		tmp->next = win->next;
	}

	releaseLock(&winsys_win_list_lock);

	if (working_window->id == win->id)
	{
		tmp = win_list;
		if (tmp)
		{
			while(tmp->next)
				tmp = tmp->next;
			working_window = 0;
			winsys_set_working_window_operation(tmp->id);
		}
		else
			working_window = 0;
	}

	winsys_display_collided_windows(win);

	kfree(win->w_buffer);
	kfree(win->w_name);
	kfree(win);

}

void winsys_remove_windows_by_pid(int pid)
{

	PWINDOW tmp = win_list;

	while(tmp)
	{

		PWINDOW to_remove = tmp;
		tmp = tmp->next;
		if (to_remove->parent_pid == pid)
		{
			if (to_remove->is_user)
			{

				// wait for winsys to finish drawing the process
				while(winsys_current_painting_process_pid == to_remove->parent_pid)
					__asm__("pause");

				for (int i = 0; i < get_win_page_amt(to_remove); i++)
					vmmngr_free_virt(getProcessByID(to_remove->parent_pid)->pageDirectory, (void *) to_remove->w_buffer + i*PAGE_SIZE);
			}

			winsys_remove_window(to_remove);
			return;
		}

	}

}

void winsys_remove_window_user(PWINDOW win)
{

	if (!win)
		return;

	win = winsys_get_window_by_id(win->id);

	if (!win)
		return;

	winsys_clear_whole_window(win);

	// maybe move this remove window to remove window by id if it's user because after freeing the w_buffer winsys might draw it again if it's there
	acquireLock(&winsys_win_list_lock);

	PWINDOW tmp = win_list;

	if (tmp->id == win->id)
		win_list = win->next;
	else
	{
		while(tmp->next->id != win->id)
			tmp = tmp->next;

		tmp->next = win->next;
	}

	releaseLock(&winsys_win_list_lock);

	if (working_window->id == win->id)
	{
		working_window = 0;

		tmp = win_list;
		while(tmp->next)
			tmp = tmp->next;

		winsys_set_working_window_operation(tmp->id);
	}

	winsys_display_collided_windows(win);
	
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

void winsys_dequeue_from_event_handler_user(PWINDOW win, PEVENT event_buff)
{

	PEVENTHAND handler = &winsys_get_window_by_id(win->id)->event_handler;

	EVENT e = handler->events[0];

	for (int i = 0; i < EVENT_HANDLER_QUEUE_SIZE-1; i++)
	{

		handler->events[i] = handler->events[i+1];

	}

	handler->events[EVENT_HANDLER_QUEUE_SIZE-1].event_type = EVENT_INVALID;

	memcpy((char *)event_buff,(char *)&e,sizeof(EVENT));

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

	if (x % 2 == 0)
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

	if (x % 2 == 0)
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

	// Make scrolling adjustment, for when we reach the bottom
	// of the screen.
	offset_y = gfx_handle_scrolling(win, inp_info, offset_y);

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
	if (offset_y < gfx_get_logical_row(win->height)-1)
	{
		return offset_y;
	}
	
	if (get_current_task())
	{
		gfx_paint_char_bg(win,'.',gfx_get_win_x(0),gfx_get_win_y(offset_y),0,0x4);
		gfx_paint_char_bg(win,'.',gfx_get_win_x(1),gfx_get_win_y(offset_y),0,0x4);
		gfx_paint_char_bg(win,'.',gfx_get_win_x(2),gfx_get_win_y(offset_y),0,0x4);
		winsys_display_window(win);

		while(1)
		{
			while(win->event_handler.events[0].event_type & EVENT_INVALID)
				continue;

			EVENT e = winsys_dequeue_from_event_handler(&win->event_handler);

			if (e.event_type & EVENT_KBD_PRESS)
				break;
		}

		gfx_fill_rect(win,0,win->height-CHAR_HEIGHT,CHAR_WIDTH*3,CHAR_HEIGHT,0);
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

	if (bmp.flags != FS_FILE)
	{

		printf("\nFile not found.\n");
		return;

	}

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
	win->parent_pid = 0;

	uint32_t color_buff;

	// read 16 color palette
	for (int i = 0; i < 16; i++)
		volReadFile(&bmp,(char *)&color_buff,4);

	uint8_t two_pixels;
	uint8_t padding_bytes = 4-(width/2+width%2)%4;
	if (padding_bytes == 4)
		padding_bytes = 0;
	
	uint32_t img_buffer_size = width*height/2;
	img_buffer_size += padding_bytes*height + img_buffer_size%2;
	char *img_buffer = kmalloc(img_buffer_size);
	for (int i = 0; i < 4; i++)
		volReadFile(&bmp,img_buffer+i*(img_buffer_size/4),img_buffer_size/4);
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

	kfree(img_buffer);

	winsys_display_window_section(win,0,0,win->width,win->height);
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
	if (padding_bytes == 4)
		padding_bytes = 0;
	
	uint32_t img_buffer_size = width*height/2;
	img_buffer_size += padding_bytes*height + img_buffer_size%2;
	char *img_buffer = kmalloc(img_buffer_size);
	for (int i = 0; i < 4; i++)
		volReadFile(&bmp,img_buffer+i*(img_buffer_size/4),img_buffer_size/4);
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

	kfree(img_buffer);

	volCloseFile(&bmp);

}
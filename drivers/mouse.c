#include "low_level.h"
#include "ps2.h"
#include "irq.h"
#include "screen.h"
#include "mouse.h"
#include "std.h"
#include "keyboard.h"
#include "graphics.h"
#include "window_sys.h"
#include <stdint.h>
#include <stdbool.h>

static int16_t PREVX = 0;
static int16_t PREVY = 0;
static int16_t MOUSEX = 0;
static int16_t MOUSEY = 0;
static uint8_t color_attr = 3;
static bool mouse_enabled = true;
static bool mouse_installed = false;

static PWINDOW dragging_window = 0;

// 18x10 cursor bitmap
uint8_t mouse_bitmap[18][10] = {
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

uint8_t mouse_placeholder_buffer[18][10];

void mouse_wait(uint8_t type)
{
	uint32_t _timeout = 1000000;
	
	if (type == 0)
	{
		while (_timeout--)
		{
			if (inb(PS_CTRL) & 1) return;
			asm ("pause");

		}
	}
	else
	{
		while (_timeout--)
		{
			if (!(inb(PS_CTRL) & 2)) return;
			asm ("pause");
		}
	}

}

void disable_mouse()
{	
	if (!mouse_installed)
		return;
	mouse_enabled = false;

	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j])
				set_pixel(MOUSEX+j,MOUSEY+i,mouse_placeholder_buffer[i][j]);

		}

	}

}

void save_to_mouse_buffer()
{
	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j])
				mouse_placeholder_buffer[i][j] = get_pixel(MOUSEX+j,MOUSEY+i);

		}

	}

}

void print_mouse()
{

	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j] == 1)
				set_pixel(MOUSEX+j,MOUSEY+i,0xf);
			else if(mouse_bitmap[i][j] == 2)
				set_pixel(MOUSEX+j,MOUSEY+i,0);

		}

	}

}

void clear_mouse()
{

	for (int i = 0; i < 18; i++)
	{

		for (int j = 0; j < 10; j++)
		{

			if (mouse_bitmap[i][j])
				set_pixel(PREVX+j,PREVY+i,mouse_placeholder_buffer[i][j]);

		}

	}

}

void enable_mouse()
{
	if (!mouse_installed)
		return;
	mouse_enabled = true;
	save_to_mouse_buffer();
	print_mouse();

}

void mouse_handler()
{

	#ifdef BOCHS
	mouse_wait(0);
	#endif
	uint8_t flags = inb(PS_DATA);
	#ifdef BOCHS
	mouse_wait(0);
	#endif
	uint8_t xmov = inb(PS_DATA);
	#ifdef BOCHS
	mouse_wait(0);
	#endif
	uint8_t ymov = inb(PS_DATA);
	#ifdef BOCHS
	mouse_wait(0);
	#endif
	uint8_t zmov = inb(PS_DATA);


	if (!mouse_enabled || !mouse_installed)
		return;

	int8_t rel_x = xmov - ((flags << 4) & 0x100); // produce 2s complement only if the neg bit is set
	int8_t rel_y = ymov - ((flags << 3) & 0x100); // produce 2s complement only if the neg bit is set
	// handle mouse mvmt

	MOUSEX += rel_x*2;

	if (MOUSEX < 0)
		MOUSEX = 0;
	else if (MOUSEX >= PIXEL_WIDTH)
		MOUSEX = (uint16_t)PIXEL_WIDTH-1;

	MOUSEY -= rel_y*2;

	if (MOUSEY < TOP*CHAR_HEIGHT)
		MOUSEY = TOP*CHAR_HEIGHT;
	else if (MOUSEY >= PIXEL_HEIGHT)
		MOUSEY = (uint16_t)PIXEL_HEIGHT-1;

	clear_mouse();

	PREVX = MOUSEX;
	PREVY = MOUSEY;

	save_to_mouse_buffer();
	
	print_mouse();

	// handle scrolling
	int8_t scroll = zmov & 0xF;
	if (scroll == VERTICAL_SCROLL_UP)
	{
		//scroll_down();
	}
	else if (scroll == VERTICAL_SCROLL_DOWN)
	{
		//scroll_up();
	}

	int left_click = flags & 1;
	int right_click = flags & 2;

	if (right_click)
	{	

		if (!dragging_window)
		{
			clear_mouse();
			PWINDOW win = winsys_get_window_from_title_collision(MOUSEX,MOUSEY);
			save_to_mouse_buffer();
			print_mouse();
			if (win)
			{
				dragging_window = win;
			}
		}
		else
		{

			winsys_move_window(dragging_window,MOUSEX+WIN_FRAME_SIZE,MOUSEY+TITLE_BAR_HEIGHT);
			save_to_mouse_buffer();
			dragging_window = 0;

		}

		// for(int i = 0; i < 20; i+=2)
		// {
		// 	outline_circle(MOUSEX,MOUSEY,i,0xf);
		// 	outline_circle(MOUSEX,MOUSEY,i,0);
		// }
		// fill_rect(get_screen_x(get_cursor_col()),get_screen_y(get_cursor_row())+12,8,4,0);
		// set_cursor_coords(get_logical_col(MOUSEX), get_logical_row(MOUSEY));
		
	}
	if (left_click)
	{

		PWINDOW win = winsys_get_window_from_collision(MOUSEX, MOUSEY);
		if (win && winsys_get_working_window() != win)
			winsys_set_working_window(win->id);

	}

	PWINDOW working_win = winsys_get_working_window();

	PEVENTHAND win_event_handler = &working_win->event_handler;

	if (win_event_handler->event_mask & GENERAL_EVENT_MOUSE)
	{

		EVENT mouse_event;

		mouse_event.event_type = 0;

		if (left_click)
			mouse_event.event_type |= EVENT_MOUSE_LEFT_CLICK;
		if (right_click)
			mouse_event.event_type |= EVENT_MOUSE_RIGHT_CLICK;
		if (rel_x != 0 || rel_y != 0)
			mouse_event.event_type |= EVENT_MOUSE_MOVE;

		mouse_event.event_data = (MOUSEX-working_win->x) | ((MOUSEY-working_win->y)<<16);

		winsys_enqueue_to_event_handler(win_event_handler, mouse_event);

	}

}


void ack()
{
	mouse_wait(0);
	uint8_t ack = inb(PS_DATA);                     // read back acknowledge. This should be 0xFA
}

void mouse_write(uint8_t byte)
{
	mouse_wait(1);
	outb(PS_CTRL, WRITE_TO_MOUSE); // tell the controller to address the mouse
	mouse_wait(1);
	outb(PS_DATA, byte); // default settings
	ack();

}

int identify()
{

	mouse_write(GET_MOUSE_ID); // get id
	mouse_wait(0);
	uint8_t id = inb(PS_DATA);
	return id;
}

void set_mouse_rate(int rate)
{

	mouse_write(MOUSE_CHANGE_SAMPLE);// change sample rate
	mouse_write(rate);
}

void mouse_install()
{
	MOUSEX = 0;
	MOUSEY = 0;
	PREVX = 0;
	PREVY = 0;
	save_to_mouse_buffer();

	__asm__ __volatile__ ("cli");

	mouse_wait(1);
	outb(PS_CTRL, 0xA8);

	mouse_wait(1);
	outb(PS_CTRL, 0x20); // read the configuration byte
	mouse_wait(0);
	char status_byte = inb(PS_DATA); // read ps status byte
	status_byte |= 2; // enable second ps2 port interrupt (irq12)
	status_byte &= 0b11011111;
	mouse_wait(1);
	outb(PS_CTRL, PS_DATA); // write to the configuration byte
	mouse_wait(1);
	outb(PS_DATA, status_byte);


	mouse_write(MOUSE_RESET); // reset
	mouse_wait(0);
	inb(PS_DATA);
	mouse_wait(0);
	inb(PS_DATA);


	mouse_write(MOUSE_DEFAULT);// default settings

	// activate scrolling
	set_mouse_rate(200);
	set_mouse_rate(100);
	set_mouse_rate(80);
	//printf("MOUSE ID: %d\n",identify());

	mouse_write(MOUSE_PACKET);// activate data send

	irq_install_handler(12, mouse_handler);

	__asm__ __volatile__ ("sti");

	mouse_installed = true;
}
#include "low_level.h"
#include "ps2.h"
#include "irq.h"
#include "screen.h"
#include "mouse.h"
#include "std.h"
#include "keyboard.h"
#include <stdint.h>
#include <stdbool.h>

static uint8_t PREVX = 40;
static uint8_t PREVY = 12;
static int8_t MOUSEX = 40;
static int8_t MOUSEY = 12;
static uint8_t color_attr = 3;
static int interval = 0;
static bool mouse_enabled = true;
static bool mouse_installed = false;

static placeholder mouset_buffer = {' ',0x0f};
static placeholder mouseb_buffer = {' ',0x0f};
static placeholder mouser_buffer = {' ',0x0f};

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
	int prev_cursor = get_cursor();
	print_char(mouset_buffer.ascii,MOUSEY,MOUSEX,mouset_buffer.attr);
	if (MOUSEY < 24)
		print_char(mouseb_buffer.ascii,MOUSEY+1,MOUSEX,mouseb_buffer.attr);
	if (MOUSEX < 79)
		print_char(mouser_buffer.ascii,MOUSEY,MOUSEX+1,mouser_buffer.attr);
	set_cursor(prev_cursor);

}

void save_to_mouse_buffer()
{
	char *video_memory = (char *)VIDEO_ADDRESS;
	mouset_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX)*2);
	mouset_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX)*2 + 1);
	mouseb_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX + 80)*2);
	mouseb_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX + 80)*2 + 1);
	mouser_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX + 1)*2);
	mouser_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX + 1)*2 + 1);

}

void enable_mouse()
{
	if (!mouse_installed)
		return;
	mouse_enabled = true;
	save_to_mouse_buffer();
	int prev_cursor = get_cursor();
	print_char(0,MOUSEY,MOUSEX,0x40);
	if (MOUSEY < 24)
		print_char(0,MOUSEY+1,MOUSEX,0x40);
	if (MOUSEX < 79)
		print_char(0,MOUSEY,MOUSEX+1,0x40);
	set_cursor(prev_cursor);

}

void mouse_handler(struct regs *r)
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

	if (interval % 5 == 0)
	{

		int8_t rel_x = xmov - ((flags << 4) & 0x100); // produce 2s complement only if the neg bit is set
		int8_t rel_y = ymov - ((flags << 3) & 0x100); // produce 2s complement only if the neg bit is set
		// handle mouse mvmt

		MOUSEX += rel_x;

		if (MOUSEX < 0)
			MOUSEX = 0;
		else if (MOUSEX > 79)
			MOUSEX = 79;

		MOUSEY -= rel_y;

		if (MOUSEY < 2)
			MOUSEY = 2;
		else if (MOUSEY > 24)
			MOUSEY = 24;
		int prev_cursor = get_cursor();
		print_char(mouset_buffer.ascii,PREVY,PREVX,mouset_buffer.attr);
		if (PREVY < 24)
			print_char(mouseb_buffer.ascii,PREVY+1,PREVX,mouseb_buffer.attr);
		if (PREVX < 79)
			print_char(mouser_buffer.ascii,PREVY,PREVX+1,mouser_buffer.attr);

		PREVX = MOUSEX;
		PREVY = MOUSEY;

		save_to_mouse_buffer();
		
		print_char(0,MOUSEY,MOUSEX,0x40);
		if (MOUSEY < 24)
			print_char(0,MOUSEY+1,MOUSEX,0x40);
		if (MOUSEX < 79)
			print_char(0,MOUSEY,MOUSEX+1,0x40);
		set_cursor(prev_cursor);

	}
	
	interval++;

	// handle scrolling
	int8_t scroll = zmov & 0xF;
	if (scroll == VERTICAL_SCROLL_UP)
	{
		scroll_down();
	}
	else if (scroll == VERTICAL_SCROLL_DOWN)
	{
		scroll_up();
	}

	int left_click = flags & 1;
	int right_click = flags & 2;

	if (left_click)
	{
		if (MOUSEX == 79) // if on scroll bar
		{
			set_scroll_pos_mouse(MOUSEY);
		}
		else
		{
			set_cursor_coords(MOUSEY, MOUSEX);
		}
	}
	if (right_click)
	{
		int prev_cursor = get_cursor();
		print_char(0,MOUSEY-1,MOUSEX-1,color_attr << 4);
		set_cursor(prev_cursor);
		color_attr += 1;
		color_attr = color_attr % 8;
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
	printf("MOUSE ID: %d\n",identify());

	mouse_write(MOUSE_PACKET);// activate data send

	irq_install_handler(12, mouse_handler);

	__asm__ __volatile__ ("sti");

	// save strings at future mouse init position
	char *video_memory = (char *)VIDEO_ADDRESS;

	mouset_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX)*2);
	mouset_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX)*2 + 1);
	mouseb_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX + 80)*2);
	mouseb_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX + 80)*2 + 1);
	mouser_buffer.ascii = *(video_memory + (MOUSEY*80 + MOUSEX + 1)*2);
	mouser_buffer.attr = *(video_memory + (MOUSEY*80 + MOUSEX + 1)*2 + 1);

	mouse_installed = true;
}
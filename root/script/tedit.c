#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "gfx.h"
#include "string.h"
#include "tedit.h"

#define WIDTH 400
#define HEIGHT 300

PTextRegion alloc_text_region()
{

	return (PTextRegion)calloc(sizeof(TextRegion));

}

PLineTextRegion alloc_line_text_region()
{

	return (PLineTextRegion)calloc(sizeof(LineTextRegion));

}


PLineTextRegion load_file(int fd)
{

	PLineTextRegion base_text_region = alloc_line_text_region();
	PLineTextRegion cur_line_text_region = base_text_region;
	PTextRegion cur_text_region = &base_text_region->r;

	while(true)
	{

		char c;

		if (!fread(fd, &c, 1))
			break;

		cur_text_region->buffer[cur_text_region->pos++] = c;

		if (cur_text_region->pos == TEXT_REGION_BUFFER_SIZE)
		{

			cur_text_region->next = alloc_text_region();
			cur_text_region->next->prev = cur_text_region;
			cur_text_region = cur_text_region->next;

		}

		if (c == '\n')
		{

			cur_line_text_region->next_line = alloc_line_text_region();
			cur_line_text_region->next_line->prev_line = cur_line_text_region;
			cur_line_text_region->next_line->line = cur_line_text_region->line + 1;
			cur_line_text_region = cur_line_text_region->next_line;
			cur_text_region = &cur_line_text_region->r;

		}

	}

	return base_text_region;

}

void write_file(int fd, PTextRegion tr)
{

	while(tr)
	{

		fwrite(fd,tr->buffer,tr->pos);

		tr = tr->next;

	}

}

void display_file(PWINDOW win, PLineTextRegion ltr, char* font)
{

	PTextRegion cur_text_region = &ltr->r;
	size_t cur_pos = 0;
	size_t cur_line_pos = 0;

	while (ltr)
	{
		while(cur_text_region)
		{

			char c;

			c = cur_text_region->buffer[cur_pos];

			if (c != '\r' && c != '\n')
				gfx_paint_char_bg(win,c,cur_pos*CHAR_WIDTH,ltr->line*CHAR_HEIGHT,0,0xF,font);
			else
				break;

			cur_pos++;
			cur_line_pos++;

			if (cur_pos >= cur_text_region->pos)
			{

				cur_text_region = cur_text_region->next;
				cur_pos = 0;

			}

			if (cur_line_pos*CHAR_WIDTH >= WIDTH)
				break;

		}

		ltr = ltr->next_line;
		if (ltr)
		{
			cur_text_region = &ltr->r;
			cur_pos = 0;
			cur_line_pos = 0;

			if (ltr->line*CHAR_HEIGHT > HEIGHT)
				break;
		}

	}

	display_window_section(win,0,0,WIDTH,HEIGHT);

}

// us keyboard layout scancode->ascii
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* poss lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

WINDOW window;
char font_buff[256*16];

void _main(int argc, char **argv)
{

	if (argc != 2)
	{

		print("INCORRECT FORMAT ERROR: tedit FILEPATH");
		terminate();

	}

	char *filepath = argv[1];

	int file_desc = fopen(filepath);

	if (!file_desc)
	{

		printf("OPEN ERROR: file %s does not exist", filepath);
		terminate();

	}

	PLineTextRegion base_text_region = load_file(file_desc);
	PLineTextRegion cur_line_text_region = base_text_region;
	PTextRegion cur_text_region = &cur_line_text_region->r;
	size_t top_row = 0;
	size_t cur_col = 0;

	fclose(file_desc);

	window.w_name = "text editor";
	window.event_handler.event_mask = GENERAL_EVENT_KBD;
	create_window(&window,100,100,WIDTH,HEIGHT);
	load_font((void *)font_buff);

	display_file(&window,cur_line_text_region,font_buff);

	while(1)
	{

		EVENT e;

		get_window_event(&window,&e);

		if (e.event_type == EVENT_KBD_PRESS)
		{

			char scancode = e.event_data&0xFF;
			bool caps = e.event_data&0x100;
			bool shift = e.event_data&0x200;

			if (scancode == K_ESCAPE)
				break;

			if (kbdus[scancode])
				cur_text_region->buffer[cur_text_region->pos++] = kbdus[scancode];

		}

	}

	file_desc = fopen(filepath);
	write_file(file_desc,&base_text_region->r);
	fclose(file_desc);

	remove_window(&window);

	terminate();
	__builtin_unreachable();
}
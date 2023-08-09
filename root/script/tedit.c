#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "gfx.h"
#include "string.h"

#define TEXT_REGION_BUFFER_SIZE 128

typedef struct _TextRegion
{

	char buffer[TEXT_REGION_BUFFER_SIZE];
	struct _TextRegion *next;

} TextRegion, *PTextRegion;

PTextRegion alloc_text_region()
{

	PTextRegion r = (PTextRegion)malloc(sizeof(TextRegion));

	memset((char *)r,0,sizeof(TextRegion));

	return r;

}

#define WIDTH 400
#define HEIGHT 300

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
    0,	/* Caps lock */
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

	PTextRegion base_text_region = alloc_text_region();

	uint32_t ptr = 0;
	while(fread(file_desc, base_text_region->buffer + (ptr++),1));

	fclose(file_desc);

	file_desc = fopen(filepath);

	window.w_name = "text editor";
	window.event_handler.event_mask = GENERAL_EVENT_KBD;
	create_window(&window,100,100,WIDTH,HEIGHT);
	load_font((void *)font_buff);

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
				base_text_region->buffer[0] = kbdus[scancode];

		}

	}

	fwrite(file_desc,base_text_region->buffer,TEXT_REGION_BUFFER_SIZE);

	remove_window(&window);
	fclose(file_desc);

	terminate();
	__builtin_unreachable();
}
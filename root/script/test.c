#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include <stdbool.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

WINDOW win;

#define BOX_SIZE 5

void _start(int argc, char **argv)
{
	printf("\nreceived argc: %U\n",argc);

	while(*argv)
	{

		printf("%s\n",*argv);

		argv++;

	}

	win.w_name = "move mouse";
	win.event_handler.event_mask = GENERAL_EVENT_MOUSE;
	create_window(&win,100,100,100,100);

	int px = 0;
	int py = 0;

	while(1)
	{

		EVENT e;

		get_window_event(&win,&e);

		if (e.event_type == EVENT_INVALID)
			suspend();

		if (e.event_type == EVENT_MOUSE_LEFT_CLICK)
		{

			break;
		}

		if (e.event_type == EVENT_MOUSE_MOVE)
		{

			int16_t mx = e.event_data&0xFFFF;
			int16_t my = (e.event_data&(0xFFFF<<16))>>16;

			gfx_fill_rect(&win,px,py,BOX_SIZE,BOX_SIZE,0);
			display_window_section(&win,px,py,BOX_SIZE,BOX_SIZE);

			int x = MIN(MAX(0,mx),win.width-BOX_SIZE);
			int y = MIN(MAX(0,my),win.height-BOX_SIZE);

			px = x;
			py = y;

			gfx_fill_rect(&win,x,y,BOX_SIZE,BOX_SIZE,0x3);
			display_window_section(&win,x,y,BOX_SIZE,BOX_SIZE);

		}

	}

	terminate();
	__builtin_unreachable();
}
#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include <stdbool.h>

WINDOW win;

void _start()
{

	win.w_name = "clickme!!";
	win.event_handler.event_mask = GENERAL_EVENT_MOUSE;
	create_window(&win,100,100,80,80);
	while(1)
	{

		EVENT e;

		get_window_event(&win,&e);

		if (e.event_type == EVENT_INVALID)
			suspend();

		if (e.event_type == EVENT_MOUSE_LEFT_CLICK)
		{

			int16_t mx = e.event_data&0xFFFF;
			int16_t my = (e.event_data&(0xFFFF<<16))>>16;

			if (mx >= 0 && mx < win.width &&  my >= 0 && my < win.height)
			{

				exec("a:\\script\\clickme.exe",0);

				break;

			}
		}

	}

	terminate();
	__builtin_unreachable();
}
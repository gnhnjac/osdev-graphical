#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include <stdbool.h>

WINDOW win;

void _main(int argc, char **argv)
{

	win.w_name = "speed test";
	create_window(&win,100,100,500,500);

	int color = 0;
	while(1)
	{

		gfx_fill_rect(&win,0,0,500,500,color);
		display_window_section(&win,0,0,500,500);

		color = (color + 1) % 16;

		sleep(30);

	}

	terminate();
	__builtin_unreachable();
}
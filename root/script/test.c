#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

WINDOW win;

void _start()
{
	win.w_name = "party";
	create_window(&win,100,100,300,300);
	gfx_paint_bmp16(&win,"a:\\pepe.bmp",0,0);
	display_window_section(&win,0,0,win.width,win.height);

	while(1);

	terminate();
	__builtin_unreachable();
}
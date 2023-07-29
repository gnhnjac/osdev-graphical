#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"

WINDOW win;
uint8_t font_buff[256*16];

void cycle(int col, int cycler_start);

#define PARTIERS 5
void _start()
{
	load_font((void *)font_buff);
	win.w_name = "party";
	create_window(&win,100,100,PARTIERS*CHAR_WIDTH,CHAR_HEIGHT);

	int off = 0;
	int cycler_start = 10;
	for (int i = 0; i < PARTIERS; i++)
	{

		int pid = fork();

		if(pid == 0)
			cycle(off, cycler_start%26);

		sleep(1000);
		off++;
		cycler_start*=123;

	}

	sleep(10000);

	terminate();
	__builtin_unreachable();
}

void cycle(int col, int cycler_start)
{

	int cycler = cycler_start;
	while(1)
	{
		gfx_paint_char_bg(&win,'a'+cycler,col*CHAR_WIDTH,0,cycler%16,(cycler+5)%16,font_buff);
		display_window_section(&win,col*CHAR_WIDTH,0,CHAR_WIDTH,CHAR_HEIGHT);
		sleep(1000);
		cycler = (cycler + 1)%26;

	}

}
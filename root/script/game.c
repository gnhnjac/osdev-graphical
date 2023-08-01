#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include <stdbool.h>

WINDOW win;
uint8_t font_buff[256*16];
char board[3][3];

int get_board_row(int y);
int get_board_col(int x);
bool check_win(char grid[3][3]);
void init_win();

#define BLOCK_SIZE 100
void _main()
{
	load_font((void *)font_buff);

	win.w_name = "tictactoe";
	win.event_handler.event_mask = GENERAL_EVENT_MOUSE;
	create_window(&win,100,100,BLOCK_SIZE*3,BLOCK_SIZE*3);
	init_win();
	display_window_section(&win,0,0,win.width,win.height);
	int turn = 0;
	int count = 0;

	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
			board[i][j] = ' ';
	}

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

			if (mx < 0 || my < 0)
				continue;

			int col = get_board_col(mx);
			int row = get_board_row(my);

			int x = col*BLOCK_SIZE+BLOCK_SIZE/2;
			int y = row*BLOCK_SIZE+BLOCK_SIZE/2;

			int color = 0xf;

			if (turn)
				color = 0x4;

			gfx_fill_rect(&win,x,y,10,10,color);
			display_window_section(&win,x,y,10,10);

			board[row][col] = (turn == 0) ? 'X' : 'O';

			if (check_win(board))
				break;

			turn = !turn;
			if (++count == 9)
				break;

		}

	}

	int color = 0xf;

	if (turn)
		color = 0x4;

	if (count == 9)
		color = 0xd;

	gfx_fill_rect(&win,0,0,win.width,win.height,color);
	display_window_section(&win,0,0,win.width,win.height);

	sleep(1000);

	terminate();
	__builtin_unreachable();
}

int get_board_row(int y)
{

	return y/BLOCK_SIZE;

}

int get_board_col(int x)
{

	return x/BLOCK_SIZE;

}

void init_win()
{

	gfx_fill_rect(&win,0,0,win.width,win.height,0);

	gfx_fill_rect(&win,BLOCK_SIZE,0,1,win.height,0xf);
	gfx_fill_rect(&win,BLOCK_SIZE*2,0,1,win.height,0xf);
	gfx_fill_rect(&win,0,BLOCK_SIZE,win.width,1,0xf);
	gfx_fill_rect(&win,0,BLOCK_SIZE*2,win.width,1,0xf);

}

typedef struct
{
    int valid;
    int rowA, colA;
    int rowB, colB;
    int rowC, colC;
} stPath;

static stPath paths[] =
{
    { true,   0, 0,   0, 1,   0, 2 },   // top row
    { true,   1, 0,   1, 1,   1, 2 },   // middle row
    { true,   2, 0,   2, 1,   2, 2 },   // bottom row
    
    { true,   0, 0,   1, 0,   2, 0 },   // left column
    { true,   0, 1,   1, 1,   2, 1 },   // middle column
    { true,   0, 2,   1, 2,   2, 2 },   // right column
    
    { true,   0, 0,   1, 1,   2, 2 },   // TL to BR diagonal
    { true,   0, 2,   1, 1,   2, 0 },   // TR to BL diagonal
    
    { false,  0, 0,   0, 0,   0, 0 }    // end of list
};

bool check_win(char grid[3][3])
{

	// assumes that board array uses 'X' 'O' and <sp> to mark each position
    
    for (stPath *pptr = paths; pptr->valid; pptr++)
    {
        int a = grid[pptr->rowA][pptr->colA];
        int b = grid[pptr->rowB][pptr->colB];
        int c = grid[pptr->rowC][pptr->colC];
        
        if (a == b && b == c && a != ' ')
            return (a == 'X') ? 1 : -1;
    }
    
    return 0;    // no winner yet

}
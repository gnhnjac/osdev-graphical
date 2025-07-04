#include "screen.h"
#include "low_level.h"
#include "memory.h"
#include "std.h"
#include "strings.h"
#include "mouse.h"
#include "timer.h"
#include "math.h"
#include "graphics.h"
#include "scheduler.h"

int screen_initialized = 0;

static uint16_t cursor_input_row = 0;
static uint16_t cursor_input_col = 0;

static uint32_t cursor_offset_x = 0;
static uint32_t cursor_offset_y = 0;

/* Print a char on the screen at col, row, or at cursor position */
void screen_print_char(const char character, int row, int col, char color) 
{

	if(character == 27 || character == '\r' || !screen_initialized) // don't print escape character
		return;

	if (!color)
		color = 0xF;

	/* Get the video memory offset for the screen location */
	int offset_x = 0;
	int offset_y = 0;
	/* If col and row are non-negative, use them for offset. */
	if ( col >= 0 && row >= 0) {
		offset_x = get_screen_x(col);
		offset_y = get_screen_y(row);
		/* Otherwise, use the current cursor position. */
	} else {
		offset_x = get_cursor_col();
		offset_y = get_cursor_row();
	}

	if (character == '\b')
	{

		// handle backspace
		offset_x-=1;
		if (offset_x < 0)
		{
			offset_y -= 1;
			offset_x = MAX_COLS-1;
		}
		if (offset_y < TOP) // can't be lower than start of screen.
		{

			offset_x = 0;
			offset_y = TOP;

		}
		fill_rect(get_screen_x(offset_x),get_screen_y(offset_y),CHAR_WIDTH,CHAR_HEIGHT,0);
		set_cursor_coords(offset_x,offset_y);
		return;
	}
	else if (character == '\t')
	{

		// handle tab
		screen_putchar(' ');
		screen_putchar(' ');
		screen_putchar(' ');
		screen_putchar(' ');
		return;

	}

	// Make scrolling adjustment, for when we reach the bottom
	// of the screen.
	offset_y = handle_scrolling(offset_y);

	// If we see a newline character, set offset to the end of
	// current row, so it will be advanced to the first col
	// of the next row.
	if (character == '\n') {
		offset_x = 0;
		offset_y += 1;
	} else {
		fill_rect(get_screen_x(offset_x),get_screen_y(offset_y),CHAR_WIDTH,CHAR_HEIGHT,0);
		display_psf1_8x16_char(character,get_screen_x(offset_x),get_screen_y(offset_y),color);
		offset_x += 1;
	}

	//if it reached the scroll bar set it to the end of the line so the next character will be in the new line.
	if (offset_x >= MAX_COLS)
	{
		offset_x = 0;
		offset_y += 1;
	}
	// Update the cursor position on the screen device.
	set_cursor_coords(offset_x,offset_y);
}

int get_screen_offset(int row, int col) 
{

	return (row * MAX_COLS * CHAR_HEIGHT * CHAR_WIDTH + col * CHAR_WIDTH);

}

int get_screen_x(int col)
{

	return col * CHAR_WIDTH;

}

int get_screen_y(int row)
{

	return row * CHAR_HEIGHT;

}

int get_logical_row(int y)
{

	return y/CHAR_HEIGHT;

}

int get_logical_col(int x)
{

	return x/CHAR_WIDTH;

}

int get_cursor_row()
{

	return cursor_offset_y;

}

int get_cursor_col()
{

	return cursor_offset_x;

}

int get_cursor() 
{
	return cursor_offset_y*MAX_COLS + cursor_offset_x;
}

void set_cursor(int offset) 
{
	cursor_offset_x = offset%MAX_COLS;
	cursor_offset_y = offset/MAX_COLS;
}

void set_cursor_input_coords(uint8_t row, uint8_t col)
{

	cursor_input_row = row;
	cursor_input_col = col;

}

void set_cursor_input_row(uint8_t row)
{

	cursor_input_row = row;

}

void attach_cursor_to_input()
{

	set_cursor_coords(cursor_input_col,cursor_input_row);

}

void set_cursor_coords(int col, int row)
{
	cursor_offset_x = col;
	cursor_offset_y = row;

}

void set_cursor_row(int row)
{

	cursor_offset_y = row;

}

void screen_putchar(char c)
{

	screen_print_char(c, -1, -1, 0xF);

}

void screen_putchar_at(char c, int row, int col, int attr_byte)
{

	if ( col >= 0 && row >= 0) {
		set_cursor_coords(col, row);
	}

	screen_print_char(c, -1, -1, attr_byte);

}

void screen_print_at(const char *msg, int row, int col, int attr_byte) 
{
	if ( col >= 0 && row >= 0) {
		set_cursor_coords(col, row);
	}

	while (*msg)
	{

		screen_print_char(*msg++, -1, -1, attr_byte);
	}
}

void screen_print(const char *msg) 
{
	screen_print_at(msg, -1, -1, 0);
}

void screen_print_color(const char *msg, int attr_byte)
{

	screen_print_at(msg, -1, -1, attr_byte);

}


int screen_printf(const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);
	screen_vprintf(fmt,valist);

}

int screen_vprintf(const char *fmt, va_list valist)
{

	const char *orig = fmt;

	while (*fmt)
	{

		if (*fmt == '%' && ((*(fmt-1) != '\\' && fmt != orig) || fmt == orig))
		{

			char buff[30];

			switch(*++fmt)
			{

				case 'd':

					int_to_str(va_arg(valist, int), buff, 10);
					screen_print(buff);
					break;

				case 'u':

					uint_to_str(va_arg(valist, int), buff, 10);
					screen_print(buff);
					break;

				case 'U':

					uint_to_str(va_arg(valist, int), buff, 16);
					screen_print(buff);
					break;

				case 'c':

					screen_putchar((char)va_arg(valist, int));
					break;

				case 's':
					screen_print((char *)va_arg(valist, int));
					break;

				case 'x':
					int_to_str(va_arg(valist, int), buff, 16);
					screen_print(buff);
					break;

				case 'X':

					byte_to_str((uint8_t)va_arg(valist, int), buff, 16);
					screen_print(buff);
					break;

				case 'b':
					int_to_str(va_arg(valist, int), buff, 2);
					screen_print(buff);
					break;



				default:

					screen_printf("Unknown format type \\%%c", fmt);
					return STDERR;
			}

		}
		else if(*fmt == '\\' && *(fmt+1) == '%')
		{	
			fmt++;
			continue;
		}
		else
		{

			screen_putchar(*fmt);

		}

		fmt++;

	}

	return STDOK;

}

// clear screen without gui elements
void clear_viewport() 
{

	disable_mouse();

	fill_rect(0,TOP*CHAR_HEIGHT,PIXEL_WIDTH,PIXEL_HEIGHT-TOP*CHAR_HEIGHT,0);

	enable_mouse();
}

// clear whole screen
void clear_screen() 
{

	fill_rect(0,0,PIXEL_WIDTH,PIXEL_HEIGHT,BG_COLOR);
}


int handle_scrolling(int offset_y)
{

	// deprecated

	return 1;

	// if (offset_y < MAX_ROWS)
	// {
	// 	return offset_y;
	// }

	// disable_mouse();


	// uint32_t char_line_size = PIXEL_WIDTH*CHAR_HEIGHT*3;
	// uint32_t scroll_diff_size = SCROLL_ROWS*char_line_size;
	// uint8_t *vram = (uint8_t *)VIDEO_ADDRESS + char_line_size;

	// uint8_t prev_mask = 0;
	
	// outb(0x3C4, MEMORY_PLANE_WRITE_ENABLE);
	// outb(0x3C5, 0xF);

	// for (int i = TOP+SCROLL_ROWS; i < MAX_ROWS; i++)
	// {

	// 	uint8_t *tmp = vram;

	// 	for (int j = 0; j < CHAR_HEIGHT; j++)
	// 	{

	// 		for (int k = 0; k < PIXEL_WIDTH; k++)
	// 		{

	// 			uint8_t mask = 0x80>>(k%PIXELS_PER_BYTE);
	// 			uint32_t off = k/PIXELS_PER_BYTE;
	// 			uint8_t *color_byte = (uint8_t *)(tmp+off+scroll_diff_size);

	// 			tmp[off] &= ~mask;

	// 			uint8_t color = 0;

	// 			tmp[off] |= mask;

	// 			//set_pixel(j,get_screen_y(i-1)+k,get_pixel(j,get_screen_y(i)+k));

	// 		}

	// 		tmp += PIXEL_WIDTH *3;

	// 	}

	// 	vram += char_line_size;

	// }

	// fill_rect(0,PIXEL_HEIGHT-CHAR_HEIGHT*SCROLL_ROWS,PIXEL_WIDTH,CHAR_HEIGHT*SCROLL_ROWS,0);

	// offset_y-=SCROLL_ROWS;

	// enable_mouse();
	// cursor_input_row -= SCROLL_ROWS;

	// return offset_y;

}

void clear_line(char *line)
{

	for (int i = 0; i < MAX_COLS; i++)
	{

		line[i*2] = ' ';
		line[i*2 + 1] = 0xF;

	}

}

void init_screen()
{

	cursor_offset_x = 0;
	cursor_offset_y = 0;

	// clear whole screen
	clear_screen();

	// // initialize top bar
	display_psf1_8x16_char_bg('a', get_screen_x(0), get_screen_y(0), WIN_FRAME_COLOR,0);
	display_psf1_8x16_char_bg('k', get_screen_x(1), get_screen_y(0), WIN_FRAME_COLOR,0);
	display_psf1_8x16_char_bg('o', get_screen_x(2), get_screen_y(0), WIN_FRAME_COLOR,0);
	display_psf1_8x16_char_bg('s', get_screen_x(3), get_screen_y(0), WIN_FRAME_COLOR,0);

	for (int i = 4; i < MAX_COLS; i++)
	{

		display_psf1_8x16_char_bg(' ', get_screen_x(i), get_screen_y(0), WIN_FRAME_COLOR,0);

	}

	set_cursor_coords(0,TOP);

	screen_initialized = 1;

}

int is_screen_initialized()
{
	return screen_initialized;
}
#include <stdarg.h>
#include "screen.h"
#include "low_level.h"
#include "memory.h"
#include "std.h"
#include "strings.h"
#include "mouse.h"
#include "timer.h"
#include "math.h"
#include "graphics.h"

char logo_pixels[];

char top_buffer[BUFFER_TOP_ROWS][2*MAX_COLS];

char bot_buffer[BUFFER_BOT_ROWS][2*MAX_COLS];

int screen_initialized = 0;

int scroll_index = 0;

bool scrolling_enabled = true;

static uint16_t cursor_input_row = 0;
static uint16_t cursor_input_col = 0;

static uint32_t cursor_offset_x = 0;
static uint32_t cursor_offset_y = 0;

/* Print a char on the screen at col, row, or at cursor position */
void print_char(const char character, int row, int col, char color) 
{

	if(character == 27 || character == '\r') // don't print escape character
		return;

	if (!color)
		color = WHITE_ON_BLACK;

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
			offset_x = RIGHT-1;
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
		putchar(' ');
		putchar(' ');
		putchar(' ');
		putchar(' ');
		return;

	}

	// Make scrolling adjustment, for when we reach the bottom
	// of the screen.
	//offset = handle_scrolling(offset);

	// If we see a newline character, set offset to the end of
	// current row, so it will be advanced to the first col
	// of the next row.
	if (character == '\n') {
		offset_x = 0;
		offset_y += 1;
	// Otherwise , write the character and its attribute byte to
	// video memory at our calculated offset .
	} else {
		fill_rect(get_screen_x(offset_x),get_screen_y(offset_y),CHAR_WIDTH,CHAR_HEIGHT,0);
		display_psf1_8x16_char(character,get_screen_x(offset_x),get_screen_y(offset_y),color);
		offset_x += 1;
	}

	//if it reached the scroll bar set it to the end of the line so the next character will be in the new line.
	if (offset_x >= RIGHT)
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

void putchar(char c)
{

	print_char(c, -1, -1, WHITE_ON_BLACK);

}

void print_at(const char *msg, int row, int col, int attr_byte) 
{
	if ( col >= 0 && row >= 0) {
		set_cursor_coords(row, col);
	}

	while (*msg)
	{

		print_char(*msg++, -1, -1, attr_byte);
	}
}

void print(const char *msg) 
{
	print_at(msg, -1, -1, 0);
}

void print_color(const char *msg, int attr_byte)
{

	print_at(msg, -1, -1, attr_byte);

}


int printf(const char *fmt, ...)
{

	va_list valist;
	va_start(valist, fmt);

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
					print(buff);
					break;

				case 'u':

					uint_to_str(va_arg(valist, int), buff, 10);
					print(buff);
					break;

				case 'U':

					uint_to_str(va_arg(valist, int), buff, 16);
					print(buff);
					break;

				case 'c':

					putchar((char)va_arg(valist, int));
					break;

				case 's':
					print((char *)va_arg(valist, int));
					break;

				case 'x':
					int_to_str(va_arg(valist, int), buff, 16);
					print(buff);
					break;

				case 'X':

					byte_to_str((uint8_t)va_arg(valist, int), buff, 16);
					print(buff);
					break;

				case 'b':
					int_to_str(va_arg(valist, int), buff, 2);
					print(buff);
					break;



				default:

					printf("Unknown format type \\%%c", fmt);
					return STDERR;
			}

		}
		else if(*fmt == '\\')
		{	
			fmt++;
			continue;
		}
		else
		{

			putchar(*fmt);

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

	fill_rect(0,0,PIXEL_WIDTH,PIXEL_HEIGHT,0);
}


int handle_scrolling(int cursor_offset)
{

	if (cursor_offset < MAX_ROWS*MAX_COLS*2)
	{
		return cursor_offset;
	}
	int cursor_row = get_cursor_row();
	int cursor_col = get_cursor_col();
	hide_scroll_bar();
	disable_mouse();
	char *first_line = (char *)(VIDEO_ADDRESS + get_screen_offset(TOP, 0));
	push_to_buffer((char *)top_buffer, first_line, BUFFER_TOP_ROWS);

	for (int i = TOP+1; i < MAX_ROWS; i++)
	{

		memcpy((char *)(VIDEO_ADDRESS + get_screen_offset(i-1, 0)), (char *)(VIDEO_ADDRESS + get_screen_offset(i, 0)),2*MAX_COLS);

	}

	char *last_line = (char *)(VIDEO_ADDRESS + get_screen_offset(MAX_ROWS-1, 0));

	for (int i = 0; i < MAX_COLS*2; i++)
	{

		*(last_line + i) = 0;

	}

	cursor_offset -= 2*MAX_COLS;

	if (scroll_index != BUFFER_TOP_ROWS)
		scroll_index++;
	draw_scroll_bar();
	enable_mouse();
	set_cursor_coords(cursor_row-1,cursor_col);
	cursor_input_row -=1;

	return cursor_offset;

}

void display_logo()
{	
	clear_screen();
	char block = '#';
	int attribute_byte = 0x3B; // Blinking Blue

	for(int i = 0; i < 25; i++)
	{

		for (int j = 0; j < 80; j++)
		{

			if (logo_pixels[i*80 + j] == 1)
			{
				print_char(block, i, j, attribute_byte);
			}
			else
			{

				print_char(' ', i, j, attribute_byte);

			}
			if (j%5==0)
				wait_milliseconds(1);
		}

	}
	char *msg = "Welcome to my operating system!";
	set_cursor_coords(22, 40 - strlen(msg)/2 - 1);
	print_color(msg, 0x3B);
	timer_wait(3);
	timer_wait(2);
	return;

}

void clear_line(char *line)
{

	for (int i = 0; i < MAX_COLS; i++)
	{

		line[i*2] = ' ';
		line[i*2 + 1] = WHITE_ON_BLACK;

	}

}

void init_screen()
{

	cursor_offset_x = 0;
	cursor_offset_y = 0;

	// clear whole screen
	clear_screen();

	// // initialize top bar
	display_psf1_8x16_char_bg('a', get_screen_x(0), get_screen_y(0), 0xf,0);
	display_psf1_8x16_char_bg('k', get_screen_x(1), get_screen_y(0), 0xf,0);
	display_psf1_8x16_char_bg('o', get_screen_x(2), get_screen_y(0), 0xf,0);
	display_psf1_8x16_char_bg('s', get_screen_x(3), get_screen_y(0), 0xf,0);

	for (int i = 4; i < MAX_COLS; i++)
	{

		display_psf1_8x16_char_bg(' ', get_screen_x(i), get_screen_y(0), 0xf,0);

	}

	set_cursor_coords(0,TOP);

	// draw_scroll_bar();

	// screen_initialized = 1;

}

int is_screen_initialized()
{
	return screen_initialized;
}

void switch_top_bar_value(int offset, int len)
{
	return;

}

void push_to_buffer(char *buffer, char *line, int buffer_rows)
{

	for (int i = buffer_rows-2; i >= 0; i--)
	{

		memcpy(&buffer[(i+1)*2*MAX_COLS], &buffer[i*2*MAX_COLS],2*MAX_COLS);

	}
	memcpy(buffer,line, 2*MAX_COLS);

}

void pop_from_buffer(char *buffer,char *dst_buffer, int buffer_rows)
{
	memcpy(dst_buffer,buffer,2*MAX_COLS);
	for (int i = 1; i < buffer_rows; i++)
	{

		memcpy(&buffer[(i-1)*2*MAX_COLS], &buffer[i*2*MAX_COLS],2*MAX_COLS);

	}

	clear_line(&buffer[(buffer_rows-1)*2*MAX_COLS]); // clear last buffer line

}

void scroll_up()
{
	if (scroll_index == 0 || !scrolling_enabled)
		return;
	int cursor_row = get_cursor_row();
	int cursor_col = get_cursor_col();
	hide_scroll_bar();
	char *last_line = (char *)(VIDEO_ADDRESS + get_screen_offset(MAX_ROWS-1, 0));
	push_to_buffer((char *)bot_buffer, last_line, BUFFER_BOT_ROWS);

	disable_mouse();
	for (int i =  MAX_ROWS-2; i >= TOP; i--)
	{

		memcpy((char *)(VIDEO_ADDRESS + get_screen_offset(i+1, 0)), (char *)(VIDEO_ADDRESS + get_screen_offset(i, 0)),2*MAX_COLS);

	}

	char *first_line = (char *)(VIDEO_ADDRESS + get_screen_offset(TOP, 0));

	pop_from_buffer((char *)top_buffer, first_line, BUFFER_TOP_ROWS);

	enable_mouse();

	scroll_index--;
	draw_scroll_bar();
	set_cursor_coords(cursor_row+1,cursor_col);
	cursor_input_row +=1;

}

void scroll_down()
{
	if (scroll_index == BUFFER_TOP_ROWS || !scrolling_enabled)
		return;
	int cursor_row = get_cursor_row();
	int cursor_col = get_cursor_col();
	hide_scroll_bar();
	disable_mouse();

	char *first_line = (char *)(VIDEO_ADDRESS + get_screen_offset(TOP, 0));
	push_to_buffer((char *)top_buffer, first_line, BUFFER_TOP_ROWS);

	for (int i = TOP+1; i < MAX_ROWS; i++)
	{

		memcpy((char *)(VIDEO_ADDRESS + get_screen_offset(i-1, 0)), (char *)(VIDEO_ADDRESS + get_screen_offset(i, 0)),2*MAX_COLS);

	}

	char *last_line = (char *)(VIDEO_ADDRESS + get_screen_offset(MAX_ROWS-1, 0));
	pop_from_buffer((char *)bot_buffer, last_line, BUFFER_BOT_ROWS);

	enable_mouse();

	scroll_index++;
	draw_scroll_bar();
	set_cursor_coords(cursor_row-1,cursor_col);
	cursor_input_row -=1;

}

void draw_scroll_bar()
{
	for (int i = TOP; i < MAX_ROWS; i++)
	{

		print_char(' ', i, 79, WHITE_ON_BLACK);
		print_char(' ', i, 78, 0x70);

	}

	int row = remap(scroll_index, 0, BUFFER_TOP_ROWS, TOP, MAX_ROWS-1);
	int col = 79;
	print_char(' ', row, col, 0x70);

}

void hide_scroll_bar()
{

	for (int i = TOP; i < MAX_ROWS; i++)
	{

		print_char(' ', i, 79, WHITE_ON_BLACK);
		print_char(' ', i, 78, WHITE_ON_BLACK);

	}

}

void set_scroll_pos_mouse(int pos_index)
{

	if(!scrolling_enabled)
		return;

	int target_scroll_index = remap(pos_index, TOP, MAX_ROWS-1, 0, BUFFER_TOP_ROWS);

	while(target_scroll_index > scroll_index)
		scroll_down();
	while(target_scroll_index < scroll_index)
		scroll_up();
	
}

void set_scroll_pos(int target_scroll_index)
{

	if(!scrolling_enabled)
		return;

	while(target_scroll_index > scroll_index)
		scroll_down();
	while(target_scroll_index < scroll_index)
		scroll_up();

}

void fit_to_scroll(int target_scroll_index)
{

	if(!scrolling_enabled)
		return;

	while(target_scroll_index - scroll_index >= VIEWPORT_ROWS)
		scroll_down();
	while(target_scroll_index < scroll_index)
		scroll_up();

}


int get_scroll_index()
{

	return scroll_index;

}

void enable_scrolling()
{

	scrolling_enabled = true;

}

void disable_scrolling()
{

	scrolling_enabled = false;

}

char logo_pixels[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
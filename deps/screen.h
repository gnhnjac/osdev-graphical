#include <stdint.h>

#define VIDEO_ADDRESS 0xa0000
#define MAX_ROWS 30
#define MAX_COLS 80

// top bar constants
#define TOP 1
#define TIME_OFF 70

#define SCROLL_ROWS 10

//refs
void print_char(const char character, int row, int col, char color);
int get_screen_offset(int row, int col);
int get_screen_x(int col);
int get_screen_y(int row);
int get_logical_row(int y);
int get_logical_col(int x);
int get_cursor_row();
int get_cursor_col();
int get_cursor();
void set_cursor(int offset);
void set_cursor_input_coords(uint8_t row, uint8_t col);
void set_cursor_input_row(uint8_t row);
void attach_cursor_to_input();
void set_cursor_coords(int col, int row);
void set_cursor_row(int row);
void putchar(char c);
void putchar_at(char c, int row, int col, int attr_byte);
void print_at(const char *msg, int row, int col, int attr_byte);
void print(const char *msg);
void print_color(const char *msg, int attr_byte);
int printf(const char *fmt, ...);
void clear_viewport();
void clear_screen();
int handle_scrolling(int offset_y);
void clear_line(char *line);
void init_screen();
int is_screen_initialized();
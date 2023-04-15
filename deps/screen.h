#include <stdint.h>

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Default color scheme
#define WHITE_ON_BLACK 0x0f

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4 // Internal register index
#define REG_SCREEN_DATA 0x3D5 // Internal register data

// scroll buffer values
#define BUFFER_TOP_ROWS 0xFF
#define BUFFER_BOT_ROWS 0xFF

// top bar constants
#define TOP 2
#define VIEWPORT_ROWS (MAX_ROWS-TOP)
#define CTRL_OFF 0
#define SHIFT_OFF 5
#define ALT_OFF 11
#define CAPS_OFF 15
#define CYCLE_OFF 34
#define VFS_OFF 45
//refs
void print_char(const char character, int row, int col, char attribute_byte);
void blink_screen();
void unblink_screen();
int get_screen_offset(int row, int col);
int get_cursor_row();
int get_cursor_col();
int get_cursor();
void set_cursor(int offset);
void set_cursor_input_coords(uint8_t row, uint8_t col);
void attach_cursor_to_input();
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void set_cursor_coords(int row, int col);
void set_cursor_row(int row);
void putchar(char c);
void print_at(const char *msg, int row, int col, int attr_byte);
void print(const char *msg);
void print_color(const char *msg, int attr_byte);
int printf(const char *fmt, ...);
void clear_viewport();
void clear_screen();
int handle_scrolling(int cursor_offset);
void display_logo();
void clear_line(char *line);
void init_screen();
int is_screen_initialized();
void switch_top_bar_value(int offset, int len);
void set_timer_ticks(unsigned int ticks);
void push_to_buffer(char buffer[][2*MAX_COLS], char *line, int buffer_rows);
void pop_from_buffer(char buffer[][2*MAX_COLS],char *dst_buffer, int buffer_rows);
void scroll_up();
void scroll_down();
void draw_scroll_bar();
void hide_scroll_bar();
void set_scroll_pos_mouse(int pos_index);
void set_scroll_pos(int target_scroll_index);
void fit_to_scroll(int target_scroll_index);
int get_scroll_index();
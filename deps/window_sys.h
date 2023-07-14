#include <stdint.h>
#include <stdbool.h>

typedef struct _window
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	void *w_buffer;
	char *w_name;
	int id;
	bool closable;
	struct _window *next;

} WINDOW, *PWINDOW;

typedef struct _inputInfo
{

	uint32_t cursor_input_row;
	uint32_t cursor_input_col;
	uint32_t cursor_offset_x;
	uint32_t cursor_offset_y;

} INPINFO, *PINPINFO;

#define TITLE_BAR_HEIGHT 20
#define WIN_FRAME_SIZE 1
#define WIN_FRAME_COLOR 0xF
#define TITLE_NAME_COLOR 0

//refs
PWINDOW winsys_get_working_window();
int winsys_get_free_id();
void winsys_init();
PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name, bool is_closable);
int winsys_set_working_window(int wid);
void winsys_paint_window_frame(PWINDOW win);
void winsys_paint_window(PWINDOW win);
void winsys_display_window(PWINDOW win);
void winsys_clear_window(PWINDOW win);
void winsys_clear_window_frame(PWINDOW win);
void winsys_clear_whole_window(PWINDOW win);
bool winsys_check_collide(PWINDOW w1, PWINDOW w2);
bool winsys_check_title_collide(PWINDOW w, int x, int y);
bool winsys_check_close_collide(PWINDOW w, int x, int y);
PWINDOW winsys_get_window_from_title_collision(int x, int y);
void winsys_move_window(PWINDOW win, int x, int y);
void winsys_remove_window(PWINDOW win);
void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor);
void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor);
void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color);
void gfx_clear_win(PWINDOW win);
int gfx_get_win_x(int col);
int gfx_get_win_y(int row);
void gfx_putchar(PWINDOW win, PINPINFO inp_info, char c);
void gfx_print(PWINDOW win, PINPINFO inp_info, char *s);
void gfx_print_color(PWINDOW win, PINPINFO inp_info, char *s, uint8_t color);
void gfx_print_char(PWINDOW win, PINPINFO inp_info, const char character, int row, int col, char color);
void gfx_printf(PWINDOW win, PINPINFO inp_info,const char *fmt, ...);
// int handle_scrolling(int offset_y);
// 	if (offset_y < MAX_ROWS);
// 	for (int i = TOP+SCROLL_ROWS; i < MAX_ROWS; i++);
// 		for (int j = 0; j < CHAR_HEIGHT; j++);
// 			for (int k = 0; k < PIXEL_WIDTH; k++);
// 				if (k%PIXELS_PER_BYTE == 0);
// 					if (*color_byte);
// 					if (*color_byte);
// 					if (*color_byte);
// 					if (*color_byte);
// 					if (prev_mask != 0xF);
// 				if (prev_mask != 0xF);
// 				if (*color_byte & mask);
// 				if (*color_byte & mask);
// 				if (*color_byte & mask);
// 				if (*color_byte & mask);
// 				if (prev_mask != color);
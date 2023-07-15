#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct _inputInfo
{

	uint32_t cursor_input_row;
	uint32_t cursor_input_col;
	uint32_t cursor_offset_x;
	uint32_t cursor_offset_y;

	bool is_taking_input;
	uint32_t input_buffer_index;
	uint32_t input_buffer_limit;
	char *input_buffer;

	bool did_scroll;

} INPINFO, *PINPINFO;

typedef enum _eventType
{

	EVENT_INVALID = 1,
	EVENT_KBD_PRESS = 2,
	EVENT_KBD_RELEASE = 4,
	EVENT_MOUSE_MOVE = 8,
	EVENT_MOUSE_LEFT_CLICK = 16,
	EVENT_MOUSE_RIGHT_CLICK = 32,


} eventType;

typedef enum _generalEventType
{

	GENERAL_EVENT_KBD = 1,
	GENERAL_EVENT_MOUSE = 2,

} generalEventType;

typedef struct _event
{

	eventType event_type;
	uint32_t event_data;

} EVENT, *PEVENT;

#define EVENT_HANDLER_QUEUE_SIZE 10

typedef struct _eventHandler
{

	EVENT events[EVENT_HANDLER_QUEUE_SIZE];
	generalEventType event_mask;

} EVENTHAND, *PEVENTHAND;

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
	EVENTHAND event_handler;
	struct _window *next;

} WINDOW, *PWINDOW;

typedef struct _terminal
{

   PWINDOW term_win;
   PINPINFO term_inp_info;

} terminal;

#define TITLE_BAR_HEIGHT 20
#define WIN_FRAME_SIZE 1
#define WIN_FRAME_COLOR 0x8
#define TITLE_NAME_COLOR 0xF

#define BG_COLOR 0x7

//refs
PWINDOW winsys_get_working_window();
int winsys_get_free_id();
void winsys_init();
PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name, bool is_closable);
int winsys_set_working_window(int wid);
void winsys_paint_window_frame(PWINDOW win);
void winsys_paint_window(PWINDOW win);
void winsys_paint_window_section(PWINDOW win, int x, int y, int width, int height);
void winsys_display_window_section(PWINDOW win, int x, int y, int width, int height);
void winsys_display_window_section_exclude_original(PWINDOW win, PWINDOW orig, int x, int y, int width, int height);
void winsys_display_window(PWINDOW win);
void winsys_display_window_exclude_original(PWINDOW win, PWINDOW orig);
void winsys_display_collided_windows(PWINDOW win);
void winsys_clear_window(PWINDOW win);
void winsys_clear_window_frame(PWINDOW win);
void winsys_clear_whole_window(PWINDOW win);
bool winsys_check_collide(PWINDOW w1, PWINDOW w2);
bool winsys_check_collide_coords(PWINDOW w, int x, int y);
bool winsys_check_title_collide(PWINDOW w, int x, int y);
bool winsys_check_close_collide(PWINDOW w, int x, int y);
PWINDOW winsys_get_window_from_collision(int x, int y);
PWINDOW winsys_get_window_from_title_collision(int x, int y);
void winsys_move_window(PWINDOW win, int x, int y);
void winsys_remove_window(PWINDOW win);
void winsys_enqueue_to_event_handler(PEVENTHAND handler, EVENT e);
EVENT winsys_dequeue_from_event_handler(PEVENTHAND handler);
void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor);
void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor);
void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color);
void gfx_clear_win(PWINDOW win);
int gfx_get_win_x(int col);
int gfx_get_win_y(int row);
int gfx_get_logical_row(int y);
int gfx_get_logical_col(int x);
void gfx_putchar(PWINDOW win, PINPINFO inp_info, char c);
void gfx_print(PWINDOW win, PINPINFO inp_info, char *s);
void gfx_print_color(PWINDOW win, PINPINFO inp_info, char *s, uint8_t color);
void gfx_print_char(PWINDOW win, PINPINFO inp_info, const char character, int row, int col, char color);
void gfx_vprintf(PWINDOW win, PINPINFO inp_info,const char *fmt, va_list valist);
void gfx_printf(PWINDOW win, PINPINFO inp_info,const char *fmt, ...);
void gfx_keyboard_input(PINPINFO inp_info, int col, int row, char *buffer, int bf_size);
int gfx_keyboard_input_character(PINPINFO inp_info, char character);
int gfx_handle_scrolling(PWINDOW win, PINPINFO inp_info, int offset_y);
void gfx_open_bmp16(char *path, int wx, int wy);
void gfx_paint_bmp16(PWINDOW win, char *path, int x, int y);
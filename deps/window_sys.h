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
	int x;
	int y;
	int width;
	int height;
	void *w_buffer;
	char *w_name;
	int id;
	int parent_pid;
	bool is_user;
	bool closable;
	bool has_frame;
	EVENTHAND event_handler;
	struct _window *next;

} WINDOW, *PWINDOW;

typedef struct _terminal
{

   PWINDOW term_win;
   PINPINFO term_inp_info;

} terminal;

typedef enum _winsysOperation
{

	WINSYS_EMPTY = 0,
	WINSYS_MOVE_MOUSE = 1,
	WINSYS_DISPLAY_WINDOW_SECTION = 2,
	WINSYS_DISPLAY_WINDOW = 3,
	WINSYS_MOVE_WINDOW = 4,
	WINSYS_SET_WORKING_WINDOW = 5,
	WINSYS_REMOVE_WINDOW = 6,

} winsysOperation;

typedef struct _winsys_op
{

	winsysOperation op;
	int wid;
	int x;
	int y;
	int w;
	int h;

} WINSYSOP, *PWINSYSOP;

typedef enum _keyCodes {
    K_NULL = 0,
    K_ESCAPE = 1,
    K_ONE = 2,
    K_TWO = 3,
    K_THREE = 4,
    K_FOUR = 5,
    K_FIVE = 6,
    K_SIX = 7,
    K_SEVEN = 8,
    K_EIGHT = 9,
    K_NINE = 10,
    K_ZERO = 11,
    K_DASH = 12,
    K_EQUALS = 13,
    K_BACKSPACE = 14,
    K_TAB = 15,
    K_Q = 16,
    K_W = 17,
    K_E = 18,
    K_R = 19,
    K_T = 20,
    K_Y = 21,
    K_U = 22,
    K_I = 23,
    K_O = 24,
    K_P = 25,
    K_OPEN_BRACKETS = 26,
    K_CLOSE_BRACKETS = 27,
    K_NEWLINE = 28,
    K_CTRL = 29,
    K_A = 30,
    K_S = 31,
    K_D = 32,
    K_F = 33,
    K_G = 34,
    K_H = 35,
    K_J = 36,
    K_K = 37,
    K_L = 38,
    K_SEMICOLON = 39,
    K_APOSTROPHE = 40,
    K_BACKTICK = 41,
    K_LSHIFT = 42,
    K_BACK_SLASH = 43,
    K_Z = 44,
    K_X = 45,
    K_C = 46,
    K_V = 47,
    K_B = 48,
    K_N = 49,
    K_M = 50,
    K_COMMA = 51,
    K_PERIOD = 52,
    K_FOR_SLASH = 53,
    K_RSHIFT = 54,
    K_ASTERISK = 55,
    K_ALT = 56,
    K_SPACE = 57,
    K_CAPS = 58,
    K_F1 = 59,
    K_F2 = 60,
    K_F3 = 61,
    K_F4 = 62,
    K_F5 = 63,
    K_F6 = 64,
    K_F7 = 65,
    K_F8 = 66,
    K_F9 = 67,
    K_F10 = 68,
    K_NUMLOCK = 69,
    K_SCRLOCK = 70,
    K_HOME = 71,
    K_UP = 72,
    K_PAGEUP = 73,
    K_MINUS = 74,
    K_LEFT = 75,
    K_RIGHT = 77,
    K_PLUS = 78,
    K_END = 79,
    K_DOWN = 80,
    K_PAGEDOWN = 81,
    K_INSERT = 82,
    K_DEL = 83,
    K_F11 = 87,
    K_F12 = 88,
} keyCodes;

#define WINSYS_QUEUE_SIZE 20

#define TITLE_BAR_HEIGHT 30
#define WIN_FRAME_SIZE 2
#define WIN_FRAME_COLOR 0x555555
#define TITLE_NAME_COLOR 0xFFFFFF
#define WORKING_TITLE_COLOR 0x5555ff

#define BG_COLOR 0xa8a8a8

//refs
void winsys_disable_mouse();
void winsys_save_to_mouse_buffer();
void winsys_print_mouse();
void winsys_clear_mouse();
void winsys_enable_mouse();
void winsys_move_mouse_operation(int x, int y);
void winsys_do_operation();
void winsys_listener();
WINSYSOP winsys_dequeue_from_winsys_listener();
void winsys_enqueue_to_winsys_listener(WINSYSOP operation);
void winsys_enqueue_to_winsys_listener_if_possible(WINSYSOP operation);
void winsys_enqueue_to_winsys_listener_if_possible_lockless(WINSYSOP operation);
void winsys_move_mouse(int new_x, int new_y);
PWINDOW winsys_get_working_window();
int winsys_get_free_id();
PWINDOW winsys_get_window_by_id(int wid);
void winsys_init();
uint32_t get_win_page_amt(PWINDOW win);
PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name, bool is_closable);
void winsys_create_win_user(PWINDOW local_win, int x, int y, int width, int height);
void winsys_set_working_window(int wid);
int winsys_set_working_window_operation(int wid);
void winsys_paint_window_frame(PWINDOW win);
void winsys_paint_window(PWINDOW win);
void winsys_paint_window_section(PWINDOW win, int x, int y, int width, int height);
void winsys_display_window_section(PWINDOW win, int x, int y, int width, int height);
void winsys_display_window_section_operation(PWINDOW win, int x, int y, int width, int height);
void winsys_display_window(PWINDOW win);
void winsys_display_window_if_possible(PWINDOW win);
void winsys_display_window_if_possible_lockless(PWINDOW win);
void winsys_display_window_operation(PWINDOW win);
void winsys_display_collided_windows(PWINDOW win);
void winsys_clear_window(PWINDOW win);
void winsys_clear_window_frame(PWINDOW win);
void winsys_clear_whole_window(PWINDOW win);
bool winsys_check_collide(PWINDOW w1, PWINDOW w2);
bool winsys_check_collide_whole(PWINDOW w1, PWINDOW w2);
bool winsys_check_collide_win_rect(PWINDOW w1, int w2_x, int w2_y, int w2_w, int w2_h);
bool winsys_check_collide_rect_rect(int w1_x, int w1_y, int w1_w, int w1_h, int w2_x, int w2_y, int w2_w, int w2_h);
bool winsys_check_collide_coords(PWINDOW w, int x, int y);
bool winsys_check_title_collide(PWINDOW w, int x, int y);
bool winsys_check_close_collide(PWINDOW w, int x, int y);
PWINDOW winsys_get_window_from_collision(int x, int y);
PWINDOW winsys_get_window_from_title_collision(int x, int y);
void winsys_move_window(PWINDOW win, int x, int y);
void winsys_move_window_through_mouse(PWINDOW win, int x, int y);
void winsys_move_window_operation(PWINDOW win, int x, int y);
void winsys_remove_window(PWINDOW win);
void winsys_remove_window_operation(PWINDOW win);
void winsys_remove_windows_by_pid(int pid);
void winsys_remove_window_user(PWINDOW win);
void winsys_enqueue_to_event_handler(PEVENTHAND handler, EVENT e);
EVENT winsys_dequeue_from_event_handler(PEVENTHAND handler);
void winsys_dequeue_from_event_handler_user(PWINDOW win, PEVENT event_buff);
void gfx_paint_char(PWINDOW win, char c, int x, int y, uint32_t fgcolor);
void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint32_t bgcolor, uint32_t fgcolor);
void gfx_set_pixel(PWINDOW win,int x, int y,uint32_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint32_t color);
void gfx_clear_win(PWINDOW win);
int gfx_get_win_x(int col);
int gfx_get_win_y(int row);
int gfx_get_logical_row(int y);
int gfx_get_logical_col(int x);
void gfx_putchar(PWINDOW win, PINPINFO inp_info, char c);
void gfx_print(PWINDOW win, PINPINFO inp_info, char *s);
void gfx_print_color(PWINDOW win, PINPINFO inp_info, char *s, uint32_t color);
void gfx_print_char(PWINDOW win, PINPINFO inp_info, const char character, int row, int col, char color);
void gfx_vprintf(PWINDOW win, PINPINFO inp_info,const char *fmt, va_list valist);
void gfx_printf(PWINDOW win, PINPINFO inp_info,const char *fmt, ...);
void gfx_keyboard_input(PINPINFO inp_info, int col, int row, char *buffer, int bf_size);
int gfx_keyboard_input_character(PINPINFO inp_info, char character);
int gfx_handle_scrolling(PWINDOW win, PINPINFO inp_info, int offset_y);
void gfx_open_bmp16(char *path, int wx, int wy);
void gfx_paint_bmp16(PWINDOW win, char *path, int x, int y);
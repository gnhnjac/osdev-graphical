#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

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
	int parent_pid;
	bool is_user;
	bool closable;
	EVENTHAND event_handler;
	struct _window *next;

} WINDOW, *PWINDOW;

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

#define BMP_SIGNATURE 0x4D42 // BM

#pragma pack(1)

typedef struct _BITMAPFILEHEADER
{

    uint16_t Signature;
    uint32_t FileSize;
    uint32_t Reserved;
    uint32_t DataOffset;

} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER
{

    uint32_t Size; // 40 (size of info header)
    uint32_t Width;
    uint32_t Height;
    uint16_t Planes;
    uint16_t BPP;
    uint32_t Compression;
    uint32_t Imagesize;
    uint32_t XpixelsPerM;
    uint32_t YpixelsPerM;
    uint32_t ColorsUsed;
    uint32_t ImportantColors;

} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#pragma pack()

void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor, void *font_buff);
void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor, void *font_buff);
void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color);
void gfx_clear_win(PWINDOW win);
void gfx_paint_bmp16(PWINDOW win, char *path, int x, int y);
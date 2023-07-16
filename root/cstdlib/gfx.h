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

void gfx_paint_char(PWINDOW win, char c, int x, int y, uint8_t fgcolor, void *font_buff);
void gfx_paint_char_bg(PWINDOW win, char c, int x, int y, uint8_t bgcolor, uint8_t fgcolor, void *font_buff);
void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color);
void gfx_clear_win(PWINDOW win);
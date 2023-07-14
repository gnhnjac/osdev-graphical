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
	struct _window *next;

} WINDOW, *PWINDOW;

#define TITLE_BAR_HEIGHT 20
#define WIN_FRAME_SIZE 2
#define WIN_FRAME_COLOR 0xF
#define TITLE_NAME_COLOR 0

//refs
PWINDOW winsys_get_working_window();
int winsys_get_free_id();
void winsys_init();
PWINDOW winsys_create_win(int x, int y, int width, int height, char *w_name);
int winsys_set_working_window(int wid);
void winsys_paint_window_frame(PWINDOW win);
void winsys_paint_window(PWINDOW win);
void winsys_display_window(PWINDOW win);
void winsys_clear_window(PWINDOW win);
void winsys_clear_window_frame(PWINDOW win);
void winsys_clear_whole_window(PWINDOW win);
bool winsys_check_collide(PWINDOW w1, PWINDOW w2);
void winsys_move_window(PWINDOW win, int x, int y);
void winsys_remove_window(PWINDOW win);
void gfx_set_pixel(PWINDOW win,int x, int y,uint8_t color);
void gfx_fill_rect(PWINDOW win, int x, int y, int width, int height, uint8_t color);
#include "gfx.h"

void print(char *str);
void terminate();
int fork();
void create_window(PWINDOW local_win, int x, int y, int width, int height);
void remove_window(PWINDOW win);
void display_window_section(PWINDOW win, int x, int y, int width, int height);
void load_font(void *buff);
void sleep(uint32_t ms);
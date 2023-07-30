#pragma once

#include "gfx.h"

void print(char *str);
void terminate();
int fork();
void create_window(PWINDOW local_win, int x, int y, int width, int height);
void remove_window(PWINDOW win);
void display_window_section(PWINDOW win, int x, int y, int width, int height);
void load_font(void *buff);
void sleep(uint32_t ms);
void get_window_event(PWINDOW win, PEVENT event_buff);
uintptr_t sbrk(uintptr_t inc);
uint32_t fopen(char *path);
void fread(uint32_t fd, unsigned char* Buffer, unsigned int Length);
void fclose(uint32_t fd);
void suspend();
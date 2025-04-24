#pragma once

#include <stdarg.h>

#define DEBUG_PORT 0xE9

int debug_print(const char *str);
int debug_printf(const char *fmt, ...);
int debug_vprintf(const char *fmt, va_list valist);
int debug_putchar(const char c);
#pragma once

// variable types

typedef unsigned int size_t

// macros

#define NULL 0

//refs
unsigned int atoi(const char *str);
void itoa(int n, char *buf, int base);
void uitoa(unsigned int n, char *buffer, int base);
unsigned int abs(int x);
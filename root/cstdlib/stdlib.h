#pragma once

unsigned int atoi(const char *str);
void itoa(int n, char *buf, int base);
void uitoa(unsigned int n, char *buffer, int base);
unsigned int abs(int x);
void *malloc(unsigned int size);
void *calloc(unsigned int size);
void free(void *addr);
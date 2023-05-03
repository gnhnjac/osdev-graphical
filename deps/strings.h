#include <stdbool.h>
#include <stdint.h>

//refs
void int_to_str_padding(int n, char *buffer, int base, int padding);
void int_to_str(int n, char *buffer, int base);
void byte_to_str(unsigned char n, char *buffer, int base);
uint32_t decimal_to_uint(char *str);
int strlen(char *str);
void to_lower(char *str);
void strip_character(char *str, char character);
void strip_from_start(char *str, char character);
char *seperate_and_take(char* str, char seperator, int index);
int count_substrings(char *str, char seperator);
void strip_from_end(char *str, char character);
bool strcmp(char *s1, char *s2);
int sprintf(char *str, const char *fmt, ...);
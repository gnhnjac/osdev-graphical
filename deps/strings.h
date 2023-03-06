#include <stdbool.h>

//refs
void num_to_str(int n, char *buffer, int base);
int strlen(char *str);
void to_lower(char *str);
void strip_character(char *str, char character);
void strip_from_start(char *str, char character);
char *seperate_and_take(char* str, char seperator, int index);
int count_substrings(char *str, char seperator);
void strip_from_end(char *str, char character);
bool strcmp(char *s1, char *s2);
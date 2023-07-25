#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

void _start()
{
	
	char *hello = "hello world!\n";
	char *lol = malloc(strlen(hello));

	strcpy(lol,hello);

	print(lol);
	terminate();
	__builtin_unreachable();
}

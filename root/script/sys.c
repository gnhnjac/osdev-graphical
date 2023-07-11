#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"

void _start()
{
	
	char *hello = "hello world!\n";

	printf("%s,calculation:%x",hello,(unsigned int)hello+3);

	terminate();
	__builtin_unreachable();
}

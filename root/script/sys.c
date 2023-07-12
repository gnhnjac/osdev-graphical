#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"

void _start()
{
	
	// char *hello = "hello world!\n";

	// printf("%s,calculation:%x",hello,(unsigned int)hello+3);

	int tid = fork();

	if (tid)
	{

		printf("hello from fork thread!\n");
		while(1);

	}
	else
	{

		printf("hello from main thread!\n");

	}

	terminate();
	__builtin_unreachable();
}

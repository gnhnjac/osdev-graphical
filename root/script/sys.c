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

		printf("hello from main thread! created new thread with tid %d",tid);
	}
	else
	{

		printf("hello from fork thread!");
			while(1);

	}

	terminate();
	__builtin_unreachable();
}

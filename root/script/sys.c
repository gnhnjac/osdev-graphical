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
		//printf("hello from main thread! created new thread with tid %d",tid);
		fork();
		fork();
		fork();
		print("sup\n");
		while(1) __asm__("pause");
	}
	else
	{

		printf("hello from first fork thread!\n");
		while(1) __asm__("pause");

	}

	terminate();
	__builtin_unreachable();
}

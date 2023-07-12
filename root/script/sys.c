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
		printf("hello from main thread! forked new thread with tid %d\nTesting fork recursion, calling fork 3 times:\n",tid);
		int tid2 = fork();
		int tid3 = fork();
		int tid4 = fork();
		print("forked, tid2: %d,tid3: %d,tid4: %d\n",tid2,tid3,tid4);
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

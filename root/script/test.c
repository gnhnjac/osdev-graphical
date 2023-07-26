#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

void _start(int argc, char **argv)
{
	printf("\nreceived argc: %U\n",argc);

	while(*argv)
	{

		printf("%s\n",*argv);

		argv++;

	}
	

	terminate();
	__builtin_unreachable();
}
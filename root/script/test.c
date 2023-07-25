#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "string.h"

void _start()
{
	
	uint32_t fd = fopen("a:\\home\\hello.txt");

	char *buff = malloc(100);

	fread(fd,buff,100);

	printf("%s\n",buff);

	terminate();
	__builtin_unreachable();
}

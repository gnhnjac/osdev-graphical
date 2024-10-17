#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "gfx.h"
#include "string.h"

void _main(int argc, char **argv)
{

	if (argc != 2)
	{

		print("INCORRECT FORMAT ERROR: cat FILEPATH");
		terminate();

	}

	char *filepath = argv[1];

	int fd = fopen(filepath);

	if (!fd)
	{

		printf("OPEN ERROR: file %s does not exist / is a directory", filepath);
		terminate();

	}

	char *buff = "x";

	bool success = true;

	while (success)
	{
		success = fread(fd,buff,1);

		if (success)
			printf("%c",*buff);
	}
	

	terminate();
	__builtin_unreachable();
}
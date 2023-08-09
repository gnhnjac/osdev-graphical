#include "syscalls.h"

void print(char *str)
{

	__asm__("mov $0, %eax");
	__asm__("mov %0, %%ebx" : : "m" (str));
	__asm__("int $0x80");

}

void terminate()
{

	__asm__("mov $1, %eax");
	__asm__("int $0x80");
}

int fork()
{
	__asm__("mov $2, %eax");
	__asm__("int $0x80");

	unsigned int tid = 0;

	__asm__("mov %%eax, %0" : "=m" (tid));

	return tid;

}

void create_window(PWINDOW local_win, int x, int y, int width, int height)
{
	__asm__("mov $3, %eax");
	__asm__("mov %0, %%ebx" : : "m" (local_win));
	__asm__("mov %0, %%ecx" : : "m" (x));
	__asm__("mov %0, %%edx" : : "m" (y));
	__asm__("mov %0, %%esi" : : "m" (width));
	__asm__("mov %0, %%edi" : : "m" (height));
	__asm__("int $0x80");

}

void remove_window(PWINDOW win)
{

	__asm__("mov $4, %eax");
	__asm__("mov %0, %%ebx" : : "m" (win));
	__asm__("int $0x80");

}

void display_window_section(PWINDOW win, int x, int y, int width, int height)
{

	__asm__("mov $5, %eax");
	__asm__("mov %0, %%ebx" : : "m" (win));
	__asm__("mov %0, %%ecx" : : "m" (x));
	__asm__("mov %0, %%edx" : : "m" (y));
	__asm__("mov %0, %%esi" : : "m" (width));
	__asm__("mov %0, %%edi" : : "m" (height));
	__asm__("int $0x80");

}

void load_font(void *buff)
{

	__asm__("mov $6, %eax");
	__asm__("mov %0, %%ebx" : : "m" (buff));
	__asm__("int $0x80");

}

void sleep(uint32_t ms)
{

	__asm__("mov $7, %eax");
	__asm__("mov %0, %%ebx" : : "m" (ms));
	__asm__("int $0x80");

}

void get_window_event(PWINDOW win, PEVENT event_buff)
{

	__asm__("mov $8, %eax");
	__asm__("mov %0, %%ebx" : : "m" (win));
	__asm__("mov %0, %%ecx" : : "m" (event_buff));
	__asm__("int $0x80");

}

uintptr_t sbrk(uintptr_t inc)
{

	__asm__("mov $9, %eax");
	__asm__("mov %0, %%ebx" : : "m" (inc));
	__asm__("int $0x80");

	uintptr_t ret = 0;

	__asm__("mov %%eax, %0" : "=m" (ret));

	return ret;

}

uint32_t fopen(char *path)
{

    __asm__("mov $10, %eax");
	__asm__("mov %0, %%ebx" : : "m" (path));
	__asm__("int $0x80");

	uintptr_t fd = 0;

	__asm__("mov %%eax, %0" : "=m" (fd));

	return fd;

}

void fclose(uint32_t fd)
{

    __asm__("mov $11, %eax");
	__asm__("mov %0, %%ebx" : : "m" (fd));
	__asm__("int $0x80");

}

int fread(uint32_t fd, unsigned char* Buffer, unsigned int Length)
{

    __asm__("mov $12, %eax");
	__asm__("mov %0, %%ebx" : : "m" (fd));
	__asm__("mov %0, %%ecx" : : "m" (Buffer));
	__asm__("mov %0, %%edx" : : "m" (Length));
	__asm__("int $0x80");

	uintptr_t return_code = 0;

	__asm__("mov %%eax, %0" : "=m" (return_code));

	return return_code;


}

void suspend()
{

	__asm__("mov $13, %eax");
	__asm__("int $0x80");

}

int exec(char* path, char *args)
{

	__asm__("mov $14, %eax");
	__asm__("mov %0, %%ebx" : : "m" (path));
	__asm__("mov %0, %%ecx" : : "m" (args));
	__asm__("int $0x80");

	uintptr_t pid = 0;

	__asm__("mov %%eax, %0" : "=m" (pid));

	return pid;
}

void printf(const char *fmt, ...)
{


    va_list valist;
    va_start(valist,fmt);

    __asm__("mov $15, %eax");
	__asm__("mov %0, %%ebx" : : "m" (fmt));
	__asm__("mov %0, %%ecx" : : "m" (valist));
	__asm__("int $0x80");

}

void fwrite(uint32_t fd, unsigned char* Buffer, unsigned int Length)
{

    __asm__("mov $16, %eax");
	__asm__("mov %0, %%ebx" : : "m" (fd));
	__asm__("mov %0, %%ecx" : : "m" (Buffer));
	__asm__("mov %0, %%edx" : : "m" (Length));
	__asm__("int $0x80");

}
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
#include "low_level.h"

unsigned char inb( unsigned short port ) {
	// A handy C wrapper function that reads a byte from the specified port
	// "=a" ( result ) means : put AL register in variable RESULT when finished
	// "d" ( port ) means : load EDX with port
	unsigned char result;
	__asm__ ("in %%dx, %%al" : "=a" ( result ) : "d" ( port ));
	return result;
}

void outb( unsigned short port , unsigned char data ) {
	// "a" ( data ) means : load EAX with data
	// "d" ( port ) means : load EDX with port
	__asm__ ("out %%al, %%dx" : :"a" ( data ), "d" ( port ));
}

unsigned short inw( unsigned short port ) {
	unsigned short result;
	__asm__ ("in %%dx, %%ax" : "=a" ( result ) : "d" ( port ));
	return result;
}

void outw( unsigned short port , unsigned short data ) {
	__asm__ ("out %%ax, %%dx" : :"a" ( data ), "d" ( port ));
}

unsigned long inl( unsigned short port ) {
	unsigned long result;
	__asm__ ("inl %%dx, %%eax" : "=a" ( result ) : "d" ( port ));
	return result;
}

void outl( unsigned short port , unsigned long data ) {
	__asm__ ("outl %%eax, %%dx" : :"a" ( data ), "d" ( port ));
}

int cpu_has_msr()
{
   static uint32_t a, d; // eax, edx
   cpuid(1, &a, &d);
   return (d & (1 << 5)) > 0;
}

void cpuid(uint32_t code, uint32_t *a, uint32_t *d) {
    asm volatile("cpuid"
                 : "=a"(*a), "=d"(*d)
                 : "a"(code)
                 : "ecx", "ebx");
}

void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

void set_eax(uint32_t val)
{

	__asm__ ("mov %0, %%eax\n\t"
                     : /* no output */
                     : "a" (val));

}
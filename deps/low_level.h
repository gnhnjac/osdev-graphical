#include <stdint.h>
//refs
unsigned char inb( unsigned short port );
void outb( unsigned short port , unsigned char data );
unsigned short inw( unsigned short port );
void outw( unsigned short port , unsigned short data );
unsigned long inl( unsigned short port );
void outl( unsigned short port , unsigned long data );
int cpu_has_msr();
void cpuid(uint32_t code, uint32_t *a, uint32_t *d);
void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi);
void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi);
void set_eax(uint32_t val);
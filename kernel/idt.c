#include "memory.h"
#include "screen.h"
#include "isrs.h"
#include "idt.h"
#include <stdint.h>

/* Defines an IDT entry */
typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t     reserved;     // Set to zero
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t    limit;
    uint16_t    base_low;
    uint16_t    base_high;
} __attribute__((packed)) idtr_t;

__attribute__((aligned(0x10))) 
idt_entry_t idt[IDT_MAX_DESCRIPTORS]; // Create an array of IDT entries; aligned for performance
idtr_t idtr;

void idt_set_gate(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // offset of code segment descriptor in the GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

extern void idt_load(void);

void idt_install(void);
/* Installs the IDT */
void idt_install()
{
    idtr.base_low = (uint32_t)&idt & 0xFFFF;
    idtr.base_high = (uint32_t)&idt >> 16;
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;
    /* Clear out the entire IDT, initializing it to zeros */
    memset((unsigned char *)&idt, 0, sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS);

    /* Add any new ISRs to the IDT here using idt_set_gate */
    isrs_install();
    idt_load();

}
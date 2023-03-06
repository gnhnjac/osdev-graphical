#define IDT_MAX_DESCRIPTORS 256
//refs
void idt_set_gate(uint8_t vector, void* isr, uint8_t flags);
void idt_install();
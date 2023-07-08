#include "screen.h"
#include "idt.h"
#include "isrs.h"
#include "scheduler.h"
#include "process.h"

/* These are function prototypes for all of the exception
*  handlers: The first 32 entries in the IDT are reserved
*  by Intel, and are designed to service exceptions! */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/* We set the access
*  flags to 0x8E. This means that the entry is present, is
*  running in ring 0 (kernel level), and has the lower 5 bits
*  set to the required '14', which is represented by 'E' in
*  hex. */
void isrs_install()
{
    idt_set_gate(0, (void *)isr0, 0x8E);
    idt_set_gate(1, (void *)isr1, 0x8E);
    idt_set_gate(2, (void *)isr2, 0x8E);
    idt_set_gate(3, (void *)isr3, 0x8E);
    idt_set_gate(4, (void *)isr4, 0x8E);
    idt_set_gate(5, (void *)isr5, 0x8E);
    idt_set_gate(6, (void *)isr6, 0x8E);
    idt_set_gate(7, (void *)isr7, 0x8E);
    idt_set_gate(8, (void *)isr8, 0x8E);
    idt_set_gate(9, (void *)isr9, 0x8E);
    idt_set_gate(10, (void *)isr10, 0x8E);
    idt_set_gate(11, (void *)isr11, 0x8E);
    idt_set_gate(12, (void *)isr12, 0x8E);
    idt_set_gate(13, (void *)isr13, 0x8E);
    idt_set_gate(14, (void *)isr14, 0x8E);
    idt_set_gate(15, (void *)isr15, 0x8E);
    idt_set_gate(16, (void *)isr16, 0x8E);
    idt_set_gate(17, (void *)isr17, 0x8E);
    idt_set_gate(18, (void *)isr18, 0x8E);
    idt_set_gate(19, (void *)isr19, 0x8E);
    idt_set_gate(20, (void *)isr20, 0x8E);
    idt_set_gate(21, (void *)isr21, 0x8E);
    idt_set_gate(22, (void *)isr22, 0x8E);
    idt_set_gate(23, (void *)isr23, 0x8E);
    idt_set_gate(24, (void *)isr24, 0x8E);
    idt_set_gate(25, (void *)isr25, 0x8E);
    idt_set_gate(26, (void *)isr26, 0x8E);
    idt_set_gate(27, (void *)isr27, 0x8E);
    idt_set_gate(28, (void *)isr28, 0x8E);
    idt_set_gate(29, (void *)isr29, 0x8E);
    idt_set_gate(30, (void *)isr30, 0x8E);
    idt_set_gate(31, (void *)isr31, 0x8E);
}

/* This is a simple string array. It contains the message that
*  corresponds to each and every exception. We get the correct
*  message by accessing like:
*  exception_message[interrupt_number] */
char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"

};

char *page_fault_exceptions[] =
{

    "Supervisory process tried to read a non-present page entry",
    "Supervisory process tried to read a page and caused a protection fault",
    "Supervisory process tried to write to a non-present page entry",
    "Supervisory process tried to write a page and caused a protection fault",
    "User process tried to read a non-present page entry",
    "User process tried to read a page and caused a protection fault",
    "User process tried to write to a non-present page entry",
    "User process tried to write a page and caused a protection fault"

};

/* All of our Exception handling Interrupt Service Routines will
*  point to this function. This will tell us what exception has
*  happened! Right now, we simply halt the system by hitting an
*  endless loop. All ISRs disable interrupts while they are being
*  serviced as a 'locking' mechanism to prevent an IRQ from
*  happening and messing up kernel data structures */
void fault_handler(struct regs *r)
{
    /* Is this a fault whose number is from 0 to 31? */
    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
        *  In this tutorial, we will simply halt the system using an
        *  infinite loop */
        //clear_screen();
        if (r->int_no > 18)
            print("Reserved");
        else
            print(exception_messages[r->int_no]);
        printf(" Exception. Error Code: 0b%b\nCode faulted at 0x%U\n",r->err_code,r->eip);

        if (r->int_no == 14)
            print(page_fault_exceptions[r->err_code]);

        if (get_running_process())
            terminateProcess();
        else
            for (;;);
    }
}
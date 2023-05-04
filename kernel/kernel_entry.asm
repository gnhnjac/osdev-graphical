; Ensures that we jump straight into the kernel’s entry function.
; Also contains external functions that have to be written in assembly.
bits 32
extern _kmain
call _kmain ; invoke main () in our C kernel
jmp $ ; Hang forever when we return from the kernel

global _idt_load
extern _idtr

_idt_load:
    lidt [_idtr]
    ret

; EXCEPTION IDT GATES
%assign i 0 
%rep    32 
    global _isr%+i
%assign i i+1 
%endrep

; CUSTOM IDT GATES
; TODO

;  0: Divide By Zero Exception
_isr0:
    cli
    push byte 0    ; A normal ISR stub that pops a dummy error code to keep a
                   ; uniform stack frame
    push byte 0
    jmp isr_common_stub

;  1: Debug Exception
_isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub
    
;  2: Non Maskable Interrupt Exception
_isr2:
   cli
   push byte 0
   push byte 2
   jmp isr_common_stub

; Breakpoint Exception
_isr3:
   cli
   push byte 0
   push byte 3
   jmp isr_common_stub

; Into Detected Overflow Exception
_isr4:
   cli
   push byte 0
   push byte 4
   jmp isr_common_stub

; Out of Bounds Exception
_isr5:
   cli
   push byte 0
   push byte 5
   jmp isr_common_stub

; Invalid Opcode Exception
_isr6:
   cli
   push byte 0
   push byte 6
   jmp isr_common_stub

; No Coprocessor Exception
_isr7:
   cli
   push byte 0
   push byte 7
   jmp isr_common_stub

;  8: Double Fault Exception (With Error Code!)
_isr8:
    cli
    push byte 8        ; This one already pushes an error code onto the stack so we only push the sequential number of the gate.
    jmp isr_common_stub

;  9: Coprocessor Segment Overrun Exception
_isr9:
   cli
   push byte 0
   push byte 9
   jmp isr_common_stub
; Bad TSS Exception
_isr10:
   cli
   push byte 10
   jmp isr_common_stub
; Segment Not Present Exception
_isr11:
   cli
   push byte 11
   jmp isr_common_stub
;  Stack Fault Exception
_isr12:
   cli
   push byte 12
   jmp isr_common_stub
;  General Protection Fault Exception
_isr13:
   cli
   push byte 13
   jmp isr_common_stub
;  Page Fault Exception
_isr14:
   cli
   push byte 14
   jmp isr_common_stub
;  Unknown Interrupt Exception
_isr15:
   cli
   push byte 0
   push byte 15
   jmp isr_common_stub
;  Coprocessor Fault Exception
_isr16:
   cli
   push byte 0
   push byte 16
   jmp isr_common_stub
;  Alignment Check Exception (486+)
_isr17:
   cli
   push byte 17
   jmp isr_common_stub
;  Machine Check Exception (Pentium/586+)
_isr18:
   cli
   push byte 0
   push byte 18
   jmp isr_common_stub
;  19-31: Reserved Exceptions
%assign i 19
%rep 13
_isr%+i:
    cli
    push byte 0
    push byte i
    jmp isr_common_stub
%assign i i+1 
%endrep

; We call a C function in here. We need to let the assembler know
; that '_fault_handler' exists in another file
extern _fault_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10   ; Load the Kernel Data Segment descriptor (16 bytes after start of GDT so 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us the stack
    push eax
    call _fault_handler       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number ()
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; IRQS
%assign i 0 
%rep    16
    global _irq%+i
%assign i i+1 
%endrep

; 32: IRQ0
%assign i 0
%rep    16
_irq%+i:
    cli
    push byte 0 ; these dont push an error so for stack consistency we push 0
    push byte i ; ith interrupt in our isr table
    jmp irq_common_stub
%assign i i+1
%endrep

extern _irq_handler

; This is a stub that we have created for IRQ based ISRs. This calls
; '_irq_handler' in our C code. We need to create this in an 'irq.c'
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, _irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
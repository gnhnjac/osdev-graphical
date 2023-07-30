; Ensures that we jump straight into the kernel’s entry function.
; Also contains external functions that have to be written in assembly.
bits 32
extern _kmain
call _kmain ; invoke main () in our C kernel
jmp $ ; Hang forever when we return from the kernel

global _read_eip

_read_eip:
   pop eax
   jmp eax

; vmm and pmm data

global _vmmngr_flush_tlb_entry

_vmmngr_flush_tlb_entry:
   push ebp
   mov ebp, esp
   %define addr [ebp+8]
   cli
   invlpg addr
   sti

   pop ebp
   ret

global _pmmngr_paging_enable

_pmmngr_paging_enable:
   push ebp
   mov ebp, esp
   %define b [ebp+8]
   pushad
   mov   eax, cr0
   cmp dword b, 1
   je enable
   jmp disable
enable:
   or eax, 0x80000000      ;set bit 31
   mov   cr0, eax
   jmp done
disable:
   and eax, 0x7FFFFFFF     ;clear bit 31
   mov   cr0, eax
done:
   popad
   pop ebp
   ret

global _pmmngr_load_PDBR

_pmmngr_load_PDBR:
   push ebp
   mov ebp, esp
   %define addr [ebp+8]
   push eax
   mov   eax, addr
   mov   cr3, eax    ; PDBR is cr3 register in i86
   pop eax
   pop ebp
   ret

global _pmmngr_get_PDBR

_pmmngr_get_PDBR:
   mov   eax, cr3
   ret

global _gdt_load
extern __gdtr

_gdt_load:
   lgdt [__gdtr]
   ret

global _flush_tss

_flush_tss:
   cli
   mov ax, 0x2b ; the tss segment selector (5 (6th entry)*8)|3 (DPL)
   ltr ax ; load the tss segment descriptor
   sti
   ret

; user/kernel mode procedures

global _enter_usermode

_enter_usermode:
   
   cli ; clear interrupts while performing the switch
   mov ax, 0x23 ; user mode data selector + 3 lower bytes is RPL 3
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax

   push 0x23      ; SS, notice it uses same selector as above
   push esp    ; ESP
   pushfd         ; EFLAGS
   pop eax
   or eax, 0x200  ; enable IF in EFLAGS
   push eax
   push 0x1b      ; CS, user mode code selector is 0x18. With RPL 3 this is 0x1b
   lea eax, [a]      ; EIP first
   push eax
   iretd
a:
   
   add esp, 4

   ret ;  jumps here then immediately returns


; idt data

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
    push byte i ; ith interrupt in our irq table
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
   add esp, 8 ; the pushes we did earlier (2 ints so 8 bytes)

   iret

; syscall stub

global _syscall_stub
extern __syscalls
%define MAX_SYSCALL 20

; ***process stack and kernel stack
_syscall_stub:
   
   sti ; very dangerous

   cmp eax, MAX_SYSCALL
   jae end

   push ebx
   push edx
   mov ebx, 4
   mul ebx
   pop edx
   pop ebx

   mov eax, [__syscalls+eax]

   push edi
   push esi
   push edx
   push ecx
   push ebx
   call eax
   add esp, 20

end:

   iret

; scheduler stub
global _scheduler_isr
extern _scheduler_tick
extern _tss_set_stack
extern _vmmngr_set_pdirectory_ptr
extern __currentTask

_scheduler_isr:

   ;
   ; clear interrupts and save context.
   ;
   cli
   pushad

   ;
   ; if no current task, just return.
   ;
   mov eax, [__currentTask]
   cmp eax, 0
   jz  interrupt_return
   ;
   ; save selectors.
   ;
   push ds
   push es
   push fs
   push gs
   ;
   ; switch to kernel segments.
   ;
   mov ax, 0x10
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   ;
   ; save esp.
   ;
   mov eax, [__currentTask]
   mov [eax], esp

   ;
   ; call scheduler.
   ;
   call _scheduler_tick

   ;
   ; restore esp.
   ;
   mov eax, [__currentTask]
   mov esp, [eax]

   ; restore page directory
   mov ebx, [eax+16] ; the parent process pointer
   mov ecx, [ebx] ; page directory is right at the start of the process struct

   mov ebx, cr3
   cmp ecx, ebx
   je same_pdir ; not need to switch pdirectory if it's the same

   mov cr3, ecx

   ; officially restore page directory now that the stack can be used
   push ecx
   call _vmmngr_set_pdirectory_ptr
   add esp, 4

same_pdir:
   ;
   ; Call tss_set_stack (kernelSS, kernelESP).
   ;
   mov eax, [__currentTask]
   push dword [eax+8]
   push dword [eax+12]
   call _tss_set_stack
   add esp, 8
   ;
   ; send EOI and restore context.
   ;
   pop gs
   pop fs
   pop es
   pop ds

interrupt_return:
   popa

   jmp _irq0

; raw scheduler function without pit

global _scheduler_raw
extern _scheduler_dispatch

_scheduler_raw:

   ;
   ; clear interrupts and save context.
   ;
   cli
   pushad

   ;
   ; if no current task, just return.
   ;
   mov eax, [__currentTask]
   cmp eax, 0
   jz  .interrupt_return
   ;
   ; save selectors.
   ;
   push ds
   push es
   push fs
   push gs
   ;
   ; switch to kernel segments.
   ;
   mov ax, 0x10
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   ;
   ; save esp.
   ;
   mov eax, [__currentTask]
   mov [eax], esp

   ;
   ; call scheduler.
   ;
   call _scheduler_dispatch

   ;
   ; restore esp.
   ;
   mov eax, [__currentTask]
   mov esp, [eax]

   ; restore page directory
   mov ebx, [eax+16] ; the parent process pointer
   mov ecx, [ebx] ; page directory is right at the start of the process struct

   mov ebx, cr3
   cmp ecx, ebx
   je .same_pdir ; not need to switch pdirectory if it's the same

   mov cr3, ecx

   ; officially restore page directory now that the stack can be used
   push ecx
   call _vmmngr_set_pdirectory_ptr
   add esp, 4

.same_pdir:
   ;
   ; Call tss_set_stack (kernelSS, kernelESP).
   ;
   mov eax, [__currentTask]
   push dword [eax+8]
   push dword [eax+12]
   call _tss_set_stack
   add esp, 8
   ;
   ; send EOI and restore context.
   ;
   pop gs
   pop fs
   pop es
   pop ds

.interrupt_return:
   popa

   iret

; lock operations

global _acquireLock
global _releaseLock


_acquireLock:
   %define locked [esp+4]
   mov ebx, locked
.try_acquire:
   mov     eax, 1          ; Set the EAX register to 1.
   xchg    eax, [ebx]   ; Atomically swap the EAX register with
                             ;  the lock variable.
                             ; This will always store 1 to the lock, leaving
                             ;  the previous value in the EAX register.
   test    eax, eax        ; Test EAX with itself. Among other things, this will
                             ;  set the processor's Zero Flag if EAX is 0.
                             ; If EAX is 0, then the lock was unlocked and
                             ;  we just locked it.
                             ; Otherwise, EAX is 1 and we didn't acquire the lock.
   jnz     .try_acquire       ; Jump back to the MOV instruction if the Zero Flag is
                             ;  not set; the lock was previously locked, and so
                             ; we need to spin until it becomes unlocked.
   ret                     ; The lock has been acquired, return to the calling
                             ;  function.


_releaseLock:
   %define locked [esp+4]
   mov ebx, locked
   xor     eax, eax        ; Set the EAX register to 0.
   xchg    eax, [ebx]   ; Atomically swap the EAX register with
                             ;  the lock variable.
   ret           
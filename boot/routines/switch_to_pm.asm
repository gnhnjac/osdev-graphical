bits 16

; ===================================
; SWITCHES TO PROTECTED MODE
; NOTES: LABELS DATA_SEG_INDEX AND CODE_SEG_INDEX MUST BE DEFINED AND POINT TO THE POSITION OF THE SEGMENT DESCRIPTORS IN THE GDT (IN BYTES FROM THE START)
; ALSO, IN THE END CALLS LABEL BEGIN_PM
; ===================================
switch_to_pm:

	cli ; clear 16 bit interrupts (IVT etc.)
	lgdt [gdt_descriptor] ; pass the gdt descriptor to the cpu
	; set first bit of control register 0
	mov eax , cr0
	or al , 0x1
	mov cr0 , eax
	
	; flush cpu pipeline to prevent faulty register usage (16 bit rm operations on a now 32-bit pm interface),
	; since jmp far cpu can't know next operation and must finish his pipeline before proceeding.
	; Also auto updates CS to point to code seg descriptor index in the GDT
	jmp CODE_SEG_INDEX:init_pm

bits 32

; Initialise registers and the stack once in PM.
init_pm:
	
	; Now in PM, our old segments are meaningless,
	; so we point our segment registers to the
	; data sector we defined in our GDT

	mov ax, DATA_SEG_INDEX
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Update our stack position so it is right
	; at the top of the free space.

	mov ebp, 0x90000
	mov esp, ebp

	call BEGIN_PM
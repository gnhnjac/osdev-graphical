org 0x7c00
bits 16

BEGIN_RM:

	mov [BOOT_DRIVE], dl ; mbr drive number saved in dl

	mov bp, 0x9000 ; init the stack
	mov sp, bp

	push rm_msg
	call print_str_mem

	call load_kernel

	; switch to 32 bit protected mode
	call switch_to_pm

load_kernel: ; note that dx is changed here!

	push load_kernel_msg
	call print_str_mem

	mov dl, [BOOT_DRIVE]
	xor dh, dh

	push 0x1000 ; es offset
	push 0 ; bx offset
	push dx ; drive number
	push 98 ; sectors to be read
	push 1 ; start sector in LBA
	call disk_load

	ret


bits 32
BEGIN_PM:

	push pm_msg
	call print_str_mem32

	call 0x10000 ; jmp into kernel code

	jmp $

; 16 bit rm files
%include "print_str_mem.asm"
;%include "print_hex_word.asm"
%include "disk_load.asm"
%include "gdt.asm"

; 32 bit pm files
%include "print_str_mem32.asm"
%include "switch_to_pm.asm"

; Global variables
	rm_msg db '16-bit Real Mode running...',0xa, 0xd,0
	load_kernel_msg db 'Loading kernel into memory at 0x10000', 0xa, 0xd, 0
	pm_msg db 'Successfully switched to 32-bit protected mode!', 0
	BOOT_DRIVE db 0

times 510-($-$$) db 0

dw 0xaa55
org 0x7c00
bits 16

BEGIN_RM:

	mov [BOOT_DRIVE], dl ; mbr drive number saved in dl

	mov bp, 0x9000 ; init the stack
	mov sp, bp

	push rm_msg
	call print_str_mem

	call load_2nd_stage

	call 0x1000 ; jump into the 2nd stage bootloader

	jmp $

load_2nd_stage: ; note that dx is changed here!

	push load_2nd_msg
	call print_str_mem

	mov dl, [BOOT_DRIVE]
	xor dh, dh

	push 0 ; es offset
	push 0x1000 ; bx offset
	push dx ; drive number
	push 8 ; sectors to be read (5 sectors for the 2nd stage + 3 for memory map)
	push 1 ; start sector in LBA
	call disk_load

	ret

; 16 bit rm files
%include "print_str_mem.asm"
;%include "print_hex_word.asm"
%include "disk_load.asm"

; Global variables
	rm_msg db '16-bit Real Mode running...',0xa, 0xd,0
	load_2nd_msg db 'Loading 2nd stage into memory at 0x1000', 0xa, 0xd, 0
	BOOT_DRIVE db 0

times 510-($-$$) db 0

dw 0xaa55
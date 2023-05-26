org 0x7c00
bits 16

start:	
	jmp	BEGIN_RM					; jump to start of bootloader
	nop

%include "bpb.asm"

BEGIN_RM:

	mov [BOOT_DRIVE], dl ; mbr drive number saved in dl

	mov bp, 0x9000 ; init the stack
	mov sp, bp

	call load_2nd_stage

	mov dl, [BOOT_DRIVE]
	call 0x3000 ; jump into the 2nd stage bootloader

	jmp $

load_2nd_stage:

	mov si,load_2nd_msg
	call print_str_mem_short

    mov ax, 0x300 ; file location is 0x3000
    mov es, ax
    xor bx, bx
    call load_fat12

    ret

; 16 bit rm files
%include "load_fat12.asm"
%include "print_str_mem_short.asm"
%include "disk_load_short.asm"

; Global variables
	load_2nd_msg db 'Loading 2nd stage at 0x3000', 0xa, 0xd, 0
	BOOT_DRIVE db 0
	IMAGE_NAME db 'STAGE2  SYS'

times 510-($-$$) db 0

dw 0xaa55
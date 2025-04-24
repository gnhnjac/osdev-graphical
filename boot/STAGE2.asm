org 0x3000
bits 16

; where the kernel is to be loaded to in protected mode
; mapped to 0x100000 phys
%define IMAGE_PMODE_BASE 0xC0000000 

; where the kernel is to be loaded to in real mode
%define IMAGE_RMODE_BASE 0x10000

mov [BOOT_DRIVE], dl ; mbr drive number saved in dl
mov [boot_info+multiboot_info.bootDevice], dl

; load kernel into memory
call load_kernel

xor	eax, eax
xor	ebx, ebx
call BiosGetMemorySize64MB

mov	word [boot_info+multiboot_info.memoryHi], bx
mov	word [boot_info+multiboot_info.memoryLo], ax

xor ax, ax
mov	es, ax
mov	di, 0x3000 + 512*5 ; 3 reserved sectors for the memory map
call BiosGetMemoryMap

push ds
pop es
mov di, VesaInfoBlockBuffer
call get_vesa_info

mov ax, 1920
mov bx, 1080
mov cl, 24
call set_vesa_mode

lea eax, [VesaInfoBlockBuffer]
mov [boot_info+multiboot_info.vbe_control_info], eax

lea eax, [VesaModeInfoBlockBuffer]
mov [boot_info+multiboot_info.vbe_mode_info], eax

; switch to 32 bit protected mode
call switch_to_pm

load_kernel: ; note that dx is changed here!

	mov si, load_kernel_msg
	call print_str_mem_short

	mov ax, IMAGE_RMODE_BASE/16 ; es offset
	mov es, ax
	xor bx, bx
	call load_fat12
	mov [READ_SECTORS], di

	ret

;	in:
;		es:di - 512-byte buffer
get_vesa_info:
	clc
	mov ax, 0x4f00
	int 0x10
	ret

;	in:
;		cx - VESA mode number
;		es:di - 256-byte buffer
get_vesa_mode_info:
	clc
	mov ax, 0x4f01
	int 0x10
	ret


;	in:
;		ax - width
;		bx - height
;       cl - bits per pixel
set_vesa_mode:

	push bp
	mov bp, sp

	%define width [bp-2]
	%define height [bp-4]
	%define bpp [bp-6]

	sub sp, 6

	mov width, ax
	mov height, bx
	mov bpp, cl

	push word [VesaInfoBlockBuffer + VesaInfoBlock.VideoModesSegment]
	pop es

	mov bx, [VesaInfoBlockBuffer+VesaInfoBlock.VideoModesOffset]

	mov di, VesaModeInfoBlockBuffer

.find_mode:

	mov cx, [bx]
	cmp cx, 0xffff
	je .not_found

	clc
	mov ax, 0x4f01
	int 0x10

	mov ax, width
	cmp [VesaModeInfoBlockBuffer+VesaModeInfoBlock.Width], ax
	jne .next_mode

	mov ax, height
	cmp [VesaModeInfoBlockBuffer+VesaModeInfoBlock.Height], ax
	jne .next_mode

	mov al, bpp
	cmp [VesaModeInfoBlockBuffer+VesaModeInfoBlock.BitsPerPixel], al
	jne .next_mode

	jmp .found

.next_mode:

	add bx, 2
	jmp .find_mode

.not_found:

	mov si, vesa_mode_not_found
	call print_str_mem_short

	hlt

.found:

	mov ax, 0x4F02
	mov bx, [bx]
	int 0x10

	mov sp, bp
	pop bp

	ret



bits 32
BEGIN_PM:
	
	push copy_kernel_msg
	call print_str_mem32

	call EnablePaging ; enable paging before we copy our kernel to 0xC0000000

COPY_KERNEL_IMG:
	
	xor eax, eax
	mov	ax, [READ_SECTORS]
 	mov ebx, 512
 	mul	ebx
 	mov	ebx, 4
 	div	ebx ; because of movsd we only need the number of bytes / 4
 	cld
 	mov esi, IMAGE_RMODE_BASE
 	mov	edi, IMAGE_PMODE_BASE
 	mov	ecx, eax
 	rep	movsd  

	;push pm_msg
	;call print_str_mem32

	mov	eax, 0x2BADB002	 ; multiboot specs say eax should be this
	mov	ebx, 0
 	
 	xor edx, edx
 	mov dx, [READ_SECTORS]
 	push edx
	push dword boot_info
	call IMAGE_PMODE_BASE ;Execute Kernel

	add	esp, 8
 
    cli
	hlt

bits 16
; 16 bit rm files
%include "print_str_mem_short.asm"
;%include "print_hex_word.asm"
%include "disk_load_short.asm"
%include "gdt.asm"
%include "bpb.asm"
%include "load_fat12.asm"
;---------------------------------------------
;	Get memory size for >64M configuations
;	ret\ ax=KB between 1MB and 16MB
;	ret\ bx=number of 64K blocks above 16MB
;	ret\ bx=0 and ax= -1 on error
;---------------------------------------------
 
BiosGetMemorySize64MB:
	push	ecx
	push	edx
	xor	ecx, ecx		;clear all registers. This is needed for testing later
	xor	edx, edx
	mov	ax, 0xe801
	int	0x15	
	jc	.error
	cmp	ah, 0x86		;unsupported function
	je	.error
	cmp	ah, 0x80		;invalid command
	je	.error
	jcxz	.use_ax			;bios may have stored it in ax,bx or cx,dx. test if cx is 0
	mov	ax, cx			;its not, so it should contain mem size; store it
	mov	bx, dx
 
.use_ax:
	pop	edx			;mem size is in ax and bx already, return it
	pop	ecx
	ret
 
.error:
	mov	ax, -1
	mov	bx, 0
	pop	edx
	pop	ecx
	ret

struc MemoryMapEntry
	.baseAddress	resq	1	; base address of address range
	.length		resq	1	; length of address range in bytes
	.type		resd	1	; type of address range
	.acpi_null	resd	1	; reserved
endstruc

;---------------------------------------------
;	Get memory map from bios
;	/in es:di->destination buffer for entries
;	/ret bp=entry count
;---------------------------------------------
 
BiosGetMemoryMap:
	pushad ; general purpose registers, not including bp for example
	xor	ebx, ebx
	xor	bp, bp			; number of entries stored here
	mov	edx, 'PAMS'		; 'SMAP'
	mov	eax, 0xe820
	mov	ecx, 24			; memory map entry struct is 24 bytes
	int	0x15			; get first entry
	jc	.error	
	cmp	eax, 'PAMS'		; bios returns SMAP in eax
	jne	.error
	test	ebx, ebx		; if ebx=0 then list is one entry long; bail out
	je	.error
	jmp	.start
.next_entry:
	mov	edx, 'PAMS'		; some bios's trash this register
	mov	ecx, 24			; entry is 24 bytes
	mov	eax, 0xe820
	int	0x15			; get next entry
.start:
	jcxz	.skip_entry		; if actual returned bytes is 0, skip entry
.notext:
	mov	ecx, [es:di + MemoryMapEntry.length]	; get length (low dword)
	test	ecx, ecx		; if length is 0 skip it
	jne	short .good_entry
	mov	ecx, [es:di + MemoryMapEntry.length + 4]; get length (upper dword)
	jecxz	.skip_entry		; if length is 0 skip it
.good_entry:
	inc	bp			; increment entry count
	add	di, 24			; point di to next entry in buffer
.skip_entry:
	cmp	ebx, 0			; if ebx return is 0, list is done
	jne	.next_entry		; get next entry
	jmp	.done
.error:
	stc ; set carry flag
.done:
	popad
	ret

struc multiboot_info
	.flags		resd	1	; required
	.memoryLo	resd	1	; memory size. Present if flags[0] is set
	.memoryHi	resd	1
	.bootDevice	resd	1	; boot device. Present if flags[1] is set
	.cmdLine	resd	1	; kernel command line. Present if flags[2] is set
	.mods_count	resd	1	; number of modules loaded along with kernel. present if flags[3] is set
	.mods_addr	resd	1
	.syms0		resd	1	; symbol table info. present if flags[4] or flags[5] is set
	.syms1		resd	1
	.syms2		resd	1
	.mmap_length	resd	1	; memory map. Present if flags[6] is set
	.mmap_addr	resd	1
	.drives_length	resd	1	; phys address of first drive structure. present if flags[7] is set
	.drives_addr	resd	1
	.config_table	resd	1	; ROM configuation table. present if flags[8] is set
	.bootloader_name resd	1	; Bootloader name. present if flags[9] is set
	.apm_table	resd	1	; advanced power management (apm) table. present if flags[10] is set
	.vbe_control_info resd	1	; video bios extension (vbe). present if flags[11] is set
	.vbe_mode_info	resd	1
	.vbe_mode	resw	1
	.vbe_interface_seg resw	1
	.vbe_interface_off resw	1
	.vbe_interface_len resw	1
endstruc

boot_info:
istruc multiboot_info
	at multiboot_info.flags,			dd 0
	at multiboot_info.memoryLo,			dd 0
	at multiboot_info.memoryHi,			dd 0
	at multiboot_info.bootDevice,		dd 0
	at multiboot_info.cmdLine,			dd 0
	at multiboot_info.mods_count,		dd 0
	at multiboot_info.mods_addr,		dd 0
	at multiboot_info.syms0,			dd 0
	at multiboot_info.syms1,			dd 0
	at multiboot_info.syms2,			dd 0
	at multiboot_info.mmap_length,		dd 0
	at multiboot_info.mmap_addr,		dd 0
	at multiboot_info.drives_length,	dd 0
	at multiboot_info.drives_addr,		dd 0
	at multiboot_info.config_table,		dd 0
	at multiboot_info.bootloader_name,	dd 0
	at multiboot_info.apm_table,		dd 0
	at multiboot_info.vbe_control_info,	dd 0
	at multiboot_info.vbe_mode_info,	dw 0
	at multiboot_info.vbe_interface_seg,dw 0
	at multiboot_info.vbe_interface_off,dw 0
	at multiboot_info.vbe_interface_len,dw 0
iend

struc VesaInfoBlock				;	VesaInfoBlock_size = 512 bytes
	.Signature		resb 4		;	must be 'VESA'
	.Version		resw 1
	.OEMNamePtr		resd 1
	.Capabilities		resd 1

	.VideoModesOffset	resw 1
	.VideoModesSegment	resw 1

	.CountOf64KBlocks	resw 1
	.OEMSoftwareRevision	resw 1
	.OEMVendorNamePtr	resd 1
	.OEMProductNamePtr	resd 1
	.OEMProductRevisionPtr	resd 1
	.Reserved		resb 222
	.OEMData		resb 256
endstruc

VesaInfoBlockBuffer: istruc VesaInfoBlock
	at VesaInfoBlock.Signature,				db "VESA"
	times 508 db 0
iend

struc VesaModeInfoBlock				;	VesaModeInfoBlock_size = 256 bytes
	.ModeAttributes		resw 1
	.FirstWindowAttributes	resb 1
	.SecondWindowAttributes	resb 1
	.WindowGranularity	resw 1		;	in KB
	.WindowSize		resw 1		;	in KB
	.FirstWindowSegment	resw 1		;	0 if not supported
	.SecondWindowSegment	resw 1		;	0 if not supported
	.WindowFunctionPtr	resd 1
	.BytesPerScanLine	resw 1

	;	Added in Revision 1.2
	.Width			resw 1		;	in pixels(graphics)/columns(text)
	.Height			resw 1		;	in pixels(graphics)/columns(text)
	.CharWidth		resb 1		;	in pixels
	.CharHeight		resb 1		;	in pixels
	.PlanesCount		resb 1
	.BitsPerPixel		resb 1
	.BanksCount		resb 1
	.MemoryModel		resb 1		;	http://www.ctyme.com/intr/rb-0274.htm#Table82
	.BankSize		resb 1		;	in KB
	.ImagePagesCount	resb 1		;	count - 1
	.Reserved1		resb 1		;	equals 0 in Revision 1.0-2.0, 1 in 3.0

	.RedMaskSize		resb 1
	.RedFieldPosition	resb 1
	.GreenMaskSize		resb 1
	.GreenFieldPosition	resb 1
	.BlueMaskSize		resb 1
	.BlueFieldPosition	resb 1
	.ReservedMaskSize	resb 1
	.ReservedMaskPosition	resb 1
	.DirectColorModeInfo	resb 1

	;	Added in Revision 2.0
	.LFBAddress		resd 1
	.OffscreenMemoryOffset	resd 1
	.OffscreenMemorySize	resw 1		;	in KB
	.Reserved2		resb 206	;	available in Revision 3.0, but useless for now
endstruc

VesaModeInfoBlockBuffer:	istruc VesaModeInfoBlock
		times VesaModeInfoBlock_size db 0
iend

bits 32
; 32 bit pm files
%include "print_str_mem32.asm"
%include "switch_to_pm.asm"
%include "enable_paging.asm"

; variables
vesa_mode_not_found db 'Vesa mode not found, aborting...', 0xa, 0xd, 0
load_kernel_msg db 'Loading kernel into memory at 0x10000', 0xa, 0xd, 0
copy_kernel_msg db 'Copying kernel into virtual memory at 0xC0000000', 0xa, 0xd, 0
pm_msg db 'Successfully switched to 32-bit protected mode!', 0
BOOT_DRIVE db 0
IMAGE_NAME db 'KERNEL  SYS'
READ_SECTORS dw 0

times 512*5-($-$$) db 0 ; occupy exactly 5 sectors

times 512*3 db 0 ; another 3 sectors for the memory map

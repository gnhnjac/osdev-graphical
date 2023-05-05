org 0x1000
bits 16

; where the kernel is to be loaded to in protected mode
%define IMAGE_PMODE_BASE 0x100000

; where the kernel is to be loaded to in real mode
%define IMAGE_RMODE_BASE 0x10000

; how many sectors to read
%define READ_SECTORS 110

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
mov	di, 0x1000 + 512*5 ; 3 reserved sectors for the memory map
call BiosGetMemoryMap

; switch to 32 bit protected mode
call switch_to_pm

load_kernel: ; note that dx is changed here!

	push load_kernel_msg
	call print_str_mem

	mov dl, [BOOT_DRIVE]
	xor dh, dh

	push IMAGE_RMODE_BASE/16 ; es offset
	push 0 ; bx offset
	push dx ; drive number
	push READ_SECTORS ; sectors to be read
	push 9 ; start sector in LBA
	call disk_load

	ret

bits 32
BEGIN_PM:
	
	push copy_kernel_msg
	call print_str_mem32

COPY_KERNEL_IMG:
	mov	eax, 110
 	mov ebx, 512
 	mul	ebx
 	mov	ebx, 4
 	div	ebx ; because of movsd we only need the number of bytes / 4
 	cld
 	mov esi, IMAGE_RMODE_BASE
 	mov	edi, IMAGE_PMODE_BASE
 	mov	ecx, eax
 	rep	movsd  

	push pm_msg
	call print_str_mem32

	mov	eax, 0x2BADB002	 ; multiboot specs say eax should be this
	mov	ebx, 0
 	
 	push dword READ_SECTORS
	push dword boot_info
	call IMAGE_PMODE_BASE ;Execute Kernel

	add	esp, 4
 
    cli
	hlt


bits 16
; 16 bit rm files
%include "print_str_mem.asm"
;%include "print_hex_word.asm"
%include "disk_load.asm"
%include "gdt.asm"

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

bits 32
; 32 bit pm files
%include "print_str_mem32.asm"
%include "switch_to_pm.asm"

; variables
load_kernel_msg db 'Loading kernel into memory at 0x3000', 0xa, 0xd, 0
copy_kernel_msg db 'Copying kernel into memory at 0x100000', 0xa, 0xd, 0
pm_msg db 'Successfully switched to 32-bit protected mode!', 0
BOOT_DRIVE db 0

times 512*5-($-$$) db 0 ; occupy exactly 5 sectors

times 512*3 db 0 ; another 3 sectors for the memory map
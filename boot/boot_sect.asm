org 0x7c00
bits 16

start:	
	jmp	BEGIN_RM					; jump to start of bootloader
	nop

;*********************************************
;	BIOS Parameter Block
;*********************************************

; BPB Begins 3 bytes from start. We do a far jump, which is 3 bytes in size.
; If you use a short jump, add a "nop" after it to offset the 3rd byte.

bpbOEM			DB "My OS   "			; OEM identifier (Cannot exceed 8 bytes!)
bpbBytesPerSector:  	DW 512 ; in 144mb floppy
bpbSectorsPerCluster: 	DB 1 ; 1 sector per fat cluster
bpbReservedSectors: 	DW 1 ; 1 reserved sector (our bootloader)
bpbNumberOfFATs: 	DB 2 ; 2 file allocation tables, 1 is a copy of the other
bpbRootEntries: 	DW 224 ; 224 files available
bpbTotalSectors: 	DW 2880 ; total sectors in floppy disk
bpbMedia: 		DB 0xf8  ;; 0xF1
bpbSectorsPerFAT: 	DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 	DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 		DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "MOS FLOPPY "
bsFileSystem: 	        DB "FAT12   "

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
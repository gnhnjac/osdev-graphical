bits 16

; ===================================
; LOAD SECTORS TO ES:BX FROM DRIVE
; PARMAMS: ES => SEGMENT VALUE, BX => OFFSET VALUE, DRIVE NUMBER IN [BOOT_DRIVE], CX => SECTORS TO BE READ IN LBA, AX => START SECTOR
; ===================================
disk_load_short:

load_loop:
	push cx
	call lba_2_chs
	push ax
	mov ah , 0x02 ; BIOS read sector function
	mov al, 1 ; read 1 sector
	mov dl, [BOOT_DRIVE] ; drive number
	int 0x13 ; BIOS interrupt
	jc _dl_disk_error ; Jump if error ( i.e. carry flag set )
	pop ax
	inc ax ; add sectors read to start_sector
	add bx, 512
	pop cx
	loop load_loop
	
	ret

_dl_disk_error:
	mov si,DISK_ERROR_MSG
	call print_str_mem_short
	jmp $


; *cyl    = lba / (2 * FLOPPY_288_SECTORS_PER_TRACK);
; *head   = ((lba % (2 * FLOPPY_288_SECTORS_PER_TRACK)) / FLOPPY_288_SECTORS_PER_TRACK);
; *sector = ((lba % (2 * FLOPPY_288_SECTORS_PER_TRACK)) % FLOPPY_288_SECTORS_PER_TRACK + 1);
; PARAMETERS: LBA IN AX
; RETURNS CYL IN CH, HEAD IN DH, SECTOR IN CL
lba_2_chs:
		
	push ax
	push bx
	xor dx, dx
	mov bx, 72 ; 288 floppy sectors per track*2
	div bx
	mov ch, al ; cyl in al moved the ch
	
	; modulo was stored in dx
	mov ax, dx
	xor dx, dx
	mov bx, 36 ; 288 floppy sectors per track
	div bx
	mov dh, al ; head in al moved to dh

	; modulo of that operation stored in dx
	mov cl, dl
	inc cl ; to get sector number
	pop bx
	pop ax
	ret 

; Variables
DISK_ERROR_MSG db "Disk read error!" , 0

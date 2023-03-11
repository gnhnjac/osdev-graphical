bits 16

; ===================================
; LOAD SECTORS TO ES:BX FROM DRIVE
; PARMAMS: ES SEGMENT VALUE, BX OFFSET VALUE, DRIVE NUMBER, SECTORS TO BE READ IN LBA, START SECTOR
; ===================================
disk_load:
	
	push bp
	mov bp, sp
	%define es_val [bp+12]
	%define bx_off [bp+10]
	%define drive_num [bp+8]
	%define num_sectors [bp+6]
	%define start_sector [bp+4]

	pusha

	mov ax, es_val
	mov es, ax
	mov bx, bx_off

load_loop:

	mov ax, start_sector
	call lba_2_chs
	mov ah , 0x02 ; BIOS read sector function
	mov al , 18 ; Read 18 sectors

	cmp word num_sectors, 18
	jae above_18
	mov al, num_sectors ; unless we need to read less than 18
above_18:
	push ax
	mov dl, drive_num ; from drive x
	int 0x13 ; BIOS interrupt
	jc _dl_disk_error ; Jump if error ( i.e. carry flag set )
	pop dx
	cmp dl , al ; if AL ( sectors read ) != DL ( sectors expected )
	jne _dl_disk_error ; display error message

	xor ah, ah
	sub num_sectors, ax ; subtract read sectors
	add start_sector,ax ; add sectors read to start_sector
	mov cx, 512 ; bytes per sector
	xor dx, dx
	mul cx ; ax=ax*512 => sectors_read=sectors_read*512
	add bx, ax
	cmp word num_sectors, 0
	jnz load_loop

	popa

	pop bp

	ret 10

_dl_disk_error:
	push DISK_ERROR_MSG
	call print_str_mem
	jmp $


; *cyl    = lba / (2 * FLOPPY_288_SECTORS_PER_TRACK);
; *head   = ((lba % (2 * FLOPPY_288_SECTORS_PER_TRACK)) / FLOPPY_288_SECTORS_PER_TRACK);
; *sector = ((lba % (2 * FLOPPY_288_SECTORS_PER_TRACK)) % FLOPPY_288_SECTORS_PER_TRACK + 1);
; PARAMETERS: LBA IN AX
; RETURNS CYL IN CH, HEAD IN DH, SECTOR IN CL
lba_2_chs:
	
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
	ret 

; Variables
DISK_ERROR_MSG db "Disk read error!" , 0

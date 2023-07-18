bits 16

; vbe_set_mode:
; Sets a VESA mode
; In\	AX = Width
; In\	BX = Height
; In\	CL = Bits per pixel
; Out\	FLAGS = Carry clear on success
; Out\	Width, height, bpp, physical buffer, all set in vbe_screen structure
 
vbe_set_mode:
	mov [.width], ax
	mov [.height], bx
	mov [.bpp], cl
 
	sti
 
	push es					; some VESA BIOSes destroy ES, or so I read
	mov ax, 0x4F00				; get VBE BIOS info
	mov di, vbe_info_block
	int 0x10
	pop es
 
	cmp ax, 0x4F				; BIOS doesn't support VBE?
	jne .error
 
	mov ax, word[vbe_info_block.video_modes]
	mov [.offset], ax
	mov ax, word[vbe_info_block.video_modes+2]
	mov [.segment], ax
 
	mov ax, [.segment]
	mov fs, ax
	mov si, [.offset]
 
.find_mode:
	mov dx, [fs:si]
	add si, 2
	mov [.offset], si
	mov [.mode], dx
	mov ax, 0
	mov fs, ax
 
	cmp WORD[.mode], 0xFFFF			; end of list?
	je .error
 
	push es
	mov ax, 0x4F01				; get VBE mode info
	mov cx, [.mode]
	mov di, mode_info_block
	int 0x10
	pop es
 
	cmp ax, 0x4F
	jne .error
 
	mov ax, [.width]
	cmp ax, [mode_info_block.width]
	jne .next_mode
 
	mov ax, [.height]
	cmp ax, [mode_info_block.height]
	jne .next_mode
 
	mov al, [.bpp]
	cmp al, [mode_info_block.bpp]
	jne .next_mode
 
	; If we make it here, we've found the correct mode!
	mov ax, [.width]
	mov word[vbe_screen.width], ax
	mov ax, [.height]
	mov word[vbe_screen.height], ax
	mov eax, [mode_info_block.framebuffer]
	mov dword[vbe_screen.framebuffer], eax
	mov ax, [mode_info_block.pitch]
	mov word[vbe_screen.bytes_per_line], ax
	mov eax, 0
	mov al, [.bpp]
	mov byte[vbe_screen.bpp], al
	shr eax, 3
	mov dword[vbe_screen.bytes_per_pixel], eax
 
	mov ax, [.width]
	shr ax, 3
	dec ax
	mov word[vbe_screen.x_max], ax
 
	mov ax, [.height]
	shr ax, 4
	dec ax
	mov word[vbe_screen.y_max], ax
 
	; Set the mode
	push es
	mov ax, 0x4F02
	mov bx, [.mode]
	or bx, 0x4000			; enable LFB
	mov di, 0			; not sure if some BIOSes need this... anyway it doesn't hurt
	int 0x10
	pop es
 
	cmp ax, 0x4F
	jne .error
 
	clc
	ret
 
.next_mode:
	mov ax, [.segment]
	mov fs, ax
	mov si, [.offset]
	jmp .find_mode
 
.error:
	stc
	ret
 
.width				dw 0
.height				dw 0
.bpp				db 0
.segment			dw 0
.offset				dw 0
.mode				dw 0
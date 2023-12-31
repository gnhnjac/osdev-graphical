bits 16

; ===================================
; PRINTS OUT A STRING FROM MEMORY (null terminated)
; PARMAMS: MESSAGE OFFSET
; ===================================
print_str_mem:
	push bp
	mov bp, sp
	%define txt [bp+4]

	push ax
	push bx
	push si

	mov ah, 0x0e
	mov si, txt
	xor bx, bx ; page 0

_printsm_loop:
	lodsb ; put in al the char in our memory + offset (si)
	cmp al, 0 ; null terminator
	jz _printsm_end
	int 0x10
	jmp _printsm_loop
_printsm_end:

	pop si
	pop bx
	pop ax
	pop bp

	ret 2
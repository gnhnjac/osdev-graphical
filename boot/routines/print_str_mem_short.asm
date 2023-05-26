bits 16

; ===================================
; PRINTS OUT A STRING FROM MEMORY (null terminated)
; DS:SI => null terminated string
; ===================================
print_str_mem_short:
	
	pusha

	mov ah, 0x0e
	xor bh, bh ; page 0

_printsm_loop:
	lodsb ; put in al the char in our memory + offset (si)
	cmp al, 0 ; null terminator
	jz _printsm_end
	int 0x10
	jmp _printsm_loop
_printsm_end:
	
	popa

	ret
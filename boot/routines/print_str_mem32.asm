bits 32
; Define some constants
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; ===================================
; PRINTS OUT A STRING FROM MEMORY (null terminated)
; PARMAMS: MESSAGE OFFSET
; ===================================
print_str_mem32:
	push ebp
	mov ebp, esp
	%define txt [ebp+8]

	pushad
	mov ebx, txt
	mov edx , VIDEO_MEMORY ; Set edx to the start of vid mem.

_printsm32_loop:
	mov al , [ ebx ] ; Store the char at EBX in AL
	mov ah , WHITE_ON_BLACK ; Store the attributes in AH

	cmp al , 0 ; if (al == 0) , at end of string , so
	je _printsm32_done ; jump to done

	mov [edx] , ax ; Store char and attributes at current character cell.
	
	inc ebx ; Increment EBX to the next char in string.
	add edx , 2 ; Move to next character cell in vid mem.
	jmp _printsm32_loop ; loop around to print the next char.

_printsm32_done:
	popad

	pop ebp
	ret 4
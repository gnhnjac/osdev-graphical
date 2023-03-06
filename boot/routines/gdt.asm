gdt_start:

	null_sd: ; null segment descriptor
	
		dd 0
		dd 0
	
	code_sd:
		; L flag (64-bit code segment) -> 0 because unused on 32-bit processor
		; AVL flag -> 0
		; Base addr -> 0
		; Default operation size -> 1 because we are using 32-bit sizes
		; Descriptor privilege level -> 00 because ring 0
		; Granularity -> 1 because now we have a limit of 0xFFFF000 bits
		; Seg limit -> 0xFFFF
		; P flag (segment present) -> 1 because it's present in memory
		; Descriptor type flag (S flag) -> 1010
			; 1 (Code) -> because its a code segment
			; 0 (Conforming) -> because we dont want any lower privilege segments to call code in this segment
			; 1 (Readable) -> 1 because we want to read constants defined in our code
			; 0 (Accessed) -> turned on by cpu for virtual memory usages
		; S flag (segment descriptor type) -> 1 because its a code/data segment descriptor
		
		; first 4 bytes
		
			; Seg limit bits 0-15 (16 bits)
			dw 0xFFFF
			
			; Base addr bits 0-15 (16 bits)
			dw 0
		
		; last 4 bytes
		
			; Base addr bits 16-23 (8 bits)
			db 0
			
			; 8 flag bits
				; Contains (ordered low to high):
				; Type flag (4 bits)
				; S flag (Segment descriptor type) (1 bit)
				; Descriptor privilege level (2 bits)
				; P flag (Segment present) (1 bit)
				
				db 10011010b
			
			; 8 bits
				; Contains (ordered low to high):
				; Seg limit bits 16-19
				; AVL flag (1 bit)
				; L flag (64-bit code segment) (1 bit)
				; Default operation size flag (1 bit)
				; Granularity flag (1 bit)
				
				db 11001111b
			
			; Base addr bits 24-31 (8 bits)
			db 0
	
	data_sd:
		; L flag (64-bit code segment) -> 0 because unused on 32-bit processor
		; AVL flag -> 0
		; Base addr -> 0
		; Default operation size -> 1 because we are using 32-bit sizes
		; Descriptor privilege level -> 00 because ring 0
		; Granularity -> 1 because now we have a limit of 0xFFFF000 bits
		; Seg limit -> 0xFFFF
		; P flag (segment present) -> 1 because it's present in memory
		; Descriptor type flag (S flag) -> 1010
			; 0 (Code) -> because its a data segment
			; 0 (Expand down) -> because we want to be able to expand it down
			; 1 (Writable) -> 1 because we want to be able to write to it, its not readonly
			; 0 (Accessed) -> turned on by cpu for virtual memory usages
		; S flag (segment descriptor type) -> 1 because its a code/data segment descriptor
		
		; first 4 bytes
		
			; Seg limit bits 0-15 (16 bits)
			dw 0xFFFF
			
			; Base addr bits 0-15 (16 bits)
			dw 0
		
		; last 4 bytes
		
			; Base addr bits 16-23 (8 bits)
			db 0
			
			; 8 flag bits
				; Contains (ordered low to high):
				; Type flag (4 bits)
				; S flag (Segment descriptor type) (1 bit)
				; Descriptor privilege level (2 bits)
				; P flag (Segment present) (1 bit)
				
				db 10010010b
			
			; 8 bits
				; Contains (ordered low to high):
				; Seg limit bits 16-19
				; AVL flag (1 bit)
				; L flag (64-bit code segment) (1 bit)
				; Default operation size flag (1 bit)
				; Granularity flag (1 bit)
				
				db 11001111b
			
			; Base addr bits 24-31 (8 bits)
			db 0
	
gdt_end:
	
gdt_descriptor:
	dw gdt_end - gdt_start - 1 ; Size of our GDT , always one less than the true size
	dd gdt_start ; Start address of our GDT

CODE_SEG_INDEX equ (code_sd - gdt_start)
DATA_SEG_INDEX equ (data_sd - gdt_start)
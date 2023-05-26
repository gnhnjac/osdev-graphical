
; ===================================
; LOADS A FAT12 FILE TO A LOCATION
; PARMAMS: FILE LOCATION IN ES:BX, IMAGE NAME IN [IMAGE_NAME]
; ===================================
load_fat12:
    push bx ; push file location for later
    push es ; push file location for later
    xor ax, ax
    mov es, ax
;----------------------------------------------------
; Load root directory table
;----------------------------------------------------

LOAD_ROOT:
     
; compute size of root directory and store in "cx" (in sectors)
 
    xor     cx, cx
    xor     dx, dx
    mov     ax, 0x0020                           ; 32 byte directory entry
    mul     WORD [bpbRootEntries]                ; total size of directory
    div     WORD [bpbBytesPerSector]             ; sectors used by directory
    xchg    ax, cx ; sectors to read in cx
      
; compute location of root directory and store in "ax" (get the start sector)
 	
 	xor ax, ax
    mov al, BYTE [bpbNumberOfFATs]            ; number of FATs
    mul WORD [bpbSectorsPerFAT]               ; sectors used by FATs
    add ax, WORD [bpbReservedSectors]         ; adjust for bootsector
    mov WORD [datasector], ax                 ; base of root directory
    add WORD [datasector], cx ; add root directory sectors to base of root directory to get data sector
 	; base sector of root directory is in ax
      
; read root directory into memory (7C00:0500)
 
    mov     bx, 0x0500                            ; copy root dir above bootcode
    call    disk_load_short

;----------------------------------------------------
; Find stage 2
;----------------------------------------------------

; browse root directory for binary image
    mov     cx, WORD [bpbRootEntries]             ; load loop counter
    mov     di, 0x0500                            ; locate first root entry
.LOOP:
    push    cx
    mov     cx, 0x000B                            ; eleven character name
    mov     si, IMAGE_NAME                         ; image name to find
    push    di
 	rep  cmpsb                                         ; test for entry match
    pop     di
    je      LOAD_FAT ; found entry with matching name
    pop     cx
    add     di, 0x0020                            ; queue next directory entry
    loop    .LOOP
    jmp     FAILURE

;----------------------------------------------------
; Load FAT
;----------------------------------------------------

LOAD_FAT:

    pop cx ; we pushed it earlier
 
; save starting cluster of boot image
 
    mov     dx, WORD [di + 0x001A]
    mov     WORD [cluster], dx                  ; file's first cluster
      
; compute size of FAT and store in "cx"
 
    xor     ax, ax
    mov     al, BYTE [bpbNumberOfFATs]          ; number of FATs
    mul     WORD [bpbSectorsPerFAT]             ; sectors used by FATs
    mov     cx, ax ; sectors to read in cx

; compute location of FAT and store in "ax"

    mov     ax, WORD [bpbReservedSectors]       ; adjust for bootsector
      
; read FAT into memory (7C00:0500)

    mov     bx, 0x0500                          ; copy FAT above bootcode
    call    disk_load_short

; read 2nd stage file into memory (0200:0000)
    
    pop es                          ; destination for image
    pop bx                          ; destination for image

;----------------------------------------------------
; Load Stage 2
;----------------------------------------------------

 LOAD_IMAGE:

    mov     ax, WORD [cluster]                  ; cluster to read
    call    ClusterLBA                          ; convert cluster to LBA, get start sector to read from
    xor     cx, cx
    mov     cl, BYTE [bpbSectorsPerCluster]     ; sectors to read
    ; offset in es:bx
    call    disk_load_short
      
; compute next cluster
 	push    bx
    mov     ax, WORD [cluster]                  ; identify current cluster
    mov     cx, ax                              ; copy current cluster
    mov     dx, ax                              ; copy current cluster
    shr     dx, 0x0001                          ; divide by two
    add     cx, dx                              ; sum for (3/2)
    mov     bx, 0x0500                          ; location of FAT in memory
    add     bx, cx                              ; index into FAT
    mov     dx, WORD [bx]                       ; read two bytes from FAT
    pop bx
    test    ax, 0x0001
    jnz     .ODD_CLUSTER
      
 .EVEN_CLUSTER:
 
      and     dx, 0000111111111111b               ; take low twelve bits
     jmp     .DONE
     
 .ODD_CLUSTER:
 
      shr     dx, 0x0004                          ; take high twelve bits
      
 .DONE:
 
      mov     WORD [cluster], dx                  ; store new cluster
      cmp     dx, 0x0FF0                          ; test for end of file
      jb      LOAD_IMAGE
      
 DONE:
      ret
      
 FAILURE:
 
      mov     si, failure_msg
      call    print_str_mem_short
      mov     ah, 0x00
      int     0x16                                ; await keypress
      int     0x19                                ; warm boot computer

	ret

;************************************************;
; Convert Cluster to disk LBA
; LBA = (cluster - 2) * sectors per cluster + start of data sector
; Cluster in ax
;************************************************;

ClusterLBA:
          sub     ax, 0x0002                          ; zero base cluster number
          xor     cx, cx
          mov     cl, BYTE [bpbSectorsPerCluster]     ; convert byte to word
          mul     cx
          add     ax, WORD [datasector]               ; base data sector
          ret

; Global variables
    failure_msg  db "MISSING OR CURRUPT STAGE2. Press Any Key to Reboot", 0x0a, 0x0d, 0x00
    cluster dw 0
    datasector dw 0
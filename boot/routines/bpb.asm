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
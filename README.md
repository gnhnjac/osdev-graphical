AKOS OPERATING SYSTEM:

Compiled using windows and gcc.

PREREQUISITES:

mingw32 -> make, ld, gcc
nasm
python 3.x
windows 10+
qemu emulator/bochs emulator (bochs is much slower)
dd
WinImage

CURRENTLY SUPPORTS:

BOOT FROM VIRTUAL FLOPPY DRIVE WITH FAT12 FS
PS/2 DRIVERS (PIT,KEYBOARD,MOUSE)
SHELL GUI
VMM AND PMM (AND KHEAP MEMORY ALLOCATION)
IDT,ISRS,IRQS
VIRTUAL FILE SYSTEM

BUILD with make in the root.

to start, run "make runq" to run with QEMU or "make run" to run with BOCHS, change the parameter in the makefile accordingly.

type help in the shell to get a list of commands, type help COMMAND to get help for a specific command.

PHYSICAL MEMORY STRUCTURE:

1MB-2MB -> KERNEL
3MB-4MB -> KHEAP
4MB-6MB -> VFS
3GB -> KERNEL
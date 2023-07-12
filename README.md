```
AKOS OPERATING SYSTEM:

Compiled using windows and gcc.

PREREQUISITES:

mingw32 -> make, ld, gcc
nasm
python 3.x
windows 32 bit
qemu emulator/bochs emulator (bochs is much slower)
dd
WinImage

CURRENTLY SUPPORTS:

Boot from virtualfloppy drive with fat12 fs minidriver
PS/2 drivers (keyboard input,visible mouse cursor)
pit timer
Shell gui
VMM and PMM (and kheap memory allocation)
IDT,ISRS,IRQS
3.5 inch floppy driver
Virtual file system with volume manager, fat12 filesystem and RAM file system
Loading PE executable into disk
Running multiprocess PE executable in usermode + kernel process + task priority
int 0x80 maskable syscalls:
0 - print
1 - terminate process
2 - fork

16 color vga 640x480 mode.

BUILD with make in the root.

to start, run "make runq" to run with QEMU or "make run" to run with BOCHS, change the parameter in the makefile accordingly.

to view fat12 filesystem run "make inspect", the filesystem builds from the folder "root" in the make directory, put all the stuff in there.

type help in the shell to get a list of commands, type "help COMMAND" to get help for a specific command.

MEMORY STRUCTURE:

0MB-1MB -> STACK,BIOS
1MB-2MB -> REAL KERNEL + GDT + IDT + PMM
4MB-2GB -> USER LAND SPACE
3GB -> VIRTUAL KERNEL
3GB + 2MB-3MB -> KHEAP
3GB + 3MB-4MB -> RAM FS
```
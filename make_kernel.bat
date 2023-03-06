gcc -ffreestanding -c kernel.c -o kernel.o

nasm kernel_entry.asm -f elf -o kernel_entry.o

ld -T NUL -o kernel.tmp -Ttext 0x1000 kernel_entry.o kernel.o

objcopy -O binary -j .text  kernel.tmp kernel.bin 

del /f kernel.tmp
del /f kernel.o

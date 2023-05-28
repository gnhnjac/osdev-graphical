# Automatically expand to a list of existing files that
# match the patterns
C_SOURCES = $(wildcard kernel/*.c drivers/*.c utils/*.c)
HEADERS = $(wildcard deps/*.h)
ASM_FILES = $(wildcard boot/routines/*.asm)

# TODO : Make sources dep on all header files.

# Create a list of object files to build , simple by replacing
# the ’.c’ extension of filenames in C_SOURCES with ’.o’
OBJ = ${C_SOURCES:.c=.o}

emu = QEMU

DEPS := $(OBJ:.o=.d)

# Default make target .
all: pre-build os-image

# Run bochs
run: all
	@bochs
	
# Run qemu
runq: all
	@run_qemu_no_net.bat

pre-build:
	@python update_headers.py

# @copy /b boot\boot_sect.bin+boot\2nd_stage.bin+kernel.bin os-image > nul
# @python check_size_validity.py
# This is the actual disk image that the computer loads,
# which is the combination of our compiled bootsector and kernel
os-image: boot/BOOTLDR.SYS boot/STAGE2.SYS KERNEL.SYS
	@dd if=/dev/zero of=floppy.img bs=512 count=2880
	@dd if=boot/BOOTLDR.SYS of=floppy.img bs=512 count=1
	@WINIMAGE floppy.img /H /I boot/STAGE2.SYS
	@WINIMAGE floppy.img /H /I KERNEL.SYS
	@dd if=floppy.img of=os-image bs=512 count=2800

# Link kernel object files into one binary, making sure the
# entry code is right at the start of the binary.
# $^ is substituted with all of the target's dependency files
KERNEL.SYS: kernel/kernel_entry.o ${OBJ}
	@ld -m i386pe -o kernel.tmp -T link.ld $^
	@objcopy --set-section-flags .bss=alloc,load,contents -O binary kernel.tmp KERNEL.SYS
	@del /f kernel.tmp

-include ${DEPS}

# Generic rule for building ’somefile.o’ from ’somefile.c’
%.o: %.c
	@gcc -m32 -ffreestanding -MMD -c $< -I ./deps -o $@ -D $(emu)

# Build our kernel entry object file.
#$< is the first dependancy and $@ is the target file
%.o: %.asm
	@nasm $< -f elf -o $@

	

# Assemble the boot sector to raw machine code
# The -I options tells nasm where to find our useful assembly
# routines that we include in boot_sect.asm
%.SYS : %.asm ${ASM_FILES}
	@nasm $< -f bin -I 'boot/routines/' -o $@

clean:
	@del /s /q *.bin 
	@del /s /q *.SYS
	@del /s /q *.o
	@del /s /q *.dis
	@del /s /q *.img
	@del /s /q *.d
	@del os-image

inspect:
	@WINIMAGE os-image

# Disassemble our kernel - might be useful for debugging .
kernel.dis: kernel.bin
	@ndisasm -b 32 $< > $@
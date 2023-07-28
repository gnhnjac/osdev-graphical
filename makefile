# Automatically expand to a list of existing files that
# match the patterns
C_SOURCES = $(wildcard kernel/*.c drivers/*.c utils/*.c)
HEADERS = $(wildcard deps/*.h)
ASM_FILES = $(wildcard boot/routines/*.asm)

ROOT_C_SOURCES = $(wildcard root/script/*.c)
ROOT_C_EXE = ${ROOT_C_SOURCES:.c=.exe}

ROOT_C_LIB = $(wildcard root/cstdlib/*.c)

# TODO : Make sources dep on all header files.

# Create a list of object files to build , simple by replacing
# the ’.c’ extension of filenames in C_SOURCES with ’.o’
OBJ = ${C_SOURCES:.c=.o}

emu = QEMU

DEPS := $(OBJ:.o=.d)

CFLAGS = -m32 -ffreestanding -MMD -c $< -I ./deps

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
os-image: boot/bootldr.sys boot/stage2.sys kernel.sys ${ROOT_C_EXE}
	@dd if=/dev/zero of=floppy.img bs=512 count=2880
	@dd if=boot/bootldr.sys of=floppy.img bs=512 count=1
	@copy /B /N /Y boot\stage2.sys root\STAGE2.SYS
	@copy /B /N /Y kernel.sys root\KERNEL.SYS
	@WINIMAGE floppy.img /H /I root
	@dd if=floppy.img of=os-image bs=512 count=2800

# Link kernel object files into one binary, making sure the
# entry code is right at the start of the binary.
# $^ is substituted with all of the target's dependency files
kernel.sys: kernel/kernel_entry.o ${OBJ}
	@ld -m i386pe -o kernel.tmp -T link.ld $^
	@objcopy --set-section-flags .bss=alloc,load,contents -O binary kernel.tmp kernel.sys
	@del /f kernel.tmp

-include ${DEPS}

# Generic rule for building ’somefile.o’ from ’somefile.c’
%.o: %.c
	gcc $(CFLAGS) -o $@ -D $(emu)

# Rule for building root c files
%.exe: %.c ${ROOT_C_LIB}
	@gcc -fno-builtin -m32 -nostdlib -I ./root/cstdlib $^ -o $@


# Build our kernel entry object file.
#$< is the first dependancy and $@ is the target file
%.o: %.asm
	@nasm $< -f elf -o $@

	

# Assemble the boot sector to raw machine code
# The -I options tells nasm where to find our useful assembly
# routines that we include in boot_sect.asm
%.sys : %.asm ${ASM_FILES}
	@nasm $< -f bin -I 'boot/routines/' -o $@

clean:
	@del /s /q *.bin 
	@del /s /q *.SYS
	@del /s /q *.o
	@del /s /q *.dis
	@del /s /q *.img
	@del /s /q *.d
	@del /s /q *.exe
	@del os-image

inspect:
	@WINIMAGE os-image

# Disassemble our kernel - might be useful for debugging .
kernel.dis: kernel.sys
	@ndisasm -b 32 $< > $@
# Automatically expand to a list of existing files that
# match the patterns
C_SOURCES = $(wildcard kernel/*.c drivers/*.c utils/*.c)
HEADERS = $(wildcard deps/*.h)

# TODO : Make sources dep on all header files.

# Create a list of object files to build , simple by replacing
# the ’.c’ extension of filenames in C_SOURCES with ’.o’
OBJ = ${C_SOURCES:.c=.o}

emu = QEMU

# Default make target .
all: clean pre-build os-image

# Run bochs
run: all
	@bochs
	
# Run qemu
runq: all
	@run_qemu.bat

pre-build:
	@python update_headers.py

# This is the actual disk image that the computer loads,
# which is the combination of our compiled bootsector and kernel
os-image: boot/boot_sect.bin kernel.bin
	@copy /b boot\boot_sect.bin+kernel.bin os-image > nul
	@python check_size_validity.py

# Link kernel object files into one binary, making sure the
# entry code is right at the start of the binary.
# $^ is substituted with all of the target's dependency files
kernel.bin: kernel/kernel_entry.o ${OBJ}
	@ld -m i386pe -o kernel.tmp -T link.ld $^
	@objcopy -O binary kernel.tmp kernel.bin
	@del /f kernel.tmp

# Generic rule for building ’somefile.o’ from ’somefile.c’
%.o: %.c ${HEADERS}
	@gcc -m32 -ffreestanding -c $< -I ./deps -o $@ -D $(emu)

# Build our kernel entry object file.
#$< is the first dependancy and $@ is the target file
%.o: %.asm
	@nasm $< -f elf -o $@

	

# Assemble the boot sector to raw machine code
# The -I options tells nasm where to find our useful assembly
# routines that we include in boot_sect.asm
%.bin : %.asm
	@nasm $< -f bin -I 'boot/routines/' -o $@

clean:
	@del /s /q *.bin 
	@del /s /q *.o
	@del /s /q *.dis
	@del os-image

# Disassemble our kernel - might be useful for debugging .
kernel.dis: kernel.bin
	@ndisasm -b 32 $< > $@
set arg1=%1

nasm -f bin %arg1% -o boot_sect.bin

copy /b boot_sect.bin+kernel.bin os-image
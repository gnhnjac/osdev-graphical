qemu-system-x86_64 -drive file=os-image,if=floppy,format=raw ^
 				   -device rtl8139,netdev=net0 ^
				   -netdev tap,id=net0,ifname=my-tap,script=no,downscript=no ^
				   -object filter-dump,id=f1,netdev=net0,file=dump.dat
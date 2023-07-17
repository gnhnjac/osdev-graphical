qemu-system-x86_64 -drive file=os-image,if=floppy,format=raw ^
 				   -net nic,model=rtl8139 ^
				   -net tap,ifname=my-tap,script=no,downscript=no ^
				   -monitor stdio
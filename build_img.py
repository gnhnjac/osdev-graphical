with open("disk.img","wb") as disk:
    with open("boot/boot_sect.bin", "rb") as boot_sect:
        with open("kernel.bin","rb") as kernel:
            b = boot_sect.read()
            k = kernel.read()
            length = len(b) + len(k)
            disk.write(b+k)
            # pad
            
            while length % 512 != 0: # 1.44mb 512 bytes per sector
                disk.write(b'\x00')
                length+=1
            
            print(f"disk length is {length} bytes")
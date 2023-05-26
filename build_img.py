with open("disk.img","wb") as disk:
    with open("os-image", "rb") as osimage:
        data = osimage.read()
        length = len(data)
        disk.write(data)
        # pad
        
        while length % 512 != 0: # 1.44mb 512 bytes per sector
            disk.write(b'\x00')
            length+=1
        
        print(f"disk length is {length} bytes")
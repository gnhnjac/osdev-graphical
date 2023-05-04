import os
import re
import sys

SECTOR_INCREMENT = 10

def query_yes_no(question, default="yes"):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
            It must be "yes" (the default), "no" or None (meaning
            an answer is required of the user).

    The "answer" return value is True for "yes" or False for "no".
    """
    valid = {"yes": True, "y": True, "ye": True, "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = input().lower()
        if default is not None and choice == "":
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' (or 'y' or 'n').\n")


filename = 'kernel.bin'

file_stats = os.stat(filename)

with open('boot/2nd_stage.asm', 'r') as f:
	boot_sector_file_contents = f.read()
	for line in boot_sector_file_contents.split('\n'):
		match = re.findall('^.*push +(\d+) +; sectors to be read.*$',line)
		if match:
			current_kernel_sectors = int(match[0])


kernel_capacity = current_kernel_sectors*512
image_size = file_stats.st_size

if image_size > kernel_capacity:
	print(f'Image size exceeds image capacity of {kernel_capacity} bytes')
	if query_yes_no('Do you want us to update it for you?'):
		with open('boot/boot_sect.asm', 'w') as f:
			f.write(boot_sector_file_contents.replace(f'push {current_kernel_sectors}', f'push {current_kernel_sectors+SECTOR_INCREMENT}'))
		print(f"New capacity: {100*image_size/((current_kernel_sectors+SECTOR_INCREMENT)*512):.1f}%")

elif image_size/kernel_capacity > 0.9:
	print(f'**WARNING** Image size is at {100*image_size/kernel_capacity:.1f}% of the image capacity')
else:
	print(f'image size: {image_size} bytes\nimage capacity is at {100*image_size/kernel_capacity:.1f}%')
import os
import subprocess

for root, subdirs, files in os.walk(os.getcwd()):

	for filename in files:

		file_path = os.path.join(root, filename)
		if filename.endswith('.c') and 'kernel' not in filename:
			print("Making headers for " + filename + ":")
			subprocess.run(["python", "header_make.py", file_path])
			print("-------------------------------")
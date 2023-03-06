import re
import sys
from pathlib import Path
from os import path

def remove_last_occurrence(string, substring):
    return ''.join(string.rsplit(substring, 1))

if len(sys.argv) != 2:
	print("Not enough parameters.")
	sys.exit(0);

source_file = sys.argv[1];

source_file_name = path.basename(source_file);

header_file = Path(f'deps/{source_file_name[:-1]}h')

header_file.touch(exist_ok=True)

source_matches = []
header_matches = []

with open(source_file, 'r') as f:
	for line in f.readlines():
		found = re.findall(r'^[^\(\)\t(    )]+ [^\(\)]+ *\(.*\) *{? *$', line)
		if found:
			source_matches.append(remove_last_occurrence(found[0], '{').rstrip())

with open(header_file, 'r') as f:
	header_file_contents = f.read().split('//refs', 1)[0] + '//refs'
with open(header_file, 'w') as f:
	f.write(header_file_contents)
	for func in source_matches:

		print(f"Adding {func}")
		f.write('\n'+func+';')
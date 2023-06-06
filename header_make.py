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

if ("root" in source_file):
	sys.exit(0)

source_file_name = path.basename(source_file);

header_file = Path(f'deps/{source_file_name[:-1]}h')

if not path.isfile(header_file):
	with open(header_file, 'w') as f:
		pass

source_matches = []

with open(source_file, 'r') as f:
	for line in f.readlines():
		found = re.findall(r'^[^\(\)\t(    )]+ [^\(\)]+ *\(.*\) *{? *$', line)
		if found:
			source_matches.append(remove_last_occurrence(found[0], '{').rstrip())

with open(header_file, 'r') as f:
	contents = f.read()
	header_file_contents = contents.split('//refs', 1)[0] + '//refs'
	header_file_functions = list(map(lambda x: x[:-1],filter(lambda x: x != '', contents.split('//refs', 1)[1].split('\n'))))

if header_file_functions == source_matches:
	sys.exit(0)

print("Making headers for " + source_file)
with open(header_file, 'w') as f:
	f.write(header_file_contents)
	for func in source_matches:

		#print(f"Adding {func}")
		f.write('\n'+func+';')
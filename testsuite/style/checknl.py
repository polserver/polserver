#!/usr/bin/env python3

'''
Checks that all files have correct line endings

@author Bodom
'''

import os, sys

import util


class Main(util.SourceChecker):

	# Extensions of binary files, will be ignored
	BINEXTS = ('.jpg', '.rar', '.zip', '.exe', '.dll', '.doc', '.lib', '.bz2', '.aps', '.ico')
	# Extensions of always-unix files
	UNIXEXTS = ('.sh', )
	# Extensions of always-windows files
	WINEXTS = ('.vcproj', '.bat')

	WHAT = 'line endings'

	def __init__(self, quiet=False):
		super().__init__(quiet)

		# Just to be sure
		if os.linesep != '\r\n' and os.linesep != '\n':
			print(os.linesep)
			raise NotImplementedError('Unhandled line ending style')

	def checkFile(self, path, ext):
		if ext in self.BINEXTS:
			return self.SKIP

		if ext in self.UNIXEXTS:
			nl = 'unix'
		elif ext in self.WINEXTS:
			nl = 'windows'
		else:
			nl = 'auto'

		return self.checkLineEndings(path, nl)

	def checkLineEndings(self, path, nl='auto'):
		''' Checks for coherent line endings on the given text file '''

		if nl not in ('auto', 'unix', 'windows'):
			raise ValueError("Unknown newline style {}".format(type))

		if nl == 'auto':
			if os.linesep == '\r\n':
				nl = 'windows'
			else:
				nl = 'unix'

		with open(path, 'rb') as f:
			lines = f.readlines()
			i = 0
			unix = False
			windows = False
			for line in lines:
				i += 1
				if line.endswith(b'\r\n'):
					windows = True
				elif line.endswith(b'\n'):
					unix = True
				elif i == len(lines):
					# Last line may have no ending
					pass
				else:
					print(line)
					raise RuntimeError('This should never happen')

		if windows and unix:
			print('ERROR: mixed line endings in "{}"'.format(path))
			return False
		elif nl=='windows' and unix:
			print('ERROR: Unix line endings in "{}"'.format(path))
			return False
		elif nl=='unix' and windows:
			print('ERROR: Windows line endings in "{}"'.format(path))
			return False

		return True


if __name__ == '__main__':
	# Includes not needed when used as module
	import argparse

	# Parse command line
	parser = argparse.ArgumentParser(description=__doc__.strip().split('\n',1)[0].strip())
	parser.add_argument('-q', '--quiet', action='store_true', help="Quiet output: only display errors and summary")
	args = parser.parse_args()

	if Main(args.quiet).run():
		sys.exit(0)
	sys.exit(1)

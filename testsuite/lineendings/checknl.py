#!/usr/bin/env python3

'''
Checks that all files have correct line endings

@author Bodom
'''

import os, sys
import subprocess


class Main:

	# Extensions of binary files, will be ignored
	BINEXTS = ('.jpg', '.rar', '.zip', '.exe', '.dll', '.doc', '.lib', '.bz2', '.aps')
	# Extensions of always-unix files
	UNIXEXTS = ('.sh', )
	# Extensions of always-windows files
	WINEXTS = ('.vcproj', '.bat')

	def __init__(self):
		mydir = os.path.dirname(os.path.realpath(__file__))
		self.polroot = os.path.realpath(os.path.join(mydir, '..', '..'))

		# Just to be sure
		if os.linesep != '\r\n' and os.linesep != '\n':
			print(os.linesep)
			raise NotImplementedError('Unhandled line ending style')

	def run(self):
		print("Checking line endings, POL root is {}".format(self.polroot))

		analyzed = 0
		errors = 0
		for root, dir, files in os.walk(self.polroot):
			rel = os.path.relpath(root, self.polroot)
			if rel.startswith('.git'):
				continue

			for file in files:
				full = os.path.join(root, file)

				if self.isGitIgnored(full):
					continue

				base, ext = os.path.splitext(file)
				if ext in self.BINEXTS:
					continue

				if ext in self.UNIXEXTS:
					nl = 'unix'
				elif ext in self.WINEXTS:
					nl = 'windows'
				else:
					nl = 'auto'

				analyzed += 1
				if not self.checkLineEndings(full, nl=nl):
					errors += 1

		print("Done. {} files analyzed, {} errors.".format(analyzed, errors))
		if errors:
			return False
		return True


	def isGitIgnored(self, path):
		''' Checks if file is ignored by git '''

		cmd = ('git', 'check-ignore', path)
		if subprocess.call(cmd):
			return False
		return True

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
	if Main().run():
		sys.exit(0)
	sys.exit(1)

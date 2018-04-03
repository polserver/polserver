#!/usr/bin/env python3

'''
Checks that all files have acceptable format

@author Bodom
'''

import sys
import difflib
import subprocess

import util


class Main(util.SourceChecker):

	# Extensions of source files to be checked
	SRCEXTS = ('.h', '.hpp', '.c', '.cpp')
	# Style options
	#ASTYLE_OPTIONS = (
	#	'--indent=spaces=2',
	#	'--convert-tabs',
	#	'--style=google',
	#	'--max-code-length=100',
	#	'--align-pointer=type',
	#	'--align-reference=type',
	#)

	IGNORE = ('.git', 'docs', 'lib', 'bin', 'bin-build')
	WHAT = 'coding style'

	def hightLightSpaces(self, lines):
		return [ l.replace(' ','.').replace('\t','<TAB>') for l in lines ]

	def checkFile(self, path, ext):
		if ext not in self.SRCEXTS:
			return self.SKIP

		if not self.checkKWStyle(path):
			return False

		if not self.checkIndent(path):
			return False

		return True

	def checkKWStyle(self, path):
		cmd = ('KWStyle', '-xml', 'style.kws.xml', '-gcc', '-v', path)
		proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		po, pe = proc.communicate()

		if proc.returncode == 0:
			assert not len(pe), pe
			return True

		print('File "{}" has invalid style'.format(path))
		print(po.decode(), pe.decode())
		return False

	def checkIndent(self, path):
		''' Indentation check for file '''
		levels = set()
		with open(path, 'rt') as file:
			for line in file:
				lspaces = len(line) - len(line.lstrip(' '))
				levels.add(lspaces)

		if 2 not in levels and (4 in levels or 8 in levels):
			print('File "{}" has 4 instead of 2 spaces indentation style'.format(path))
			return False

		return True

	def unusedClangcheckFile(self, path, ext):
		if ext not in self.SRCEXTS:
			return self.SKIP

		with open(path, 'rt') as f:
			try:
				originalraw = f.read()
			except UnicodeDecodeError as e:
				print('File "{}" has invalid unicode: {}'.format(path, e))
				return False
			original = originalraw.splitlines(keepends=True)

		#cmd = ['astyle'] + list(self.ASTYLE_OPTIONS)
		cmd = ('clang-format', '-style=file')
		proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		po, pe = proc.communicate(originalraw.encode())

		assert proc.returncode == 0, "Return code {}".format(proc.returncode)
		assert len(pe) == 0, "clang-format error: {}".format(pe)
		try:
			formatted = po.decode().splitlines(keepends=True)
		except UnicodeDecodeError as e:
			print('File "{}" has invalid unicode during formatting: {}'.format(path, e))
			return False

		diff = list(difflib.unified_diff(original, formatted, 'current', 'formatted'))
		if not len(diff):
			return True

		print('File "{}" has invalid style'.format(path))
		for d in difflib.unified_diff(self.hightLightSpaces(original), self.hightLightSpaces(formatted), 'current', 'formatted'):
			print(d, end='')
		return False


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

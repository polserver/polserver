#!/usr/bin/env python3

'''
Cleans the source tree

@author Bodom
'''

import os
import re
import shutil
import subprocess


# Files CMake generates beside the sources or inside a build directory, matched by
# exact name on purpose: the tree also holds hand-written cmake modules (cmake/,
# pol-core/*/CMakeSources.cmake) that a ".cmake" suffix match would destroy.
GENERATED_NAMES = frozenset((
	'CMakeCache.txt',
	'CPackConfig.cmake',
	'CPackSourceConfig.cmake',
	'CTestTestfile.cmake',
	'Makefile',
	'cmake_install.cmake',
	'pol_global_config.h',
))

# Never descended into when looking for build leftovers. "lib" holds the fetched
# dependencies, whose own sources carry files named like generated ones (zlib ships
# a Makefile); they come back by re-fetching, not by a rebuild.
SKIP_DIRS = frozenset(('.git', 'lib', 'testsuite'))


class Cleaner:
	''' Cleans the POL root '''

	def __init__(self, root, dryRun=False):
		self.root = os.path.abspath(root)
		self.dryRun = dryRun
		self.tracked, self.trackedDirs = self.__readIndex()

	def __readIndex(self):
		''' Returns (tracked files, their parent folders), or (None, None) if git cannot say '''
		try:
			out = subprocess.check_output(
				['git', '-C', self.root, 'ls-files', '-z'],
				stderr=subprocess.DEVNULL, universal_newlines=True)
		except (OSError, subprocess.CalledProcessError):
			return None, None

		root = os.path.normcase(self.root)
		files = set()
		dirs = set([root])
		for rel in out.split('\0'):
			if not rel:
				continue
			path = os.path.normcase(os.path.join(self.root, rel.replace('/', os.sep)))
			files.add(path)
			parent = os.path.dirname(path)
			while len(parent) > len(root) and parent not in dirs:
				dirs.add(parent)
				parent = os.path.dirname(parent)
		return files, dirs

	def __isTracked(self, path):
		''' True if git tracks this file, or tracks anything below this folder '''
		if self.tracked is None:
			return False
		path = os.path.normcase(os.path.abspath(path))
		return path in self.tracked or path in self.trackedDirs

	def __keep(self, paths):
		''' Drops whatever git tracks: the last guard against a misclassification '''
		return [p for p in paths if not self.__isTracked(p)]

	def __delFromList(self, paths):
		''' For internal usage '''
		for f in self.__keep(paths):
			if self.dryRun:
				print('  {}'.format(f))
			elif os.path.isdir(f):
				shutil.rmtree(f)
			else:
				os.unlink(f)

	def findDoxygen(self):
		''' Returns list of doxygen-generated files '''
		doxygen = os.path.join(self.root, 'docs', 'doxygen', 'html')
		if os.path.exists(doxygen):
			return self.__keep([doxygen])
		return []

	def clearDoxygen(self):
		self.__delFromList(self.findDoxygen())

	def findBoost(self):
		''' Returns list of boost extracted files '''
		ret = []
		lib = os.path.join(self.root, 'lib')
		if not os.path.isdir(lib):
			return ret
		for f in os.listdir(lib):
			path = os.path.join(lib, f)
			if os.path.isdir(path) and re.match(r'^boost_[0-9][0-9_.\-]*$', f):
				for f2 in os.listdir(path):
					if not f2.startswith('buildboost') and not f2 == '.gitignore':
						ret.append(os.path.join(path,f2))
		return self.__keep(ret)

	def clearBoost(self):
		self.__delFromList(self.findBoost())

	def findCmake(self):
		''' Returns list of cmake-generated files '''
		ret = []
		def readFolder(path):
			names = os.listdir(path)
			# A configured build folder goes whole, unless git tracks something in
			# it: such a folder keeps sources as well, so it is cleaned file by
			# file. Never the root either, where an in-source configure leaves a
			# CMakeCache.txt and the rule would take the checkout with it.
			if ('CMakeCache.txt' in names and path != self.root
					and not self.__isTracked(path)):
				ret.append(path)
				return
			for f in names:
				fp = os.path.join(path, f)
				if os.path.isdir(fp):
					if f == 'CMakeFiles':
						ret.append(fp)
					elif f not in SKIP_DIRS:
						readFolder(fp)
				elif f in GENERATED_NAMES:
					ret.append(fp)
		readFolder(self.root)
		return self.__keep(ret)

	def clearCmake(self):
		self.__delFromList(self.findCmake())

	def findBinaries(self):
		''' Returns list of binary files '''
		bin = os.path.join(self.root, 'bin')
		if not os.path.isdir(bin):
			return []
		return self.__keep([os.path.join(bin, f) for f in os.listdir(bin)
							if f != '.gitignore'])

	def clearBinaries(self):
		self.__delFromList(self.findBinaries())

	def findEmpty(self):
		''' Returns list of empty folders, innermost first '''
		ret = []
		def scan(path):
			''' Collects the empty folders below path, tells whether path ends up empty '''
			empty = True
			for f in os.listdir(path):
				fp = os.path.join(path, f)
				if os.path.isdir(fp) and f != '.git' and scan(fp):
					ret.append(fp)
				else:
					empty = False
			return empty
		scan(self.root)
		return self.__keep(ret)

	def clearEmpty(self):
		self.__delFromList(self.findEmpty())


if __name__ == '__main__':
	import argparse

	def yesNo(question, default=None):
		''' Asks the user a yes/no question '''
		valid = {
			"yes": True, "y": True, "ye": True,
			"no": False, "n": False
		}
		if default is None:
			prompt = "[y/n]"
		elif default:
			prompt = "[Y/n]"
		else:
			prompt = "[y/N]"

		while True:
			print("{} {} ".format(question,prompt), end='', flush=True)
			choice = input().lower()
			if default is not None and choice == '':
				return default
			elif choice in valid.keys():
				return valid[choice]
			print('Please respond with "y" or "n"')

	parser = argparse.ArgumentParser(description='Cleans the POL source tree')
	parser.add_argument('-n', '--dry-run', action='store_true',
						help='list what would be deleted and delete nothing')
	args = parser.parse_args()

	polRoot = os.path.dirname(os.path.abspath(__file__))
	c = Cleaner(polRoot, args.dry_run)

	print('POL cleaning utility, will delete files inside "{}"'.format(polRoot))
	if c.tracked is None:
		print('WARNING: git could not be read, files tracked by git are not protected')

	def step(question, found, clear, default):
		''' Reports how much a step would delete, then runs it unless refused '''
		if not found:
			return
		question = '{} ({} item{})'.format(
			question, len(found), '' if len(found) == 1 else 's')
		if c.dryRun:
			print(question)
			clear()
		elif yesNo(question, default):
			clear()

	step('Delete doxygen generated files?', c.findDoxygen(), c.clearDoxygen, False)
	step('Delete boost unpacked files?', c.findBoost(), c.clearBoost, False)
	step('Delete cmake and intermediate build files?', c.findCmake(), c.clearCmake, True)
	step('Delete built binaries?', c.findBinaries(), c.clearBinaries, True)
	step('Delete empty folders?', c.findEmpty(), c.clearEmpty, True)

	if not c.dryRun and yesNo('Show list of files ignored by git?', False):
		subprocess.call(['git', '-C', polRoot, 'ls-files', '--others', '-i',
						 '--exclude-standard'])

	print('completed.')

#!/usr/bin/env python3

'''
Cleans the source tree

@author Bodom
'''

import os
import shutil
import re


class Cleaner:
	''' Cleans the POL root '''

	def __init__(self, root):
		self.root = root

	def __delFromList(self, list):
		''' For internal usage '''
		for f in list:
			if os.path.isdir(f):
				shutil.rmtree(f)
			else:
				os.unlink(f)

	def findDoxygen(self):
		''' Returns list of doxygen-generated files '''
		doxygen = os.path.join(self.root, 'docs', 'doxygen', 'html')
		if os.path.exists(doxygen):
			return [doxygen]
		return None

	def clearDoxygen(self):
		self.__delFromList(self.findDoxygen())

	def findBoost(self):
		''' Returns list of boost extracted files '''
		ret = []
		lib = os.path.join(self.root, 'lib')
		for f in os.listdir(lib):
			path = os.path.join(lib, f)
			if os.path.isdir(path) and re.match(r'^boost_[0-9_]+$', f):
				for f2 in os.listdir(path):
					if not f2.startswith('buildboost') and not f2 == '.gitignore':
						ret.append(os.path.join(path,f2))
		return ret

	def clearBoost(self):
		self.__delFromList(self.findBoost())

	def findCmake(self):
		''' Returns list of cmake-generated files '''
		ret = []
		def readFolder(path):
			r = []
			for f in os.listdir(path):
				fp = os.path.join(path, f)
				if os.path.isdir(fp):
					if f == 'CMakeFiles':
						ret.append(fp)
					elif f != 'testsuite':
						ret.extend(readFolder(fp))
				else:
					if f == 'FindSetEnv.cmake':
						pass
					elif f in ('Makefile', 'CMakeCache.txt', 'pol_global_config.h'):
						ret.append(fp)
					elif f.endswith('.cmake'):
						ret.append(fp)
			return r
		ret.extend(readFolder(self.root))
		return ret

	def clearCmake(self):
		self.__delFromList(self.findCmake())

	def findBinaries(self):
		''' Returns list of binary files '''
		ret = []
		bin = os.path.join(self.root, 'pol-core', 'bin')
		for f in os.listdir(bin):
			fp = os.path.join(bin, f)
			if f != '.gitignore' and os.path.isfile(fp):
				ret.append(fp)
		return ret

	def clearBinaries(self):
		self.__delFromList(self.findBinaries())

	def findEmpty(self):
		''' Returns list of empty folders '''
		ret = []
		for folder, subfolders, files in os.walk(self.root):
			if not subfolders and not files:
				ret.append(os.path.join(self.root,folder))
		return ret

	def clearEmpty(self):
		self.__delFromList(self.findEmpty())


if __name__ == '__main__':
	import argparse
	import subprocess

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

	polRoot = os.path.dirname(os.path.abspath(__file__))
	c = Cleaner(polRoot)

	print('POL cleaning utility, will delte files inside "{}"'.format(polRoot))

	if c.findDoxygen() and yesNo('Delete doxygen generated files?', False):
		c.clearDoxygen()

	if c.findBoost() and yesNo('Delete boost unpacked files?', False):
		c.clearBoost()

	if c.findCmake() and yesNo('Delete cmake and intermediate build files?', True):
		c.clearCmake()

	if c.findBinaries() and yesNo('Delete built binaries?', True):
		c.clearBinaries()

	if c.findEmpty() and yesNo('Delete empty folders?', True):
		c.clearEmpty()

	if yesNo('Show list of files ignored by git?', False):
		c = ['git', 'ls-files', '--others', '-i', '--exclude-standard']
		subprocess.call(c)

	print('completed.')

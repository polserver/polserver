#!/usr/bin/env python3

'''
Runs formatting on the source tree using aformat

@author Bodom
'''

import os
import shutil
import re
import subprocess


class Formatter:
	''' Formats the source code '''

	ASTYLE_OPTIONS = (
		'--indent=spaces=2',
		'--indent-col1-comments',
		'--convert-tabs',
		'--style=allman',
		'--align-pointer=type',
		'--align-reference=type',
	)

	def __init__(self, root):
		self.root = root

	def run(self):
		count = 0
		baseCmd = ['/usr/bin/env', 'astyle']
		baseCmd.extend(self.ASTYLE_OPTIONS)

		for src in self.findCSources():
			count += 1
			cmd = baseCmd[:]
			cmd.append(src)
			subprocess.check_call(cmd)

		return count

	def findCSources(self):
		''' Returns list of C sources '''
		ret = []
		core = os.path.join(self.root, 'pol-core')
		for folder, subfolders, files in os.walk(core):
			for file in files:
				path = os.path.join(core, folder, file)
				root, ext = os.path.splitext(file)
				if ext in ('.c', '.h', '.cpp', '.hpp'):
					ret.append(path)
		return ret

if __name__ == '__main__':
	import argparse

	polRoot = os.path.dirname(os.path.abspath(__file__))
	f = Formatter(polRoot)

	print('POL formatting utility, will format files inside of "{}"')

	count = f.run()

	print('completed. {} files formatted.'.format(count))

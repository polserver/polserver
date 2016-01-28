#!/usr/bin/env python3

'''
	Reformats the archaic "history" file headers into doxygen syntax.
	This is not meant to be a fully automated script, some manual intervention
	will still be needed

	This is probably a one-shot script that can be deleted.
'''

import os
import re


class Main:

	def __init__(self):
		self.cRe = re.compile(rb'\/\*.*?\*\/', re.MULTILINE | re.DOTALL)

	def run(self):
		myFolder = os.path.dirname(os.path.realpath(__file__))
		polRoot = os.path.join(myFolder,'..','pol-core')

		exts = ('.c', '.cpp', '.h', '.hpp')
		for subdir, dirs, files in os.walk(polRoot):
			for file in files:
				for ext in exts:
					if file.endswith(ext):
						try:
							self.reformatFile(os.path.join(polRoot,subdir,file))
						except NoHistoryBlockFound as e:
							pass
						break

	def reformatFile(self, path):

		with open(path, 'rb') as f:
			source = f.read()

		m = self.cRe.search(source)
		if not m:
			# Multicomment block not found at all
			return

		# Analyze and replace
		i = 0
		nl = None
		lines = m.group(0).split(b'\n')
		new = b''
		status = 'start'
		firstNote = True
		for i, line in enumerate(lines):
			ls = line.strip()

			if i == len(lines) - 1:
				if status not in ('inHistory', 'inNotes'):
					raise NoHistoryBlockFound('invalid status {} on last line'.format(status), line)

				if ls != b'*/':
					raise NoHistoryBlockFound('no closing tag on last line', line)

				new += b' */' +  nl
				break

			if status == 'start':
				if ls != b'/*':
					raise NoHistoryBlockFound('first line is not an empty comment start', line)

				# Detect line endings
				if line[-2:] == b'\r\n':
					nl = b'\r\n'
				else:
					nl = b'\n'

				new += b'/** @file' + nl
				new += b' *' + nl
				status = 'waitHistory'
				continue

			if status == 'waitHistory':
				# Waits for the "history" tag
				if ls == b'':
					continue

				if ls.lower() != b'history':
					raise NoHistoryBlockFound('this does not seem to be an history comment', line)

				new += b' * @par History' + nl
				status = 'inHistory'
				continue

			if status in ('inHistory', 'inNotes'):
				if ls == b'' or ls.strip(b'=') == b'':
					# Ignoring a "=======" or empty line
					continue

			if status == 'inHistory':
				# Reading history, be aware of notes
				if ls.lower() == b'notes':
					# Found start of notes
					status = 'inNotes'
					continue

				new += b' * - ' + ls + nl
				continue

			if status == 'inNotes':
				# Reading notes, if any
				if firstNote:
					new += b' *' + nl
					firstNote = False
				new += b' * @note ' + ls + nl
				continue

			raise RuntimeError('Unhandled status {}'.format(status))

		#print(b'\n'.join(lines).decode())
		#print(new.decode())

		rep = self.cRe.sub(new, source, 1)

		with open(path, 'wb') as f:
			source = f.write(rep)


class NoHistoryBlockFound(Exception):
	''' Was useful during debug '''
	def __init__(self, message, line):
		super().__init__(self, message)
		self.line = line


if __name__ == '__main__':
	Main().run()

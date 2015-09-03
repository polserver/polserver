#!/usr/bin/env python3

'''
This script will roughly check doc consistency for .em files.
At the moment, problems founds are intended to be solved by hand.
'''

import os
import re
import xml.etree.ElementTree as ET


class EmFile:
	''' Rough .em file parser '''

	CONSTRE = re.compile(r'^const\s+([A-Z0-9_]+)\s*:=\s*(.*);?$')
	FUNCRE = re.compile(r'^([A-Za-z0-9_]+)\s*\(([^)]*)\)\s*;?$')

	def __init__(self, path, name):
		self.path = path
		self.name = name
		self.consts = {}
		self.funcs = {}

		with open(self.path, 'rt') as f:
			for line in f:
				# Clean the line and strip comments
				line = self.stripLine(line)

				if not line:
					continue

				# Check if this is a constant declaration
				name, val = self.__parseConst(line)
				if name:
					self.consts[name] = val
					continue

				# Check if this is a function declaration
				name, args = self.__parseFunc(line)
				if name:
					self.funcs[name] = args
					continue

				raise NotImplementedError("Couldn't parse line:\n{}".format(line))

	@staticmethod
	def stripLine(line):
		line = line.strip()
		commentStart = line.find('//')
		if commentStart != -1:
			line = line[:commentStart].strip()
		return line

	def __parseConst(self, line):
		m = self.CONSTRE.match(line)
		if not m:
			return None, None

		return m.group(1), m.group(2)

	def __parseFunc(self, line):
		m = self.FUNCRE.match(line)
		if not m:
			return None, None

		return m.group(1), list(map(lambda i: i.strip(), m.group(2).split(',')))

	def __str__(self):
		return "EMFile<{}, consts: {}, funcs: {}".format(self.path, self.consts, self.funcs)


class Main:

	def run(self):
		# 1. Load and parse the .em files
		emfiles = {}
		for root, subdirs, files in os.walk(os.path.join('..','pol-core','support','scripts')):
			for f in files:
				if f.endswith('.em'):
					path = os.path.join(root,f)
					name = os.path.splitext(f)[0]
					print('Loading {}...'.format(f), end='')
					emfiles[name] = EmFile(path, name)
					print('done.')

		# 2. Load and parse the mail modules.xml, start doing some checks
		docPath = os.path.join('..','docs','docs.polserver.com','pol099')
		xmlfiles = {}
		modules = ET.parse(os.path.join(docPath,'modules.xml'))
		modlist = []
		for m in modules.getroot().findall('em'):
			name = m.get('name')[:-2]
			if name not in emfiles.keys():
				print("ERROR: unknown module {} defined in modules.xml".format(name))
			modlist.append(name)
		for name in emfiles.keys():
			if name not in modlist:
				print("ERROR: module {} is not defined in modules.xml".format(name))

		# 3. Load and parse every module's XML defintion, do rest of the checks
		for modfile in modlist:
			xmldoc = ET.parse(os.path.join(docPath,modfile+'em.xml'))

			consts = []
			for const in xmldoc.findall('./fileheader/constant'):
				txt = EmFile.stripLine(const.text.strip())
				if not txt:
					continue
				m = EmFile.CONSTRE.match(txt)
				if not m:
					raise NotImplementedError('Unable to parse constant "{}"'.format(txt))
				name = m.group(1)
				val = m.group(2)
				consts.append(name)

				if name not in emfiles[modfile].consts.keys():
					print("ERROR: constant {} is documented in {}em.xml but no longer existing in {}.em".format(name, modfile, modfile))
				elif val != emfiles[modfile].consts[name]:
					print('ERROR: constant {} value differs. em: "{}", xml: "{}"'.format(name, val, emfiles[modfile].consts[name]))

			for const in emfiles[modfile].consts.keys():
				if const not in consts:
					print("ERROR: constant {} is defined in {}.em but not documented in {}em.xml".format(const, modfile, modfile))

			funcs = []
			for func in xmldoc.getroot().findall('function'):
				proto = EmFile.stripLine(func.find('prototype').text.replace('\n',' '))
				m = EmFile.FUNCRE.match(proto)
				if not m:
					raise NotImplementedError('Unable to parse function "{}"'.format(proto))
				name = m.group(1)
				args = list(map(lambda i: i.strip(), m.group(2).split(',')))
				funcs.append(name)

				infile = True
				if name not in emfiles[modfile].funcs.keys():
					infile = False
					print("ERROR: function {} is documented in {}em.xml but no longer existing in {}.em".format(name, modfile, modfile))
				elif sorted(map(lambda i: i.lower(),args)) != sorted(map(lambda i: i.lower(),emfiles[modfile].funcs[name])):
					print('ERROR: function {} args differs. em: "{}", xml: "{}"'.format(name, emfiles[modfile].funcs[name], args))

				if infile:
					parameters = list(map(lambda i: i.get('name').strip().split(' ')[0].split(':')[0].strip().lower(), func.findall('parameter')))
					empars = list(map(lambda i:                 i.strip().split(' ')[0].split(':')[0].strip().lower(), emfiles[modfile].funcs[name]))
					for p in parameters:
						if p and p not in empars:
							print("ERROR: parameter {} for function {} is documented in {}em.xml but no longer existing in {}.em".format(p, name, modfile, modfile))
					for p in empars:
						if p and p not in parameters:
							print("ERROR: parameter {} for function {} is defined in {}.em but not documented in {}em.xml".format(p, name, modfile, modfile))

			for func in emfiles[modfile].funcs.keys():
				if func not in funcs:
					print("ERROR: function {} is defined in {}.em but not documented in {}em.xml".format(func, modfile, modfile))

		# 4. Done!
		print()
		print("Done. If you didn't see any error above, then em files and their doc seems to be in sync. Good luck.")


if __name__ == '__main__':
	Main().run()

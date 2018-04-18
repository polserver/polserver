#!/usr/bin/env python3

'''
Generates the unicodedata C sources
'''

import os
import re


class UCDataEntry:
	def __init__(self, raw):
		fields = raw.split(';')
		assert len(fields) == 15, fields

		self.name = int(fields[0], 16)
		self.slower = int(fields[12], 16) if len(fields[12]) else None
		self.supper = int(fields[13], 16) if len(fields[13]) else None
		self.stitle = int(fields[14], 16) if len(fields[14]) else None

	def __repr__(self):
		return "{name}: L{slower} U{supper} T{stitle}".format(**self.__dict__)


class Main:
	UNICODEDATA = 'UnicodeData.txt'
	CPPTPL = 'unicodedata-tpl.h'
	OUTH = 'unicodedata.h'

	def __init__(self):
		self.path = os.path.dirname( os.path.abspath(__file__) )
		self.unicodeDataPath = os.path.join(self.path, self.UNICODEDATA)
		self.cppTplPath = os.path.join(self.path, self.CPPTPL)
		self.outPath = os.path.join(self.path, self.OUTH)

		self.lowre = re.compile(r'(\s*)TOLOWER_CASES')
		self.uppre = re.compile(r'(\s*)TOUPPER_CASES')

		self.udata = {}

	def run(self):
		# Load UnicodeData.txt
		with open(self.unicodeDataPath, 'rt') as f:
			for line in f:
				ude = UCDataEntry( line.rstrip('\n') )
				assert ude.name not in self.udata
				self.udata[ude.name] = ude

		# Load template an write output
		with open(self.cppTplPath, 'rt') as fi, open(self.outPath, 'wt') as fo:
			for line in fi:
				m = self.lowre.match(line)
				if m:
					self.outputLowerCode(fo, m.group(1))
					continue

				m = self.uppre.match(line)
				if m:
					self.outputUpperCode(fo, m.group(1))
					continue

				fo.write(line)

	def __outputCaseCode(self, fo, indent, propname):
		for key in sorted(self.udata):
			ude = self.udata[key]
			if getattr(ude, propname):
				fo.write("{}case 0x{:02x}:\n".format(indent, ude.name))
				fo.write("{}  return 0x{:02x};\n".format(indent, getattr(ude, propname)))

	def outputLowerCode(self, fo, indent):
		return self.__outputCaseCode(fo, indent, 'supper')

	def outputUpperCode(self, fo, indent):
		return self.__outputCaseCode(fo, indent, 'slower')


if __name__ == '__main__':
	Main().run()

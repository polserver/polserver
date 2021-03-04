#!/usr/bin/env python3

'''
This script builds corechanges.xml from core-changes.txt
'''

import os
import re
import datetime
import xml.etree.ElementTree as ET
import xml.dom.minidom


class PolRelease:
	''' Prepresents a POL release '''
	def __init__(self, name):
		self.name = name
		self.changes = []

	def __iter__(self):
		return self.changes.__iter__()

	def __next__(self):
		self.changes.__next__()


class Change:
	''' Represents a single change '''
	def __init__(self, date, author):
		if date is None:
			self.date = None
		else:
			if len(date) == 4 or len(date) == 3:
				date = '-'.join(map(lambda i: i.zfill(2), date.split('-')))

			if len(date) == 5:
				self.date = datetime.datetime.strptime(date, '%m-%d').date()
			elif len(date) == 8:
				self.date = datetime.datetime.strptime(date, '%m-%d-%y').date()
			else:
				self.date = datetime.datetime.strptime(date, '%m-%d-%Y').date()
		self.author = author.strip() if author is not None else None
		self.subs = []

	def __str__(self):
		return "{} {}\n".format(self.date, self.author) + "\n".join(map(str,self.subs))

	def __iter__(self):
		return self.subs.__iter__()

	def __next__(self):
		self.subs.__next__()


class SubChange:
	''' Repsesents a sub-change '''
	def __init__(self, type, indent):
		self.type = type.strip() if type is not None else None
		self.lines = []
		self.indentlevel = self.__calcIndentLevel(indent)

	def __calcIndentLevel(self, string):
		return len(string.replace('\t','   '))

	def addLine(self, line, indent):
		inddiff = self.__calcIndentLevel(indent) - self.indentlevel
		if inddiff < 0:
			inddiff = 0
		self.lines.append(' '*inddiff + line)

	def __str__(self):
		return "{}:\n".format(self.type) + "\n".join(self.lines)


class CoreChanges:
	''' Represents a core-changes file '''

	RELEASERE = re.compile(r'^-*\s*(POL[0-9.-A-Za-z_]+)\s*-*$')
	CHANGERE = re.compile(r'^([0-9]{1,2}-[0-9]{1,2}(?:-(?:[0-9]{2}){1,2})?)(?:\s+(.*))?:?$')
	CHANGEFIRST = re.compile(r'^(\s+)([A-za-z]+)(\s*):(\s+)(.*)$')
	CHANGELINE = re.compile(r'^(\s+)(.*)$')
	ENDRE = re.compile(r'^=+$')

	def __init__(self, path):
		self.path = path
		self.releases = []

		with open(self.path, 'rt', encoding='utf-8') as f:
			# Status variables
			currel = None
			curchange = None
			cursub = None

			l = 0
			for line in f:
				l += 1
				# Do some clieaning first (don't know why 0xfeff is there)
				line = line.rstrip('\n').rstrip('\r').rstrip().lstrip('\ufeff')

				# Skip empty lines
				if not line:
					continue

				# Will not go any further: format changes drammatically
				if self.ENDRE.match(line):
					if currel:
						if curchange:
							if cursub:
								curchange.subs.append(cursub)
								cursub = None
							currel.changes.append(curchange)
							curchange = None
							#print(curchange)
						self.releases.append(currel)
					break

				# Check for release line
				m = self.RELEASERE.match(line)
				if m:
					if currel:
						# Go on for next release
						if curchange:
							if cursub:
								curchange.subs.append(cursub)
								cursub = None
							currel.changes.append(curchange)
							curchange = None
							#print(curchange)
						self.releases.append(currel)
					currel = PolRelease(m.group(1))
					continue
				if not currel:
					raise LineParseError('Core Changes should start with latest release name', line, l)

				# Check for change start
				special = False
				if line == '(sometime in the past) Syzygy':
					# Had to hardcode this... no way to standardize :D
					special = True
				m = self.CHANGERE.match(line)
				if m or special:
					if curchange:
						# Go on for next change
						if cursub:
							curchange.subs.append(cursub)
							cursub = None
						currel.changes.append(curchange)
						#print(curchange)
					if special:
						curchange = Change(None, 'Syzygy')
					else:
						curchange = Change(m.group(1), m.group(2))
					continue
				if not curchange:
					raise LineParseError('Expecting a core change start', line, l)

				# Check for first change line
				m = self.CHANGEFIRST.match(line)
				if m:
					if not curchange:
						raise LineParseError('Got a change start line, but there is no active change', line, l)
					if cursub:
						curchange.subs.append(cursub)
					cursub = SubChange(m.group(2), m.group(1) + m.group(2) + m.group(3) + ':' + m.group(4))
					cursub.lines.append(m.group(5).strip())
					continue

				# Check for standard change line
				m = self.CHANGELINE.match(line)
				if m:
					if not curchange:
						raise LineParseError('Got a change line, but there is no active change', line, l)
					if not cursub:
						if currel.name not in ['POL096','POL095']:
							print("WARNING: malformed subchange at line {}".format(l))
						cursub = SubChange(None, m.group(1))
					cursub.addLine(m.group(2), m.group(1))
					continue

				# Unknown line
				raise LineParseError('Unknown line', line, l)

			else:
				raise RuntimeError('End of file reached, should\'ve got an end marker before')

	def __iter__(self):
		return self.releases.__iter__()

	def __next__(self):
		self.releases.__next__()


class LineParseError(Exception):
	def __init__(self, mex, line, linenum):
		super().__init__(mex)
		self.line = line
		self.linenum = linenum


class Main:

	def run(self):
		mydir = os.path.dirname(os.path.realpath(__file__))

		# Load core changes
		try:
			changes = CoreChanges(os.path.join(mydir,'..','pol-core','doc','core-changes.txt'))
		except LineParseError as e:
			print('ERROR parsing line {}: {}\nLine: "{}"'.format(e.linenum, e, e.line))
			return

		# Now generate the output XML
		root = ET.Element('ESCRIPT')
		head = ET.SubElement(root, 'header')
		topic = ET.SubElement(head, 'topic')
		topic.text = 'Latest Core Changes'
		datemod = ET.SubElement(head, 'datemodified')
		datemod.text = datetime.date.today().strftime('%m-%d-%Y')

		for rel in changes:
			version = ET.SubElement(root, 'version', {'name': rel.name})
			for change in rel:
				entry = ET.SubElement(version, 'entry')
				if change.date:
					date = ET.SubElement(entry, 'date')
					date.text = change.date.strftime('%m-%d-%Y')
					if date.text[-4:] == '1900':
						date.text = date.text[:-5]
				if change.author:
					author = ET.SubElement(entry, 'author')
					author.text = change.author

				for sub in change:
					chel = ET.SubElement(entry, 'change')
					if sub.type:
						chel.set('type', sub.type)
					if sub.lines:
						chel.text = '~~~NEWLINE~~~'.join(sub.lines)

		# Pretty format, with an ugly but functional workaround
		parsed = xml.dom.minidom.parseString(ET.tostring(root))
		out = parsed.toprettyxml()
		out = out.replace('~~~NEWLINE~~~', '<br/>\n')

		# Write
		outFile = os.path.join(mydir,'..','docs','docs.polserver.com','pol100','corechanges.xml')
		with open(outFile, 'wt', encoding='utf-8') as f:
			f.write(out)

		print("{} written.".format(outFile))


if __name__ == '__main__':
	Main().run()

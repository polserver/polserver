#!/usr/bin/env python3
import os, subprocess
import shutil
import sys
import codecs

def colorprint(text, color):
	if os.name == 'nt':
		print(text)
		return

	if color == 'cyan':
		code = 96
	elif color == 'green':
		code = 92
	elif color == 'red':
		code = 91
	else:
		raise NotImplementedError();

	out = '\033[{}m'.format(str(code)) + text + '\033[0m'
	print(out)


class Compare:
	@staticmethod
	def txtcompare(file1,file2):
		with codecs.open(file1,'r',encoding='utf-8',errors='replace') as f1, codecs.open(file2,'r',encoding='utf-8',errors='replace') as f2:
			l1=f1.readlines()
			l2=f2.readlines()
			if len(l1) != len(l2):
				print('line count differs')
				return False
			for c1,c2 in zip(l1,l2):
				if c1!=c2:
					print('"',c1,'"')
					print('"',c2,'"')
					return False
			return True

	@staticmethod
	def outputcompare(file):
		basename=os.path.splitext(file)[0]
		if not os.path.exists(basename+'.out'):
			with open(basename+'.tst', 'r') as tst:
				print(tst.read())
			return False
		return Compare.txtcompare(basename+'.out',basename+'.tst')

class Executor:
	def __init__(self,runecl):
		self.runecl=runecl

	def __call__(self,file,instructions=False):
		basename=os.path.splitext(file)[0]
		cmd='{0} -q {1}.ecl > {1}.{2}'.format(self.runecl,basename,'tst')
		try:
			return subprocess.check_output(cmd,shell=True,stderr=subprocess.STDOUT)
		except subprocess.CalledProcessError as e:
			print(e.cmd,e.output)

class Compiler:
	def __init__(self,comp):
		self.comp=comp

	def __call__(self, file):
		try:
			return subprocess.check_output(self.comp+' -l -xt -q -C ecompile.cfg '+file,shell=True, stderr=subprocess.STDOUT)
		except subprocess.CalledProcessError as e:
			print(e.cmd, e.output)

class TestFailed(Exception):
	pass

class StdTests:
	'''
	This script runs unit tests to validate the functionality of the escript
	compiler and perser.

	Unit test are organized in folders (packeges). Folders whose name starts with an
	underscore character, will be ignoed (disabled).
	To create a test, create a .src file into a package. That file will be executed
	and its output checked agains a .out file with the same name on the same
	package. If output matches, test is succesfull.
	'''

	def __init__(self, compiler, runecl, script=None):
		spkg, sfile = script.split('/')

		self.files = []
		for pkg in sorted(os.listdir('.')):
			if os.path.isdir(pkg):
				if pkg.startswith('_'):
					continue
				if spkg and pkg != spkg:
					continue
				for f in sorted(os.listdir(pkg)):
					file = os.path.join(pkg, f)
					if os.path.isfile(file) and file.endswith('.src'):
						if sfile and f != sfile + '.src':
							continue
						self.files.append(file)
		self.compiler = compiler
		self.runecl = runecl

	def cleanFile(self, file):
		base=os.path.splitext(file)[0]
		for ext in ('.ecl','.tst','.dbg','.lst','.dbg.txt','.dep'):
			try:
				os.unlink(base+ext)
			except:
				pass

	def testFile(self, file):
		if self.compiler(file) is None:
			raise TestFailed('failed to compile')

		if self.runecl(file) is None:
			raise TestFailed('failed to execute')

		if not Compare.outputcompare(file):
			raise TestFailed('output differs')

	def __call__(self, haltOnError=False):
		tested = 0
		passed = 0
		for f in self.files:
			colorprint('Testing {}'.format(f), 'cyan')
			tested += 1
			try:
				self.testFile(f)
			except TestFailed as e:
				colorprint('FAILED: {}'.format(e), 'red')
				if haltOnError:
					break
			else:
				passed += 1
			finally:
				self.cleanFile(f)
		status = True if tested == passed else False

		color = 'green' if status else 'red'
		print('')
		print('*** TEST SUMMARY ***')
		colorprint('Overall status: {}. {} files tested, {} passed, {} failed.'.format(
				'OK' if status else 'FAILED', tested, passed, tested-passed), color)
		return status


if __name__ == '__main__':
	# Includes not needed when used as module
	import argparse

	# Parse command line
	descr = ''
	for line in StdTests.__doc__.splitlines():
		descr += line.strip() + '\n'
	parser = argparse.ArgumentParser(description=descr)
	parser.add_argument('ecompile', help="Full path to ecompile executable")
	parser.add_argument('runecl', help="Full path to runecl executable")
	parser.add_argument('script', nargs='?', help='If specified, tests a single script (package/name)')
	parser.add_argument('-a', '--halt', action='store_true', help="Halt on first error")
	args = parser.parse_args()

	compiler=Compiler(args.ecompile)
	runecl=Executor(args.runecl)

	test=StdTests(compiler, runecl, args.script)
	res=test(args.halt)

	sys.exit(0)

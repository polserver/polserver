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
	def forceLinuxEOL(text):
		return string.replace(text, '\r\n', '\n')

	@staticmethod
	def txtcompare(file1,file2):
		with codecs.open(file1,'r',encoding='utf-8',errors='replace') as f1, codecs.open(file2,'r',encoding='utf-8',errors='replace') as f2:
			l1=f1.readlines()
			l2=f2.readlines()
			if len(l1) != len(l2):
				print('line count differs')
				print('EXPECTED:')
				for l in l1:
					print('\t'+l.rstrip('\r\n'))
				print('GOT:')
				for l in l2:
					print('\t'+l.rstrip('\r\n'))
				return False
			i=1
			for c1,c2 in zip(l1,l2):
				c1=Compare.forceLinuxEOL(c1)
				c2=Compare.forceLinuxEOL(c2)
				if c1!=c2:
					print('line: {}'.format(i))
					print('"',c1,'"')
					print('"',c2,'"')
					return False
				i+=1
			return True

	@staticmethod
	def outputcompare(file):
		basename=os.path.splitext(file)[0]
		if not os.path.exists(basename+'.out'):
			with open(basename+'.tst', 'r') as tst:
				print(tst.read())
			return False
		return Compare.txtcompare(basename+'.out',basename+'.tst')

class ExtUtil:
	def __call__(self, file):
		''' Try to compile a file. IF a errExt filex esists, then compilation is
		    expetcted to fail, and the text in the errExt file must be found in the
		    error message for the compilation to succeed

		    @param file path to the file to compile
		    @return a tuple of booleans (<compiled>, <success>)
		'''
		if os.path.exists(self.baseName(file) + self.errExt):
			expectErr = True
			with open(self.baseName(file) + self.errExt, 'rb') as err:
				errorMatch = err.read().strip()
		else:
			expectErr = False

		try:
			cmd = self.cmd.format(base=self.baseName(file), file=file)
			subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT)
			compiled = True
		except subprocess.CalledProcessError as e:
			cmd = e.cmd
			err = e.output
			compiled = False

		if compiled:
			if expectErr:
				return (True, False)
			else:
				return (True, True)
		else:
			if expectErr:
				errorMatch=Compare.forceLinuxEOL(errorMatch)
				err=Compare.forceLinuxEOL(err)
				if errorMatch in err:
					return (False, True)
				else:
					print(errorMatch)
					print('NOT FOUND IN')
					print(err)
					return (False, False)
			else:
				print(cmd)
				print(err)
				return (False, False)

		raise RuntimeError('This should never happen')

	def baseName(self, file):
		return os.path.splitext(file)[0]

class Executor(ExtUtil):
	def __init__(self,runecl):
		self.cmd = runecl + ' -q {base}.ecl > {base}.tst'
		self.errExt = '.exr'

class Compiler(ExtUtil):
	def __init__(self,comp):
		self.cmd = comp + ' -l -xt -q -C ecompile.cfg {file}'
		self.errExt = '.err'


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
	If a test is supposed to give an error on compile, create a .err file instead
	and put the text to be matched on the error message inside it.
	If a test is supposed to give an error on execute, create a .exr file instead
	and put the text to be matched on the error message inside it.
	'''

	def __init__(self, compiler, runecl, what=None):
		if what:
			splits = what.split('/')
			if len(splits) > 2:
				raise ValueError('Invalid package/script name {}'.format(what))
			elif len(splits) == 2:
				spkg, sfile = splits
			elif len(splits) == 1:
				spkg = splits[0]
				sfile = None
		else:
			spkg = None
			sfile = None

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
		compiled, compSuccess = self.compiler(file)
		if not compSuccess:
			raise TestFailed("shouldn't compile" if compiled else 'failed to compile')

		if compiled:
			runned, runSuccess = self.runecl(file)
			if not runSuccess:
				raise TestFailed('unexpected execute' if runned else 'failed to execute')

		if compiled and not Compare.outputcompare(file):
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
		success = True if tested == passed else False

		color = 'green' if success else 'red'
		print('')
		print('*** TEST SUMMARY ***')
		colorprint('Overall status: {}. {} files tested, {} passed, {} failed.'.format(
				'OK' if success else 'FAILED', tested, passed, tested-passed), color)
		return success


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
	parser.add_argument('what', nargs='?', help='If specified, tests a single package or package/script')
	parser.add_argument('-a', '--halt', action='store_true', help="Halt on first error")
	args = parser.parse_args()

	compiler=Compiler(args.ecompile)
	runecl=Executor(args.runecl)

	test=StdTests(compiler, runecl, args.what)
	if test(args.halt):
		sys.exit(0)
	sys.exit(1)

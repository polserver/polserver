import os, subprocess
import optparse
import shutil
import sys
import codecs

class Compare:
  @staticmethod
  def txtcompare(file1,file2):
    with codecs.open(file1,'r',encoding='utf-8',errors='replace') as f1, codecs.open(file2,'r',encoding='utf-8',errors='replace') as f2:
      l1=f1.readlines()
      l2=f2.readlines()
      if len(l1) != len(l2):
        print 'line count differs'
        return False
      for c1,c2 in zip(l1,l2):
        if c1!=c2:
          print '"',c1,'"'
          print '"',c2,'"'
          return False
      return True

  @staticmethod
  def outputcompare(file):
    basename=os.path.splitext(file)[0]
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
      print e.cmd,e.output

class Compiler:
  def __init__(self,comp):
    self.comp=comp

  def __call__(self, file):
    try:
      return subprocess.check_output(self.comp+' -l -xt -q -C ecompile.cfg '+file,shell=True, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
      print e.cmd, e.output

class StdTests:
  def __init__(self):
    self.files = [
      os.path.join(root,file)
        for root,dirs,files in os.walk('.')
          for file in sorted(files)
            if (file.endswith('.src') and
              os.path.exists(os.path.join(root,os.path.splitext(file)[0]+'.out')))
      ]

  def __call__(self,compiler,runecl):
    all_passed=True
    for f in self.files:
      print 'Testing',f
      if compiler(f) is None:
        all_passed=False
        print 'failed to compile'
        continue

      if runecl(f) is None:
        all_passed=False
        print 'failed to execute'
        continue

      if not Compare.outputcompare(f):
        print 'output differs'
        all_passed=False
    return all_passed

def opts():
  parser = optparse.OptionParser()
  parser.add_option('-c',dest='c_path')
  parser.add_option('-r',dest='r_path')
  return parser.parse_args()

if __name__ == '__main__':
  options,args = opts()
  test=StdTests()
  compiler=Compiler(options.c_path)
  runecl=Executor(options.r_path)

  res=test(compiler,runecl)
  sys.exit(0)


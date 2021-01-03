#!/usr/bin/env python3

'''
runs clang-format on the complete codebase
and performs a diff to find style errors
since clang-format output differs between versions only a few differences will be marked as error
Via GitHub CI integration a annotation with the diff will be added
'''

import subprocess
import concurrent.futures as cf
import os,re,time,sys


class FormatTest:
  def __init__(self):
    mydir = os.path.dirname(os.path.realpath(__file__))
    self.polroot = os.path.realpath(os.path.join(mydir, '..', '..', 'pol-core'))

    self.files = [os.path.join(dp, f) 
      for dp, dn, filenames in os.walk(self.polroot) 
        for f in filenames 
          if os.path.splitext(f)[1] in ['.h', '.cpp']]

  def clangformat(self, file):
    cmd=('clang-format','-i',file)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,cwd=self.polroot)
    r=proc.communicate()
    if proc.returncode !=0:
      print(r[0].decode(), r[1].decode())
      return False
    return True

  def run_clang(self):
    print('Formating {} files in {} threads'.format(len(self.files), os.cpu_count()))
    cmd=('clang-format','--version')
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,cwd=self.polroot)
    r=proc.communicate()
    print(r[0].decode())
    res = 0
    with cf.ThreadPoolExecutor(os.cpu_count()) as p:
      futures = [p.submit(self.clangformat,f) for f in self.files]
      for future in cf.as_completed(futures):
        if not future.result():
          res = 1
    return res
 
  def check_files(self):
    print('Checking diff')
    cmd=('git','diff','*.cpp','*.h')
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,cwd=self.polroot)
    po, pe = proc.communicate()
    out=po.decode()
    pattern=re.compile('diff --git a/(.*) b/.*')
    pline=re.compile('@@ -([0-9]+)[0-9,]* \+([0-9]+)[,0-9]* @@')
    # split per file
    filesplit=[f for f in pattern.split(out) if f]
    exit_code=0
    for f in zip(filesplit[0::2],filesplit[1::2]):
      # split per diff entry
      linesplit=[f for f in pline.split(f[1]) if f]
      del linesplit[0] #remove index line
      for orig_lo,new_lo,l in zip(linesplit[0::3], linesplit[1::3], linesplit[2::3]):
        l=l.splitlines()
        orig_lo=int(orig_lo) -1
        # remove context before
        while not l[0].startswith('+') and not l[0].startswith('-'):
          del l[0]
          orig_lo+=1
        added=[a[1:] for a in l if a.startswith('+')]
        removed=[r[1:] for r in l if r.startswith('-')]
        if not self.check_format(f[0],orig_lo, l, added, removed):
          exit_code=1
    return exit_code


  def check_format(self, file, lineno, context, added, removed):
    res=True
    # error if clang could reduce linelength
    any_length_added=any([len(l)>100 for l in added])
    any_length_removed=any([len(l)>100 for l in removed])
    if (not any_length_added and any_length_removed):
      print('::error file={},line={},col=0::{}'.format(file,lineno,'%0A'.join(['Invalid line length:']+context)))
      res=False
    
    # error if clang removed tab
    any_tab_added=any(['\t' in l for l in added])
    any_tab_removed=any(['\t' in l for l in removed])
    if (not any_tab_added and any_tab_removed):
      print('::error file={},line={},col=0::{}'.format(file,lineno,'%0A'.join(['Tabulator in line:']+context)))
      res=False

# disabled: older clang did not enforce it
#    ident_width_added=[len(l)-len(l.lstrip(' ')) for l in added]
#    ident_width_removed=[len(l)-len(l.lstrip(' ')) for l in removed]
#    any_ident_added=any([i%2 for i in ident_width_added])
#    any_ident_removed=any([i%2 for i in ident_width_removed])
#    if (not any_ident_added and any_ident_removed):
#      print('::error file={},line={},col=0::{}'.format(file,lineno,'%0A'.join(['Invalid ident in line:']+context)))
#      res=False
    return res

if __name__ == '__main__':
  test = FormatTest()
  res_c = test.run_clang()
  res = test.check_files()
  if res_c:
    sys.exit(res_c)
  sys.exit(res)


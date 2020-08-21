'''
print testfile.cpp content for multi.mul for all found
multis.cfg entries in defined file
'''
import os
multi_file = 'multis.cfg'
with open(multi_file,"r") as f:
  intile=False
  for l in f.readlines():
      l=l.strip()
      if not l:
          continue
      if l.startswith("Boat") or l.startswith("House"):
          print('multis['+l.split()[1]+'] = std::vector<T>{')
          continue
      if l.startswith("}"):
          print('};')
          continue
      l=l.split()
      if len(l)<5:
          continue
      print('elem( {}, {}, {}, {}, {}),'.format(
          l[1],l[2],l[3],l[4],0 if l[0]=='static' else 1))

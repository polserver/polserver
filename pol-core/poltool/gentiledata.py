'''
print testfile.cpp entries for tiledata.mul (statics)
searches in defined tiles_file for tiles in tiles list
'''
import os

tiles_file='tiles.cfg'
tiles=[0x3]

tile=['','','','0','','']
with open(tiles_file,"r") as f:
  intile=False
  for l in f.readlines():
      l=l.strip()
      if not l:
          continue
      if l.startswith("tile"):
          if int(l.split()[1],16) in tiles:
              tile=[l.split()[1],'','','0','','']
              intile=True
      if l.startswith("}"):
          if intile:
              print('additem( &item[{}], {}, {}, {}, {}, "{}");'.format(*tile))
          intile=False
      if intile:
        if l.startswith("Desc"):
            tile[5]=' '.join(l.split()[1:])
        if l.startswith("UoFlags"):
          tile[1]=l.split()[1]
        if l.startswith("Height"):
          tile[4]=l.split()[1]
        if l.startswith("Layer"):
          tile[3]=l.split()[1]
        if l.startswith("Weight"):
          tile[2]=l.split()[1]

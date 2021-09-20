## lazy way to check member docs
## does so in a very primitive way

from objnames import translate
import os
from pprint import pprint
def parseMemberDec():
    with open('../pol-core/bscript/objmembers.h','r') as f:
        in_dec=False
        decs=[]
        for l in f.readlines():
            l=l.strip();
            if l=='enum MemberID':
                in_dec=True
                continue
            if not in_dec:
                continue
            if l.startswith('MBR_'):
                decs.append([l.split(',')[0],None])
            elif l.startswith('};'):
                break
    return decs

def parseMemberDef(decs):
    defs=dict()
    with open('../pol-core/bscript/objaccess.cpp','r') as f:
        in_def=False
        for l in f.readlines():
            l=l.strip();
            if l.startswith('ObjMember object_members[]'):
                in_def=True
                continue
            if not in_def:
                continue
            if l.startswith('{ MBR_'):
                l=l.replace('{','').replace('"','')
                l=[_.strip() for _ in l.split(',')]
                for d in decs:
                    if d[0]==l[0]:
                        defs[d[0]]=l[1].lower()
                        del d
                        break
            elif l.startswith('};'):
                break
    return defs

def parseFile(file,defs,members):
    with open(file,'r') as f:
        in_d=False
        active=None
        supported=[]
        openb=0
        linec=0
        in_c=False
        for l in f.readlines():
            linec+=1
            l=l.strip()
            if l.startswith('//'):
                continue
            cs=l.rfind('/*')
            ce=l.rfind('*/')
            if cs>ce:
                in_c=True
            elif ce>cs:
                in_c=False
            if in_c:
                continue
            if l.startswith('return') or l.startswith('#'):
                continue
            if not in_d and '::get_member' in l:
                l=l[:l.find('::get_member')]
                active=l.split()[-1]
                in_d=True
            elif not in_d and '::get_script_member_id' in l:
                l=l[:l.find('::get_script_member_id')]
                active=l.split()[-1]
                in_d=True
            elif not in_d and 'GetCoreVariable' in l:
                active='PolCore'
                in_d=True
            elif in_d:
                if '{' in l:
                    openb+=1
                if '}' in l:
                    openb-=1
                    if not openb and in_d:
                        if len(supported):
                            if active in members:
                                members[active]+=supported
                            else:
                                members[active]=supported
                        supported=[]
                        in_d=False
                if l.startswith('case'):
                    try:
                        if '::' in l:
                            m=l.split('::')[1]
                        else:
                            m=l
                        m=m.replace('(','').replace(')','')
                        m=m.split(':')[0].split()[-1]
                        supported.append(defs[m])
                    except Exception as e:
                        #print(e)
                        #print(file)
                        #print(l)
                        #print(linec)
                        continue
                        raise

                elif 'if ( stricmp' in l:
                    try:
                        m=l.split('"')[1]
                        supported.append(m)
                    except Exception as e:
                        #print(e)
                        #print(file)
                        #print(l)
                        #print(linec)
                        continue
                elif active=='PolCore' and 'LONG_COREVAR' in l:
                    m=l.split(',')[0].split()[1]
                    supported.append(m)
    return members

def checkDocs():
    docs=dict()
    with open('../docs/docs.polserver.com/pol100/objref.xml','r') as f:
        in_c=None
        for l in f.readlines():
            l=l.strip()
            if l.startswith('<class'):
                n=l.split('name="')[1].split('"')[0]
                in_c=n
            elif l.startswith('</class>'):
                in_c=None
            elif in_c and l.startswith('<member'):
                p=l.split('mname="')[1].split('(')[0].split('"')[0].lower()
                if in_c in docs:
                    docs[in_c].append(p)
                else:
                    docs[in_c]=[p]
    return docs


                
decs=parseMemberDec()
defs =parseMemberDef(decs)
#pprint(defs)

members=dict()
for r,d,files in os.walk('../pol-core/'):
    for f in files:
        if f.endswith('.cpp'):
            parseFile(os.path.join(r,f),defs,members)
#pprint(members)
#pprint(members.keys())
docs=checkDocs()
#pprint(docs)

for dc,dm in docs.items():
    pc=translate.get(dc,dc)
    
    if pc in members:
        print('checking {}'.format(dc))
        for m in members[pc]:
            if m not in dm:
                print('   {} not in {}'.format(m,dc))
        for m in dm:
            if m not in members[pc]:
                print('   {} docentry not found'.format(m))
    else:
        print('Class {} docentry not found'.format(pc))

for pc, pm in members.items():
    dc=translate.get(pc,pc)
    if dc not in docs:
        print('Class {} not found with {}'.format(dc,pm))

## lazy way to check member docs
## does so in a very primitive way

from objnames import translate
import os
from pprint import pprint
def parseMethodDec():
    with open('../pol-core/bscript/objmethods.h','r') as f:
        in_dec=False
        decs=[]
        for l in f.readlines():
            l=l.strip();
            if l=='enum MethodID':
                in_dec=True
                continue
            if not in_dec:
                continue
            if l.startswith('MTH_'):
                decs.append([l.split(',')[0],None])
            elif l.startswith('};'):
                break
    return decs

def parseMethodDef(decs):
    defs=dict()
    with open('../pol-core/bscript/objaccess.cpp','r') as f:
        in_def=False
        for l in f.readlines():
            l=l.strip();
            if l.startswith('ObjMethod object_methods[]'):
                in_def=True
                continue
            if not in_def:
                continue
            if l.startswith('{ MTH_'):
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

def parseFile(file,defs,methods):
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
            if not in_d and '::call_method' in l:
                l=l[:l.find('::call_method')]
                active=l.split()[-1]
                in_d=True
            elif not in_d and '::call_polmethod' in l:
                l=l[:l.find('::call_polmethod')]
                active=l.split()[-1]
                in_d=True
            elif not in_d and '::script_method_id' in l:
                l=l[:l.find('::script_method_id')]
                active=l.split()[-1]
                in_d=True
            elif not in_d and 'CallPropertyListMethod_id' in l:
                active='!CPROP!'
                in_d=True
            elif in_d:
                if '{' in l:
                    openb+=1
                if '}' in l:
                    openb-=1
                    if not openb and in_d:

                        if len(supported):
                            if active in methods:
                                methods[active]+=supported
                            else:
                                methods[active]=supported
                        supported=[]
                        in_d=False
                if l.startswith('case'):
                    try:
                        if '::' in l:
                            m=l.split('::')[1]
                        else:
                            m=l
                        m=m.split(':')[0].split()[-1]
                        supported.append(defs[m])
                    except Exception as e:
                        print(e)
                        print(file)
                        print(l)
                        print(linec)
                        raise
                if 'if ( stricmp' in l:
                    try:
                        m=l.split('"')[1]
                        supported.append(m)
                    except:
                        pass
                if 'CallPropertyListMethod' in l:
                    supported.append('!CPROP!')
    return methods

def checkDocs(methods):
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
            elif in_c and l.startswith('<method'):
                p=l.split('proto="')[1].split('(')[0].split('"')[0].lower()
                if in_c in docs:
                    docs[in_c].append(p)
                else:
                    docs[in_c]=[p]
    return docs


                
decs=parseMethodDec()
defs =parseMethodDef(decs)
methods=dict()
for r,d,files in os.walk('../pol-core/'):
    for f in files:
        if f.endswith('.cpp'):
            parseFile(os.path.join(r,f),defs,methods)
#pprint(methods)
#pprint(methods.keys())
docs=checkDocs(methods)
#pprint(docs)

for dc,dm in docs.items():
    if dc =='!CPROP!':
        continue
    pm=translate.get(dc,dc)

    if pm in methods:
        print('checking {}'.format(dc))
        for m in methods[pm]:
            if m=='!CPROP!':
                for mcprop in methods['!CPROP!']:
                    if mcprop not in dm:
                        print('   {} not in {}'.format(m,dc))
            elif m not in dm:
                print('   {} not in {}'.format(m,dc))
        for m in dm:
            if m not in methods[pm]:
                #print(m,methods[pm])
                if '!CPROP!' in methods[pm]:
                    if m in methods['!CPROP!']:
                        continue
                print('   {} docentry not found'.format(m))
    else:
        print('Class {} docentry not found'.format(pm))

for pc, pm in methods.items():
    if pc =='!CPROP!':
        continue
    dc=translate.get(pc,pc)
    if dc not in docs:
        print('Class {} not found with {}'.format(dc,pm))


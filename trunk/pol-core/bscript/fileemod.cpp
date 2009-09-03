#include "clib/stl_inc.h"
#include <string>

//#include <io.h>
//#include <process.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "clib/clib.h"
#include "clib/rawtypes.h"

#include "modules.h"
#include "token.h"

#include "execmodl.h"
#include "impstr.h"
#include "fileemod.h"
#include "fileverb.h"

#include "ofile.h"

// this is currently broken!

#if 0

int FILECont::close(void)
{
    int res = -1;
    if (fp) res = fclose(fp);
    fp = NULL;
    return res;
}

int FILECont::read(char *buf, int maxlen)
{
    if (!fp) return -1;

    char *s = fgets(buf, maxlen, fp);
    if (!s) return -1;
    char *t = strpbrk(buf,"\r\n");
    if (t) *t = '\0';
    return 0;
}

int FILECont::write(const char *buf)
{
    if (!fp) return -1;

    return fprintf(fp, "%s\n", buf);
}

int FILECont::seek(long posn)
{
    if (!fp) return -1;
    return fseek(fp, posn, SEEK_SET);
}


int FILECont::tell(long& posn)
{
    if (!fp) return -1;
    long x = ftell(fp);
    if (x == -1) return -1;

    posn =x;
    return 0;
}

/*
int FILECont::chsize(long length)
{
    int res = -1;
    if (fp) {
         fflush(fp);
         if (ftell(fp) > length) fseek(fp, length, SEEK_SET);
         ::chsize(fileno(fp), length);
         res = 0;
    }
    return res;
}
*/


BObjectImp *OFile::copy(void) const
{
    return new OFile(fc);
}

std::string OFile::getStringRep() const
{
    return "<file>";
}

OFile::~OFile()
{
    fc->nCopies--;
    if (!fc->nCopies) 
    {
        delete fc;
        fc = NULL;
    }
}

int OFile::close(void)
{
    if (fc) return fc->close();
    else return -1;
}
int OFile::read(char *buf, int maxlen)
{
    if (fc) return fc->read(buf, maxlen);
    else return -1;
}
int OFile::write(const char *buf)
{
    if (fc) return fc->write(buf);
    else return -1;
}
int OFile::seek(long posn)
{
    if (fc) return fc->seek(posn);
    else return -1;
}
int OFile::tell(long& posn)
{
    if (fc) return fc->tell(posn);
    else return -1;
}
/*int OFile::chsize(long length)
{
    if (fc) return fc->chsize(length);
    else return -1;
}
*/

FileFunctionDef FileExecutorModule::function_table[] =
{
	{ "fopen", FileOpen },
	{ "fclose", FileClose },
	{ "fread", FileRead },
	{ "fwrite", FileWrite },
	{ "fseek", FileSeek },
	{ "ftell", FileTell }
	//	{ "chsize", FileChSize }
};

int FileExecutorModule::functionIndex( const char *name )
{
	for( unsigned idx = 0; idx < arsize(function_table); idx++ )
	{
		if (stricmp( name, function_table[idx].funcname ) == 0)
			return idx;
	}
	return -1;
}

BObjectImp* FileExecutorModule::execFunc( unsigned funcidx )
{
	return callMemberFunction(*this, function_table[funcidx].fptr)();
};

BObjectImp* FileExecutorModule::FileOpen()
{
	const char *filename;
	const char *type;
	FILE *fp;

	filename = paramAsString(0);
	type = paramAsString(1);
	fp = fopen(filename, type);
	if (!fp) return NULL;
	return new OFile(fp);
}

BObjectImp* FileExecutorModule::FileClose()
{
	BObjectImp *imp;

	imp = getParamImp(0);
	if (!imp->isA("File")) 
		return new BLong(-1);
	
	OFile *of = static_cast<OFile*>(imp);
	return new BLong( of->close() );
}

BObjectImp* FileExecutorModule::FileRead()
{
	BObjectImp *imp;

	imp = getParamImp(0);
	if (!imp->isA("File")) 
		return new BLong(-1);
	
	OFile *of = static_cast<OFile*>(imp);

	char buf[200];
	int res = of->read(buf, 200);
	if (!res) 
		return new String(buf);
	else
		return new BLong(res);
}

BObjectImp* FileExecutorModule::FileWrite()
{
	BObjectImp *imp;

	imp = getParamImp(0);
	if (!imp->isA("File")) 
		return new BLong(-1);
	
	OFile *of = static_cast<OFile*>(imp);

	imp = getParamImp(1);
	
    std::string temp = imp->getStringRep();
	
    int res = of->write( temp.data() );
	
    return new BLong(res);
}

BObjectImp* FileExecutorModule::FileSeek()
{
	BObjectImp *imp;

	imp = getParamImp(0);
	if (!imp->isA("File")) 
		return new BLong(-1);
	

	OFile *of = static_cast<OFile*>(imp);

	long posn;
	posn = paramAsLong(1);
	return new BLong( of->seek(posn) );
}

BObjectImp* FileExecutorModule::FileTell()
{
	BObjectImp *imp;

	imp = getParamImp(0);
	if (!imp->isA("File")) 
		return new BLong(-1);
	

	OFile *of = static_cast<OFile*>(imp);

	long x;
	int res = of->tell(x);
	return new BLong(x);
}
#endif

#if 0
Object *FileExecutorModule::execFunc(const Token& token)
{
    int res = 0;
    Object *result = NULL;
    switch(token.id) {
       case FILE_OPEN:
           break;
       case FILE_CLOSE:
       case FILE_READ:
       case FILE_WRITE:
       case FILE_SEEK:
       case FILE_TELL:
       case FILE_CHSIZE:
           Object *obj;

           obj = getParam(0);
           if (!obj->isA("File")) {
               res = -1;
               break;
           }
           OFile *of;
           of = (OFile *) obj;
           switch(token.id) {
               case FILE_READ:
               case FILE_WRITE:
               case FILE_SEEK:
               case FILE_TELL:
               case FILE_CHSIZE:
                   long newLen = (long)doubleParam(1);
                   res = of->chsize(newLen);
                   break;
           }
		   break;
       default:
           passert(0);
           break;
    }
    if (result == NULL) result = new Double((double)res);

    return result;
}
#endif

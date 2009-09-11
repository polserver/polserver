/*
History
=======
2006/10/06 Shinigami: malloc.h -> stdlib.h

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../clib/strutil.h"

#include "tokens.h"
#include "symcont.h"

SymbolContainer::SymbolContainer(int PgrowBy)
{
    s = NULL;
    usedLen = allocLen = 0u;
    growBy = PgrowBy;
}
SymbolContainer::~SymbolContainer()
{
    if (s) free(s);
    s = NULL;
}

void SymbolContainer::erase()
{
    if (s) free(s);
    s = NULL;
    usedLen = allocLen = 0;
}

void SymbolContainer::resize(unsigned lengthToAdd)
{
    while (usedLen + lengthToAdd > allocLen) 
    {
        allocLen += growBy;
        if (s == NULL) 
        {
            s = (char *) calloc(1, (unsigned) allocLen);
            usedLen = 0;
        } 
        else 
        {
            char *t = (char *) realloc(s, allocLen);
            if (t) 
                s = t;
            else 
                throw runtime_error( "allocation failure in SymbolContainer::resize(" + decint(allocLen) + ")" );
        }
    }
}

bool SymbolContainer::findexisting( const void *data, int datalen, unsigned& position )
{
    int nstarting = usedLen - datalen+1;
      // don't use the first byte, because that's the "fake null"
    for( int i = 1; i < nstarting; i++ )
    {
        if (memcmp( s+i, data, datalen ) == 0)
        {
            position = i;
            return true;
        }
    }
    return false;
}

void SymbolContainer::append(const char *string, unsigned& position)
{
    int nChars = strlen(string)+1;
    if (findexisting(string,nChars,position))
        return;
    resize(nChars);
    strcpy(s+usedLen, string);
    position = usedLen;
    usedLen += nChars;
}

void SymbolContainer::append(long lvalue, unsigned& position)
{
    resize(sizeof lvalue);
    *((long *) (s+usedLen)) = lvalue;
    position = usedLen;
    usedLen += sizeof lvalue;
}

void SymbolContainer::append(double dvalue, unsigned& position)
{
    resize(sizeof dvalue);
    *((double *) (s+usedLen)) = dvalue;
    position = usedLen;
    usedLen += sizeof dvalue;
}

void SymbolContainer::append(void *data, unsigned datalen, unsigned& position)
{
    resize(datalen);
    if (findexisting( data, datalen, position))
        return;
    memcpy(s+usedLen, data, datalen);
    position = usedLen;
    usedLen += datalen;
}

void SymbolContainer::write(FILE *fp)
{
    if (fwrite(&usedLen, sizeof usedLen, 1, fp) != 1)
        throw runtime_error( "SymbolContainer::write failed" );
    if (fwrite(s, usedLen, 1, fp) != 1)
        throw runtime_error( "SymbolContainer::write failed" );
}

unsigned long SymbolContainer::get_write_length() const
{
	// we write the length, followed by the actual data.
	return sizeof usedLen + usedLen;
}

void SymbolContainer::write(char *fname)
{
    FILE *fp = fopen(fname, "wb");
    if (!fp) throw runtime_error( string("Unable to open ") + fname + " for writing." );
    write(fp);
    fclose(fp);
}

void SymbolContainer::read(FILE *fp)
{
    fread(&usedLen, sizeof usedLen, 1, fp);
    char *new_s = (char *) realloc(s, usedLen);
    if (!new_s) throw runtime_error( "allocation failure in SymbolContainer::read()." );
    s = new_s;
    fread(s, usedLen, 1, fp);
    allocLen = usedLen;
}

void StoredTokenContainer::read(FILE *fp)
{
    SymbolContainer::read(fp);
    ST = (StoredToken *) s;
}

void SymbolContainer::read(char *fname)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp) throw runtime_error( string("Unable to open ") + fname + " for reading." );
    read(fp);
    fclose(fp);
}

void StoredTokenContainer::append_tok(const StoredToken& sToken, unsigned *pposition)
{
	resize(sizeof sToken);
    unsigned position = usedLen / sizeof(StoredToken);
    usedLen += sizeof sToken;
    atPut1(sToken, position);
    if (pposition) *pposition = position;
}

void StoredTokenContainer::atPut1(const StoredToken& sToken, unsigned position)
{
    if (position >= count())
        throw runtime_error( "Assigning token at invalid position " + decint(position) + ", range is 0.." + decint(count()-1) );

    char *dst = s + position * sizeof(StoredToken);
    StoredToken *st = (StoredToken*) dst;
    *st = sToken;
}

void StoredTokenContainer::atGet1(unsigned position, StoredToken& sToken) const
{
    if (position >= count())
        throw runtime_error( "Retrieving token at invalid position " + decint(position) + ", range is 0.." + decint(count()-1) );

    char *src = s + position * sizeof(StoredToken);
    StoredToken *st = (StoredToken*) src;

    sToken = *st;
}

void StoredTokenContainer::pack(void)
{
    SymbolContainer::pack();
    ST = (StoredToken *) s;
}

void *StoredTokenContainer::detach(void)
{
    void *d = SymbolContainer::detach();
    ST = NULL;
    return d;
}

void StoredTokenContainer::resize(unsigned lengthToAdd)
{
    SymbolContainer::resize(lengthToAdd);
    ST = (StoredToken *) s;
}

StoredToken::StoredToken( unsigned char aModule,
                          int aID,
                          BTokenType aType ,
                          unsigned aOffset ):
	   type(static_cast<unsigned char>(aType)),
	   id(static_cast<unsigned char>(aID)), 
       offset(static_cast<unsigned short>(aOffset)), 
       module(aModule) 
{
    if (offset != aOffset)
    {
        cerr << "Data segment overflowed." << endl
             << "Flog the programmer for using 2-byte offsets in datafiles." << endl;
        throw runtime_error( "Data segment overflowed" );
    }
}

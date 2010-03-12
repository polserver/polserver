/*
History
=======

Notes
=======

*/

#include <stdlib.h>
#include <string.h>

#include "cmdargs.h"
#include "clib.h"

#ifdef __BORLANDC__
#include <dos.h>
#define argc _argc
#define argv _argv
#else
static int argc;
static char **argv;
#endif

void StoreCmdArgs( int in_argc, char **in_argv )
{
#ifdef __BORLANDC__
#pragma argsused
#else
    argc = in_argc;
    argv = in_argv;
#endif
}


/* 
	Search for a command of type "-cARG" or "/cARG" where {c} == tag
	Return the ARG portion.
	Example:	Tag="F", parm = "/Ffilename"
				Returns: pointer to "filename"
 */
const char *FindArg( const char *tag )
{
    int i;
	int taglen = strlen( tag );
	for( i = 1; i < argc; i++ )
    {
		const char *parm = argv[i];
		switch( parm[0] )
        {
			case '/': case '-':
				if (strnicmp( tag, parm+1, taglen )==0)
					return parm+1+taglen;
				break;
		}
	}
	return NULL;
}

/*
	FindArg2: Given the entire tag, return the default or the tag specified.
	Used for arguments of type "tag=value"
	For example: alltag="cmd=", parm="cmd=a.exe"
	Returns: "a.exe"
 */
const char *FindArg2( const char *alltag, const char *dflt )
{
    int i;
	int taglen = strlen( alltag );
	for( i = 1; i < argc; i++ )
    {
		const char *parm = argv[i];
		if (strnicmp( alltag, parm, taglen )==0)
        {
		    return parm+taglen;
        }
	}
	return dflt;
}
const char *FindArg2( const char *alltag )
{
    int i;
	int taglen = strlen( alltag );
	for( i = 1; i < argc; i++ )
    {
		const char *parm = argv[i];
		if (strnicmp( alltag, parm, taglen )==0)
        {
		    return parm+taglen;
        }
	}
	return NULL;
}

/* 
	IntArg, LongArg: Find parameters of the form "/TAGvalue" and return the value
	portion, or the default.
 */
int IntArg( const char *tag, int deflt )
{
	const char *parm = FindArg( tag );
    if (parm)
        return atoi( parm );
    else
        return deflt;
}

long LongArg( const char *tag, long deflt )
{
	const char *parm = FindArg( tag );
    if (parm)
        return atol( parm );
    else
        return deflt;
}

long LongArg2( const char* alltag, long dflt )
{
    int i;
	int taglen = strlen( alltag );
	for( i = 1; i < argc; i++ )
    {
		const char *parm = argv[i];
		if (strnicmp( alltag, parm, taglen )==0)
        {
		    return atol(parm+taglen);
        }
	}
	return dflt;
}

/* assume parameter following tag is hexadecimal (0x unnecessary) */
long LongHexArg( const char *tag, long deflt )
{
    const char *parm = FindArg( tag );
    if (parm)
        return strtoul( parm, NULL, 16 );
    else
        return deflt;
}

long LongHexArg2( const char *tag, long deflt )
{
	const char *parm = FindArg2( tag );
	if (parm)
		return strtoul( parm, NULL, 16 );
	else
		return deflt;
}

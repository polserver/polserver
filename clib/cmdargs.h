/*
History
=======

Notes
=======

*/

#ifndef __CMDARGS_H
#define __CMDARGS_H


/* See CMDARGS.CPP for comments */

const char *FindArg( const char *tag ); /* args of type /C1, /Afilename */
const char *FindArg2( const char *alltag, const char *dflt ); /* tag=value */
const char *FindArg2( const char *alltag ); /* tag=value */
int IntArg( const char *tag, int deflt );	/* /V2, /B76 */
long LongArg( const char *tag, long deflt ); /* /V42 */
long LongHexArg( const char *tag, long deflt ); /* /A01010016 , /La6004003 */
void StoreCmdArgs( int argc, char **argv );
long LongArg2( const char* alltag, long dflt );
bool BoolArg2( const char* alltag, bool dflt );

#endif


/*
History
=======

Notes
=======

*/

#ifndef __CMDARGS_H
#define __CMDARGS_H


/* See CMDARGS.CPP for comments */
namespace Pol {
  namespace Clib {
	const char *FindArg( const char *tag ); /* args of type /C1, /Afilename */
	const char *FindArg2( const char *alltag, const char *dflt ); /* tag=value */
	const char *FindArg2( const char *alltag ); /* tag=value */
	int IntArg( const char *tag, int deflt );	/* /V2, /B76 */
	int LongArg( const char *tag, int deflt ); /* /V42 */
	int LongHexArg( const char *tag, int deflt ); /* /A01010016 , /La6004003 */
	void StoreCmdArgs( int argc, char **argv );
	int LongArg2( const char* alltag, int dflt );
	bool BoolArg2( const char* alltag, bool dflt );
	int LongHexArg2( const char *tag, int deflt );
  }
}
#endif


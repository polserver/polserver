/*
History
=======

2009/08/25 Shinigami: STLport-5.2.1 fix: Log and Log2 changed little bit

Notes
=======

*/

#ifndef __CLIB_LOGFILE_H
#define __CLIB_LOGFILE_H

#include <stdio.h>


extern FILE *logfile;
int OpenLogFileName( const char* namebase, bool rollover );
int OpenLogFile(void);
void CloseLogFile(void);
void Log( const char *fmt, ... );
void Log2( const char *fmt, ... ); // outputs to stderr also


#endif

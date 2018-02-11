#ifndef __DIRFUNC_H

#define __DIRFUNC_H

#if defined( WINDOWS )
#include "Header_Windows.h"
#include <stdlib.h>
#else
#include <dir.h>
#endif
namespace Pol
{
namespace Clib
{
#define MAXFULLDIR ( MAXDRIVE + MAXDIR - 1 )

#define MAXFNAME ( MAXFILE + MAXEXT - 1 )

typedef char Directory[MAXFULLDIR];
typedef char File1[MAXFILE];
typedef char FFile[MAXFNAME];
typedef char Pathname[MAXPATH];

extern char temp_path[MAXPATH];
extern char temp_drive[MAXDRIVE];
extern char temp_dir[MAXDIR];
extern char temp_fname[MAXFILE];
extern char temp_ext[MAXEXT];

int fullsplit( const char* path );
char* fullmerge( char* path );
char* mergeFnExt( char* path );

char* nodefile( const char* directory, const char* filename, int node );
char* buildfn( const char* directory, const char* filename );
char* buildfnext( const char* dir, const char* file, const char* ext );
void normalize_dir( char* dir );


enum
{
  SRC_NO_EXIST = 1,
  DST_ALREADY_EXIST,
  SRC_OPEN_ERROR,
  DST_OPEN_ERROR,
  WRITE_ERROR
};

extern int mydir_errno;

int copyFile( const char* src, const char* dst );
int copyFileNoRep( const char* src, const char* dst );

int moveFile( const char* src, const char* dst );
int moveFileNoRep( const char* src, const char* dst );

int chddir( const char* directory );
}
}
#endif

#ifndef __DIRFUNC_H

#define __DIRFUNC_H

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#define MAXDRIVE _MAX_DRIVE
#define MAXDIR   _MAX_DIR
#define MAXFILE  _MAX_FNAME
#define MAXPATH  _MAX_PATH
#define MAXEXT   _MAX_EXT
#else

#ifndef __DIR_H
#include <dir.h>
#endif
#endif
namespace Pol {
  namespace Clib {

#define MAXFULLDIR (MAXDRIVE+MAXDIR-1)

#define MAXFNAME (MAXFILE+MAXEXT-1)

	typedef char Directory[MAXFULLDIR];
	typedef char File1[MAXFILE];
	typedef char FFile[MAXFNAME];
	typedef char Pathname[MAXPATH];

	extern char temp_path[MAXPATH];
	extern char temp_drive[MAXDRIVE];
	extern char temp_dir[MAXDIR];
	extern char temp_fname[MAXFILE];
	extern char temp_ext[MAXEXT];

	int fullsplit( const char *path );
	char *fullmerge( char *path );
	char *mergeFnExt( char *path );

	char *nodefile( const char *directory, const char *filename, int node );
	char *buildfn( const char *directory, const char *filename );
	char *buildfnext( const char *dir, const char *file, const char *ext );
	void normalize_dir( char *dir );
	int strip_one( char *direc );


	enum
	{
	  SRC_NO_EXIST = 1,
	  DST_ALREADY_EXIST,
	  SRC_OPEN_ERROR,
	  DST_OPEN_ERROR,
	  WRITE_ERROR
	};

	extern int mydir_errno;

	int copyFile( const char *src, const char *dst );
	int copyFileNoRep( const char *src, const char *dst );

	int moveFile( const char *src, const char *dst );
	int moveFileNoRep( const char *src, const char *dst );

	int chddir( const char *directory );


  }
}
#endif

/** @file
 *
 * @par History
 * - 2008/03/01 Shinigami: VS2005 will not use STLport, but VS2003 will use it
 */


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#pragma hdrstop

#include "clib.h"

#include "dirfunc.h"
namespace Pol
{
namespace Clib
{
// temp stuff for fnsplit/ fnmerge
char temp_path[MAXPATH];
char temp_drive[MAXDRIVE];
char temp_dir[MAXDIR];
char temp_fname[MAXFILE];
char temp_ext[MAXEXT];

int fullsplit( const char* path )
{
#ifdef _WIN32
// FIXME: 2008 Upgrades needed here?
#if defined( _MSC_VER ) && ( _MSC_VER <= 1310 )  // up to VS2003
  _splitpath( path, temp_drive, temp_dir, temp_fname, temp_ext );
#else  // VS2005 using MS STL, boooooo
  _splitpath_s( path, temp_drive, MAXDRIVE, temp_dir, MAXDIR, temp_fname, MAXFILE, temp_ext,
                MAXEXT );
#endif
  return 0;
#else
  return fnsplit( path, temp_drive, temp_dir, temp_fname, temp_ext );
#endif
}

char* fullmerge( char* path )
{
#ifdef _WIN32
// FIXME: 2008 Upgrades needed here?
#if defined( _MSC_VER ) && ( _MSC_VER <= 1310 )  // up to VS2003
  _makepath( path, temp_drive, temp_dir, temp_fname, temp_ext );
#else  // VS2005 using MS STL, booooo
  _makepath_s( path, MAXPATH, temp_drive, temp_dir, temp_fname, temp_ext );
#endif
#else
  fnmerge( path, temp_drive, temp_dir, temp_fname, temp_ext );
#endif
  return path;
}

char* mergeFnExt( char* fname )
{
  stracpy( fname, temp_fname, MAXFILE );
  strncat( fname, temp_ext, MAXEXT - 1 );
  return fname;
}


static Pathname workspace2, fullpath2;

#ifndef _WIN32
int chddir( const char* dir )
{
  // assume: drive is full pathname.
  int ch = toupper( dir[0] );
  if ( isalpha( ch ) )
    setdisk( ch - 'A' );
  return chdir( dir );
}
#endif
/*
    the build directory functions:
    params: directory, filename, node
    directory: should not include trailing backslash

    */
namespace
{
const char* use_dir;
const char* use_fname;
const char* use_template;

void find_usefns( const char* dir, const char* fname )
{
  /* first, the directory. */
  if ( dir && dir[0] ) /* this is not null */
    use_dir = dir;
  else
    use_dir = ".";  // current directory.

  if ( fname && fname[0] )
    use_fname = fname;
  else
    use_fname = ".";

  // now, if dir has a trailing backslash, we use %s%s
  // else, we use %s\\%s
  const char* s = strrchr( use_dir, '\\' );
  if ( s == nullptr )
    s = strrchr( use_dir, '/' );

  if ( s != nullptr && ( s[1] == '\0' ) )  // is trailing
    use_template = "%s%s";
  else
    use_template = "%s\\%s";
}
}  // namespace

char* nodefile( const char* directory, const char* filename, int node )
{
  char buf[20];
  find_usefns( directory, filename );
  sprintf( buf, "%s.%%d", use_template );
  sprintf( workspace2, buf, directory, filename, node );
  _fullpath( fullpath2, workspace2, sizeof fullpath2 );

  return fullpath2;
}

char* buildfn( const char* directory, const char* filename )
{
  // oughta, first, strip off the traling backslash.
  find_usefns( directory, filename );
  sprintf( workspace2, use_template, use_dir, use_fname );
  _fullpath( fullpath2, workspace2, sizeof fullpath2 );

  return fullpath2;
}

char* buildfnext( const char* directory, const char* filename, const char* extension )
{
  find_usefns( directory, filename );
  sprintf( workspace2, use_template, directory, filename );
  strcat( workspace2, "." );
  strncat( workspace2, extension, 3 );
  _fullpath( fullpath2, workspace2, sizeof fullpath2 );

  return fullpath2;
}


/*
    normalize_dir: strip a trailin backslash from a directory.
    */
void normalize_dir( char* dir )
{
  char* s = strrchr( dir, '\\' );
  if ( s == nullptr )
    return;
  if ( *( s + 1 ) == '\0' )
    *s = '\0';
}


int mydir_errno;

#ifndef _WIN32
static int inner_copy( const char* src, const char* dst, int replaceOk )
{
  int in = -1, out = -1;  // file handles
  int result = -1;        // default to error

  int bufsize = 0;
  void* buf = nullptr;

  for ( bufsize = 0x4000; buf == nullptr && bufsize >= 128; bufsize /= 2 )
    buf = malloc( bufsize );
  if ( buf == nullptr )
  { /* no errors allowed here! */
    static char dummy[4];
    buf = dummy;
    bufsize = 4;
  }


  in = open( src, O_RDONLY | O_BINARY );
  if ( in >= 0 )
  {
    out = open( dst, O_BINARY | O_CREAT | O_RDWR | ( replaceOk ? O_TRUNC : O_EXCL ),
                S_IREAD | S_IWRITE );
    if ( out >= 0 )
    {
      int lastwrite, nbytes;
      while ( ( nbytes = read( in, buf, bufsize ) ) > 0 )
      {
        lastwrite = write( out, buf, nbytes );
        if ( lastwrite == -1 )
          break;
      }
      if ( lastwrite >= 0 )
      {
#ifdef _WIN32
        struct _timeb ftime;
#else
        struct ftime ftime;
#endif
        getftime( in, &ftime );
        setftime( out, &ftime );
        result = 0;
      }
      else
        mydir_errno = WRITE_ERROR;

      close( out );
      // put this here so if fail 'cause of existence,
      // we don't remove the unreplaced file!
      // also, the file must be closed to remove
      if ( result )
        remove( dst );
    }
    else
    {
      mydir_errno = DST_OPEN_ERROR;
      switch ( errno )
      {
      case EEXIST:
        mydir_errno = DST_ALREADY_EXIST;
        break;
      }
    }
    close( in );
  }
  else
  {
    mydir_errno = SRC_OPEN_ERROR;
    switch ( errno )
    {
    case ENOENT:
      mydir_errno = SRC_NO_EXIST;
      break;
    }
  }

  assert( buf );
  if ( bufsize != 4 )
    free( buf );

  return result;
}

int copyFile( const char* src, const char* dst )
{
  int replaceOk = 1;
  return inner_copy( src, dst, replaceOk );
}

int copyFileNoRep( const char* src, const char* dst )
{
  int replaceOk = 0;
  return inner_copy( src, dst, replaceOk );
}

static int inner_move( const char* src, const char* dst, int replaceOk )
{
  if ( replaceOk && access( dst, 0 ) == 0 )
    remove( dst );  // let rename work
  if ( rename( src, dst ) == 0 )
    return 0;  // same drive, no replace.
  else if ( inner_copy( src, dst, replaceOk ) == 0 )
  {
    remove( src );
    return 0;
  }
  else
    return -1;
}

int moveFile( const char* src, const char* dst )
{
  int replaceOk = 1;
  return inner_move( src, dst, replaceOk );
}
int moveFileNoRep( const char* src, const char* dst )
{
  int replaceOk = 0;
  return inner_move( src, dst, replaceOk );
}
#endif
}  // namespace Clib
}  // namespace Pol

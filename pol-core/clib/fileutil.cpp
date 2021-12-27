/** @file
 *
 * @par History
 */


#include "fileutil.h"
#include "Header_Windows.h"

#include <filesystem>
#include <limits.h>
#include <sys/stat.h>

#ifdef LINUX
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif

namespace Pol
{
namespace Clib
{
std::string normalized_dir_form( const std::string& istr )
{
  std::string str = istr;

  {
    std::string::size_type bslashpos;
    while ( std::string::npos != ( bslashpos = str.find( '\\' ) ) )
    {
      str.replace( bslashpos, 1, 1, '/' );
    }
  }

  if ( str.size() == 0 )
  {
    return "/";
  }
  else if ( str[str.size() - 1] == '/' || str[str.size() - 1] == '\\' )
  {
    return str;
  }
  else
  {
    return str + "/";
  }
}

bool IsDirectory( const char* dir )
{
  std::string sdir( dir );
  if ( sdir[sdir.length() - 1] == '/' )
    sdir = sdir.erase( sdir.length() - 1, 1 );
  struct stat st;
  if ( stat( sdir.c_str(), &st ) )
    return false;
  return ( st.st_mode & S_IFDIR ? true : false );
}

void MakeDirectory( const char* dir )
{
#if defined( __unix__ ) || defined( __APPLE__ )
  mkdir( dir, 0777 );
#else
  mkdir( dir );
#endif
}

int strip_one( std::string& direc )
{
  std::string::size_type pos = direc.find_last_of( "\\/" );
  if ( pos == std::string::npos )
    return -1;
  if ( pos >= 1 && direc[pos - 1] == ':' )  // at "C:\"
    return -1;
  direc = direc.substr( 0, pos );
  return 0;
}

int make_dir( const char* dir )
{
  if ( access( dir, 0 ) )
  {
#ifdef _WIN32
    if ( CreateDirectory( dir, nullptr ) )  // why is windows too good for POSIX?
#else
    if ( mkdir( dir, 0777 ) == 0 )
#endif
    {
      return 0; /* made it okay */
    }
    else
    {
      // if didn't make it,
      std::string parent_dir = dir;
      if ( strip_one( parent_dir ) )
        return -1;
      if ( make_dir( parent_dir.c_str() ) )
        return -1;
#ifdef _WIN32
      if ( CreateDirectory( dir, nullptr ) )
        return 0;
#else
      if ( mkdir( dir, 0777 ) == 0 )
        return 0;
#endif
      // this test is mostly for the case where the path is in normalized form
      if ( access( dir, 0 ) == 0 )
        return 0;
      return -1;
    }
  }
  return 0;
}

bool FileExists( const char* filename )
{
  return ( access( filename, 0 ) == 0 );
}
bool FileExists( const std::string& filename )
{
  return ( access( filename.c_str(), 0 ) == 0 );
}

int filesize( const char* fname )
{
  struct stat st;
  if ( stat( fname, &st ) )
    return 0;
  return st.st_size;
}

unsigned int GetFileTimestamp( const char* fname )
{
  struct stat st;
  if ( stat( fname, &st ) )
    return 0;
  return (unsigned int)st.st_mtime;
}

void RemoveFile( const std::string& fname )
{
  unlink( fname.c_str() );
}

std::string FullPath( const char* filename )
{
#if defined( __unix__ ) || defined( __APPLE__ )
  char tmp[PATH_MAX];
  if ( realpath( filename, tmp ) )
    return tmp;
  else
    return "";
#else
  char p[1025];
  _fullpath( p, filename, sizeof p );
  return p;
#endif
}

std::string GetTrueName( const char* filename )
{
  std::error_code ec;
  auto canonical = std::filesystem::canonical( filename, ec );
  if ( ec )
    return filename;
  else
    return canonical.filename().string();
}

std::string GetFilePart( const char* filename )
{
  return std::filesystem::path( filename ).filename().string();
}
}  // namespace Clib
}  // namespace Pol

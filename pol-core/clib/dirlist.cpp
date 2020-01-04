/** @file
 *
 * @par History
 */


#include "dirlist.h"

#include <stddef.h>

#ifdef __unix__
#include "logfacility.h"
#include <errno.h>
#include <unistd.h>
#endif

#include "fileutil.h"

#ifdef WINDOWS

namespace Pol
{
namespace Clib
{
DirList::DirList( const char* dirname )
{
  memset( &fd_, 0, sizeof( fd_ ) );
  std::string srch = dirname;
  srch += "*";

  hfd_ = FindFirstFile( srch.c_str(), &fd_ );
}
DirList::DirList() : hfd_( INVALID_HANDLE_VALUE )
{
  memset( &fd_, 0, sizeof( fd_ ) );
}

DirList::~DirList()
{
  if ( hfd_ != INVALID_HANDLE_VALUE )
  {
    FindClose( hfd_ );
    hfd_ = INVALID_HANDLE_VALUE;
  }
}

void DirList::open( const char* filespec )
{
  if ( hfd_ != INVALID_HANDLE_VALUE )
    FindClose( hfd_ );

  hfd_ = FindFirstFile( filespec, &fd_ );
}

bool DirList::at_end() const
{
  return ( hfd_ == INVALID_HANDLE_VALUE );
}

std::string DirList::name() const
{
  return fd_.cFileName;
}

void DirList::next()
{
  if ( hfd_ != INVALID_HANDLE_VALUE )
  {
    if ( !FindNextFile( hfd_, &fd_ ) )
    {
      FindClose( hfd_ );
      hfd_ = INVALID_HANDLE_VALUE;
    }
  }
}
}  // namespace Clib
}  // namespace Pol
#else  // non-brain dead places where we can use POSIX functions

#include <dirent.h>

namespace Pol
{
namespace Clib
{
DirList::DirList( const char* dirname )
{
  dir_ = opendir( dirname );
  if ( !dir_ )
  {
    ERROR_PRINT << "FAILED TO OPEN DIR " << dirname << " " << errno << "\n";
  }
  else
  {
    ERROR_PRINT << "READ DIR " << dirname << "\n";
  }
  next();
}
DirList::DirList() : dir_( nullptr ) {}
DirList::~DirList()
{
  if ( dir_ != nullptr )
  {
    closedir( dir_ );
    dir_ = nullptr;
  }
}
void DirList::open( const char* /*filespec*/ )
{
  // TODO
}
bool DirList::at_end() const
{
  return ( dir_ == nullptr );
}

std::string DirList::name() const
{
  return cur_name_;
}

void DirList::next()
{
  if ( dir_ != nullptr )
  {
    struct dirent* de = readdir( dir_ );
    if ( de != nullptr )
    {
      ERROR_PRINT << de->d_name << "\n";
      cur_name_ = de->d_name;
    }
    else
    {
      if ( !de )
      {
        ERROR_PRINT << "FAILED TO read DIR  " << errno << "\n";
      }
      closedir( dir_ );
      dir_ = nullptr;
    }
  }
}
}  // namespace Clib
}  // namespace Pol
#endif

#ifdef _WIN32
#include <direct.h>
#endif
namespace Pol
{
namespace Clib
{
PushDir::PushDir( const char* dir )
{
  if ( getcwd( savedir_, sizeof savedir_ ) == nullptr )
    ok_ = ( chdir( dir ) == 0 );
  else
    ok_ = false;
}

PushDir::~PushDir()
{
  if ( ok_ )
  {
    ok_ = ( chdir( savedir_ ) == 0 );
  }
}

bool PushDir::ok() const
{
  return ok_;
}

std::string curdir()
{
#ifdef MAX_PATH
  char cdir[_MAX_PATH];
#else
  char cdir[256];
#endif
  if ( getcwd( cdir, sizeof cdir ) == nullptr )
    return normalized_dir_form( cdir );
  return "";
}
}  // namespace Clib
}  // namespace Pol

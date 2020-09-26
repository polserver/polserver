/** @file
 *
 * @par History
 */


#include "dirlist.h"

#include <stddef.h>

#ifdef __unix__
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
}
}
#else  // non-brain dead places where we can use POSIX functions

#include <dirent.h>

namespace Pol
{
namespace Clib
{
DirList::DirList( const char* dirname )
{
  dir_ = opendir( dirname );
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
      cur_name_ = de->d_name;
    }
    else
    {
      closedir( dir_ );
      dir_ = nullptr;
    }
  }
}
}
}
#endif


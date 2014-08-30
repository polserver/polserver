/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

#ifdef __unix__
#include <unistd.h>
#endif

#include "dirlist.h"


#ifdef _WIN32
#include <windows.h> 
namespace Pol {
  namespace Clib {
	DirList::DirList( const char* dirname )
	{
	  memset( &fd_, 0, sizeof( fd_ ) );
	  string srch = dirname;
	  srch += "*";

	  hfd_ = FindFirstFile( srch.c_str(), &fd_ );
	}
	DirList::DirList() :
	  hfd_( INVALID_HANDLE_VALUE )
	{
	  memset( &fd_, 0, sizeof( fd_ ) );
	}

	DirList::~DirList()
	{
	  if( hfd_ != INVALID_HANDLE_VALUE )
	  {
		FindClose( hfd_ );
		hfd_ = INVALID_HANDLE_VALUE;
	  }
	}

	void DirList::open( const char* filespec )
	{
	  if( hfd_ != INVALID_HANDLE_VALUE )
		FindClose( hfd_ );

	  hfd_ = FindFirstFile( filespec, &fd_ );
	}

	bool DirList::at_end() const
	{
	  return ( hfd_ == INVALID_HANDLE_VALUE );
	}

	string DirList::name() const
	{
	  return fd_.cFileName;
	}

	void DirList::next()
	{
	  if( hfd_ != INVALID_HANDLE_VALUE )
	  {
		if( !FindNextFile( hfd_, &fd_ ) )
		{
		  FindClose( hfd_ );
		  hfd_ = INVALID_HANDLE_VALUE;
		}
	  }
	}
  }
}
#else // non-brain dead places where we can use POSIX functions

#include <sys/types.h>
#include <dirent.h>
namespace Pol {
  namespace Clib {
DirList::DirList( const char* dirname )
{
    dir_ = opendir( dirname );
    next();
}
DirList::DirList() :
    dir_( NULL )
{
}
DirList::~DirList()
{
    if (dir_ != NULL)
    {
        closedir( dir_ );
        dir_ = NULL;
    }
}
void DirList::open( const char* filespec )
{
    // TODO
}
bool DirList::at_end() const
{
    return (dir_ == NULL);
}

string DirList::name() const
{
    return cur_name_;
}

void DirList::next()
{
    if (dir_ != NULL)
    {
        struct dirent* de = readdir( dir_ );
        if (de != NULL)
        {
            cur_name_ = de->d_name;
        }
        else
        {
            closedir( dir_ );
            dir_ = NULL;
        }
    }
}
  }
}
#endif

#ifdef _WIN32
#include <direct.h>
#endif
namespace Pol {
  namespace Clib {
	PushDir::PushDir( const char* dir )
	{
	  if( getcwd( savedir_, sizeof savedir_ ) == NULL )
		ok_ = ( chdir( dir ) == 0 );
	  else
		ok_ = false;
	}

	PushDir::~PushDir()
	{
	  if( ok_ )
	  {
		ok_ = ( chdir( savedir_ ) == 0 );
	  }
	}

	bool PushDir::ok() const
	{
	  return ok_;
	}

	string curdir()
	{
#ifdef MAX_PATH
	  char cdir[_MAX_PATH];
#else
	  char cdir[ 256 ];
#endif
	  if( getcwd( cdir, sizeof cdir ) == NULL )
		return normalized_dir_form( cdir );
	  return "";
	}

  }
}
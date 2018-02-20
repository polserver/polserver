/** @file
 *
 * @par History
 */


#ifndef CLIB_DIRLIST_H
#define CLIB_DIRLIST_H

#include <string>

#include "Header_Windows.h"

#ifndef WINDOWS
#include <dirent.h>
#include <sys/types.h>
#endif

namespace Pol
{
namespace Clib
{
class DirList
{
public:
  explicit DirList( const char* path );
  explicit DirList();
  ~DirList();

  void open( const char* filespec );

  bool at_end() const;
  std::string name() const;
  void next();


private:
#ifdef _WIN32
  WIN32_FIND_DATA fd_;
  HANDLE hfd_;
#else
  DIR* dir_;
  std::string cur_name_;
#endif


private:  // not implemented
  DirList( const DirList& );
  DirList& operator=( const DirList& );
};

std::string curdir();

class PushDir
{
public:
  PushDir( const char* dir );
  ~PushDir();

  bool ok() const;

private:
  bool ok_;
#ifdef MAX_PATH
  char savedir_[MAX_PATH];
#else
  char savedir_[256];
#endif
};
}
}
#endif

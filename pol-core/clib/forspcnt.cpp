/** @file
 *
 * @par History
 */


#include "Header_Windows.h"
#include "clib.h"
#include <stdio.h>

namespace Pol
{
namespace Clib
{
int forspec( const char* spec, void ( *func )( const char* pathname ) )
{
  int nProc = 0;
  HANDLE hFind;
  WIN32_FIND_DATA find;
  char fullpath[MAX_PATH];
  size_t lenleft;
  char* fname;

  GetFullPathName( spec, sizeof fullpath, fullpath, &fname );
  lenleft = sizeof fullpath - ( fname - fullpath );

  hFind = FindFirstFile( spec, &find );
  if ( hFind != INVALID_HANDLE_VALUE )
  {
    do
    {
      ++nProc;
      stracpy( fname, find.cFileName, lenleft );
      if ( func != nullptr )
        ( *func )( fullpath );
    } while ( FindNextFile( hFind, &find ) );
    FindClose( hFind );
  }
  return nProc;
}
}  // namespace Clib
}  // namespace Pol

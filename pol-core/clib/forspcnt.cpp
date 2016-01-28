/** @file
 *
 * @par History
 */


#include "Header_Windows.h"
#include <stdio.h>
#include "clib.h"

namespace Pol {
  namespace Clib {
	int forspec( const char *spec, void( *func )( const char *pathname ) )
	{
	  int nProc = 0;
	  HANDLE hFind;
	  WIN32_FIND_DATA find;
	  char fullpath[MAX_PATH];
	  size_t lenleft;
	  char *fname;

	  GetFullPathName( spec, sizeof fullpath, fullpath, &fname );
	  lenleft = sizeof fullpath - ( fname - fullpath );

	  hFind = FindFirstFile( spec, &find );
	  if( hFind != INVALID_HANDLE_VALUE )
	  {
		do
		{
		  ++nProc;
		  strzcpy( fname, find.cFileName, lenleft );
		  if( func != NULL )
			( *func )( fullpath );
		} while( FindNextFile( hFind, &find ) );
		FindClose( hFind );
	  }
	  return nProc;
	}
  }
}

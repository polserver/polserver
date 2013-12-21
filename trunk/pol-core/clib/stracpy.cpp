/*
History
=======

Notes
=======

*/

#include <assert.h>
#include "string.h"
namespace Pol {
  namespace Clib {
	char *stracpy( char *dest, const char *src, size_t maxlen )
	{
	  assert( dest );
	  if( maxlen )
	  {
		strncpy( dest, src, maxlen - 1 );
		dest[maxlen - 1] = '\0';
	  }
	  return dest;
	}
  }
}

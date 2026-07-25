/** @file
 *
 * @par History
 */


#include <assert.h>
#include <string.h>

#include "clib/Header_Windows.h"
#include "clib/clib.h"


namespace Pol::Clib
{
char* stracpy( char* dest, const char* src, size_t maxlen )
{
  assert( dest );
  if ( maxlen )
  {
    strncpy( dest, src, maxlen - 1 );
    dest[maxlen - 1] = '\0';
  }
  return dest;
}
}  // namespace Pol::Clib

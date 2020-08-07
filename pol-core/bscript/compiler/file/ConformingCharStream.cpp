#include "ConformingCharStream.h"

#include <cctype>

#include "../clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
size_t ConformingCharStream::LA( ssize_t i )
{
  int c = stream->LA( i );
  // INFO_PRINT << " " << c << " ";
  if ( c <= 0 )
  {
    return c;
  }
  // c = c & 0x7f;

  if ( std::isupper( c ) )
    return std::tolower( c );

  return c;
}

}  // namespace Pol::Bscript::Compiler

#include "ConformingCharStream.h"

#include <cctype>

namespace Pol::Bscript::Compiler
{
size_t ConformingCharStream::LA( ssize_t i )
{
  int c = stream->LA( i );
  if ( c <= 0 )
  {
    return c;
  }

  if ( std::isupper( c ) )
    return std::tolower( c );

  return c;
}

}  // namespace Pol::Bscript::Compiler

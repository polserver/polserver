#include "ConformingCharStream.h"

#include <cctype>

namespace Pol::Bscript::Compiler
{
size_t ConformingCharStream::LA( ssize_t i )
{
  size_t c = stream->LA( i );
  if ( c == CharStream::EOF )
  {
    return c;
  }

  if ( c < 128 ) // to avoid std::tolower "undefined behavior"
    return std::tolower( static_cast<unsigned char>( c ) );

  return c;
}

}  // namespace Pol::Bscript::Compiler

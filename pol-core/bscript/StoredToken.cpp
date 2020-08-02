#include "StoredToken.h"

#include "../clib/logfacility.h"

namespace Pol
{
namespace Bscript
{
StoredToken::StoredToken( unsigned char aModule, int aID, BTokenType aType, unsigned aOffset )
    : type( static_cast<unsigned char>( aType ) ),
      id( static_cast<unsigned char>( aID ) ),
      offset( static_cast<unsigned short>( aOffset ) ),
      module( aModule )
{
  if ( offset != aOffset )
  {
    ERROR_PRINT << "Data segment overflowed.\n"
                << "Flog the programmer for using 2-byte offsets in datafiles.\n";
    throw std::runtime_error( "Data segment overflowed" );
  }
}

}  // namespace Bscript
}  // namespace Pol

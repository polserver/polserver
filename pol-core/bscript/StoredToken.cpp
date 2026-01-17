#include "StoredToken.h"

#include "../clib/logfacility.h"


namespace Pol::Bscript
{
StoredToken::StoredToken( unsigned char aModule, int aID, BTokenType aType, unsigned aOffset )
    : type( static_cast<unsigned char>( aType ) ),
      id( static_cast<unsigned char>( aID ) ),
      offset( static_cast<unsigned short>( aOffset ) ),
      module( aModule )
{
  if ( offset != aOffset )
  {
    ERROR_PRINTLN(
        "Data segment overflowed.\nFlog the programmer for using 2-byte offsets in datafiles." );
    throw std::runtime_error( "Data segment overflowed" );
  }
}

}  // namespace Pol::Bscript

/** @file
 *
 * @par History
 * - 2006/10/06 Shinigami: malloc.h -> stdlib.h
 */


#include "bscript/symcont.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "clib/strutil.h"


namespace Pol::Bscript
{
SymbolContainer::SymbolContainer() : s( nullptr ), usedLen( 0 ), allocLen( 0 ) {}

SymbolContainer::~SymbolContainer()
{
  if ( s )
    free( s );
  s = nullptr;
}

void SymbolContainer::read( FILE* fp )
{
  size_t fread_res = fread( &usedLen, sizeof usedLen, 1, fp );
  if ( fread_res != 1 )
    throw std::runtime_error( "failed to read in SymbolContainer::read()." );
  char* new_s = (char*)realloc( s, usedLen );
  if ( !new_s )
    throw std::runtime_error( "allocation failure in SymbolContainer::read()." );
  s = new_s;
  fread_res = fread( s, usedLen, 1, fp );
  if ( fread_res != 1 )
    throw std::runtime_error( "failed to read in SymbolContainer::read()." );
  allocLen = usedLen;
}

void StoredTokenContainer::atGet1( unsigned position, StoredToken& sToken ) const
{
  if ( position >= count() )
    throw std::runtime_error( "Retrieving token at invalid position " + Clib::tostring( position ) +
                              ", range is 0.." + Clib::tostring( count() - 1 ) );

  char* src = s + position * sizeof( StoredToken );
  StoredToken* st = (StoredToken*)src;

  sToken = *st;
}

}  // namespace Pol::Bscript

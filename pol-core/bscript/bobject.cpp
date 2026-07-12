/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2008/02/11 Turley:    ObjArray::unpack() will accept zero length Arrays and Erros from
 * Array-Elements
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */

#include "bobject.h"


namespace Pol::Bscript
{

Clib::fixed_allocator<BObject, 256> bobject_alloc;

size_t BObjectRef::sizeEstimate() const
{
  if ( get() )
    return sizeof( BObjectRef ) + get()->sizeEstimate();
  return sizeof( BObjectRef );
}
size_t BObject::sizeEstimate() const
{
  if ( objimp.get() )
    return sizeof( BObject ) + objimp.get()->sizeEstimate();
  return sizeof( BObject );
}

BObject* BObject::clone() const
{
  return new BObject( objimp->copy() );
}

bool BObject::operator!=( const BObject& obj ) const
{
  return *objimp != *( obj.objimp );
}
bool BObject::operator==( const BObject& obj ) const
{
  return *objimp == *( obj.objimp );
}
bool BObject::operator<( const BObject& obj ) const
{
  return *objimp < *( obj.objimp );
}
bool BObject::operator<=( const BObject& obj ) const
{
  return *objimp <= *( obj.objimp );
}
bool BObject::operator>( const BObject& obj ) const
{
  return *objimp > *( obj.objimp );
}
bool BObject::operator>=( const BObject& obj ) const
{
  return *objimp >= *( obj.objimp );
}
}  // namespace Pol::Bscript

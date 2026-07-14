#include "buninit.h"

#include "bobject.h"

namespace Pol::Bscript
{

Clib::fixed_allocator<UninitObject, 256> uninit_alloc;

UninitObject* UninitObject::SharedInstance;
ref_ptr<BObjectImp> UninitObject::SharedInstanceOwner;
BObjectRef UninitObject::SharedInstanceRef;

UninitObject::UninitObject() : BObjectImp( OTUninit ) {}

BObjectImp* UninitObject::copy() const
{
  return create();
}

size_t UninitObject::sizeEstimate() const
{
  return sizeof( UninitObject );
}

bool UninitObject::isTrue() const
{
  return false;
}

/**
 * An uninit is equal to any other error or uninit
 */
bool UninitObject::operator==( const BObjectImp& imp ) const
{
  return ( imp.isa( OTError ) || imp.isa( OTUninit ) );
}

bool UninitObject::operator<( const BObjectImp& imp ) const
{
  if ( imp.isa( OTError ) || imp.isa( OTUninit ) )
    return false;  // Because it's equal can't be lesser

  return true;
}
}  // namespace Pol::Bscript

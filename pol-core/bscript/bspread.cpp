#include "bspread.h"

namespace Pol::Bscript
{

BSpread::BSpread( BObjectRef obj ) : BObjectImp( OTSpread ), object( std::move( obj ) ) {}

BSpread::BSpread( const BSpread& B ) : BObjectImp( OTSpread ), object( B.object ) {}

size_t BSpread::sizeEstimate() const
{
  return sizeof( BSpread ) + object.sizeEstimate();
}

BObjectImp* BSpread::copy() const
{
  return new BSpread( *this );
}

bool BSpread::isTrue() const
{
  return object->isTrue();
}

std::string BSpread::getStringRep() const
{
  return "Spread";
}
}  // namespace Pol::Bscript

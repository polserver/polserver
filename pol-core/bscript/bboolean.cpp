#include "bboolean.h"

#include "berror.h"

#include <fmt/compile.h>
#include <fmt/format.h>
#include <istream>

namespace Pol::Bscript
{
using namespace fmt::literals;

#if BOBJECTIMP_DEBUG
BBoolean::BBoolean( bool bval ) : BObjectImp( OTBoolean ), bval_( bval ) {}
BBoolean::BBoolean( const BBoolean& B ) : BBoolean( B.bval_ ) {}
#endif

BObjectImp* BBoolean::unpack( std::istream& is )
{
  int lv;
  if ( is >> lv )
  {
    return new BBoolean( lv != 0 );
  }

  return new BError( "Error extracting Boolean value" );
}

void BBoolean::packonto( std::string& str ) const
{
  fmt::format_to( std::back_inserter( str ), "b{}"_cf, bval_ ? 1 : 0 );
}

BObjectImp* BBoolean::copy() const
{
  return new BBoolean( *this );
}

size_t BBoolean::sizeEstimate() const
{
  return sizeof( BBoolean );
}

bool BBoolean::isTrue() const
{
  return bval_;
}

bool BBoolean::operator==( const BObjectImp& objimp ) const
{
  return bval_ == objimp.isTrue();
}

std::string BBoolean::getStringRep() const
{
  return bval_ ? "true" : "false";
}
}  // namespace Pol::Bscript

/** @file
 *
 * @par History
 */


#include <sstream>
#include <string>

#include "../clib/stlutil.h"
#include "berror.h"
#include "bobject.h"
#include "impstr.h"

namespace Pol
{
namespace Bscript
{
#if BOBJECTIMP_DEBUG
BLong::BLong( int lval ) : BObjectImp( OTLong ), lval_( static_cast<int>( lval ) ) {}

BLong::BLong( const BLong& L ) : BObjectImp( OTLong ), lval_( L.lval_ ) {}
#endif


std::string BLong::pack() const
{
  OSTRINGSTREAM os;
  os << "i" << lval_;
  return OSTRINGSTREAM_STR( os );
}

std::string BLong::pack( int val )
{
  OSTRINGSTREAM os;
  os << "i" << val;
  return OSTRINGSTREAM_STR( os );
}


void BLong::packonto( std::ostream& os ) const
{
  os << "i" << lval_;
}

BObjectImp* BLong::unpack( std::istream& is )
{
  int lv;
  if ( is >> lv )
  {
    return new BLong( lv );
  }
  else
  {
    return new BError( "Error extracting Integer value" );
  }
}

BObjectImp* BLong::copy() const
{
  return new BLong( *this );
}

size_t BLong::sizeEstimate() const
{
  return sizeof( BLong );
}

bool BLong::isTrue() const
{
  return ( lval_ != 0 );
}

bool BLong::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTLong ) )
  {
    return lval_ == ( (BLong&)objimp ).lval_;
  }
  else if ( objimp.isa( OTDouble ) )
  {
    return lval_ == ( (Double&)objimp ).value();
  }
  else if ( objimp.isa( OTBoolean ) )
  {
    return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();
  }
  else
  {
    return false;
  }
}

bool BLong::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTLong ) )
  {
    return lval_ < ( (BLong&)objimp ).lval_;
  }
  else if ( objimp.isa( OTDouble ) )
  {
    return lval_ < ( (Double&)objimp ).value();
  }

  return base::operator<( objimp );
}

std::string BLong::getStringRep() const
{
  OSTRINGSTREAM os;

  os << lval_;

  return OSTRINGSTREAM_STR( os );
}

BObjectImp* BLong::selfPlusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfPlusObj( *this );
}
BObjectImp* BLong::selfPlusObj( const BLong& objimp ) const
{
  return new BLong( lval_ + objimp.lval_ );
}
BObjectImp* BLong::selfPlusObj( const Double& objimp ) const
{
  return new Double( lval_ + objimp.value() );
}
BObjectImp* BLong::selfPlusObj( const String& objimp ) const
{
  return new String( getStringRep() + objimp.value() );
}

void BLong::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void BLong::selfPlusObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ += objimp.value();
}
void BLong::selfPlusObj( Double& objimp, BObject& obj )
{
  obj.setimp( selfPlusObj( objimp ) );
}
void BLong::selfPlusObj( String& objimp, BObject& obj )
{
  obj.setimp( selfPlusObj( objimp ) );
}

BObjectImp* BLong::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* BLong::selfMinusObj( const BLong& objimp ) const
{
  return new BLong( lval_ - objimp.value() );
}
BObjectImp* BLong::selfMinusObj( const Double& objimp ) const
{
  return new Double( lval_ - objimp.value() );
}
BObjectImp* BLong::selfMinusObj( const String& objimp ) const
{
  String s( getStringRep() );
  return s.selfMinusObj( objimp );
}
void BLong::selfMinusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfMinusObj( *this, obj );
}
void BLong::selfMinusObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ -= objimp.value();
}
void BLong::selfMinusObj( Double& objimp, BObject& obj )
{
  obj.setimp( selfMinusObj( objimp ) );
}
void BLong::selfMinusObj( String& objimp, BObject& obj )
{
  obj.setimp( selfMinusObj( objimp ) );
}

BObjectImp* BLong::selfTimesObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfTimesObj( *this );
}
BObjectImp* BLong::selfTimesObj( const BLong& objimp ) const
{
  return new BLong( lval_ * objimp.lval_ );
}
BObjectImp* BLong::selfTimesObj( const Double& objimp ) const
{
  return new Double( lval_ * objimp.value() );
}
void BLong::selfTimesObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfTimesObj( *this, obj );
}
void BLong::selfTimesObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ *= objimp.lval_;
}
void BLong::selfTimesObj( Double& objimp, BObject& obj )
{
  obj.setimp( selfTimesObj( objimp ) );
}

BObjectImp* BLong::selfDividedByObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfDividedByObj( *this );
}
BObjectImp* BLong::selfDividedByObj( const BLong& objimp ) const
{
  int divisor = objimp.lval_;
  if ( !divisor )
    return new BError( "Divide by Zero" );
  else
    return new BLong( lval_ / divisor );
}
BObjectImp* BLong::selfDividedByObj( const Double& objimp ) const
{
  double divisor = objimp.value();
  if ( divisor == 0.0 )
    return new BError( "Divide by Zero" );
  else
    return new Double( lval_ / divisor );
}
void BLong::selfDividedByObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfDividedByObj( *this, obj );
}
void BLong::selfDividedByObj( BLong& objimp, BObject& obj )
{
  if ( !objimp.lval_ )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    lval_ /= objimp.lval_;
}
void BLong::selfDividedByObj( Double& objimp, BObject& obj )
{
  obj.setimp( selfDividedByObj( objimp ) );
}

BObjectImp* BLong::selfModulusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfModulusObj( *this );
}
BObjectImp* BLong::selfModulusObj( const BLong& objimp ) const
{
  int divisor = objimp.lval_;
  if ( !divisor )
    return new BError( "Divide by Zero" );
  return new BLong( lval_ % divisor );
}
BObjectImp* BLong::selfModulusObj( const Double& objimp ) const
{
  if ( !objimp.value() )
    return new BError( "Divide by Zero" );
  return new Double( fmod( lval_, objimp.value() ) );
}
void BLong::selfModulusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfModulusObj( *this, obj );
}
void BLong::selfModulusObj( BLong& objimp, BObject& obj )
{
  if ( !objimp.lval_ )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    lval_ %= objimp.lval_;
}
void BLong::selfModulusObj( Double& objimp, BObject& obj )
{
  obj.setimp( selfModulusObj( objimp ) );
}


BObjectImp* BLong::selfBitShiftRightObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftRightObj( *this );
}
BObjectImp* BLong::selfBitShiftRightObj( const BLong& objimp ) const
{
  return new BLong( lval_ >> objimp.lval_ );
}
void BLong::selfBitShiftRightObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitShiftRightObj( *this, obj );
}
void BLong::selfBitShiftRightObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ >>= objimp.lval_;
}

BObjectImp* BLong::selfBitShiftLeftObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitShiftLeftObj( *this );
}
BObjectImp* BLong::selfBitShiftLeftObj( const BLong& objimp ) const
{
  return new BLong( lval_ << objimp.lval_ );
}
void BLong::selfBitShiftLeftObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitShiftLeftObj( *this, obj );
}
void BLong::selfBitShiftLeftObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ <<= objimp.lval_;
}

BObjectImp* BLong::selfBitAndObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitAndObj( *this );
}
BObjectImp* BLong::selfBitAndObj( const BLong& objimp ) const
{
  return new BLong( lval_ & objimp.lval_ );
}
void BLong::selfBitAndObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitAndObj( *this, obj );
}
void BLong::selfBitAndObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ &= objimp.lval_;
}

BObjectImp* BLong::selfBitOrObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitOrObj( *this );
}
BObjectImp* BLong::selfBitOrObj( const BLong& objimp ) const
{
  return new BLong( lval_ | objimp.lval_ );
}
void BLong::selfBitOrObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitOrObj( *this, obj );
}
void BLong::selfBitOrObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ |= objimp.lval_;
}

BObjectImp* BLong::selfBitXorObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfBitXorObj( *this );
}
BObjectImp* BLong::selfBitXorObj( const BLong& objimp ) const
{
  return new BLong( lval_ ^ objimp.lval_ );
}
void BLong::selfBitXorObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfBitXorObj( *this, obj );
}
void BLong::selfBitXorObj( BLong& objimp, BObject& /*obj*/ )
{
  lval_ ^= objimp.lval_;
}

BObjectImp* BLong::bitnot() const
{
  return new BLong( ~lval_ );
}
}  // namespace Bscript
}  // namespace Pol

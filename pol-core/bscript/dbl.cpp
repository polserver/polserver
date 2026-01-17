/** @file
 *
 * @par History
 */


#include <charconv>
#include <cmath>
#include <sstream>
#include <string>
#include <system_error>

#include "../clib/stlutil.h"
#include "berror.h"
#include "bobject.h"
#include "impstr.h"


namespace Pol::Bscript
{
std::string Double::double_to_string( double val )
{
  std::string buff( 100, '\0' );
  auto [buffptr, ec] = std::to_chars( buff.data(), buff.data() + buff.size(), val );
  if ( ec == std::errc() )
  {
    buff.resize( buffptr - buff.data() );
    return buff;
  }
  throw std::system_error( std::make_error_code( ec ) );
}

std::string Double::pack() const
{
  return std::string( "r" ) + double_to_string( dval_ );
}

void Double::packonto( std::ostream& os ) const
{
  os << "r" << double_to_string( dval_ );
}

BObjectImp* Double::unpack( std::istream& is )
{
  double dv;
#ifndef __APPLE__
  if ( is >> dv )
#else
  // well this (and the pack format) is terrible:
  // 1) the pack format depends on operator>>(double) magically reading a whole double,
  //    but stopping without an error if it's followed by something else
  // 2) on osx, operator>>(double) reports an error if the double is followed
  //    by something else
  // 3) the pack format is ambiguous, mostly, if an error follows a double
  // 4) fortunately, nobody will probably ever really run a server on osx
  // 5) our options are a bit limited in peeking at an iostream, I think
  // so:
  //    4.5e-16   a double
  //    4.5e+16   a double
  //    4.5e62...  a double, followed by an error with 62 elements
  //                 - note that technically, this could be a double, but it seems
  //                   that the double formatter is being nice and always including - or +
  std::string tmp;
  tmp.reserve( 16 );
  while ( !is.eof() )
  {
    char ch = is.peek();
    if ( std::isdigit( ch ) || ch == '.' || ch == '-' || ch == '+' )
    {
      tmp.push_back( is.get() );
    }
    else
    {
      if ( ch == 'e' )  // might be an exponent, or an error struct following the double (sadface)
      {
        is.get();  // the 'e'

        if ( std::isdigit( is.peek() ) )  // assume it's followed by an error struct
        {
          is.unget();
        }
        else  // assume it's an exponent
        {
          tmp.push_back( ch );        // the e
          tmp.push_back( is.get() );  // the '+' or '-'
          while ( !is.eof() && std::isdigit( is.peek() ) )
          {
            tmp.push_back( is.get() );
          }
        }
      }
      break;
    }
  }
  ISTRINGSTREAM is2( tmp );
  if ( is2 >> dv )
#endif
  {
    return new Double( dv );
  }

  return new BError( "Error extracting Real value" );
}

size_t Double::sizeEstimate() const
{
  return sizeof( Double );
}


bool Double::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTDouble ) )
  {
    double diff = dval_ - ( (Double&)objimp ).dval_;
    return fabs( diff ) < 0.00000001;
  }
  if ( objimp.isa( OTLong ) )
  {
    double diff = dval_ - ( (BLong&)objimp ).value();
    return fabs( diff ) < 0.00000001;
  }
  if ( objimp.isa( OTBoolean ) )
  {
    return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();
  }
  else
  {
    return false;
  }
}

bool Double::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( OTDouble ) )
  {
    return ( dval_ < ( (Double&)objimp ).dval_ );
  }
  if ( objimp.isa( OTLong ) )
  {
    return ( dval_ < ( (BLong&)objimp ).value() );
  }

  return base::operator<( objimp );
}

std::string Double::getStringRep() const
{
  return double_to_string( dval_ );
}

BObjectImp* Double::selfPlusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfPlusObj( *this );
}

BObjectImp* Double::selfPlusObj( const BLong& objimp ) const
{
  return new Double( dval_ + objimp.value() );
}
BObjectImp* Double::selfPlusObj( const Double& objimp ) const
{
  return new Double( dval_ + objimp.dval_ );
}
BObjectImp* Double::selfPlusObj( const String& objimp ) const
{
  return new String( getStringRep() + objimp.value() );
}
void Double::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void Double::selfPlusObj( BLong& objimp, BObject& obj )
{
  obj.setimp( selfPlusObj( objimp ) );
}
void Double::selfPlusObj( Double& objimp, BObject& /*obj*/ )
{
  dval_ += objimp.dval_;
}
void Double::selfPlusObj( String& objimp, BObject& obj )
{
  obj.setimp( selfPlusObj( objimp ) );
}

BObjectImp* Double::selfMinusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfMinusObj( *this );
}
BObjectImp* Double::selfMinusObj( const BLong& objimp ) const
{
  return new Double( dval_ - objimp.value() );
}
BObjectImp* Double::selfMinusObj( const Double& objimp ) const
{
  return new Double( dval_ - objimp.dval_ );
}
BObjectImp* Double::selfMinusObj( const String& objimp ) const
{
  String s( getStringRep() );
  return s.selfMinusObj( objimp );
}
void Double::selfMinusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfMinusObj( *this, obj );
}
void Double::selfMinusObj( BLong& objimp, BObject& /*obj*/ )
{
  dval_ -= objimp.value();
}
void Double::selfMinusObj( Double& objimp, BObject& /*obj*/ )
{
  dval_ -= objimp.value();
}
void Double::selfMinusObj( String& objimp, BObject& obj )
{
  obj.setimp( selfMinusObj( objimp ) );
}

BObjectImp* Double::selfTimesObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfTimesObj( *this );
}
BObjectImp* Double::selfTimesObj( const BLong& objimp ) const
{
  return new Double( dval_ * objimp.value() );
}
BObjectImp* Double::selfTimesObj( const Double& objimp ) const
{
  return new Double( dval_ * objimp.value() );
}
void Double::selfTimesObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfTimesObj( *this, obj );
}
void Double::selfTimesObj( BLong& objimp, BObject& /*obj*/ )
{
  dval_ *= objimp.value();
}
void Double::selfTimesObj( Double& objimp, BObject& /*obj*/ )
{
  dval_ *= objimp.value();
}


BObjectImp* Double::selfDividedByObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfDividedByObj( *this );
}
BObjectImp* Double::selfDividedByObj( const BLong& objimp ) const
{
  int divisor = objimp.value();
  if ( !divisor )
    return new BError( "Divide by Zero" );
  return new Double( dval_ / divisor );
}
BObjectImp* Double::selfDividedByObj( const Double& objimp ) const
{
  double divisor = objimp.value();
  if ( divisor == 0.0 )
    return new BError( "Divide by Zero" );
  return new Double( dval_ / divisor );
}
void Double::selfDividedByObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfDividedByObj( *this, obj );
}
void Double::selfDividedByObj( BLong& objimp, BObject& obj )
{
  if ( !objimp.value() )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    dval_ /= objimp.value();
}
void Double::selfDividedByObj( Double& objimp, BObject& obj )
{
  if ( !objimp.value() )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    dval_ /= objimp.value();
}

BObjectImp* Double::selfModulusObjImp( const BObjectImp& objimp ) const
{
  return objimp.selfModulusObj( *this );
}
void Double::selfModulusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfModulusObj( *this, obj );
}
BObjectImp* Double::selfModulusObj( const BLong& objimp ) const
{
  if ( !objimp.value() )
    return new BError( "Divide by Zero" );
  return new Double( fmod( dval_, objimp.value() ) );
}
BObjectImp* Double::selfModulusObj( const Double& objimp ) const
{
  if ( !objimp.value() )
    return new BError( "Divide by Zero" );
  return new Double( fmod( dval_, objimp.value() ) );
}
void Double::selfModulusObj( BLong& objimp, BObject& obj )
{
  if ( !objimp.value() )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    dval_ = fmod( dval_, objimp.value() );
}
void Double::selfModulusObj( Double& objimp, BObject& obj )
{
  if ( !objimp.value() )
    obj.setimp( new BError( "Divide by Zero" ) );
  else
    dval_ = fmod( dval_, objimp.value() );
}
}  // namespace Pol::Bscript

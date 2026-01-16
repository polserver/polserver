/** @file
 *
 * @par History
 */


#include "berror.h"

#include "contiter.h"
#include "impstr.h"

namespace Pol
{
namespace Bscript
{
unsigned int BError::creations_ = 0;
unsigned int BError::creations()
{
  return creations_;
}

BError::BError() : BStruct( OTError )
{
  ++creations_;
}

BError::BError( const BError& other ) : BStruct( other, OTError )
{
  ++creations_;
}

BError::BError( std::istream& is, unsigned size ) : BStruct( is, size, OTError )
{
  ++creations_;
}

BError::BError( const char* err ) : BStruct( OTError )
{
  ++creations_;
  addMember( "errortext", new String( err ) );
}

BError::BError( const std::string& err ) : BStruct( OTError )
{
  ++creations_;
  addMember( "errortext", new String( err ) );
}


BObjectImp* BError::copy() const
{
  return new BError( *this );
}

char BError::packtype() const
{
  return 'e';
}

const char* BError::typetag() const
{
  return "error";
}

const char* BError::typeOf() const
{
  return "Error";
}

u8 BError::typeOfInt() const
{
  return OTError;
}

BObjectImp* BError::unpack( std::istream& is )
{
  unsigned size;
  char colon;
  if ( !( is >> size >> colon ) )
  {
    return new BError( "Unable to unpack struct elemcount" );
  }
  if ( (int)size <= 0 )
  {
    return new BError(
        "Unable to unpack struct elemcount. Length given must be positive integer!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack struct elemcount. Bad format. Colon not found!" );
  }
  return new BError( is, size );
}


bool BError::isTrue() const
{
  return false;
}
/**
 * An error is equal to any other error or uninit
 */
bool BError::operator==( const BObjectImp& imp ) const
{
  return ( imp.isa( OTError ) || imp.isa( OTUninit ) );
}

bool BError::operator<( const BObjectImp& imp ) const
{
  if ( imp.isa( OTError ) || imp.isa( OTUninit ) )
    return false;  // Because it's equal can't be lesser

  return true;
}

ContIterator* BError::createIterator( BObject* /*pIterVal*/ )
{
  return new ContIterator();
}

BObjectRef BError::OperSubscript( const BObject& /*obj*/ )
{
  return BObjectRef( this );
}

BObjectImp* BError::array_assign( BObjectImp* /*idx*/, BObjectImp* /*target*/, bool /*copy*/ )
{
  return this;
}
}  // namespace Bscript
}  // namespace Pol

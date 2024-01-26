/** @file
 *
 * @par History
 * - 2005/09/12 Shinigami: added ObjMethods packet.GetIntxxFlipped and packet.SetIntxxFlipped (Byte
 * Order)
 * - 2006/09/16 Shinigami: added ObjMethods packet.GetUnicodeStringFlipped and
 * packet.SetUnicodeStringFlipped (Byte Order)
 * - 2006/09/16 Shinigami: fixed Memory Overwrite Bug in packet.SetUnicodeString* ->
 * convertArrayToUC
 * - 2008/12/17 MuadDib:   fixed Memory Leak in SetSize() where it returns BObjects back
 * - to calling Methods where they do not handle a return value.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: oldsize not used
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "packetscrobj.h"

#include <iomanip>
#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/clib_endian.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "base/position.h"
#include "globals/network.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/clienttransmit.h"
#include "realms/realm.h"
#include "realms/realms.h"
#include "uoexec.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;

BPacket::BPacket() : PolObjectImp( OTPacket ), is_variable_length( false ) {}
BPacket::BPacket( const BPacket& copyfrom )
    : PolObjectImp( OTPacket ),
      buffer( copyfrom.buffer ),
      is_variable_length( copyfrom.is_variable_length )
{
}
BPacket::BPacket( u8 type, signed short length ) : PolObjectImp( OTPacket )
{
  if ( length == -1 )
  {
    is_variable_length = true;
    buffer.resize( 1, type );
  }
  else
  {
    is_variable_length = false;
    if ( length > 0 )
    {
      buffer.resize( length, 0 );
      buffer[0] = type;
    }
  }
}
BPacket::BPacket( const unsigned char* data, unsigned short length, bool variable_len )
    : PolObjectImp( OTPacket ), buffer( data, data + length )
{
  is_variable_length = variable_len;
}
BPacket::~BPacket() {}

BObjectRef BPacket::get_member_id( const int /*id*/ )  // id test
{
  return BObjectRef( new BLong( 0 ) );
}
BObjectRef BPacket::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  else
    return BObjectRef( UninitObject::create() );
}

BObjectImp* BPacket::call_polmethod_id( const int id, UOExecutor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_SENDPACKET:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SendPacket requires 1 parameter." );

    Mobile::Character* chr = nullptr;
    Network::Client* client = nullptr;
    if ( ex.getCharacterOrClientParam( 0, chr, client ) )
    {
      if ( chr != nullptr )
      {
        if ( !chr->has_active_client() )
          return new BLong( 0 );

        client = chr->client;
      }

      if ( client != nullptr )
      {
        if ( client->isConnected() )
        {
          Core::networkManager.clientTransmit->AddToQueue( client, (void*)( &buffer[0] ),
                                                           static_cast<int>( buffer.size() ) );
          return new BLong( 1 );
        }
        else
          return new BLong( 0 );
      }
    }
    break;
  }

  case MTH_SENDAREAPACKET:
  {
    if ( ex.numParams() != 4 )
      return new BError( "SendAreaPacket requires 4 parameters." );
    Core::Pos2d pos;
    Realms::Realm* realm;
    unsigned short range;
    if ( ex.getRealmParam( 3, &realm ) && ex.getPos2dParam( 0, 1, &pos, realm ) &&
         ex.getParam( 2, range ) )
    {
      unsigned short num_sent_to = 0;
      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
          pos, realm, range,
          [&]( Mobile::Character* chr )
          {
            Core::networkManager.clientTransmit->AddToQueue( chr->client, (void*)( &buffer[0] ),
                                                             static_cast<int>( buffer.size() ) );
            num_sent_to++;
          } );
      return new BLong( num_sent_to );
    }
    break;
  }

  case MTH_GETINT8:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetInt8 requires 1 parameter." );
    unsigned short offset;
    if ( ex.getParam( 0, offset ) )
    {
      if ( offset >= buffer.size() )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      u8* data = reinterpret_cast<u8*>( &buffer[offset] );
      return new BLong( *data );
    }
    break;
  }

  case MTH_GETINT16:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetInt16 requires 1 parameter." );
    unsigned short offset;
    if ( ex.getParam( 0, offset ) )
    {
      if ( offset > buffer.size() - sizeof( u16 ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      u16* data = reinterpret_cast<u16*>( &buffer[offset] );
      return new BLong( cfBEu16( *data ) );
    }
    break;
  }

  case MTH_GETINT32:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetInt32 requires 1 parameter." );
    unsigned short offset;
    if ( ex.getParam( 0, offset ) )
    {
      if ( offset > buffer.size() - sizeof( u32 ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      u32* data = reinterpret_cast<u32*>( &buffer[offset] );
      return new BLong( cfBEu32( *data ) );
    }
    break;
  }

  case MTH_GETINT16FLIPPED:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetInt16Flipped requires 1 parameter." );
    unsigned short offset;
    if ( ex.getParam( 0, offset ) )
    {
      if ( offset > buffer.size() - sizeof( u16 ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      u16* data = reinterpret_cast<u16*>( &buffer[offset] );
      return new BLong( cfLEu16( *data ) );
    }
    break;
  }

  case MTH_GETINT32FLIPPED:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetInt32Flipped requires 1 parameter." );
    unsigned short offset;
    if ( ex.getParam( 0, offset ) )
    {
      if ( offset > buffer.size() - sizeof( u32 ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      u32* data = reinterpret_cast<u32*>( &buffer[offset] );
      return new BLong( cfLEu32( *data ) );
    }
    break;
  }

  case MTH_GETSTRING:
  {
    if ( ex.numParams() != 2 )
      return new BError( "GetString requires 2 parameter." );
    unsigned short offset, len;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, len ) )
    {
      if ( ( offset >= buffer.size() ) ||
           ( static_cast<u16>( offset + len ) >
             buffer.size() ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );

      const char* str_offset = reinterpret_cast<const char*>( &buffer[offset] );
      size_t real_len = 0;

      // Returns maximum of len characters or up to the first null-byte
      while ( real_len < len && *( str_offset + real_len ) )
        ++real_len;

      return new String( str_offset, real_len, String::Tainted::YES );
    }
    break;
  }

  case MTH_GETUNICODESTRING:
  {
    if ( ex.numParams() != 2 )
      return new BError( "GetUnicodeString requires 2 parameter." );
    unsigned short offset, len;
    if ( ex.getParam( 0, offset ) &&
         ex.getParam( 1, len ) )  // len is in unicode characters, not bytes
    {
      if ( ( offset >= buffer.size() ) ||
           ( static_cast<u16>( offset + len * 2 ) >
             buffer.size() ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      std::string str =
          Bscript::String::fromUTF16( reinterpret_cast<u16*>( &buffer[offset] ), len, true );
      return new Bscript::String( str );
    }
    break;
  }

  case MTH_GETUNICODESTRINGFLIPPED:
  {
    if ( ex.numParams() != 2 )
      return new BError( "GetUnicodeStringFlipped requires 2 parameter." );
    unsigned short offset, len;
    if ( ex.getParam( 0, offset ) &&
         ex.getParam( 1, len ) )  // len is in unicode characters, not bytes
    {
      if ( ( offset >= buffer.size() ) ||
           ( static_cast<u16>( offset + len * 2 ) >
             buffer.size() ) )  // don't allow getting bytes past end of buffer
        return new BError( "Offset too high" );
      std::string str =
          Bscript::String::fromUTF16( reinterpret_cast<u16*>( &buffer[offset] ), len );
      return new Bscript::String( str );
    }
    break;
  }

  case MTH_GETSIZE:
    return new BLong( static_cast<int>( buffer.size() ) );

  case MTH_SETSIZE:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetSize requires 1 parameter." );
    unsigned short newsize;
    if ( ex.getParam( 0, newsize ) )
    {
      return SetSize( newsize, true );
    }
    break;
  }

  case MTH_SETINT8:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetInt8 requires 2 parameters." );
    unsigned short offset, value;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, value ) )
    {
      if ( is_variable_length )
        if ( offset >= buffer.size() )
        {
          if ( !SetSize( ( offset + sizeof( u8 ) ) ) )
          {
            return new BError( "Offset value out of range on a fixed length packet" );
            ;
          }
        }
      buffer[offset] = static_cast<u8>( value );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_SETINT16:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetInt16 requires 2 parameters." );
    unsigned short offset, value;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, value ) )
    {
      if ( static_cast<u16>( offset + sizeof( u16 ) ) > buffer.size() )
      {
        if ( !SetSize( ( offset + sizeof( u16 ) ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
          ;
        }
      }

      u16* bufptr = reinterpret_cast<u16*>( &buffer[offset] );
      *bufptr = ctBEu16( static_cast<u16>( value ) );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_SETINT32:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetInt32 requires 2 parameters." );
    unsigned short offset;
    int lvalue;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, lvalue ) )
    {
      if ( static_cast<u32>( offset + sizeof( u32 ) ) > buffer.size() )
      {
        if ( !SetSize( offset + sizeof( u32 ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
          ;
        }
      }

      u32* bufptr = reinterpret_cast<u32*>( &buffer[offset] );
      *bufptr = ctBEu32( static_cast<u32>( lvalue ) );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_SETINT16FLIPPED:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetInt16Flipped requires 2 parameters." );
    unsigned short offset, value;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, value ) )
    {
      if ( static_cast<u16>( offset + sizeof( u16 ) ) > buffer.size() )
      {
        if ( !SetSize( ( offset + sizeof( u16 ) ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
          ;
        }
      }

      u16* bufptr = reinterpret_cast<u16*>( &buffer[offset] );
      *bufptr = ctLEu16( static_cast<u16>( value ) );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_SETINT32FLIPPED:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetInt32Flipped requires 2 parameters." );
    unsigned short offset;
    int lvalue;
    if ( ex.getParam( 0, offset ) && ex.getParam( 1, lvalue ) )
    {
      if ( static_cast<u32>( offset + sizeof( u32 ) ) > buffer.size() )
      {
        if ( !SetSize( ( offset + sizeof( u32 ) ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
          ;
        }
      }
      u32* bufptr = reinterpret_cast<u32*>( &buffer[offset] );
      *bufptr = ctLEu32( static_cast<u32>( lvalue ) );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_SETSTRING:
  {
    if ( ex.numParams() != 3 )
      return new BError( "SetString requires 3 parameters." );
    unsigned short offset, nullterm;
    const String* text;
    if ( ex.getParam( 0, offset ) && ex.getStringParam( 1, text ) && ex.getParam( 2, nullterm ) )
    {
      std::string cp1252text;
      if ( text->hasUTF8Characters() )
      {
        cp1252text = Clib::strUtf8ToCp1252( text->value() );
      }
      else
      {
        cp1252text = text->value();
      }
      u16 textlen = static_cast<u16>( cp1252text.length() );
      if ( static_cast<u16>( offset + textlen + nullterm ) > buffer.size() )
      {
        if ( !SetSize( ( offset + textlen + nullterm ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
        }
      }
      u8* bufptr = reinterpret_cast<u8*>( &buffer[offset] );
      const char* textptr = cp1252text.c_str();
      for ( u16 i = 0; i < textlen; i++ )
      {
        bufptr[i] = textptr[i];
      }

      if ( nullterm )
        bufptr[textlen] = 0;

      return new BLong( 1 );
    }
    break;
  }
  case MTH_SETUTF8STRING:
  {
    if ( ex.numParams() != 3 )
      return new BError( "SetUtf8String requires 3 parameters." );
    unsigned short offset, nullterm;
    const String* text;
    if ( ex.getParam( 0, offset ) && ex.getStringParam( 1, text ) && ex.getParam( 2, nullterm ) )
    {
      u16 textlen = static_cast<u16>( text->value().length() );
      if ( static_cast<u16>( offset + textlen + nullterm ) > buffer.size() )
      {
        if ( !SetSize( ( offset + textlen + nullterm ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
        }
      }
      u8* bufptr = reinterpret_cast<u8*>( &buffer[offset] );
      const char* textptr = text->value().c_str();
      for ( u16 i = 0; i < textlen; i++ )
        bufptr[i] = textptr[i];

      if ( nullterm )
        bufptr[textlen] = 0;

      return new BLong( 1 );
    }
    break;
  }
  case MTH_SETUNICODESTRING:
  {
    if ( ex.numParams() != 3 )
      return new BError( "SetUnicodeString requires 3 parameters." );
    unsigned short offset, nullterm;
    const String* unitext;
    if ( ex.getParam( 0, offset ) && ex.getUnicodeStringParam( 1, unitext ) &&
         ex.getParam( 2, nullterm ) )
    {
      std::vector<u16> gwtext = unitext->toUTF16();
      if ( nullterm )
        gwtext.push_back( 0 );
      u16 bytelen = static_cast<u16>( gwtext.size() ) * 2;
      if ( static_cast<u16>( offset + bytelen ) > buffer.size() )
      {
        if ( !SetSize( ( offset + bytelen ) ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
        }
      }
      for ( const auto& c : gwtext )
      {
        u16 fc = cfBEu16( c );
        std::memcpy( &buffer[offset], &fc, sizeof( fc ) );
        offset += 2;
      }

      return new BLong( 1 );
    }
    break;
  }
  case MTH_SETUNICODESTRINGFLIPPED:
  {
    if ( ex.numParams() != 3 )
      return new BError( "SetUnicodeStringFlipped requires 3 parameters." );
    unsigned short offset, nullterm;
    const String* unitext;
    if ( ex.getParam( 0, offset ) && ex.getUnicodeStringParam( 1, unitext ) &&
         ex.getParam( 2, nullterm ) )
    {
      std::vector<u16> gwtext = unitext->toUTF16();
      if ( nullterm )
        gwtext.push_back( 0 );
      u16 bytelen = static_cast<u16>( gwtext.size() ) * 2;

      if ( static_cast<u16>( offset + bytelen ) > buffer.size() )
      {
        if ( !SetSize( offset + bytelen ) )
        {
          return new BError( "Offset value out of range on a fixed length packet" );
          ;
        }
      }
      for ( const auto& c : gwtext )
      {
        std::memcpy( &buffer[offset], &c, sizeof( c ) );
        offset += 2;
      }
      return new BLong( 1 );
    }
    break;
  }
  default:
    return nullptr;
  }
  return new BError( "Invalid parameter" );
}


BObjectImp* BPacket::call_polmethod( const char* methodname, UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex );
  else
    return nullptr;
}
BObjectImp* BPacket::copy() const
{
  return new BPacket( *this );
}
std::string BPacket::getStringRep() const
{
  OSTRINGSTREAM os;
  for ( auto itr = buffer.begin(); itr != buffer.end(); ++itr )
    os << std::setfill( '0' ) << std::setw( 2 ) << std::hex << static_cast<u16>( *itr );

  return OSTRINGSTREAM_STR( os );
}

bool BPacket::SetSize( u16 newsize )
{
  if ( !is_variable_length )
    return false;
  // unsigned short oldsize = buffer.size();
  buffer.resize( newsize );
  u16* sizeptr = reinterpret_cast<u16*>( &buffer[1] );
  *sizeptr = ctBEu16( newsize );
  return true;
}

BObjectImp* BPacket::SetSize( u16 newsize, bool /*giveReturn*/ )
{
  if ( !is_variable_length )
    return new BError( "Attempted to resize a fixed length packet" );
  unsigned short oldsize = static_cast<unsigned short>( buffer.size() );
  buffer.resize( newsize );
  u16* sizeptr = reinterpret_cast<u16*>( &buffer[1] );
  *sizeptr = ctBEu16( newsize );
  return new BLong( oldsize );
}
}  // namespace Core
}  // namespace Pol

/** @file
 *
 * @par History
 * - 2009-03-03 Nando - private_say_above_cl(), say_above_cl(), send_sysmessage_cl() Crash fix when
 * arguments are NULL
 * - 2009/12/04 Turley: if arguments are NULL still add the terminator
 */


#include "clfunc.h"

#include "../clib/clib.h"
#include "../clib/clib_endian.h"
#include "../clib/passert.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/clienttransmit.h"
#include "network/packethelper.h"
#include "pktoutid.h"
#include "sockio.h"
#include "ufunc.h"

#include <cstddef>

namespace Pol
{
namespace Core
{
using namespace Network;

void send_sysmessage_cl( Client* client, /*Character *chr_from, ObjArray* oText,*/
                         unsigned int cliloc_num, const u16* arguments, unsigned short font,
                         unsigned short color )
{
  PktHelper::PacketOut<PktOut_C1> msg;
  msg->offset += 2;
  unsigned textlen = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( 0xFFFFFFFFu );  // serial
  msg->Write<u16>( 0xFFFFu );      // body
  msg->Write<u8>( 6u );            // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write( "System", 30, false );
  if ( arguments != NULL )
    msg->Write( arguments, static_cast<u16>( textlen ), true );  // ctLEu16
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}

void say_above_cl( UObject* obj, unsigned int cliloc_num, const u16* arguments, unsigned short font,
                   unsigned short color )
{
  PktHelper::PacketOut<PktOut_C1> msg;
  msg->offset += 2;
  unsigned textlen = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( obj->serial_ext );
  msg->WriteFlipped<u16>( obj->graphic );  // body
  msg->Write<u8>( 7u );                    // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write( obj->description().c_str(), 30, false );
  if ( arguments != NULL )
    msg->Write( arguments, static_cast<u16>( textlen ), true );  // ctLEu16
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  //  MuadDib - FIXME: only send to those that I'm visible to.
  transmit_to_inrange( obj, &msg->buffer, len );
}

void private_say_above_cl( Mobile::Character* chr, const UObject* obj, unsigned int cliloc_num,
                           const u16* arguments, unsigned short font, unsigned short color )
{
  PktHelper::PacketOut<PktOut_C1> msg;
  msg->offset += 2;
  unsigned textlen = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( obj->serial_ext );
  msg->WriteFlipped<u16>( obj->graphic );  // body
  msg->Write<u8>( 7u );                    // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write( obj->description().c_str(), 30, false );
  if ( arguments != NULL )
    msg->Write( arguments, static_cast<u16>( textlen ), true );  // ctLEu16
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( chr->client, len );
}


void send_sysmessage_cl_affix( Client* client, unsigned int cliloc_num, const char* affix,
                               bool prepend, const u16* arguments, unsigned short font,
                               unsigned short color )
{
  PktHelper::PacketOut<PktOut_CC> msg;
  msg->offset += 2;
  unsigned textlen = 0, affix_len = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }
  affix_len = static_cast<unsigned>( strlen( affix ) + 1 );
  if ( affix_len > SPEECH_MAX_LEN + 1 )
    affix_len = SPEECH_MAX_LEN + 1;

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( 0xFFFFFFFFu );  // serial
  msg->Write<u16>( 0xFFFFu );      // body
  msg->Write<u8>( 6u );            // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write<u8>( ( prepend ) ? 1u : 0u );
  msg->Write( "System", 30, false );
  msg->Write( affix, static_cast<u16>( affix_len ) );
  if ( arguments != NULL )
    msg->WriteFlipped( arguments, static_cast<u16>( textlen ), true );
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}

void say_above_cl_affix( UObject* obj, unsigned int cliloc_num, const char* affix, bool prepend,
                         const u16* arguments, unsigned short font, unsigned short color )
{
  PktHelper::PacketOut<PktOut_CC> msg;
  msg->offset += 2;
  unsigned textlen = 0, affix_len = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }
  affix_len = static_cast<unsigned>( strlen( affix ) + 1 );
  if ( affix_len > SPEECH_MAX_LEN + 1 )
    affix_len = SPEECH_MAX_LEN + 1;

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( obj->serial_ext );      // serial
  msg->WriteFlipped<u16>( obj->graphic );  // body
  msg->Write<u8>( 7u );                    // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write<u8>( ( prepend ) ? 1u : 0u );
  msg->Write( obj->description().c_str(), 30, false );
  msg->Write( affix, static_cast<u16>( affix_len ) );
  if ( arguments != NULL )
    msg->WriteFlipped( arguments, static_cast<u16>( textlen ), true );  // ctLEu16
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  // MuadDib - FIXME: only send to those that I'm visible to.
  transmit_to_inrange( obj, &msg->buffer, len );
}

void private_say_above_cl_affix( Mobile::Character* chr, const UObject* obj,
                                 unsigned int cliloc_num, const char* affix, bool prepend,
                                 const u16* arguments, unsigned short font, unsigned short color )
{
  PktHelper::PacketOut<PktOut_CC> msg;
  msg->offset += 2;
  unsigned textlen = 0, affix_len = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }
  affix_len = static_cast<unsigned>( strlen( affix ) + 1 );
  if ( affix_len > SPEECH_MAX_LEN + 1 )
    affix_len = SPEECH_MAX_LEN + 1;

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( obj->serial_ext );      // serial
  msg->WriteFlipped<u16>( obj->graphic );  // body
  msg->Write<u8>( 7u );                    // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write<u8>( ( prepend ) ? 1u : 0u );
  msg->Write( obj->description().c_str(), 30, false );
  msg->Write( affix, static_cast<u16>( affix_len ) );
  if ( arguments != NULL )
    msg->WriteFlipped( arguments, static_cast<u16>( textlen ), true );
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( chr->client, len );
}

void build_sysmessage_cl( PktOut_C1* msg, unsigned int cliloc_num, const u16* arguments,
                          unsigned short font, unsigned short color )
{
  msg->offset += 2;
  unsigned textlen = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( 0xFFFFFFFFu );  // serial
  msg->Write<u16>( 0xFFFFu );      // body
  msg->Write<u8>( 6u );            // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write( "System", 30, false );
  if ( arguments != NULL )
    msg->Write( arguments, static_cast<u16>( textlen ), true );  // ctLEu16
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg->offset = len;
}

void build_sysmessage_cl_affix( PktOut_CC* msg, unsigned int cliloc_num, const char* affix,
                                bool prepend, const u16* arguments, unsigned short font,
                                unsigned short color )
{
  msg->offset += 2;
  unsigned textlen = 0, affix_len = 0;

  if ( arguments != NULL )
  {
    while ( arguments[textlen] != L'\0' )
      ++textlen;
  }
  affix_len = static_cast<unsigned>( strlen( affix ) + 1 );
  if ( affix_len > SPEECH_MAX_LEN + 1 )
    affix_len = SPEECH_MAX_LEN + 1;

  if ( textlen > ( SPEECH_MAX_LEN ) )
    textlen = SPEECH_MAX_LEN;

  msg->Write<u32>( 0xFFFFFFFu );  // serial
  msg->Write<u16>( 0xFFFFu );     // body
  msg->Write<u8>( 6u );           // type 6 lower left, 7 on player
  msg->WriteFlipped<u16>( color );
  msg->WriteFlipped<u16>( font );
  msg->WriteFlipped<u32>( cliloc_num );
  msg->Write<u8>( ( prepend ) ? 1u : 0u );
  msg->Write( "System", 30, false );
  msg->Write( affix, static_cast<u16>( affix_len ) );
  if ( arguments != NULL )
    msg->WriteFlipped( arguments, static_cast<u16>( textlen ), true );
  else
    msg->offset += 2;
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg->offset = len;
}
}
}

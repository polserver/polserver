/** @file
 *
 * @par History
 * - 2007/04/07 MuadDib:   send_object_cache_to_inrange updated from just UObject* to
 *                         const UObject* for compatibility across more areas.
 * - 2009/01/03 MuadDib:   Some generic recoding to remove useless code. O_O
 * - 2009/01/27 MuadDib:   Rewrote Obj Cache Building/Sending.
 * - 2009/07/26 MuadDib:   Packet struct refactoring.
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 */


#include "pol/tooltips.h"

#include <stddef.h>
#include <string>

#include "bscript/bstring.h"
#include "clib/clib_endian.h"
#include "clib/rawtypes.h"
#include "plib/uoexpansion.h"

#include "pol/item/item.h"
#include "pol/item/itemdesc.h"
#include "pol/mobile/charactr.h"
#include "pol/network/client.h"
#include "pol/network/packetdefs.h"
#include "pol/network/packethelper.h"
#include "pol/network/packets.h"
#include "pol/network/pktin.h"
#include "pol/ufunc.h"
#include "pol/uobject.h"
#include "pol/uworld.h"


namespace Pol::Core
{
using namespace Network::PktHelper;


void handle_request_tooltip( Network::Client* client, PKTIN_B6* msgin )
{
  u32 serial = cfBEu32( msgin->serial );
  if ( !IsCharacter( serial ) )
  {
    Items::Item* item = find_legal_item( client->chr, serial );
    if ( item != nullptr )
    {
      const Items::ItemDesc& id = item->itemdesc();
      if ( !id.tooltip.empty() )
      {
        PacketOut<Network::PktOut_B7> msg;
        msg->offset += 2;
        msg->Write<u32>( item->serial_ext );
        msg->WriteFlipped( Bscript::String::toUTF16( id.tooltip ) );
        u16 len = msg->offset;
        msg->offset = 1;
        msg->WriteFlipped<u16>( len );
        msg.Send( client, len );
      }
    }
  }
}

// needed if A9 flag is sent with 0x20, single click no longer works. see about text# 1042971 for
// 0xD6
void send_object_cache( Network::Client* client, const UObject* obj )
{
  if ( !settingsManager.ssopt.features.supportsAOS() )
    return;
  auto pkt_rev = Network::ObjRevisionPkt( obj->serial_ext, obj->rev() );
  pkt_rev.Send( client );
}

void send_object_cache_to_inrange( const UObject* obj )
{
  if ( !settingsManager.ssopt.features.supportsAOS() )
    return;
  auto pkt_rev = Network::ObjRevisionPkt( obj->serial_ext, obj->rev() );

  WorldIterator<OnlinePlayerFilter>::InMaxVisualRange( obj,
                                                       [&]( Mobile::Character* chr )
                                                       {
                                                         if ( chr->in_visual_range( obj ) )
                                                           pkt_rev.Send( chr->client );
                                                         // FIXME need to check character's
                                                         // additional_legal_items.
                                                       } );
}


void SendAOSTooltip( Network::Client* client, UObject* obj, bool vendor_content )
{
  std::string desc;
  if ( obj->isa( UOBJ_CLASS::CLASS_CHARACTER ) )
  {
    Mobile::Character* chr = (Mobile::Character*)obj;

    const auto& ssopt = settingsManager.ssopt;
    desc = fmt::format( "{}{}\t{}\t{}{}", chr->title_prefix(), ssopt.title_prefix_separator,
                        chr->name(), ssopt.title_suffix_separator, chr->title_suffix() );
    if ( chr->has_title_race() )
      desc += fmt::format( "{}({})", chr->has_title_suffix() ? ssopt.title_race_separator : "",
                           chr->title_race() );
    if ( chr->has_title_guild() )
      desc += fmt::format( "{}[{}]",
                           ( chr->has_title_suffix() || chr->has_title_race() )
                               ? ssopt.title_guild_separator
                               : "",
                           chr->title_guild() );
  }
  else if ( vendor_content )
  {
    Items::Item* item = (Items::Item*)obj;
    desc = item->merchant_description();
  }
  else
    desc = obj->description();

  PacketOut<Network::PktOut_D6> msg;
  msg->offset += 2;
  msg->WriteFlipped<u16>( 1u );  // u16 unk1
  msg->Write<u32>( obj->serial_ext );
  msg->offset += 2;  // u8 unk2,unk3
  msg->WriteFlipped<u32>( obj->rev() );
  if ( obj->isa( UOBJ_CLASS::CLASS_CHARACTER ) )
    msg->WriteFlipped<u32>( 1050045u );  // ~1_PREFIX~~2_NAME~~3_SUFFIX~
  else
    msg->WriteFlipped<u32>( 1042971u );  // ~1_NOTHING~

  std::vector<u16> utf16 = Bscript::String::toUTF16( desc );
  // Clamp before narrowing to u16, or a longer text wraps and passes the
  // check. 25 bytes of the packet are not text.
  constexpr size_t max_text_length = ( 0xFFFF - 25 ) / 2;
  if ( utf16.size() > max_text_length )
    utf16.resize( max_text_length );
  u16 textlen = static_cast<u16>( utf16.size() );
  msg->WriteFlipped<u16>( textlen * 2u );
  msg->Write( utf16, false );
  msg->offset += 4;  // indicates end of property list
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}
}  // namespace Pol::Core

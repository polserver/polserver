/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/10/12 Turley:    whisper/yell/say-range ssopt definition
 */


#include "../clib/rawtypes.h"
#include "containr.h"
#include "item/item.h"
#include "menu.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "tooltips.h"
#include "ufunc.h"


namespace Pol
{
namespace Core
{
using namespace Network;

bool send_menu( Client* client, Menu* menu )
{
  // build up the message so it gets sent as a unit.
  if ( menu->menuitems_.size() > 255 )
    return false;

  PktHelper::PacketOut<PktOut_7C> msg;
  msg->offset += 2;
  msg->offset += 4;  // used_item_serial
  msg->WriteFlipped<u16>( menu->menu_id );
  std::string convertedText = Clib::strUtf8ToCp1252( menu->title );
  size_t stringlen = convertedText.length();
  if ( stringlen > 80 )
    stringlen = 80;
  msg->Write<u8>( stringlen );  // NOTE null-term not included!
  msg->Write( convertedText.c_str(), static_cast<u16>( stringlen ), false );
  msg->Write<u8>( menu->menuitems_.size() );

  for ( unsigned idx = 0; idx < menu->menuitems_.size(); idx++ )
  {
    if ( msg->offset + 85 > static_cast<u16>( sizeof msg->buffer ) )
    {
      return false;
    }
    MenuItem* mi = &menu->menuitems_[idx];
    msg->WriteFlipped<u16>( mi->graphic_ );
    msg->WriteFlipped<u16>( mi->color_ );
    convertedText = Clib::strUtf8ToCp1252( mi->title );
    stringlen = convertedText.length();
    if ( stringlen > 80 )
      stringlen = 80;
    msg->Write<u8>( stringlen );  // NOTE null-term not included!
    msg->Write( convertedText.c_str(), static_cast<u16>( stringlen ), false );
  }
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
  return true;
}

void send_open_gump( Client* client, const UContainer& cont )
{
  PktHelper::PacketOut<PktOut_24> msg;
  msg->Write<u32>( cont.serial_ext );
  msg->WriteFlipped<u16>( cont.gump() );
  if ( client->ClientType & CLIENTTYPE_7090 )
    msg->WriteFlipped<u16>( 0x7Du );
  msg.Send( client );
}

// dave changed 11/9/3, don't send invis items to those who can't see invis
void send_container_contents( Client* client, const UContainer& cont )
{
  PktHelper::PacketOut<PktOut_3C> msg;
  msg->offset += 4;  // msglen+count
  u16 count = 0;
  for ( UContainer::const_iterator itr = cont.begin(), itrend = cont.end(); itr != itrend; ++itr )
  {
    const Items::Item* item = *itr;
    if ( !item->invisible() || client->chr->can_seeinvisitems() )
    {
      msg->Write<u32>( item->serial_ext );
      msg->WriteFlipped<u16>( item->graphic );
      msg->offset++;  // unk6
      msg->WriteFlipped<u16>( item->get_senditem_amount() );
      msg->WriteFlipped<u16>( item->x() );
      msg->WriteFlipped<u16>( item->y() );
      if ( client->ClientType & CLIENTTYPE_6017 )
        msg->Write<u8>( item->slot_index() );
      msg->Write<u32>( cont.serial_ext );
      msg->WriteFlipped<u16>( item->color );  // color
      ++count;
    }
    else
    {
      send_remove_object(
          client, item );  // TODO: Doesn't this send a list of invisible items on the corpse?
    }
  }
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg->WriteFlipped<u16>( count );
  msg.Send( client, len );

  if ( client->acctSupports( Plib::ExpansionVersion::AOS ) )
  {
    // 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send
    // revision
    for ( UContainer::const_iterator itr = cont.begin(), itrend = cont.end(); itr != itrend; ++itr )
    {
      const Items::Item* item = *itr;
      if ( !item->invisible() || client->chr->can_seeinvisitems() )
      {
        send_object_cache( client, item );
      }
    }
  }
}
}  // namespace Core
}  // namespace Pol

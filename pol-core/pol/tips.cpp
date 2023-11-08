/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 */


#include <string>

#include "../clib/clib_endian.h"
#include "../clib/rawtypes.h"
#include "globals/uvars.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktdef.h"
#include "network/pktin.h"

namespace Pol
{
namespace Core
{
bool send_tip( Network::Client* client, const char* tipname, unsigned short tipnum )
{
  std::string convertedText = Clib::strUtf8ToCp1252( tipname );
  size_t textlen = convertedText.length() + 1;
  if ( textlen > 0 && unsigned( textlen ) <= 9999 )
  {
    Network::PktHelper::PacketOut<Network::PktOut_A6> msg;
    msg->WriteFlipped<u16>( textlen + 10 );
    msg->Write<u8>( PKTOUT_A6_TYPE_TIP );
    msg->offset += 2;  // unk4,5
    msg->WriteFlipped<u16>( tipnum );
    msg->WriteFlipped<u16>( textlen );
    msg->Write( convertedText.c_str(), static_cast<u16>( textlen ) );
    msg.Send( client );
    return true;
  }
  else
  {
    return false;
  }
}

void send_tip( Network::Client* client, const std::string& tiptext )
{
  std::string convertedText = Clib::strUtf8ToCp1252(tiptext);
  size_t textlen = convertedText.size() + 1;
  if ( textlen >= 10000 )
    textlen = 9999;

  Network::PktHelper::PacketOut<Network::PktOut_A6> msg;
  msg->WriteFlipped<u16>( textlen + 10 );
  msg->Write<u8>( PKTOUT_A6_TYPE_TIP );
  msg->offset += 2;  // unk4,5
  msg->offset += 2;  // tipnum
  msg->WriteFlipped<u16>( textlen );
  msg->Write( convertedText.c_str(), static_cast<u16>( textlen ) );
  msg.Send( client );
}

void handle_get_tip( Network::Client* client, PKTIN_A7* msg )
{
  u16 tipnum = cfBEu16( msg->lasttip );
  if ( !gamestate.tipfilenames.empty() )
  {
    if ( msg->prevnext )
    {
      ++tipnum;
      if ( tipnum >= gamestate.tipfilenames.size() )
        tipnum = 0;
    }
    else
    {
      --tipnum;
      if ( tipnum >= gamestate.tipfilenames.size() )
        tipnum = static_cast<u16>( gamestate.tipfilenames.size() ) - 1;
    }

    send_tip( client, gamestate.tipfilenames[tipnum].c_str(), tipnum );
  }
}
}  // namespace Core
}  // namespace Pol

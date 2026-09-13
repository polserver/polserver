/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 */


#include <fmt/format.h>

#include <filesystem>
#include <string>

#include "clib/clib_endian.h"
#include "clib/filecont.h"
#include "clib/rawtypes.h"
#include "pol/globals/uvars.h"
#include "pol/network/packethelper.h"
#include "pol/network/packets.h"
#include "pol/network/pktdef.h"
#include "pol/network/pktin.h"
#include "pol/tips.h"


namespace Pol::Core
{
namespace fs = std::filesystem;

constexpr const char* TIPS_DIRECTORY = "tips";
// longest text a tip packet is sent, terminator included; a longer file is truncated to it
constexpr size_t MAX_TIP_TEXT = 9999;

void load_tips()
{
  gamestate.tipfilenames.clear();

  std::error_code ec;
  for ( const auto& dir_entry : fs::directory_iterator( TIPS_DIRECTORY, ec ) )
  {
    if ( !dir_entry.is_regular_file() )
      continue;
    if ( auto fn = dir_entry.path().filename().string(); !fn.empty() && *fn.begin() == '.' )
      continue;
    const auto& path = dir_entry.path();
    if ( !path.extension().compare( ".txt" ) )
    {
      gamestate.tipfilenames.push_back( path.filename().string() );
    }
  }
}

void send_tip( Network::Client* client, const std::string& text, unsigned short tipnum )
{
  if ( text.empty() )
    return;

  std::string convertedText = Clib::strUtf8ToCp1252( text );
  size_t textlen = convertedText.length() + 1;
  if ( textlen > MAX_TIP_TEXT )
    textlen = MAX_TIP_TEXT;

  Network::PktHelper::PacketOut<Network::PktOut_A6> msg;
  msg->WriteFlipped<u16>( textlen + 10 );
  msg->Write<u8>( PKTOUT_A6_TYPE_TIP );
  msg->offset += 2;  // unk4,5
  msg->WriteFlipped<u16>( tipnum );
  msg->WriteFlipped<u16>( textlen );
  msg->Write( convertedText.c_str(), static_cast<u16>( textlen ) );
  msg.Send( client );
}

void handle_get_tip( Network::Client* client, PKTIN_A7* msg )
{
  if ( gamestate.tipfilenames.empty() )
    return;

  u16 tipnum = cfBEu16( msg->lasttip );
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

  // load_tips keeps only the name, so the file is read here to send what is in it
  try
  {
    Clib::FileContents file(
        fmt::format( "{}/{}", TIPS_DIRECTORY, gamestate.tipfilenames[tipnum] ).c_str(), true );
    send_tip( client, file.str_contents(), tipnum );
  }
  catch ( const std::runtime_error& )
  {
    // listed at startup and not readable now; an empty one is dropped by send_tip
  }
}
}  // namespace Pol::Core

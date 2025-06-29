/** @file
 *
 * @par History
 * - 2005/01/24 Shinigami: added message handler for packet 0xd9 (Spy on Client 2)
 * - 2005/01/27 Shinigami: using little/big endian functions provided by endian.h
 * - 2005/04/03 Shinigami: send_feature_enable() call moved from start_client_char()
 *                         to send_start() to send before char selection
 * - 2005/04/03 Shinigami: FLAG_UPTO_SIX_CHARACTERS disabled in uo_feature_enable
 * - 2005/04/04 Shinigami: added candelete script
 * - 2005/08/29 Shinigami: get-/setspyonclient2 renamed to get-/setclientinfo
 * - 2007/07/09 Shinigami: modified PKTOUT_8C.new_key to not conflict with UO:KR detection
 *                         added message handler for packet 0xe4 (UO:KR Verifier Response)
 * - 2008/02/09 Shinigami: removed hardcoded MAX_CHARS from send_start()
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/12/04 Turley:    Crypto cleanup - Tomi
 * - 2010/01/14 Turley:    fixed ip byteorder for pkt 0xa8 if hostname is given - Arkham
 */


#include <cstring>
#include <string>

#include "../clib/clib.h"
#include "../clib/clib_MD5.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "../plib/uoexpansion.h"
#include "accounts/account.h"
#include "accounts/accounts.h"
#include "core.h"
#include "crypt/cryptbase.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "servdesc.h"
#include "startloc.h"
#include "ufunc.h"

namespace Pol
{
namespace Network
{
bool is_banned_ip( Client* client );
}

namespace Core
{
void call_ondelete_scripts( Mobile::Character* chr );
bool can_delete_character( Mobile::Character* chr, int delete_by );

void send_login_error( Network::Client* client, unsigned char reason )
{
  Network::PktHelper::PacketOut<Network::PktOut_82> msg;
  msg->Write<u8>( reason );
  msg.Send( client );
}

// TODO: rewrite this loop to use stl algorithms + stlutil.h case insensitive compare function
//        -- and I'm leaving the warning here to remember that --
bool acct_check( Network::Client* client, int i )
{
  if ( networkManager.servers[i]->acct_match.empty() )
    return true;

  for ( unsigned j = 0; j < networkManager.servers[i]->acct_match.size(); ++j )
  {
    if ( stricmp( networkManager.servers[i]->acct_match[j].c_str(), client->acct->name() ) == 0 )
      return true;
  }

  return false;
}

bool ip_check( Network::Client* client, int i )
{
  if ( networkManager.servers[i]->ip_match.empty() )
    return true;

  for ( unsigned j = 0; j < networkManager.servers[i]->ip_match.size(); ++j )
  {
    unsigned int addr1part, addr2part;
    struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>( &client->ipaddr );

    addr1part =
        networkManager.servers[i]->ip_match[j] & networkManager.servers[i]->ip_match_mask[j];
#ifdef _WIN32
    addr2part = sockin->sin_addr.S_un.S_addr & networkManager.servers[i]->ip_match_mask[j];
#else
    addr2part = sockin->sin_addr.s_addr & networkManager.servers[i]->ip_match_mask[j];
#endif
    if ( addr1part == addr2part )
      return true;
  }
  return false;
}

bool proxy_check( Network::Client* client, int i )
{
  bool is_proxied = client->ipaddr_proxy.sa_family != AF_UNSPEC;

  if ( networkManager.servers[i]->proxy_match.empty() )
  {
    // If there is no ProxyMatch element but client is connecting through proxy
    // do not present this server.
    return !is_proxied;
  }

  if ( !is_proxied )
  {
    // This server has ProxyMatch element(s) but client is not connecting through proxy
    // so do not present this server.
    return false;
  }

  for ( unsigned j = 0; j < networkManager.servers[i]->proxy_match.size(); ++j )
  {
    unsigned int addr1part, addr2part;
    struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>( &client->ipaddr_proxy );

    addr1part =
        networkManager.servers[i]->proxy_match[j] & networkManager.servers[i]->proxy_match_mask[j];
#ifdef _WIN32
    addr2part = sockin->sin_addr.S_un.S_addr & networkManager.servers[i]->proxy_match_mask[j];
#else
    addr2part = sockin->sin_addr.s_addr & networkManager.servers[i]->proxy_match_mask[j];
#endif
    if ( addr1part == addr2part )
      return true;
  }
  return false;
}

bool server_applies( Network::Client* client, int i )
{
  return ip_check( client, i ) && proxy_check( client, i ) && acct_check( client, i );
}

void loginserver_login( Network::Client* client, PKTIN_80* msg )
{
  unsigned idx;

  if ( Network::is_banned_ip( client ) )
  {
    send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
    client->Disconnect();
    return;
  }

  Accounts::Account* acct = Accounts::find_account( msg->name );
  if ( !acct )
  {
    send_login_error( client, LOGIN_ERROR_NO_ACCOUNT );
    client->Disconnect();
    return;
  }
  else if ( Plib::systemstate.config.min_cmdlevel_to_login > acct->default_cmdlevel() )
  {
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }

  bool correct_password = false;

  std::string msgpass = msg->password;
  std::string acctname = acct->name();
  std::string temp;
  Clib::MD5_Encrypt( acctname + msgpass, temp );  // MD5
  correct_password = Clib::MD5_Compare( acct->passwordhash(), temp );

  if ( !correct_password )
  {
    send_login_error( client, LOGIN_ERROR_WRONG_PASSWORD );
    client->Disconnect();
    POLLOGLN( "Incorrect password for account {} from {}", acct->name(), client->ipaddrAsString() );
    return;
  }
  else
  {
    if ( Plib::systemstate.config.retain_cleartext_passwords )
    {
      if ( acct->password().empty() )
        acct->set_password( msgpass );
    }
  }

  if ( !acct->enabled() || acct->banned() )
  {
    send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
    client->Disconnect();
    return;
  }

  POLLOG_INFOLN( "Account {} logged in from {}", acct->name(), client->ipaddrAsString() );

  client->acct = acct;

  Network::PktHelper::PacketOut<Network::PktOut_A8> msgA8;
  msgA8->offset += 2;
  msgA8->Write<u8>( 0xFFu );
  msgA8->offset += 2;  // servcount

  unsigned short servcount = 0;


  for ( idx = 0; idx < networkManager.servers.size(); idx++ )
  {
    ServerDescription* server = networkManager.servers[idx];

    if ( !server->hostname.empty() )
    {
      struct hostent* he =
          gethostbyname( server->hostname.c_str() );  // FIXME: here is a potential server lockup
      if ( he != nullptr && he->h_addr_list[0] )
      {
        char* addr = he->h_addr_list[0];
        server->ip[0] = addr[3];
        server->ip[1] = addr[2];
        server->ip[2] = addr[1];
        server->ip[3] = addr[0];
      }
      else
      {
        POLLOGLN( "gethostbyname(\"{}\") failed for server {}", server->hostname, server->name );
        continue;
      }
    }

    if ( server_applies( client, idx ) )
    {
      ++servcount;
      msgA8->WriteFlipped<u16>( idx + 1u );
      msgA8->Write( Clib::strUtf8ToCp1252( server->name ).c_str(), 30 );
      msgA8->WriteFlipped<u16>( idx + 1u );
      msgA8->offset += 2;  // u8 percentfull, s8 timezone
      msgA8->Write( server->ip, 4 );
    }
  }
  u16 len = msgA8->offset;
  msgA8->offset = 1;
  msgA8->WriteFlipped<u16>( len );
  msgA8->offset++;
  msgA8->WriteFlipped<u16>( servcount );

  msgA8.Send( client, len );

  if ( servcount == 0 )
  {
    POLLOGLN( "No applicable servers for client connecting from {}", client->ipaddrAsString() );
  }
}

void handle_A4( Network::Client* /*client*/, PKTIN_A4* /*msg*/ ) {}

void handle_D9( Network::Client* client, PKTIN_D9* msg )
{
  PKTIN_D9 _msg;  // got crashes here under *nix -> modify a new local instance
  // Transform Little-Endian <-> Big-Endian
  _msg.instance = cfBEu32( msg->instance );              // Unique Instance ID of UO
  _msg.os_major = cfBEu32( msg->os_major );              // OS Major
  _msg.os_minor = cfBEu32( msg->os_minor );              // OS Minor
  _msg.os_revision = cfBEu32( msg->os_revision );        // OS Revision
  _msg.cpu_family = cfBEu32( msg->cpu_family );          // CPU Family
  _msg.cpu_model = cfBEu32( msg->cpu_model );            // CPU Model
  _msg.cpu_clockspeed = cfBEu32( msg->cpu_clockspeed );  // CPU Clock Speed [Mhz]
  _msg.memory = cfBEu32( msg->memory );                  // Memory [MB]
  _msg.screen_width = cfBEu32( msg->screen_width );      // Screen Width
  _msg.screen_height = cfBEu32( msg->screen_height );    // Screen Height
  _msg.screen_depth = cfBEu32( msg->screen_depth );      // Screen Depth [Bit]
  _msg.directx_major = cfBEu16( msg->directx_major );    // DirectX Major
  _msg.directx_minor = cfBEu16( msg->directx_minor );    // DirectX Minor

  for ( unsigned i = 0; i < sizeof( msg->video_description ) / sizeof( msg->video_description[0] );
        ++i )
    _msg.video_description[i] =
        cfBEu16( msg->video_description[i] );  // Video Card Description [wide-character]

  _msg.video_vendor = cfBEu32( msg->video_vendor );  // Video Card Vendor ID
  _msg.video_device = cfBEu32( msg->video_device );  // Video Card Device ID
  _msg.video_memory = cfBEu32( msg->video_memory );  // Video Card Memory [MB]

  for ( unsigned i = 0; i < sizeof( msg->langcode ) / sizeof( msg->langcode[0] ); ++i )
    _msg.langcode[i] = cfBEu16( msg->langcode[i] );  // Language Code [wide-character]

  client->setclientinfo( &_msg );
}

void select_server( Network::Client* client, PKTIN_A0* msg )  // Relay player to a certain IP
{
  unsigned servernum = cfBEu16( msg->servernum ) - 1;

  if ( servernum >= networkManager.servers.size() )
  {
    client->forceDisconnect();
    return;
  }

  ServerDescription* svr = networkManager.servers[servernum];

  Network::PktHelper::PacketOut<Network::PktOut_8C> rsp;
  rsp->Write<u8>( svr->ip[3] );
  rsp->Write<u8>( svr->ip[2] );
  rsp->Write<u8>( svr->ip[1] );
  rsp->Write<u8>( svr->ip[0] );

  if ( client->listen_port != 0 )
    rsp->WriteFlipped<u16>( client->listen_port );
  else
    rsp->WriteFlipped<u16>( svr->port );
  // MuadDib Added new seed system. This is for transferring KR/6017/Normal client detection from
  // loginserver
  // to the gameserver. Allows keeping client flags from remote loginserver to gameserver for 6017
  // and kr
  // packets.

  unsigned int nseed = 0xFEFE0000 | client->ClientType;
  rsp->WriteFlipped<u32>( nseed );  // This was set to 0xffffffff in the past but this will conflict
                                    // with UO:KR detection

  rsp.Send( client );

  client->init_crypto( &nseed, Crypt::CCryptBase::typeGame );
}

void send_start( Network::Client* client )
{
  send_feature_enable(
      client );  // Shinigami: moved from start_client_char() to send before char selection

  u8 char_slots = client->acct->expansion().getCharSlots( settingsManager.ssopt.features );
  // client always expects at least 5 chars
  u8 char_count = std::max( char_slots, 5_u8 );

  Network::PktHelper::PacketOut<Network::PktOut_A9> msg;
  msg->offset += 2;
  msg->Write<u8>( char_count );

  for ( u8 i = 0; i < char_count; i++ )
  {
    if ( i < char_slots )  // Small kludge to have a minimum of 5 chars in the packet
    {
      // name only 30 long rest is password seems to fix the password promt problem
      Mobile::Character* chr = client->acct->get_character( i );
      if ( chr )
      {
        msg->Write( Clib::strUtf8ToCp1252( chr->name() ).c_str(), 30, false );
        msg->offset += 30;  // password
      }
      else
        msg->offset += 60;
    }
    else
      msg->offset += 60;
  }

  msg->Write<u8>( gamestate.startlocations.size() );

  for ( size_t i = 0; i < gamestate.startlocations.size(); i++ )
  {
    msg->Write<u8>( i );
    if ( client->ClientType & Network::CLIENTTYPE_70130 )
    {
      msg->Write( Clib::strUtf8ToCp1252( gamestate.startlocations[i]->city ).c_str(), 32, false );
      msg->Write( Clib::strUtf8ToCp1252( gamestate.startlocations[i]->desc ).c_str(), 32, false );

      Pos3d coord = gamestate.startlocations[i]->coords[0];

      msg->WriteFlipped<u32>( coord.x() );
      msg->WriteFlipped<u32>( coord.y() );
      msg->WriteFlipped<s32>( coord.z() );
      msg->WriteFlipped<u32>( gamestate.startlocations[i]->mapid );        // MapID
      msg->WriteFlipped<u32>( gamestate.startlocations[i]->cliloc_desc );  // Cliloc Description
      msg->offset += 4;
    }
    else
    {
      msg->Write( Clib::strUtf8ToCp1252( gamestate.startlocations[i]->city ).c_str(), 31, false );
      msg->Write( Clib::strUtf8ToCp1252( gamestate.startlocations[i]->desc ).c_str(), 31, false );
    }
  }

  auto clientflag =
      client->acct->expansion().calculateFeatureFlags( settingsManager.ssopt.features );

  msg->WriteFlipped<u32>( static_cast<u32>( clientflag ) );
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}

void login2( Network::Client* client, PKTIN_91* msg )  // Gameserver login and character listing
{
  client->start_encrypted_server_stream();

  if ( Network::is_banned_ip( client ) )
  {
    send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
    client->Disconnect();
    return;
  }

  /* Hmm, might have to re-search for account.
     For now, we already have the account in client->acct.
     Might work different if real loginservers were used. */
  Accounts::Account* acct = Accounts::find_account( msg->name );
  if ( acct == nullptr )
  {
    send_login_error( client, LOGIN_ERROR_NO_ACCOUNT );
    client->Disconnect();
    return;
  }

  // First check the password - if wrong, you can't find out anything else.
  bool correct_password = false;

  // dave changed 6/5/3, always authenticate with hashed user+pass
  std::string msgpass = msg->password;
  std::string acctname = acct->name();
  std::string temp;
  Clib::MD5_Encrypt( acctname + msgpass, temp );  // MD5
  correct_password = Clib::MD5_Compare( acct->passwordhash(), temp );

  if ( !correct_password )
  {
    send_login_error( client, LOGIN_ERROR_WRONG_PASSWORD );
    client->Disconnect();
    POLLOGLN( "Incorrect password for account {} from {}", acct->name(), client->ipaddrAsString() );
    return;
  }
  else
  {
    // write out cleartext if necessary
    if ( Plib::systemstate.config.retain_cleartext_passwords )
    {
      if ( acct->password().empty() )
        acct->set_password( msgpass );
    }
  }

  if ( !acct->enabled() || acct->banned() )
  {
    send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
    client->Disconnect();
    return;
  }

  //
  // Dave moved the max_clients check to pol.cpp so character cmdlevel could be checked.
  //

  POLLOGLN( "Account {} logged in from {}", acct->name(), client->ipaddrAsString() );

  // ENHANCEMENT: could authenticate with real loginservers.

  client->acct = acct;
  /* NOTE: acct->client is not set here.  It is possible that another client
     is still connected, or a connection is stuck open, or similar.  When
     a character is selected, if another client is connected, measures will
     be taken. */

  // Tell the client about the starting locations and his characters (up to 5).

  // MuadDib Added new seed system. This is for transferring KR/6017/Normal client detection from
  // loginserver
  // to the gameserver. Allows keeping client flags from remote loginserver to gameserver for 6017
  // and kr
  // packets.
  client->ClientType = cfBEu16( msg->unk3_4_ClientType );

  send_start( client );
}

void delete_character( Accounts::Account* acct, Mobile::Character* chr, int charidx )
{
  if ( !chr->logged_in() )
  {
    POLLOGLN( "Account {} deleting character {:#x}", acct->name(), chr->serial );

    chr->acct.clear();
    acct->clear_character( charidx );
    chr->on_delete_from_account();
    chr->destroy();
  }
}


void handle_delete_character( Network::Client* client, PKTIN_83* msg )
{
  u32 charidx = cfBEu32( msg->charidx );

  if ( ( charidx >= Plib::systemstate.config.character_slots ) || ( client->acct == nullptr ) ||
       ( client->acct->get_character( charidx ) == nullptr ) )
  {
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }

  Accounts::Account* acct = client->acct;
  Mobile::Character* chr = acct->get_character( charidx );
  if ( chr->client != nullptr || ( !Plib::systemstate.config.allow_multi_clients_per_account &&
                                   acct->has_active_characters() ) )
  {
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }

  if ( can_delete_character( chr, DELETE_BY_PLAYER ) )
  {
    call_ondelete_scripts( chr );
    delete_character( acct, chr, charidx );
  }

  send_start( client );
}

void KR_Verifier_Response( Network::Client* /*client*/, PKTIN_E4* /*msg*/ )
{
  //
}
}  // namespace Core
}  // namespace Pol

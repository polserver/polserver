/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of _leaderserial
 *                         STLport-5.2.1 fix: UCconv, bytemsg and wtextoffset not used
 *                         STLport-5.2.1 fix: "wtext[ SPEECH_MAX_LEN+1 ];" has no effect
 *                         STLport-5.2.1 fix: illegal usage of nullptr instead of 0
 *                         STLport-5.2.1 fix: initialization of member in handle_party_msg()
 * - 2009/08/26 Turley:    changed convertUCtoArray() to if check
 * - 2009/11/17 Turley:    fixed hang due to illegal pointers
 */

#include "pol_global_config.h"

#include "party.h"

#include <ctype.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "../bscript/impstr.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cfgsect.h"
#include "../clib/clib_endian.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/streamsaver.h"
#include "../plib/systemstate.h"
#include "clfunc.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "module/partymod.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "network/pktdef.h"
#include "party_cfg.h"
#include "polclock.h"
#include "schedule.h"
#include "statmsg.h"
#include "syshook.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoclient.h"

#ifdef MEMORYLEAK
#include "../bscript/bobject.h"
#endif


namespace Pol
{
namespace Core
{
void handle_unknown_packet( Network::Client* client );

Party::Party( u32 serial )
    : _member_serials(),
      _candidates_serials(),
      _offlinemember_serials(),
      _leaderserial( serial ),
      _proplist( Core::CPropProfiler::Type::PARTY )
{
  _member_serials.push_back( serial );
}

Party::Party( Clib::ConfigElem& elem )
    : _member_serials(),
      _candidates_serials(),
      _offlinemember_serials(),
      _leaderserial( 0 ),
      _proplist( Core::CPropProfiler::Type::PARTY )
{
  _leaderserial = elem.remove_ulong( "LEADER" );
  unsigned int tmp;
  while ( elem.remove_prop( "MEMBER", &tmp ) )
  {
    _member_serials.push_back( tmp );
  }
  _proplist.readProperties( elem );
}

void load_party_cfg_general( Clib::ConfigElem& elem )
{
  settingsManager.party_cfg.General.MaxPartyMembers = elem.remove_ushort( "MaxPartyMembers", 10 );
  settingsManager.party_cfg.General.TreatNoAsPrivate =
      elem.remove_bool( "TreatNoAsPrivate", false );
  settingsManager.party_cfg.General.DeclineTimeout = elem.remove_ushort( "DeclineTimeout", 10 );
  settingsManager.party_cfg.General.RemoveMemberOnLogoff =
      elem.remove_bool( "RemoveMemberOnLogoff", false );
  if ( settingsManager.party_cfg.General.RemoveMemberOnLogoff )
    settingsManager.party_cfg.General.RejoinPartyOnLogon =
        elem.remove_bool( "RejoinPartyOnLogon", false );
  else
    settingsManager.party_cfg.General.RejoinPartyOnLogon = false;
  std::string tmp = elem.remove_string( "PrivateMsgPrefix", "" );
  if ( !tmp.empty() )
  {
    Clib::sanitizeUnicodeWithIso( &tmp );
    tmp += " ";
    settingsManager.party_cfg.General.PrivateMsgPrefix = tmp;
  }
  else
    settingsManager.party_cfg.General.PrivateMsgPrefix = "";
}

void load_party_cfg_hooks( Clib::ConfigElem& elem )
{
  std::string temp;
  if ( elem.remove_prop( "CanAddToParty", &temp ) )
    settingsManager.party_cfg.Hooks.CanAddToParty = FindExportedFunction( elem, nullptr, temp, 2 );
  if ( elem.remove_prop( "CanRemoveMember", &temp ) )
    settingsManager.party_cfg.Hooks.CanRemoveMember =
        FindExportedFunction( elem, nullptr, temp, 2 );
  if ( elem.remove_prop( "CanLeaveParty", &temp ) )
    settingsManager.party_cfg.Hooks.CanLeaveParty = FindExportedFunction( elem, nullptr, temp, 1 );
  if ( elem.remove_prop( "OnPublicChat", &temp ) )
    settingsManager.party_cfg.Hooks.OnPublicChat = FindExportedFunction( elem, nullptr, temp, 2 );
  if ( elem.remove_prop( "OnPrivateChat", &temp ) )
    settingsManager.party_cfg.Hooks.OnPrivateChat = FindExportedFunction( elem, nullptr, temp, 3 );
  if ( elem.remove_prop( "OnDisband", &temp ) )
    settingsManager.party_cfg.Hooks.OnDisband = FindExportedFunction( elem, nullptr, temp, 1 );
  if ( elem.remove_prop( "ChangePublicChat", &temp ) )
    settingsManager.party_cfg.Hooks.ChangePublicChat =
        FindExportedFunction( elem, nullptr, temp, 2 );
  if ( elem.remove_prop( "ChangePrivateChat", &temp ) )
    settingsManager.party_cfg.Hooks.ChangePrivateChat =
        FindExportedFunction( elem, nullptr, temp, 3 );
  if ( elem.remove_prop( "OnLeaveParty", &temp ) )
    settingsManager.party_cfg.Hooks.OnLeaveParty = FindExportedFunction( elem, nullptr, temp, 2 );
  if ( elem.remove_prop( "OnAddToParty", &temp ) )
    settingsManager.party_cfg.Hooks.OnAddToParty = FindExportedFunction( elem, nullptr, temp, 1 );
  if ( elem.remove_prop( "OnPartyCreate", &temp ) )
    settingsManager.party_cfg.Hooks.OnPartyCreate = FindExportedFunction( elem, nullptr, temp, 1 );
  if ( elem.remove_prop( "OnDecline", &temp ) )
    settingsManager.party_cfg.Hooks.OnDecline = FindExportedFunction( elem, nullptr, temp, 1 );
  if ( elem.remove_prop( "OnLootPermChange", &temp ) )
    settingsManager.party_cfg.Hooks.OnLootPermChange =
        FindExportedFunction( elem, nullptr, temp, 1 );
}
void load_party_cfg( bool reload )
{
  if ( !Clib::FileExists( "config/party.cfg" ) )
  {
    settingsManager.party_cfg.General.MaxPartyMembers = 10;
    settingsManager.party_cfg.General.TreatNoAsPrivate = false;
    settingsManager.party_cfg.General.DeclineTimeout = 10;
    settingsManager.party_cfg.General.RemoveMemberOnLogoff = false;
    settingsManager.party_cfg.General.RejoinPartyOnLogon = false;
    settingsManager.party_cfg.General.PrivateMsgPrefix = "";
  }
  else
  {
    Clib::ConfigFile cf( "config/party.cfg" );

    Clib::ConfigSection general( cf, "General" );
    Clib::ConfigSection hooks( cf, "HookList", Clib::CST_NORMAL );

    Clib::ConfigElem elem;

    while ( cf.read( elem ) )
    {
      if ( general.matches( elem ) )
        load_party_cfg_general( elem );
      else if ( hooks.matches( elem ) && !reload )
        load_party_cfg_hooks( elem );
    }
  }
}

void unload_party_hooks()
{
  if ( settingsManager.party_cfg.Hooks.CanAddToParty != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.CanAddToParty;
    settingsManager.party_cfg.Hooks.CanAddToParty = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.CanRemoveMember != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.CanRemoveMember;
    settingsManager.party_cfg.Hooks.CanRemoveMember = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.CanLeaveParty != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.CanLeaveParty;
    settingsManager.party_cfg.Hooks.CanLeaveParty = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnPublicChat != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnPublicChat;
    settingsManager.party_cfg.Hooks.OnPublicChat = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnPrivateChat != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnPrivateChat;
    settingsManager.party_cfg.Hooks.OnPrivateChat = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnDisband != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnDisband;
    settingsManager.party_cfg.Hooks.OnDisband = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.ChangePublicChat != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.ChangePublicChat;
    settingsManager.party_cfg.Hooks.ChangePublicChat = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.ChangePrivateChat != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.ChangePrivateChat;
    settingsManager.party_cfg.Hooks.ChangePrivateChat = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnLeaveParty != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnLeaveParty;
    settingsManager.party_cfg.Hooks.OnLeaveParty = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnAddToParty != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnAddToParty;
    settingsManager.party_cfg.Hooks.OnAddToParty = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnPartyCreate != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnPartyCreate;
    settingsManager.party_cfg.Hooks.OnPartyCreate = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnDecline != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnDecline;
    settingsManager.party_cfg.Hooks.OnDecline = nullptr;
  }
  if ( settingsManager.party_cfg.Hooks.OnLootPermChange != nullptr )
  {
    delete settingsManager.party_cfg.Hooks.OnLootPermChange;
    settingsManager.party_cfg.Hooks.OnLootPermChange = nullptr;
  }
}
void unload_party()
{
  unload_party_hooks();

  for ( auto& party : gamestate.parties )
  {
    party->cleanup();
  }
  gamestate.parties.clear();
}

void Party::cleanup()
{
  _member_serials.clear();
  _candidates_serials.clear();
  _offlinemember_serials.clear();
}

u32 Party::leader() const
{
  return _leaderserial;
}

bool Party::is_member( u32 serial ) const
{
  return std::find( _member_serials.begin(), _member_serials.end(), serial ) !=
         _member_serials.end();
}

bool Party::is_candidate( u32 serial ) const
{
  return std::find( _candidates_serials.begin(), _candidates_serials.end(), serial ) !=
         _candidates_serials.end();
}

bool Party::register_with_members()
{
  std::vector<u32>::iterator itr = _member_serials.begin();
  while ( itr != _member_serials.end() )
  {
    Mobile::Character* chr = system_find_mobile( *itr );
    if ( chr != nullptr )
    {
      chr->party( this );
      ++itr;
    }
    else
      itr = _member_serials.erase( itr );
  }
  if ( _member_serials.empty() )
    return ( false );
  if ( !test_size() )
    return ( false );
  if ( system_find_mobile( _leaderserial ) == nullptr )
    _leaderserial = *_member_serials.begin();

  return true;
}

bool Party::is_leader( u32 serial ) const
{
  return _leaderserial == serial;
}

u32 Party::get_member_at( unsigned short pos ) const
{
  if ( _member_serials.size() < pos )
    return 0;
  return _member_serials[pos];
}

bool Party::add_candidate( u32 serial )
{
  if ( ( _member_serials.size() + _candidates_serials.size() ) >=
       settingsManager.party_cfg.General.MaxPartyMembers )
    return false;
  _candidates_serials.push_back( serial );
  return true;
}

bool Party::add_member( u32 serial )
{
  if ( ( _member_serials.size() + _candidates_serials.size() ) >=
       settingsManager.party_cfg.General.MaxPartyMembers )
    return false;
  _member_serials.push_back( serial );
  return true;
}

void Party::add_offline_mem( u32 serial )
{
  _offlinemember_serials.push_back( serial );
}

bool Party::remove_candidate( u32 serial )
{
  auto itr = std::find_if( _candidates_serials.begin(), _candidates_serials.end(),
                           [&]( u32& i ) { return i == serial; } );
  if ( itr != _candidates_serials.end() )
  {
    _candidates_serials.erase( itr );
    return true;
  }
  return false;
}

bool Party::remove_member( u32 serial )
{
  auto itr = std::find_if( _member_serials.begin(), _member_serials.end(),
                           [&]( u32& i ) { return i == serial; } );
  if ( itr != _member_serials.end() )
  {
    _member_serials.erase( itr );
    return true;
  }
  return false;
}

bool Party::remove_offline_mem( u32 serial )
{
  auto itr = std::find_if( _offlinemember_serials.begin(), _offlinemember_serials.end(),
                           [&]( u32& i ) { return i == serial; } );
  if ( itr != _offlinemember_serials.end() )
  {
    _offlinemember_serials.erase( itr );
    return true;
  }
  return false;
}

void Party::set_leader( u32 serial )
{
  auto itr = std::find_if( _member_serials.begin(), _member_serials.end(),
                           [&]( u32& i ) { return i == serial; } );
  if ( itr != _member_serials.end() )
  {
    _member_serials.erase( itr );
  }
  _member_serials.insert( _member_serials.begin(), serial );
  _leaderserial = serial;
}

bool Party::test_size() const
{
  if ( ( _candidates_serials.empty() ) && ( _member_serials.size() <= 1 ) )
    return false;
  return true;
}

bool Party::can_add() const
{
  if ( ( _member_serials.size() + _candidates_serials.size() ) >=
       settingsManager.party_cfg.General.MaxPartyMembers )
    return false;
  return true;
}

void Party::send_member_list( Mobile::Character* to_chr )
{
  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
  msg->offset += 4;  // len+sub
  msg->Write<u8>( PKTBI_BF_06::PARTYCMD_ADD );
  msg->offset++;  // nummembers

  // TODO: refactor the loop below to use std::remove_if() + extract a method
  auto itr = _member_serials.begin();
  while ( itr != _member_serials.end() )
  {
    Mobile::Character* chr = system_find_mobile( *itr );
    if ( chr != nullptr )
    {
      msg->Write<u32>( chr->serial_ext );
      ++itr;
    }
    else
      itr = _member_serials.erase( itr );
  }

  u16 len = msg->offset;
  msg->offset = 6;
  msg->Write<u8>( _member_serials.size() );
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  if ( to_chr == nullptr )
  {
    for ( const auto& serial : _member_serials )
    {
      Mobile::Character* chr = system_find_mobile( serial );
      if ( chr != nullptr )
      {
        if ( chr->has_active_client() )
          msg.Send( chr->client, len );
      }
    }
  }
  else
    msg.Send( to_chr->client, len );
}

void Party::disband()
{
  if ( settingsManager.party_cfg.Hooks.OnDisband )
    settingsManager.party_cfg.Hooks.OnDisband->call( Module::CreatePartyRefObjImp( this ) );

  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* chr = system_find_mobile( serial );
    if ( chr != nullptr )
    {
      chr->party( nullptr );
      if ( chr->has_active_client() )
      {
        send_empty_party( chr );
        send_sysmessage_cl( chr->client, CLP_Disbanded );  // Your party has disbanded.
      }
    }
  }
  for ( const auto& serial : _candidates_serials )
  {
    Mobile::Character* chr = system_find_mobile( serial );
    if ( chr != nullptr )
      chr->candidate_of( nullptr );
  }
  for ( const auto& serial : _offlinemember_serials )
  {
    Mobile::Character* chr = system_find_mobile( serial );
    if ( chr != nullptr )
      chr->offline_mem_of( nullptr );
  }

  _member_serials.clear();
  _candidates_serials.clear();
  _offlinemember_serials.clear();
}

void Party::send_remove_member( Mobile::Character* remchr, bool* disband )
{
  *disband = false;
  if ( remchr->serial == _leaderserial )
  {
    *disband = true;
    return;
  }
  else
  {
    Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
    msg->offset += 4;  // len+sub
    msg->Write<u8>( PKTBI_BF_06::PARTYCMD_REMOVE );
    msg->offset++;  // nummembers
    msg->Write<u32>( remchr->serial_ext );

    // TODO: refactor the loop below to use std::remove_if() + extract a method
    auto itr = _member_serials.begin();
    while ( itr != _member_serials.end() )
    {
      Mobile::Character* chr = system_find_mobile( *itr );
      if ( chr != nullptr )
      {
        msg->Write<u32>( chr->serial_ext );
        ++itr;
      }
      else
        itr = _member_serials.erase( itr );
    }
    u16 len = msg->offset;
    msg->offset = 6;
    msg->Write<u8>( _member_serials.size() );
    msg->offset = 1;
    msg->WriteFlipped<u16>( len );

    for ( const auto& serial : _member_serials )
    {
      Mobile::Character* chr = system_find_mobile( serial );
      if ( chr != nullptr )
      {
        if ( chr->has_active_client() )
          msg.Send( chr->client, len );
      }
    }
    send_msg_to_all( CLP_Player_Removed );  // A player has been removed from your party.
    if ( !test_size() )
    {
      send_msg_to_all( CLP_Last_Person );  // The last person has left the party...
      *disband = true;
    }
  }
}

void Party::send_msg_to_all( unsigned int clilocnr, const std::string& affix,
                             Mobile::Character* exeptchr ) const
{
  Network::PktHelper::PacketOut<Network::PktOut_C1> msgc1;
  Network::PktHelper::PacketOut<Network::PktOut_CC> msgcc;
  if ( !affix.empty() )
    build_sysmessage_cl_affix( msgcc.Get(), clilocnr, affix, true );
  else
    build_sysmessage_cl( msgc1.Get(), clilocnr );

  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* chr = system_find_mobile( serial );
    if ( chr != nullptr )
    {
      if ( chr != exeptchr )
      {
        if ( chr->has_active_client() )
        {
          if ( !affix.empty() )
            msgcc.Send( chr->client );
          else
            msgc1.Send( chr->client );
        }
      }
    }
  }
}

void Party::send_stat_to( Mobile::Character* chr, Mobile::Character* bob ) const
{
  if ( ( chr != bob ) && ( is_member( bob->serial ) ) && ( chr->realm == bob->realm ) &&
       ( pol_distance( chr->x, chr->y, bob->x, bob->y ) < 20 ) )
  {
    if ( !chr->is_visible_to_me( bob ) )
      send_short_statmsg( chr->client, bob );

    send_attributes_normalized( chr, bob );
  }
}

void Party::send_stats_on_add( Mobile::Character* newmember ) const
{
  if ( newmember == nullptr )
    return;
  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* chr = system_find_mobile( serial );
    if ( chr != nullptr )
    {
      if ( newmember != chr )
      {
        if ( chr->has_active_client() )
        {
          send_short_statmsg( chr->client, newmember );
          send_attributes_normalized( chr, newmember );
        }
        if ( newmember->has_active_client() )
        {
          send_short_statmsg( newmember->client, chr );
          send_attributes_normalized( newmember, chr );
        }
      }
    }
  }
}

void Party::on_mana_changed( Mobile::Character* chr ) const
{
  Network::PktHelper::PacketOut<Network::PktOut_A2> msg;
  msg->Write<u32>( chr->serial_ext );

  const auto& vital = chr->vital( networkManager.uoclient_general.mana.id );
  int h = vital.current_ones();
  if ( h > 0xFFFF )
    h = 0xFFFF;
  int mh = vital.maximum_ones();
  if ( mh > 0xFFFF )
    mh = 0xFFFF;
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* mem = system_find_mobile( serial );
    if ( mem != nullptr )
    {
      if ( mem != chr )
      {
        if ( mem->has_active_client() )
          msg.Send( mem->client );
      }
    }
  }
}
void Party::on_stam_changed( Mobile::Character* chr ) const
{
  Network::PktHelper::PacketOut<Network::PktOut_A3> msg;
  msg->Write<u32>( chr->serial_ext );

  const auto& vital = chr->vital( networkManager.uoclient_general.stamina.id );
  int h = vital.current_ones();
  if ( h > 0xFFFF )
    h = 0xFFFF;
  int mh = vital.maximum_ones();
  if ( mh > 0xFFFF )
    mh = 0xFFFF;
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* mem = system_find_mobile( serial );
    if ( mem != nullptr )
    {
      if ( mem != chr )
      {
        if ( mem->has_active_client() )
          msg.Send( mem->client );
      }
    }
  }
}

void Party::send_member_msg_public( Mobile::Character* chr, const std::string& text ) const
{
  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
  msg->offset += 4;  // len+sub
  msg->Write<u8>( PKTBI_BF_06::PARTYCMD_PARTY_MSG );
  msg->Write<u32>( chr->serial_ext );

  std::vector<u16> utf16text = Bscript::String::toUTF16( text );
  if ( settingsManager.party_cfg.Hooks.ChangePublicChat )
  {
    Bscript::BObject obj = settingsManager.party_cfg.Hooks.ChangePublicChat->call_object(
        chr->make_ref(), new Bscript::String( text ) );

    if ( obj->isa( Bscript::BObjectImp::OTString ) || obj->isa( Bscript::BObjectImp::OTArray ) )
    {
      if ( obj->isa( Bscript::BObjectImp::OTArray ) )
        utf16text =
            std::unique_ptr<Bscript::String>(
                Bscript::String::fromUCArray( static_cast<Bscript::ObjArray*>( obj.impptr() ) ) )
                ->toUTF16();
      else
        utf16text = static_cast<Bscript::String*>( obj.impptr() )->toUTF16();
    }
    else if ( obj->isa( Bscript::BObjectImp::OTLong ) )  // break on return(0)
    {
      if ( !obj->isTrue() )
        return;
    }
  }
  if ( utf16text.size() > SPEECH_MAX_LEN )
    utf16text.resize( SPEECH_MAX_LEN );
  msg->WriteFlipped( utf16text, true );
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  for ( const auto& serial : _member_serials )
  {
    Mobile::Character* mem = system_find_mobile( serial );
    if ( mem != nullptr )
    {
      if ( mem->has_active_client() )
        msg.Send( mem->client, len );
    }
  }
}

void Party::send_member_msg_private( Mobile::Character* chr, Mobile::Character* tochr,
                                     const std::string& text ) const
{
  if ( !tochr->has_active_client() )
    return;
  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
  msg->offset += 4;  // len+sub
  msg->Write<u8>( PKTBI_BF_06::PARTYCMD_MEMBER_MSG );
  msg->Write<u32>( chr->serial_ext );

  std::vector<u16> utf16text = Bscript::String::toUTF16( text );
  if ( settingsManager.party_cfg.Hooks.ChangePrivateChat )
  {
    Bscript::BObject obj = settingsManager.party_cfg.Hooks.ChangePrivateChat->call_object(
        chr->make_ref(), tochr->make_ref(), new Bscript::String( text ) );
    if ( obj->isa( Bscript::BObjectImp::OTString ) || obj->isa( Bscript::BObjectImp::OTArray ) )
    {
      if ( obj->isa( Bscript::BObjectImp::OTArray ) )
        utf16text =
            std::unique_ptr<Bscript::String>(
                Bscript::String::fromUCArray( static_cast<Bscript::ObjArray*>( obj.impptr() ) ) )
                ->toUTF16();
      else
        utf16text = static_cast<Bscript::String*>( obj.impptr() )->toUTF16();
    }
    else if ( obj->isa( Bscript::BObjectImp::OTLong ) )  // break on return(0)
    {
      if ( !obj->isTrue() )
        return;
    }
  }
  if ( !settingsManager.party_cfg.General.PrivateMsgPrefix.empty() )
  {
    std::vector<u16> ptext =
        Bscript::String::toUTF16( settingsManager.party_cfg.General.PrivateMsgPrefix );
    utf16text.insert( utf16text.begin(), ptext.begin(), ptext.end() );
  }

  if ( utf16text.size() > SPEECH_MAX_LEN )
    utf16text.resize( SPEECH_MAX_LEN );
  msg->WriteFlipped( utf16text, true );
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( tochr->client, len );
}

void Party::printOn( Clib::StreamWriter& sw ) const
{
  if ( _member_serials.size() <= 1 )
    return;
  if ( system_find_mobile( _leaderserial ) == nullptr )
    return;

  sw() << "Party" << pf_endl << "{" << pf_endl << "\tLeader\t0x" << fmt::hex( _leaderserial )
       << pf_endl;

  for ( const auto& mserial : _member_serials )
  {
    Mobile::Character* mem = system_find_mobile( mserial );
    if ( mem != nullptr )
      sw() << "\tMember\t0x" << fmt::hex( mserial ) << pf_endl;
  }
  _proplist.printProperties( sw );
  sw() << "}" << pf_endl << pf_endl;
  // sw.flush();
}

std::vector<Mobile::Character*> Party::get_members() const
{
  std::vector<Mobile::Character*> ret;

  for ( auto& serial : _member_serials )
  {
    Mobile::Character* chr = Core::find_character( serial );
    if ( chr )
      ret.push_back( chr );
  }
  return ret;
}

size_t Party::estimateSize() const
{
  return 3 * sizeof( u32* ) + _member_serials.capacity() * sizeof( u32 ) + 3 * sizeof( u32* ) +
         _candidates_serials.capacity() * sizeof( u32 ) + 3 * sizeof( u32* ) +
         _offlinemember_serials.capacity() * sizeof( u32 ) + sizeof( u32 ) /*_leaderserial*/
         + _proplist.estimatedSize();
}

void send_empty_party( Mobile::Character* chr )
{
  if ( chr != nullptr && chr->has_active_client() )
  {
    Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
    msg->WriteFlipped<u16>( 11u );
    msg->offset += 2;  // sub
    msg->Write<u8>( PKTBI_BF_06::PARTYCMD_REMOVE );
    msg->offset++;  // nummembers
    msg->Write<u32>( chr->serial_ext );
    msg.Send( chr->client );
  }
}

void register_party_members()
{
  Parties::iterator itr = gamestate.parties.begin();
  while ( itr != gamestate.parties.end() )
  {
    if ( !( *itr )->register_with_members() )
    {
      ( *itr )->disband();
      itr = gamestate.parties.erase( itr );
    }
    else
      ++itr;
  }
}

void disband_party( u32 leader )
{
  auto itr = std::find_if( gamestate.parties.begin(), gamestate.parties.end(),
                           [&]( PartyRef& party ) { return party->is_leader( leader ); } );
  if ( itr != gamestate.parties.end() )
  {
    ( *itr )->disband();
    gamestate.parties.erase( itr );
  }
}

void read_party_dat()
{
  std::string partyfile = Plib::systemstate.config.world_data_path + "parties.txt";

  if ( !Clib::FileExists( partyfile ) )
    return;
  if ( settingsManager.party_cfg.General.RemoveMemberOnLogoff )
    return;

  INFO_PRINT << "  " << partyfile << ":";
  Clib::ConfigFile cf( partyfile );
  Clib::ConfigSection sect_party( cf, "PARTY", Clib::CST_NORMAL );
  Clib::ConfigElem elem;
  static int num_until_dot = 1000;
  clock_t start = clock();
  while ( cf.read( elem ) )
  {
    if ( --num_until_dot == 0 )
    {
      INFO_PRINT << ".";
      num_until_dot = 1000;
    }
    if ( sect_party.matches( elem ) )
      gamestate.parties.push_back( ref_ptr<Party>( new Party( elem ) ) );
  }
  clock_t end = clock();
  int ms = static_cast<int>( ( end - start ) * 1000.0 / CLOCKS_PER_SEC );

  INFO_PRINT << " " << gamestate.parties.size() << " elements in " << ms << " ms.\n";

  register_party_members();
}

void write_party( Clib::StreamWriter& sw )
{
  if ( settingsManager.party_cfg.General.RemoveMemberOnLogoff )
    return;
  for ( const auto& party : gamestate.parties )
  {
    party->printOn( sw );
  }
}

void on_loggoff_party( Mobile::Character* chr )
{
  if ( chr->has_party() )
  {
    if ( settingsManager.party_cfg.General.RemoveMemberOnLogoff )
    {
      Party* party = chr->party();
      if ( party->remove_member( chr->serial ) )
      {
        bool disband = false;
        if ( party->is_leader( chr->serial ) )
        {
          if ( !party->test_size() )
          {
            disband_party( party->leader() );
            chr->party( nullptr );
            return;
          }
          party->set_leader( party->get_member_at( 0 ) );
          party->send_msg_to_all(
              CLP_Player_Removed );  // A player has been removed from your party.
          party->send_member_list( nullptr );
        }
        else
          party->send_remove_member( chr, &disband );
        chr->party( nullptr );
        if ( settingsManager.party_cfg.Hooks.OnLeaveParty )
          settingsManager.party_cfg.Hooks.OnLeaveParty->call( chr->make_ref(), chr->make_ref() );

        if ( disband )
          disband_party( party->leader() );
        else
        {
          if ( settingsManager.party_cfg.General.RejoinPartyOnLogon )
          {
            party->add_offline_mem( chr->serial );
            chr->offline_mem_of( party );
          }
        }
      }
    }
  }
}


void on_loggon_party( Mobile::Character* chr )
{
  if ( chr->has_offline_mem_of() )
  {
    Party* party = chr->offline_mem_of();
    if ( party->remove_offline_mem( chr->serial ) )
    {
      if ( party->add_member( chr->serial ) )
      {
        chr->party( party );
        send_sysmessage_cl( chr->client, CLP_Rejoined );  // You have rejoined the party.
        party->send_member_list( nullptr );
        party->send_stats_on_add( chr );
        party->send_msg_to_all( CLP_Player_Rejoined, chr->name(),
                                chr );  //: rejoined the party.
      }
    }
    chr->offline_mem_of( nullptr );
  }
  else if ( !chr->has_party() )
    send_empty_party( chr );
  else
  {
    Party* party = chr->party();
    send_sysmessage_cl( chr->client, CLP_Rejoined );  // You have rejoined the party.
    party->send_member_list( chr );
    party->send_stats_on_add( chr );
    party->send_msg_to_all( CLP_Player_Rejoined, chr->name(),
                            chr );  //: rejoined the party.
  }
}


void party_cmd_handler( Network::Client* client, PKTBI_BF* msg )
{
  switch ( msg->partydata.partycmd )
  {
  case PKTBI_BF_06::PARTYCMD_ADD:
    handle_add( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_REMOVE:
    handle_remove( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_MEMBER_MSG:
    handle_member_msg( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_PARTY_MSG:
    handle_party_msg( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_LOOT_PERMISSION:
    handle_loot_perm( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_ACCEPT_INVITE:
    handle_accept_invite( client, msg );
    break;
  case PKTBI_BF_06::PARTYCMD_DECLINE_INVITE:
    handle_decline_invite( client, msg );
    break;
  default:
    handle_unknown_packet( client );
  }
}

void handle_add_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
{
  if ( chr->client != nullptr )
  {
    Mobile::Character* add = system_find_mobile( cfBEu32( msgin->selected_serial ) );
    add_candidate( add, chr );
  }
}

void handle_add( Network::Client* client, PKTBI_BF* msg )
{
  if ( msg->partydata.partyadd.memberid == 0x0 )
  {
    send_sysmessage_cl( client, CLP_Who_To_Add );  // Who would you like to add to your party?
    gamestate.target_cursors.add_member_cursor.send_object_cursor( client );
  }
  else
  {
    Mobile::Character* member = system_find_mobile( cfBEu32( msg->partydata.partyadd.memberid ) );
    add_candidate( member, client->chr );
  }
}

void handle_remove_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
{
  if ( chr->client != nullptr )
  {
    Mobile::Character* rem = system_find_mobile( cfBEu32( msgin->selected_serial ) );
    if ( rem != nullptr )
    {
      Party* party = chr->party();
      if ( ( party == nullptr ) || ( !party->is_leader( chr->serial ) ) ||
           ( !party->is_member( rem->serial ) ) )
        return;
      if ( chr == rem )
        send_sysmessage_cl( chr->client, CLP_Cannot_Remove_Self );  // You may only remove yourself
                                                                    // from a party if you are not
                                                                    // the leader.
      else
      {
        if ( settingsManager.party_cfg.Hooks.CanRemoveMember )
        {
          if ( !settingsManager.party_cfg.Hooks.CanRemoveMember->call( chr->make_ref(),
                                                                       rem->make_ref() ) )
          {
            party->send_member_list( chr );  // resend list
            return;
          }
        }
        if ( party->remove_member( rem->serial ) )
        {
          bool disband;
          party->send_remove_member( rem, &disband );
          rem->party( nullptr );
          if ( settingsManager.party_cfg.Hooks.OnLeaveParty )
            settingsManager.party_cfg.Hooks.OnLeaveParty->call( rem->make_ref(), chr->make_ref() );

          send_sysmessage_cl( rem->client, CLP_Removed );  // You have been removed from the party.
          send_empty_party( rem );
          if ( disband )
            disband_party( party->leader() );
        }
      }
    }
  }
}

void handle_remove( Network::Client* client, PKTBI_BF* msg )
{
  Party* party = client->chr->party();
  if ( party == nullptr )
  {
    send_sysmessage_cl( client, CLP_No_Party );  // You are not in a party.
    return;
  }
  Mobile::Character* member = system_find_mobile( cfBEu32( msg->partydata.partyremove.memberid ) );
  // leader wants target
  if ( ( party->is_leader( client->chr->serial ) ) && ( member == nullptr ) )
  {
    send_sysmessage_cl( client,
                        CLP_Who_to_Remove );  // Who would you like to remove from your party?
    gamestate.target_cursors.remove_member_cursor.send_object_cursor( client );
  }
  // leader kicks member, or member kicks himself
  else if ( ( ( party->is_leader( client->chr->serial ) ) || ( client->chr == member ) ) &&
            ( party->is_member( member->serial ) ) )
  {
    if ( client->chr == member )
    {
      if ( settingsManager.party_cfg.Hooks.CanLeaveParty )
      {
        if ( !settingsManager.party_cfg.Hooks.CanLeaveParty->call( client->chr->make_ref() ) )
        {
          party->send_member_list( client->chr );  // resend list
          return;
        }
      }
    }
    else
    {
      if ( settingsManager.party_cfg.Hooks.CanRemoveMember )
      {
        if ( !settingsManager.party_cfg.Hooks.CanRemoveMember->call( client->chr->make_ref(),
                                                                     member->make_ref() ) )
        {
          party->send_member_list( client->chr );  // resend list
          return;
        }
      }
    }
    if ( party->remove_member( member->serial ) )
    {
      bool disband;
      member->party( nullptr );
      party->send_remove_member( member, &disband );
      if ( settingsManager.party_cfg.Hooks.OnLeaveParty )
        settingsManager.party_cfg.Hooks.OnLeaveParty->call( member->make_ref(),
                                                            client->chr->make_ref() );
      if ( member->has_active_client() )
      {
        send_sysmessage_cl( member->client, CLP_Removed );  // You have been removed from the party.
        send_empty_party( member );
      }
      if ( disband )
        disband_party( party->leader() );
    }
  }
}

void handle_member_msg( Network::Client* client, PKTBI_BF* msg )
{
  using std::wcout;
  Mobile::Character* member =
      system_find_mobile( cfBEu32( msg->partydata.partymembermsg.memberid ) );
  if ( member != nullptr )
  {
    Party* party = client->chr->party();

    if ( ( party != nullptr ) && ( member->has_party() ) && ( party->is_member( member->serial ) ) )
    {
      u16* themsg = msg->partydata.partymembermsg.wtext;
      int intextlen =
          ( cfBEu16( msg->msglen ) - 10 ) / sizeof( msg->partydata.partymembermsg.wtext[0] ) - 1;

      if ( intextlen < 0 )
        intextlen = 0;
      if ( intextlen > SPEECH_MAX_LEN )
        intextlen = SPEECH_MAX_LEN;

      std::string text = Bscript::String::fromUTF16( themsg, intextlen, true );
      if ( settingsManager.party_cfg.Hooks.OnPrivateChat )
      {
        settingsManager.party_cfg.Hooks.OnPrivateChat->call(
            client->chr->make_ref(), member->make_ref(), new Bscript::String( text ) );
      }

      party->send_member_msg_private( client->chr, member, text );
    }
    else
      send_sysmessage_cl( client, CLP_No_Party );  // You are not in a party.
  }
}

void handle_party_msg( Network::Client* client, PKTBI_BF* msg )
{
  Party* party = client->chr->party();
  if ( party != nullptr )
  {
    u16* themsg = msg->partydata.partymsg.wtext;
    Mobile::Character* member = nullptr;
    int intextlen = ( cfBEu16( msg->msglen ) - 6 ) / sizeof( msg->partydata.partymsg.wtext[0] ) - 1;

    //  intextlen does not include the null terminator.

    // Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
    if ( intextlen < 0 )
      intextlen = 0;
    if ( intextlen > SPEECH_MAX_LEN )
      intextlen = SPEECH_MAX_LEN;
    std::string text = Bscript::String::fromUTF16( themsg, intextlen, true );

    if ( settingsManager.party_cfg.General.TreatNoAsPrivate )
    {
      if ( text.size() > 2 && ( isdigit( text[0] ) ) && ( text[1] == ' ' ) )
      {
        int no = atoi( &text[0] );
        if ( no == 0 )
          no = 9;
        else
          no--;
        u32 mem = party->get_member_at( (unsigned short)no );
        if ( mem != 0 )
        {
          member = system_find_mobile( mem );
          if ( ( member != nullptr ) && ( member->has_active_client() ) )
            text = text.substr( 2 );
        }
      }
    }

    if ( member != nullptr )  // private chat
    {
      if ( settingsManager.party_cfg.Hooks.OnPrivateChat )
      {
        settingsManager.party_cfg.Hooks.OnPrivateChat->call(
            client->chr->make_ref(), member->make_ref(), new Bscript::String( text ) );
      }
      party->send_member_msg_private( client->chr, member, text );
    }
    else
    {
      if ( settingsManager.party_cfg.Hooks.OnPublicChat )
      {
        settingsManager.party_cfg.Hooks.OnPublicChat->call( client->chr->make_ref(),
                                                            new Bscript::String( text ) );
      }

      party->send_member_msg_public( client->chr, text );
    }
  }
  else
    send_sysmessage_cl( client, CLP_No_Party );  // You are not in a party.
}

void handle_loot_perm( Network::Client* client, PKTBI_BF* msg )
{
  if ( !client->chr->has_party() )
    send_sysmessage_cl( client, CLP_No_Party );  // You are not in a party.
  else
  {
    bool loot = msg->partydata.partylootperm.canloot ? true : false;
    client->chr->set_party_can_loot( loot );
    if ( settingsManager.party_cfg.Hooks.OnLootPermChange )
      settingsManager.party_cfg.Hooks.OnLootPermChange->call( client->chr->make_ref() );
    // You have chosen to allow your party to loot your corpse.
    // You have chosen to prevent your party from looting your corpse.
    send_sysmessage_cl( client, loot ? CLP_Allow_Loot : CLP_Prevent_Loot );
  }
}

void handle_accept_invite( Network::Client* client, PKTBI_BF* msg )
{
  Mobile::Character* leader =
      system_find_mobile( cfBEu32( msg->partydata.partyaccinvite.leaderid ) );
  if ( leader != nullptr )
  {
    Party* party = leader->party();
    if ( party != nullptr )
    {
      if ( party->remove_candidate( client->chr->serial ) )
      {
        client->chr->cancel_party_invite_timeout();
        client->chr->candidate_of( nullptr );
        if ( party->add_member( client->chr->serial ) )
        {
          client->chr->party( party );
          if ( settingsManager.party_cfg.Hooks.OnAddToParty )
            settingsManager.party_cfg.Hooks.OnAddToParty->call( client->chr->make_ref() );
          send_sysmessage_cl( client, CLP_Added );  // You have been added to the party.

          party->send_msg_to_all( CLP_Joined, client->chr->name(),
                                  client->chr );  //  : joined the party.
          party->send_member_list( nullptr );
          party->send_stats_on_add( client->chr );
        }
      }
    }
    else
      send_sysmessage_cl( client, CLP_No_Invite );  // No one has invited you to be in a party.
  }
}

void handle_decline_invite( Network::Client* client, PKTBI_BF* msg )
{
  Mobile::Character* leader =
      system_find_mobile( cfBEu32( msg->partydata.partydecinvite.leaderid ) );
  if ( leader != nullptr )
  {
    Party* party = leader->party();
    if ( party != nullptr )
    {
      if ( party->remove_candidate( client->chr->serial ) )
      {
        client->chr->cancel_party_invite_timeout();
        client->chr->candidate_of( nullptr );
        send_sysmessage_cl(
            client, CLP_Decline );  // You notify them that you do not wish to join the party.
        if ( leader->has_active_client() )
          send_sysmessage_cl_affix( leader->client, CLP_Notify_Decline, client->chr->name(),
                                    true );  //: Does not wish to join the party.
        if ( settingsManager.party_cfg.Hooks.OnDecline )
          settingsManager.party_cfg.Hooks.OnDecline->call( client->chr->make_ref() );
        if ( !party->test_size() )
          disband_party( leader->serial );
      }
      else if ( party->is_member( client->chr->serial ) )
        send_sysmessage_cl( client, CLP_Too_Late_Decline );  // Too late to decline, you are already
                                                             // in the party. Try /quit.
    }
    else
      send_sysmessage_cl( client, CLP_No_Invite );  // No one has invited you to be in a party.
  }
}

void add_candidate( Mobile::Character* member, Mobile::Character* leader )
{
  if ( leader == nullptr )
    return;
  if ( member == nullptr )
  {
    send_sysmessage_cl( leader->client,
                        CLP_Add_Living );  // You may only add living things to your party!
    return;
  }
  Party* party = leader->party();
  if ( member == leader )
    send_sysmessage_cl( leader->client, CLP_Add_Yourself );  // You cannot add yourself to a party.
  else if ( ( party != nullptr ) && ( !party->is_leader( leader->serial ) ) )
    send_sysmessage_cl(
        leader->client,
        CLP_Add_No_Leader );  // You may only add members to the party if you are the leader.
  else if ( ( party != nullptr ) && ( !party->can_add() ) )
    send_sysmessage_cl(
        leader->client,
        CLP_Max_Size );  // You may only have 10 in your party (this includes candidates).
  else if ( member->isa( UOBJ_CLASS::CLASS_NPC ) )
    send_sysmessage_cl( leader->client, CLP_Ignore_Offer );  // The creature ignores your offer.
  else if ( ( party != nullptr ) && ( party->is_member( member->serial ) ) )
    send_sysmessage_cl( leader->client,
                        CLP_Already_Your_Party );  // This person is already in your party!
  else if ( ( party != nullptr ) && ( party->is_candidate( member->serial ) ) )
    send_sysmessage_cl( leader->client,
                        CLP_Already_Your_Party );  // This person is already in your party!
  else if ( member->has_party() )
    send_sysmessage_cl( leader->client,
                        CLP_Already_in_a_Party );  // This person is already in a party!
  else
  {
    if ( member->has_candidate_of() )
      send_sysmessage_cl( leader->client,
                          CLP_Already_in_a_Party );  // This person is already in a party!
    else
    {
      if ( settingsManager.party_cfg.Hooks.CanAddToParty )
      {
        if ( !settingsManager.party_cfg.Hooks.CanAddToParty->call( leader->make_ref(),
                                                                   member->make_ref() ) )
          return;
      }
      if ( settingsManager.party_cfg.General.DeclineTimeout > 0 )
        member->set_party_invite_timeout();

      if ( party == nullptr )
      {
        party = new Party( leader->serial );
        gamestate.parties.push_back( ref_ptr<Party>( party ) );
        leader->party( party );
        if ( settingsManager.party_cfg.Hooks.OnPartyCreate )
          settingsManager.party_cfg.Hooks.OnPartyCreate->call(
              Module::CreatePartyRefObjImp( party ) );
      }
      if ( party->add_candidate( member->serial ) )
      {
        member->candidate_of( party );
        send_invite( member, leader );
      }
      else
        send_sysmessage_cl(
            leader->client,
            CLP_Max_Size );  // You may only have 10 in your party (this includes candidates).
    }
  }
}

void invite_timeout( Mobile::Character* mem )
{
  Party* party = mem->candidate_of();
  if ( party != nullptr )
  {
    if ( party->remove_candidate( mem->serial ) )
    {
      if ( mem->has_active_client() )
        send_sysmessage_cl(
            mem->client, CLP_Decline );  // You notify them that you do not wish to join the party.
      Mobile::Character* leader = system_find_mobile( party->leader() );
      if ( settingsManager.party_cfg.Hooks.OnDecline )
        settingsManager.party_cfg.Hooks.OnDecline->call( mem->make_ref() );
      if ( leader != nullptr )
      {
        if ( leader->has_active_client() )
          send_sysmessage_cl_affix( leader->client, CLP_Notify_Decline, mem->name(),
                                    true );  //: Does not wish to join the party.
        if ( !party->test_size() )
          disband_party( leader->serial );
      }
    }
    mem->candidate_of( nullptr );
  }
}

void send_invite( Mobile::Character* member, Mobile::Character* leader )
{
  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
  msg->WriteFlipped<u16>( 10u );
  msg->offset += 2;  // sub
  msg->Write<u8>( PKTBI_BF_06::PARTYCMD_INVITE_MEMBER );
  msg->Write<u32>( leader->serial_ext );
  msg.Send( member->client );

  // : You are invited to join the party. Type /accept to join or /decline to decline the offer.
  send_sysmessage_cl_affix( member->client, CLP_Invite, leader->name(), true );
  send_sysmessage_cl( leader->client, CLP_Invited );  // You have invited them to join the party.
}

void send_attributes_normalized( Mobile::Character* chr, Mobile::Character* bob )
{
  Network::PktHelper::PacketOut<Network::PktOut_2D> msg;
  msg->Write<u32>( bob->serial_ext );

  const auto& hits = bob->vital( networkManager.uoclient_general.hits.id );
  int h = hits.current_ones();
  if ( h > 0xFFFF )
    h = 0xFFFF;
  int mh = hits.maximum_ones();
  if ( mh > 0xFFFF )
    mh = 0xFFFF;
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

  const auto& mana = bob->vital( networkManager.uoclient_general.mana.id );
  h = mana.current_ones();
  if ( h > 0xFFFF )
    h = 0xFFFF;
  mh = mana.maximum_ones();
  if ( mh > 0xFFFF )
    mh = 0xFFFF;
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

  const auto& stam = bob->vital( networkManager.uoclient_general.stamina.id );
  h = stam.current_ones();
  if ( h > 0xFFFF )
    h = 0xFFFF;
  mh = stam.maximum_ones();
  if ( mh > 0xFFFF )
    mh = 0xFFFF;
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

  msg.Send( chr->client );
}
}  // namespace Core
namespace Mobile
{
bool Character::party_can_loot() const
{
  return mob_flags_.get( MOB_FLAGS::PARTY_CAN_LOOT );
}

void Character::set_party_can_loot( bool b )
{
  mob_flags_.change( MOB_FLAGS::PARTY_CAN_LOOT, b );
}

void Character::set_party_invite_timeout()
{
  if ( this->party_decline_timeout_ != nullptr )
    this->party_decline_timeout_->cancel();
  Core::polclock_t timeout =
      Core::polclock() +
      Core::settingsManager.party_cfg.General.DeclineTimeout * Core::POLCLOCKS_PER_SEC;
  new Core::OneShotTaskInst<Character*>( &this->party_decline_timeout_, timeout,
                                         Core::invite_timeout, this );
}

bool Character::has_party_invite_timeout() const
{
  if ( this->party_decline_timeout_ != nullptr )
    return false;
  return true;
}

void Character::cancel_party_invite_timeout()
{
  if ( this->party_decline_timeout_ != nullptr )
    this->party_decline_timeout_->cancel();
}
}  // namespace Mobile
}  // namespace Pol

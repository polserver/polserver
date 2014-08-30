/*
History
=======

2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of _leaderserial
STLport-5.2.1 fix: UCconv, bytemsg and wtextoffset not used
STLport-5.2.1 fix: "wtext[ SPEECH_MAX_LEN+1 ];" has no effect
STLport-5.2.1 fix: illegal usage of NULL instead of 0
STLport-5.2.1 fix: initialization of member in handle_party_msg()
2009/08/26 Turley:    changed convertUCtoArray() to if check
2009/11/17 Turley:    fixed hang due to illegal pointers

Notes
=======

*/
#include <iostream>
#include "../clib/stl_inc.h"

#include "../bscript/berror.h"
#ifdef MEMORYLEAK
#include "../bscript/bobject.h"
#endif
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cfgsect.h"
#include "../clib/endian.h"
#include "../clib/fileutil.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../clib/unicode.h"
#include "../clib/logfacility.h"
#include "../clib/streamsaver.h"
#include "../plib/realm.h"
#include "clfunc.h"
#include "fnsearch.h"
#include "mobile/charactr.h"
#include "module/partymod.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "party.h"
#include "pktboth.h"
#include "polcfg.h"
#include "schedule.h"
#include "statmsg.h"
#include "syshook.h"
#include "target.h"
namespace Pol {
  namespace Core {
    void handle_unknown_packet( Network::Client* client );

	Parties parties;
	Party_Cfg party_cfg;

	Party::Party( u32 serial ) :
	  _member_serials(),
	  _candidates_serials(),
	  _offlinemember_serials(),
	  _leaderserial( serial )
	{
	  _member_serials.push_back( serial );
	}

	Party::Party( Clib::ConfigElem& elem ) :
	  _member_serials(),
	  _candidates_serials(),
	  _offlinemember_serials(),
	  _leaderserial( 0 )
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
	  party_cfg.General.MaxPartyMembers = elem.remove_ushort( "MaxPartyMembers", 10 );
	  party_cfg.General.TreatNoAsPrivate = elem.remove_bool( "TreatNoAsPrivate", false );
	  party_cfg.General.DeclineTimeout = elem.remove_ushort( "DeclineTimeout", 10 );
	  party_cfg.General.RemoveMemberOnLogoff = elem.remove_bool( "RemoveMemberOnLogoff", false );
	  if ( party_cfg.General.RemoveMemberOnLogoff )
		party_cfg.General.RejoinPartyOnLogon = elem.remove_bool( "RejoinPartyOnLogon", false );
	  else
		party_cfg.General.RejoinPartyOnLogon = false;
	  string tmp = elem.remove_string( "PrivateMsgPrefix", "" );
	  if ( tmp.size() == 0 )
		party_cfg.General.PrivateMsgPrefixLen = 0;
	  else
	  {
        std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
		tmp += " ";
		for ( unsigned i = 0; i < tmp.size(); ++i )
		{
          arr->addElement( new Bscript::BLong( static_cast<unsigned char>( tmp[i] ) ) );
		}
		party_cfg.General.PrivateMsgPrefixLen = (unsigned char)arr->ref_arr.size();
		if ( party_cfg.General.PrivateMsgPrefixLen>SPEECH_MAX_LEN )
		  party_cfg.General.PrivateMsgPrefixLen = SPEECH_MAX_LEN;

        Bscript::ObjArray* arrPtr = arr.get( );
		if ( !Clib::convertArrayToUC( arrPtr, party_cfg.General.PrivateMsgPrefix, party_cfg.General.PrivateMsgPrefixLen, true ) )
		  party_cfg.General.PrivateMsgPrefixLen = 0;
	  }
	}

    void load_party_cfg_hooks( Clib::ConfigElem& elem )
	{
	  string temp;
	  if ( elem.remove_prop( "CanAddToParty", &temp ) )
		party_cfg.Hooks.CanAddToParty = FindExportedFunction( elem, NULL, temp, 2 );
	  if ( elem.remove_prop( "CanRemoveMember", &temp ) )
		party_cfg.Hooks.CanRemoveMember = FindExportedFunction( elem, NULL, temp, 2 );
	  if ( elem.remove_prop( "CanLeaveParty", &temp ) )
		party_cfg.Hooks.CanLeaveParty = FindExportedFunction( elem, NULL, temp, 1 );
	  if ( elem.remove_prop( "OnPublicChat", &temp ) )
		party_cfg.Hooks.OnPublicChat = FindExportedFunction( elem, NULL, temp, 2 );
	  if ( elem.remove_prop( "OnPrivateChat", &temp ) )
		party_cfg.Hooks.OnPrivateChat = FindExportedFunction( elem, NULL, temp, 3 );
	  if ( elem.remove_prop( "OnDisband", &temp ) )
		party_cfg.Hooks.OnDisband = FindExportedFunction( elem, NULL, temp, 1 );
	  if ( elem.remove_prop( "ChangePublicChat", &temp ) )
		party_cfg.Hooks.ChangePublicChat = FindExportedFunction( elem, NULL, temp, 2 );
	  if ( elem.remove_prop( "ChangePrivateChat", &temp ) )
		party_cfg.Hooks.ChangePrivateChat = FindExportedFunction( elem, NULL, temp, 3 );
	  if ( elem.remove_prop( "OnLeaveParty", &temp ) )
		party_cfg.Hooks.OnLeaveParty = FindExportedFunction( elem, NULL, temp, 2 );
	  if ( elem.remove_prop( "OnAddToParty", &temp ) )
		party_cfg.Hooks.OnAddToParty = FindExportedFunction( elem, NULL, temp, 1 );
	  if ( elem.remove_prop( "OnPartyCreate", &temp ) )
		party_cfg.Hooks.OnPartyCreate = FindExportedFunction( elem, NULL, temp, 1 );
	  if ( elem.remove_prop( "OnDecline", &temp ) )
		party_cfg.Hooks.OnDecline = FindExportedFunction( elem, NULL, temp, 1 );
	  if ( elem.remove_prop( "OnLootPermChange", &temp ) )
		party_cfg.Hooks.OnLootPermChange = FindExportedFunction( elem, NULL, temp, 1 );
	}
	void load_party_cfg( bool reload )
	{
	  if ( !Clib::FileExists( "config/party.cfg" ) )
	  {
		party_cfg.General.MaxPartyMembers = 10;
		party_cfg.General.TreatNoAsPrivate = false;
		party_cfg.General.DeclineTimeout = 10;
		party_cfg.General.PrivateMsgPrefixLen = 0;
		party_cfg.General.RemoveMemberOnLogoff = false;
		party_cfg.General.RejoinPartyOnLogon = false;
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
	  if ( party_cfg.Hooks.CanAddToParty != NULL )
	  {
		delete party_cfg.Hooks.CanAddToParty;
		party_cfg.Hooks.CanAddToParty = NULL;
	  }
	  if ( party_cfg.Hooks.CanRemoveMember != NULL )
	  {
		delete party_cfg.Hooks.CanRemoveMember;
		party_cfg.Hooks.CanRemoveMember = NULL;
	  }
	  if ( party_cfg.Hooks.CanLeaveParty != NULL )
	  {
		delete party_cfg.Hooks.CanLeaveParty;
		party_cfg.Hooks.CanLeaveParty = NULL;
	  }
	  if ( party_cfg.Hooks.OnPublicChat != NULL )
	  {
		delete party_cfg.Hooks.OnPublicChat;
		party_cfg.Hooks.OnPublicChat = NULL;
	  }
	  if ( party_cfg.Hooks.OnPrivateChat != NULL )
	  {
		delete party_cfg.Hooks.OnPrivateChat;
		party_cfg.Hooks.OnPrivateChat = NULL;
	  }
	  if ( party_cfg.Hooks.OnDisband != NULL )
	  {
		delete party_cfg.Hooks.OnDisband;
		party_cfg.Hooks.OnDisband = NULL;
	  }
	  if ( party_cfg.Hooks.ChangePublicChat != NULL )
	  {
		delete party_cfg.Hooks.ChangePublicChat;
		party_cfg.Hooks.ChangePublicChat = NULL;
	  }
	  if ( party_cfg.Hooks.ChangePrivateChat != NULL )
	  {
		delete party_cfg.Hooks.ChangePrivateChat;
		party_cfg.Hooks.ChangePrivateChat = NULL;
	  }
	  if ( party_cfg.Hooks.OnLeaveParty != NULL )
	  {
		delete party_cfg.Hooks.OnLeaveParty;
		party_cfg.Hooks.OnLeaveParty = NULL;
	  }
	  if ( party_cfg.Hooks.OnAddToParty != NULL )
	  {
		delete party_cfg.Hooks.OnAddToParty;
		party_cfg.Hooks.OnAddToParty = NULL;
	  }
	  if ( party_cfg.Hooks.OnPartyCreate != NULL )
	  {
		delete party_cfg.Hooks.OnPartyCreate;
		party_cfg.Hooks.OnPartyCreate = NULL;
	  }
	  if ( party_cfg.Hooks.OnDecline != NULL )
	  {
		delete party_cfg.Hooks.OnDecline;
		party_cfg.Hooks.OnDecline = NULL;
	  }
	  if ( party_cfg.Hooks.OnLootPermChange != NULL )
	  {
		delete party_cfg.Hooks.OnLootPermChange;
		party_cfg.Hooks.OnLootPermChange = NULL;
	  }
	}
	void unload_party()
	{
	  unload_party_hooks();

	  for ( auto &party : parties )
	  {
		party->cleanup();
	  }
	  parties.clear();
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
      return std::find( _member_serials.begin(), _member_serials.end(), serial ) != _member_serials.end();
	}

    bool Party::is_candidate( u32 serial ) const
	{
      return std::find( _candidates_serials.begin(), _candidates_serials.end(), serial ) != _candidates_serials.end();
	}

	bool Party::register_with_members()
	{
	  vector<u32>::iterator itr = _member_serials.begin();
	  while ( itr != _member_serials.end() )
	  {
		Mobile::Character* chr = system_find_mobile( *itr );
		if ( chr != NULL )
		{
		  chr->party( this );
		  ++itr;
		}
		else
		  itr = _member_serials.erase( itr );
	  }
	  if ( _member_serials.empty() )
		return( false );
	  if ( !test_size() )
		return( false );
	  if ( system_find_mobile( _leaderserial ) == NULL )
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
	  if ( ( _member_serials.size() + _candidates_serials.size() ) >= party_cfg.General.MaxPartyMembers )
		return false;
	  _candidates_serials.push_back( serial );
	  return true;
	}

	bool Party::add_member( u32 serial )
	{
	  if ( ( _member_serials.size() + _candidates_serials.size() ) >= party_cfg.General.MaxPartyMembers )
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
      auto itr = std::find_if( _candidates_serials.begin(), _candidates_serials.end(), [&]( u32 &i ) { return i == serial; } );
      if ( itr != _candidates_serials.end() )
        {
		  _candidates_serials.erase( itr );
		  return true;
		}
	  return false;
	}

	bool Party::remove_member( u32 serial )
	{
      auto itr = std::find_if( _member_serials.begin(), _member_serials.end(), [&]( u32 &i ) { return i == serial; } );
      if ( itr != _member_serials.end() )
		{
		  _member_serials.erase( itr );
		  return true;
		}
	  return false;
	}

	bool Party::remove_offline_mem( u32 serial )
	{
      auto itr = std::find_if( _offlinemember_serials.begin(), _offlinemember_serials.end(), [&]( u32 &i ) { return i == serial; } );
      if ( itr != _offlinemember_serials.end() )
	  {
		_offlinemember_serials.erase( itr );
		return true;
	  }
	  return false;
	}

	void Party::set_leader( u32 serial )
	{
      auto itr = std::find_if( _member_serials.begin(), _member_serials.end(), [&]( u32 &i ) { return i == serial; } );
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
	  if ( ( _member_serials.size() + _candidates_serials.size() ) >= party_cfg.General.MaxPartyMembers )
		return false;
	  return true;
	}

	void Party::send_member_list( Mobile::Character* to_chr )
	{
      Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
	  msg->offset += 4; //len+sub
	  msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_ADD ) );
	  msg->offset++; //nummembers
	  vector<u32>::iterator itr = _member_serials.begin();
	  while ( itr != _member_serials.end() )
	  {
		Mobile::Character* chr = system_find_mobile( *itr );
		if ( chr != NULL )
		{
		  msg->Write<u32>( chr->serial_ext );
		  ++itr;
		}
		else
		  itr = _member_serials.erase( itr );
	  }
	  u16 len = msg->offset;
	  msg->offset = 6;
	  msg->Write<u8>( static_cast<u8>( _member_serials.size() ) );
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );

	  if ( to_chr == NULL )
	  {
		for ( const auto& serial : _member_serials)
		{
          Mobile::Character* chr = system_find_mobile( serial );
		  if ( chr != NULL )
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
	  if ( party_cfg.Hooks.OnDisband )
		party_cfg.Hooks.OnDisband->call( Module::CreatePartyRefObjImp( this ) );

      for ( const auto& serial : _member_serials )
	  {
        Mobile::Character* chr = system_find_mobile( serial );
		if ( chr != NULL )
		{
		  chr->party( NULL );
		  if ( chr->has_active_client() )
		  {
			send_empty_party( chr );
			send_sysmessage_cl( chr->client, CLP_Disbanded ); // Your party has disbanded.
		  }
		}
	  }
      for ( const auto& serial : _candidates_serials )
	  {
        Mobile::Character* chr = system_find_mobile( serial );
		if ( chr != NULL )
		  chr->candidate_of( NULL );
	  }
      for ( const auto& serial : _offlinemember_serials )
	  {
        Mobile::Character* chr = system_find_mobile( serial );
		if ( chr != NULL )
		  chr->offline_mem_of( NULL );
	  }

	  _member_serials.clear();
	  _candidates_serials.clear();
	  _offlinemember_serials.clear();
	}

    void Party::send_remove_member( Mobile::Character* remchr, bool *disband )
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
		msg->offset += 4; //len+sub
		msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_REMOVE ) );
		msg->offset++; // nummembers
		msg->Write<u32>( remchr->serial_ext );

		vector<u32>::iterator itr = _member_serials.begin();
		while ( itr != _member_serials.end() )
		{
		  Mobile::Character* chr = system_find_mobile( *itr );
		  if ( chr != NULL )
		  {
			msg->Write<u32>( chr->serial_ext );
			++itr;
		  }
		  else
			itr = _member_serials.erase( itr );
		}
		u16 len = msg->offset;
		msg->offset = 6;
		msg->Write<u8>( static_cast<u8>( _member_serials.size() ) );
		msg->offset = 1;
		msg->WriteFlipped<u16>( len );

        for ( const auto& serial : _member_serials )
		{
          Mobile::Character* chr = system_find_mobile( serial );
		  if ( chr != NULL )
		  {
			if ( chr->has_active_client() )
			  msg.Send( chr->client, len );
		  }
		}
		send_msg_to_all( CLP_Player_Removed );//A player has been removed from your party.
		if ( !test_size() )
		{
		  send_msg_to_all( CLP_Last_Person ); //The last person has left the party...
		  *disband = true;
		}
	  }
	}

    void Party::send_msg_to_all( unsigned int clilocnr, const char* affix, Mobile::Character* exeptchr ) const
	{
      Network::PktHelper::PacketOut<Network::PktOut_C1> msgc1;
      Network::PktHelper::PacketOut<Network::PktOut_CC> msgcc;
	  if ( affix != NULL )
		build_sysmessage_cl_affix( msgcc.Get(), clilocnr, affix, true );
	  else
		build_sysmessage_cl( msgc1.Get(), clilocnr );

      for ( const auto& serial : _member_serials )
	  {
        Mobile::Character* chr = system_find_mobile( serial );
		if ( chr != NULL )
		{
		  if ( chr != exeptchr )
		  {
			if ( chr->has_active_client() )
			{
			  if ( affix != NULL )
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
	  if ( ( chr != bob ) && ( is_member( bob->serial ) ) && ( chr->realm == bob->realm )
		   && ( pol_distance( chr->x, chr->y, bob->x, bob->y ) < 20 ) )
	  {
		if ( !chr->is_visible_to_me( bob ) )
		  send_short_statmsg( chr->client, bob );

		send_attributes_normalized( chr, bob );
	  }
	}

    void Party::send_stats_on_add( Mobile::Character* newmember ) const
	{
	  if ( newmember == NULL ) return;
      for ( const auto& serial : _member_serials )
	  {
		Mobile::Character* chr = system_find_mobile( serial );
		if ( chr != NULL )
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

	  int h, mh;
	  h = chr->vital( uoclient_general.mana.id ).current_ones();
	  if ( h > 0xFFFF )
		h = 0xFFFF;
	  mh = chr->vital( uoclient_general.mana.id ).maximum_ones();
	  if ( mh > 0xFFFF )
		mh = 0xFFFF;
	  msg->WriteFlipped<u16>( static_cast<u16>( 1000 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

      for ( const auto& serial : _member_serials )
	  {
		Mobile::Character* mem = system_find_mobile( serial );
		if ( mem != NULL )
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

	  int h, mh;
	  h = chr->vital( uoclient_general.mana.id ).current_ones();
	  if ( h > 0xFFFF )
		h = 0xFFFF;
	  mh = chr->vital( uoclient_general.mana.id ).maximum_ones();
	  if ( mh > 0xFFFF )
		mh = 0xFFFF;
	  msg->WriteFlipped<u16>( static_cast<u16>( 1000 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

      for ( const auto& serial : _member_serials )
	  {
        Mobile::Character* mem = system_find_mobile( serial );
		if ( mem != NULL )
		{
		  if ( mem != chr )
		  {
			if ( mem->has_active_client() )
			  msg.Send( mem->client );
		  }
		}
	  }
	}

    void Party::send_member_msg_public( Mobile::Character* chr, u16* wtext, size_t wtextlen ) const
	{
      Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
	  msg->offset += 4; //len+sub
	  msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_PARTY_MSG ) );
	  msg->Write<u32>( chr->serial_ext );

	  if ( party_cfg.Hooks.ChangePublicChat )
	  {
		Bscript::ObjArray* arr;
		if ( !Clib::convertUCtoArray( wtext, arr, static_cast<unsigned int>( wtextlen ), true ) ) // convert back with ctBEu16()
		  return;
        Bscript::BObject obj = party_cfg.Hooks.ChangePublicChat->call_object( chr->make_ref( ), arr );
        if ( obj->isa( Bscript::BObjectImp::OTArray ) )
		{
          arr = static_cast<Bscript::ObjArray*>( obj.impptr( ) );
		  unsigned len = static_cast<unsigned int>( arr->ref_arr.size() );
		  if ( len > SPEECH_MAX_LEN )
			len = SPEECH_MAX_LEN;
		  if ( !Clib::convertArrayToUC( arr, wtext, len, true ) )
			return;
		  wtextlen = len + 1;
		}
        else if ( obj->isa( Bscript::BObjectImp::OTLong ) ) // break on return(0)
		{
		  if ( !obj->isTrue() )
			return;
		}
	  }
	  msg->Write( &wtext[0], static_cast<u16>( wtextlen ), false );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );

      for ( const auto& serial : _member_serials )
	  {
        Mobile::Character* mem = system_find_mobile( serial );
		if ( mem != NULL )
		{
		  if ( mem->has_active_client() )
			msg.Send( mem->client, len );
		}
	  }
	}

    void Party::send_member_msg_private( Mobile::Character* chr, Mobile::Character* tochr, u16* wtext, size_t wtextlen ) const
	{
	  if ( !tochr->has_active_client() )
		return;
      Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
	  msg->offset += 4; //len+sub
	  msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_MEMBER_MSG ) );
	  msg->Write<u32>( chr->serial_ext );

	  if ( party_cfg.Hooks.ChangePrivateChat )
	  {
        Bscript::ObjArray* arr;
		if ( !Clib::convertUCtoArray( wtext, arr, static_cast<unsigned int>( wtextlen ), true ) ) // convert back with ctBEu16()
		  return;
        Bscript::BObject obj = party_cfg.Hooks.ChangePrivateChat->call_object( chr->make_ref( ), tochr->make_ref( ), arr );
        if ( obj->isa( Bscript::BObjectImp::OTArray ) )
		{
          arr = static_cast<Bscript::ObjArray*>( obj.impptr( ) );
		  unsigned len = static_cast<unsigned int>( arr->ref_arr.size() );
		  if ( len > SPEECH_MAX_LEN )
			len = SPEECH_MAX_LEN;
		  if ( !Clib::convertArrayToUC( arr, wtext, len, true ) )
			return;
		  wtextlen = len + 1;
		}
        else if ( obj->isa( Bscript::BObjectImp::OTLong ) ) // break on return(0)
		{
		  if ( !obj->isTrue() )
			return;
		}
	  }
	  if ( ( wtextlen + party_cfg.General.PrivateMsgPrefixLen ) > SPEECH_MAX_LEN )
		wtextlen = SPEECH_MAX_LEN - party_cfg.General.PrivateMsgPrefixLen;
	  if ( party_cfg.General.PrivateMsgPrefixLen )
		msg->Write( &party_cfg.General.PrivateMsgPrefix[0], party_cfg.General.PrivateMsgPrefixLen, false );

	  msg->Write( &wtext[0], static_cast<u16>( wtextlen ), false );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );

	  msg.Send( tochr->client, len );
	}

	void Party::printOn( Clib::StreamWriter& sw ) const
	{
	  if ( _member_serials.size() <= 1 )
		return;
	  if ( system_find_mobile( _leaderserial ) == NULL )
		return;

	  sw() << "Party" << pf_endl
		<< "{" << pf_endl
		<< "\tLeader\t0x" << fmt::hex( _leaderserial ) << pf_endl;

	  for ( const auto &mserial : _member_serials )
	  {
        Mobile::Character* mem = system_find_mobile( mserial );
		if ( mem != NULL )
		  sw() << "\tMember\t0x" << fmt::hex( mserial ) << pf_endl;
	  }
	  _proplist.printProperties( sw );
	  sw() << "}" << pf_endl
		<< pf_endl;
	  //sw.flush();
	}

    void send_empty_party( Mobile::Character* chr )
	{
	  if ( chr != NULL && chr->has_active_client() )
	  {
        Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
		msg->WriteFlipped<u16>( static_cast<u16>( 11 ) );
		msg->offset += 2; //sub
		msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_REMOVE ) );
		msg->offset++; //nummembers
		msg->Write<u32>( chr->serial_ext );
		msg.Send( chr->client );
	  }
	}

	void register_party_members()
	{
	  Parties::iterator itr = parties.begin();
	  while ( itr != parties.end() )
	  {
		if ( !( *itr )->register_with_members() )
		{
		  ( *itr )->disband();
		  itr = parties.erase( itr );
		}
		else
		  ++itr;
	  }
	}

	void disband_party( u32 leader )
	{
      auto itr = std::find_if( parties.begin(), parties.end(), [&]( PartyRef &party ) { return party->is_leader( leader ); } );
	  if (itr != parties.end() )
	  {
		( *itr )->disband();
		parties.erase( itr );
	  }
	}

	void read_party_dat()
	{
	  string partyfile = config.world_data_path + "parties.txt";

	  if ( !Clib::FileExists( partyfile ) )
		return;
	  if ( party_cfg.General.RemoveMemberOnLogoff )
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
		  parties.push_back( ref_ptr<Party>( new Party( elem ) ) );
	  }
	  clock_t end = clock();
	  int ms = static_cast<int>( ( end - start ) * 1000.0 / CLOCKS_PER_SEC );

      INFO_PRINT << " " << parties.size() << " elements in " << ms << " ms.\n";

	  register_party_members();
	}

	void write_party( Clib::StreamWriter& sw )
	{
	  if ( party_cfg.General.RemoveMemberOnLogoff )
		return;
	  for ( const auto &party : parties )
	  {
		party->printOn( sw );
	  }
	}

	void on_loggoff_party( Mobile::Character* chr )
	{
	  if ( chr->party() != NULL )
	  {
		if ( party_cfg.General.RemoveMemberOnLogoff )
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
                chr->party( NULL );
				return;
			  }
			  party->set_leader( party->get_member_at( 0 ) );
			  party->send_msg_to_all( CLP_Player_Removed );//A player has been removed from your party.
			  party->send_member_list( NULL );
			}
			else
			  party->send_remove_member( chr, &disband );
			chr->party( NULL );
			if ( party_cfg.Hooks.OnLeaveParty )
			  party_cfg.Hooks.OnLeaveParty->call( chr->make_ref(), chr->make_ref() );

			if ( disband )
			  disband_party( party->leader() );
			else
			{
			  if ( party_cfg.General.RejoinPartyOnLogon )
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
	  if ( chr->offline_mem_of() != NULL )
	  {
		Party* party = chr->offline_mem_of();
		if ( party->remove_offline_mem( chr->serial ) )
		{
		  if ( party->add_member( chr->serial ) )
		  {
			chr->party( party );
			send_sysmessage_cl( chr->client, CLP_Rejoined ); //You have rejoined the party.
			party->send_member_list( NULL );
			party->send_stats_on_add( chr );
			party->send_msg_to_all( CLP_Player_Rejoined, chr->name().c_str(), chr ); //: rejoined the party.
		  }
		}
		chr->offline_mem_of( NULL );
	  }
	  else if ( chr->party() == NULL )
		send_empty_party( chr );
	  else
	  {
		send_sysmessage_cl( chr->client, CLP_Rejoined ); //You have rejoined the party.
		chr->party()->send_member_list( chr );
		chr->party()->send_stats_on_add( chr );
		chr->party()->send_msg_to_all( CLP_Player_Rejoined, chr->name().c_str(), chr ); //: rejoined the party.
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
	FullMsgTargetCursor add_member_cursor( handle_add_member_cursor );
	void handle_add_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
	{
	  if ( chr->client != NULL )
	  {
		Mobile::Character* add = system_find_mobile( cfBEu32( msgin->selected_serial ) );
		add_candidate( add, chr );
	  }
	}

	void handle_add( Network::Client* client, PKTBI_BF* msg )
	{
	  if ( msg->partydata.partyadd.memberid == 0x0 )
	  {
		send_sysmessage_cl( client, CLP_Who_To_Add ); //Who would you like to add to your party?
		add_member_cursor.send_object_cursor( client );
	  }
	  else
	  {
		Mobile::Character* member = system_find_mobile( cfBEu32( msg->partydata.partyadd.memberid ) );
		add_candidate( member, client->chr );
	  }
	}
	FullMsgTargetCursor remove_member_cursor( handle_remove_member_cursor );
	void handle_remove_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
	{
	  if ( chr->client != NULL )
	  {
		Mobile::Character* rem = system_find_mobile( cfBEu32( msgin->selected_serial ) );
		if ( rem != NULL )
		{
		  Party* party = chr->party();
		  if ( ( party == NULL ) || ( !party->is_leader( chr->serial ) ) || ( !party->is_member( rem->serial ) ) )
			return;
		  if ( chr == rem )
			send_sysmessage_cl( chr->client, CLP_Cannot_Remove_Self ); //You may only remove yourself from a party if you are not the leader.
		  else
		  {
			if ( party_cfg.Hooks.CanRemoveMember )
			{
			  if ( !party_cfg.Hooks.CanRemoveMember->call( chr->make_ref(), rem->make_ref() ) )
			  {
				party->send_member_list( chr ); //resend list
				return;
			  }
			}
			if ( party->remove_member( rem->serial ) )
			{
			  bool disband;
			  party->send_remove_member( rem, &disband );
			  rem->party( NULL );
			  if ( party_cfg.Hooks.OnLeaveParty )
				party_cfg.Hooks.OnLeaveParty->call( rem->make_ref(), chr->make_ref() );

			  send_sysmessage_cl( rem->client, CLP_Removed ); //You have been removed from the party.
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
	  if ( party == NULL )
	  {
		send_sysmessage_cl( client, CLP_No_Party ); //You are not in a party.
		return;
	  }
	  Mobile::Character* member = system_find_mobile( cfBEu32( msg->partydata.partyremove.memberid ) );
	  // leader wants target
	  if ( ( party->is_leader( client->chr->serial ) ) && ( member == NULL ) )
	  {
		send_sysmessage_cl( client, CLP_Who_to_Remove ); //Who would you like to remove from your party?
		remove_member_cursor.send_object_cursor( client );
	  }
	  // leader kicks member, or member kicks himself
	  else if ( ( ( party->is_leader( client->chr->serial ) ) || ( client->chr == member ) ) && ( party->is_member( member->serial ) ) )
	  {
		if ( client->chr == member )
		{
		  if ( party_cfg.Hooks.CanLeaveParty )
		  {
			if ( !party_cfg.Hooks.CanLeaveParty->call( client->chr->make_ref() ) )
			{
			  party->send_member_list( client->chr ); //resend list
			  return;
			}
		  }
		}
		else
		{
		  if ( party_cfg.Hooks.CanRemoveMember )
		  {
			if ( !party_cfg.Hooks.CanRemoveMember->call( client->chr->make_ref(), member->make_ref() ) )
			{
			  party->send_member_list( client->chr ); //resend list
			  return;
			}
		  }
		}
		if ( party->remove_member( member->serial ) )
		{
		  bool disband;
		  member->party( NULL );
		  party->send_remove_member( member, &disband );
		  if ( party_cfg.Hooks.OnLeaveParty )
			party_cfg.Hooks.OnLeaveParty->call( member->make_ref(), client->chr->make_ref() );
		  if ( member->has_active_client() )
		  {
			send_sysmessage_cl( member->client, CLP_Removed ); //You have been removed from the party.
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
	  Mobile::Character* member = system_find_mobile( cfBEu32( msg->partydata.partymembermsg.memberid ) );
	  if ( member != NULL )
	  {
		Party* party = client->chr->party();

		if ( ( party != NULL ) && ( member->party() != NULL ) && ( party->is_member( member->serial ) ) )
		{
		  int intextlen;
		  u16 * themsg = msg->partydata.partymembermsg.wtext;
		  //u8 *  bytemsg = ((u8 *) themsg);
		  //int wtextoffset = 0;
		  int i;

		  u16 wtextbuf[SPEECH_MAX_LEN + 1];
		  size_t wtextbuflen;

		  intextlen = ( cfBEu16( msg->msglen ) - 10 )
			/ sizeof( msg->partydata.partymembermsg.wtext[0] ) - 1;

		  //	intextlen does not include the null terminator.

		  // Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
		  if ( intextlen < 0 )
			intextlen = 0;
		  if ( intextlen > SPEECH_MAX_LEN )
			intextlen = SPEECH_MAX_LEN;

		  wtextbuflen = 0;
		  for ( i = 0; i < intextlen; i++ )
		  {
			u16 wc = cfBEu16( themsg[i] );
			if ( wc == 0 ) break;		// quit early on embedded nulls
			if ( wc == L'~' ) continue;	// skip unprintable tildes. 
			wtextbuf[wtextbuflen++] = ctBEu16( wc );
		  }
		  wtextbuf[wtextbuflen++] = (u16)0;

		  if ( party_cfg.Hooks.OnPrivateChat )
		  {
            Bscript::ObjArray* arr;
            if ( Clib::convertUCtoArray( wtextbuf, arr, wtextbuflen, true ) ) // convert back with ctBEu16()
			  party_cfg.Hooks.OnPrivateChat->call( client->chr->make_ref(), member->make_ref(), arr );
		  }

		  party->send_member_msg_private( client->chr, member, wtextbuf, wtextbuflen );
		}
		else
		  send_sysmessage_cl( client, CLP_No_Party );//You are not in a party.
	  }
	}

	void handle_party_msg( Network::Client* client, PKTBI_BF* msg )
	{
	  using std::wcout; // wcout.narrow() function r0x! :-)
	  if ( client->chr->party() != NULL )
	  {
		int intextlen;
		u16 * themsg = msg->partydata.partymsg.wtext;
		//u8 *  bytemsg = ((u8 *) themsg);
		//int wtextoffset = 0;
		int i, starti;
		Mobile::Character* member = NULL;

		u16 wtextbuf[SPEECH_MAX_LEN + 1];
		size_t wtextbuflen;

		intextlen = ( cfBEu16( msg->msglen ) - 6 )
		  / sizeof( msg->partydata.partymsg.wtext[0] ) - 1;

		//	intextlen does not include the null terminator.

		// Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
		if ( intextlen < 0 )
		  intextlen = 0;
		if ( intextlen > SPEECH_MAX_LEN )
		  intextlen = SPEECH_MAX_LEN;

		wtextbuflen = 0;
		starti = 0;
		if ( party_cfg.General.TreatNoAsPrivate )
		{
		  char no_c = wcout.narrow( (wchar_t)cfBEu16( themsg[0] ), '?' );
		  if ( ( isdigit( no_c ) ) && ( cfBEu16( themsg[1] ) == L' ' ) )
		  {
			int no = atoi( &no_c );
			if ( no == 0 )
			  no = 9;
			else
			  no--;
			u32 mem = client->chr->party()->get_member_at( (unsigned short)no );
			if ( mem != 0 )
			{
			  member = system_find_mobile( mem );
			  if ( ( member != NULL ) && ( member->has_active_client() ) )
				starti = 2;
			}
		  }
		}

		for ( i = starti; i < intextlen; i++ )
		{
		  u16 wc = cfBEu16( themsg[i] );
		  if ( wc == 0 ) break;		// quit early on embedded nulls
		  if ( wc == L'~' ) continue;	// skip unprintable tildes. 
		  wtextbuf[wtextbuflen++] = ctBEu16( wc );
		}
		wtextbuf[wtextbuflen++] = (u16)0;

		if ( starti == 2 ) //private chat
		{
		  if ( party_cfg.Hooks.OnPrivateChat )
		  {
			Bscript::ObjArray* arr;
            if ( Clib::convertUCtoArray( wtextbuf, arr, wtextbuflen, true ) ) // convert back with ctBEu16()
			  party_cfg.Hooks.OnPrivateChat->call( client->chr->make_ref(), member->make_ref(), arr );
		  }
		  client->chr->party()->send_member_msg_private( client->chr, member, wtextbuf, wtextbuflen );
		}
		else
		{
		  if ( party_cfg.Hooks.OnPublicChat )
		  {
			Bscript::ObjArray* arr;
            if ( Clib::convertUCtoArray( wtextbuf, arr, wtextbuflen, true ) ) // convert back with ctBEu16()
			  party_cfg.Hooks.OnPublicChat->call( client->chr->make_ref(), arr );
		  }

		  client->chr->party()->send_member_msg_public( client->chr, wtextbuf, wtextbuflen );
		}
	  }
	  else
		send_sysmessage_cl( client, CLP_No_Party );//You are not in a party.
	}

	void handle_loot_perm( Network::Client* client, PKTBI_BF* msg )
	{
	  if ( client->chr->party() == NULL )
		send_sysmessage_cl( client, CLP_No_Party ); //You are not in a party.
	  else
	  {
		bool loot = msg->partydata.partylootperm.canloot ? true : false;
		client->chr->set_party_can_loot( loot );
		if ( party_cfg.Hooks.OnLootPermChange )
		  party_cfg.Hooks.OnLootPermChange->call( client->chr->make_ref() );
		//You have chosen to allow your party to loot your corpse.
		//You have chosen to prevent your party from looting your corpse.
		send_sysmessage_cl( client, loot ? CLP_Allow_Loot : CLP_Prevent_Loot );
	  }
	}

	void handle_accept_invite( Network::Client* client, PKTBI_BF* msg )
	{
	  Mobile::Character* leader = system_find_mobile( cfBEu32( msg->partydata.partyaccinvite.leaderid ) );
	  if ( leader != NULL )
	  {
		if ( leader->party() != NULL )
		{
		  Party* party = leader->party();
		  if ( party->remove_candidate( client->chr->serial ) )
		  {
			client->chr->cancel_party_invite_timeout();
			client->chr->candidate_of( NULL );
			if ( party->add_member( client->chr->serial ) )
			{
			  client->chr->party( party );
			  if ( party_cfg.Hooks.OnAddToParty )
				party_cfg.Hooks.OnAddToParty->call( client->chr->make_ref() );
			  send_sysmessage_cl( client, CLP_Added ); // You have been added to the party.

			  party->send_msg_to_all( CLP_Joined, client->chr->name().c_str(), client->chr );//  : joined the party.
			  party->send_member_list( NULL );
			  party->send_stats_on_add( client->chr );
			}
		  }
		}
		else
		  send_sysmessage_cl( client, CLP_No_Invite ); //No one has invited you to be in a party.
	  }
	}

	void handle_decline_invite( Network::Client* client, PKTBI_BF* msg )
	{
	  Mobile::Character* leader = system_find_mobile( cfBEu32( msg->partydata.partydecinvite.leaderid ) );
	  if ( leader != NULL )
	  {
		if ( leader->party() != NULL )
		{
		  Party* party = leader->party();
		  if ( party->remove_candidate( client->chr->serial ) )
		  {
			client->chr->cancel_party_invite_timeout();
			client->chr->candidate_of( NULL );
			send_sysmessage_cl( client, CLP_Decline ); // You notify them that you do not wish to join the party.
			if ( leader->has_active_client() )
			  send_sysmessage_cl_affix( leader->client, CLP_Notify_Decline, client->chr->name().c_str(), true ); //: Does not wish to join the party.
			if ( party_cfg.Hooks.OnDecline )
			  party_cfg.Hooks.OnDecline->call( client->chr->make_ref() );
			if ( !party->test_size() )
			  disband_party( leader->serial );
		  }
		  else if ( party->is_member( client->chr->serial ) )
			send_sysmessage_cl( client, CLP_Too_Late_Decline ); // Too late to decline, you are already in the party. Try /quit.
		}
		else
		  send_sysmessage_cl( client, CLP_No_Invite ); //No one has invited you to be in a party.
	  }
	}

	void add_candidate( Mobile::Character* member, Mobile::Character* leader )
	{
	  if (leader == nullptr)
		return;
	  if ( member == NULL )
		send_sysmessage_cl( leader->client, CLP_Add_Living ); //You may only add living things to your party!
	  else if ( member == leader )
		send_sysmessage_cl( leader->client, CLP_Add_Yourself ); //You cannot add yourself to a party.
	  else if ( ( leader->party() != NULL ) && ( !leader->party()->is_leader( leader->serial ) ) )
		send_sysmessage_cl( leader->client, CLP_Add_No_Leader ); //You may only add members to the party if you are the leader.
	  else if ( ( leader->party() != NULL ) && ( !leader->party()->can_add() ) )
		send_sysmessage_cl( leader->client, CLP_Max_Size ); //You may only have 10 in your party (this includes candidates).
	  else if ( member->isa( UObject::CLASS_NPC ) )
		send_sysmessage_cl( leader->client, CLP_Ignore_Offer ); //The creature ignores your offer.
	  else if ( ( leader->party() != NULL ) && ( leader->party()->is_member( member->serial ) ) )
		send_sysmessage_cl( leader->client, CLP_Already_Your_Party ); //This person is already in your party!
	  else if ( ( leader->party() != NULL ) && ( leader->party()->is_candidate( member->serial ) ) )
		send_sysmessage_cl( leader->client, CLP_Already_Your_Party ); //This person is already in your party!
	  else if ( member->party() != NULL )
		send_sysmessage_cl( leader->client, CLP_Already_in_a_Party ); //This person is already in a party!
	  else
	  {
		if ( member->candidate_of() != NULL )
		  send_sysmessage_cl( leader->client, CLP_Already_in_a_Party ); //This person is already in a party!
		else
		{
		  if ( party_cfg.Hooks.CanAddToParty )
		  {
			if ( !party_cfg.Hooks.CanAddToParty->call( leader->make_ref(), member->make_ref() ) )
			  return;
		  }
		  if ( party_cfg.General.DeclineTimeout > 0 )
			member->set_party_invite_timeout();

		  if ( leader->party() == NULL )
		  {
			Party* party = new Party( leader->serial );
			parties.push_back( ref_ptr<Party>( party ) );
			leader->party( party );
			if ( party_cfg.Hooks.OnPartyCreate )
			  party_cfg.Hooks.OnPartyCreate->call( Module::CreatePartyRefObjImp( party ) );
		  }
		  if ( leader->party()->add_candidate( member->serial ) )
		  {
			member->candidate_of( leader->party() );
			send_invite( member, leader );
		  }
		  else
			send_sysmessage_cl( leader->client, CLP_Max_Size ); //You may only have 10 in your party (this includes candidates).
		}
	  }
	}

	void invite_timeout( Mobile::Character* mem )
	{
	  Party* party = mem->candidate_of();
	  if ( party != NULL )
	  {
		if ( party->remove_candidate( mem->serial ) )
		{
		  if ( mem->has_active_client() )
			send_sysmessage_cl( mem->client, CLP_Decline ); // You notify them that you do not wish to join the party.
		  Mobile::Character* leader = system_find_mobile( party->leader() );
		  if ( party_cfg.Hooks.OnDecline )
			party_cfg.Hooks.OnDecline->call( mem->make_ref() );
		  if ( leader != NULL )
		  {
			if ( leader->has_active_client() )
			  send_sysmessage_cl_affix( leader->client, CLP_Notify_Decline, mem->name().c_str(), true ); //: Does not wish to join the party.
			if ( !party->test_size() )
			  disband_party( leader->serial );
		  }
		}
		mem->candidate_of( NULL );
	  }
	}

	void send_invite( Mobile::Character* member, Mobile::Character* leader )
	{
	  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub6> msg;
	  msg->WriteFlipped<u16>( static_cast<u16>( 10 ) );
	  msg->offset += 2; //sub
	  msg->Write<u8>( static_cast<u8>( PKTBI_BF_06::PARTYCMD_INVITE_MEMBER ) );
	  msg->Write<u32>( leader->serial_ext );
	  msg.Send( member->client );

	  // : You are invited to join the party. Type /accept to join or /decline to decline the offer.
	  send_sysmessage_cl_affix( member->client, CLP_Invite, leader->name().c_str(), true );
	  send_sysmessage_cl( leader->client, CLP_Invited ); // You have invited them to join the party.
	}

    void send_attributes_normalized( Mobile::Character* chr, Mobile::Character* bob )
	{
      Network::PktHelper::PacketOut<Network::PktOut_2D> msg;
	  msg->Write<u32>( bob->serial_ext );
	  int h, mh;

	  h = bob->vital( uoclient_general.hits.id ).current_ones();
	  if ( h > 0xFFFF )
		h = 0xFFFF;
	  mh = bob->vital( uoclient_general.hits.id ).maximum_ones();
	  if ( mh > 0xFFFF )
		mh = 0xFFFF;
	  msg->WriteFlipped<u16>( static_cast<u16>( 1000 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

	  h = bob->vital( uoclient_general.mana.id ).current_ones();
	  if ( h > 0xFFFF )
		h = 0xFFFF;
	  mh = bob->vital( uoclient_general.mana.id ).maximum_ones();
	  if ( mh > 0xFFFF )
		mh = 0xFFFF;
	  msg->WriteFlipped<u16>( static_cast<u16>( 1000 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

	  h = bob->vital( uoclient_general.stamina.id ).current_ones();
	  if ( h > 0xFFFF )
		h = 0xFFFF;
	  mh = bob->vital( uoclient_general.stamina.id ).maximum_ones();
	  if ( mh > 0xFFFF )
		mh = 0xFFFF;
	  msg->WriteFlipped<u16>( static_cast<u16>( 1000 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );

	  msg.Send( chr->client );
	}


    }
    namespace Mobile {
      Core::Party* Character::party() const
      {
        return party_;
      }
      void Character::party( Core::Party* p )
      {
        party_ = p;
      }

      bool Character::party_can_loot() const
      {
        return party_can_loot_;
      }

      void Character::set_party_can_loot( bool b )
      {
        party_can_loot_ = b;
      }

      Core::Party* Character::candidate_of( ) const
      {
        return candidate_of_;
      }

      void Character::candidate_of( Core::Party* c )
      {
        candidate_of_ = c;
      }

      Core::Party* Character::offline_mem_of( ) const
      {
        return offline_mem_of_;
      }

      void Character::offline_mem_of( Core::Party* c )
      {
        offline_mem_of_ = c;
      }

      void Character::set_party_invite_timeout()
      {
        if ( this->party_decline_timeout_ != NULL )
          this->party_decline_timeout_->cancel();
        Core::polclock_t timeout = Core::polclock( ) + Core::party_cfg.General.DeclineTimeout*Core::POLCLOCKS_PER_SEC;
        new Core::OneShotTaskInst<Character*>( &this->party_decline_timeout_,
                                         timeout,
                                         Core::invite_timeout,
                                         this );
      }

      bool Character::has_party_invite_timeout() const
      {
        if ( this->party_decline_timeout_ != NULL )
          return false;
        return true;
      }

      void Character::cancel_party_invite_timeout()
      {
        if ( this->party_decline_timeout_ != NULL )
          this->party_decline_timeout_->cancel();
      }
  }
}
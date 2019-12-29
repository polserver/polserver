/** @file
 *
 * @par History
 * - 2005/11/25 MuadDib:   Added PKTBI_BF::TYPE_SESPAM: to do away with spam.
 * - 2005/11/23 MuadDib:   Altered handle_mode_set for 0x72 Packet. Nows reads
 *                         from combat.cfg for warmode_wait object. Sends the
 *                         0x77 Packet and returns 0 when out of timer.
 * - 2005/12/09 MuadDib:   Added TYPE_CLIENT_LANGUAGE for setting member uclang.
 * - 2006/05/24 Shinigami: Added PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER to support Elfs
 * - 2006/05/30 Shinigami: Changed params of character_race_changer_handler()
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet IDs
 * - 2009/08/14 Turley:    Added PKTBI_BF::TYPE_SCREEN_SIZE & TYPE_CLOSED_STATUS_GUMP (anti spam)
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 *                         Added 0xE1 packet (UO3D clienttype packet)
 * - 2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi
 * - 2009/12/03 Turley:    toggle gargoyle flying support
 */


/* MISCMSG.CPP: Miscellaneous message handlers.  Handlers shouldn't stay here long,
   only until they find a better home - but this is better than putting them in POL.CPP. */

#include <cstddef>
#include <ctype.h>
#include <string>

#include "../bscript/eprog.h"
#include "../bscript/impstr.h"
#include "../clib/clib_endian.h"
#include "../clib/fdump.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../plib/systemstate.h"
#include "../plib/uconst.h"
#include "accounts/account.h"
#include "cmbtcfg.h"
#include "fnsearch.h"
#include "gameclck.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "multi/customhouses.h"
#include "multi/multi.h"
#include "network/client.h"
#include "network/clientio.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "network/sockio.h"
#include "scrstore.h"
#include "spells.h"
#include "tooltips.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoscrobj.h"
#include <format/format.h>

namespace Pol
{
namespace Module
{
void character_race_changer_handler( Network::Client* client, Core::PKTBI_BF* msg );
}
namespace Core
{
using namespace Network;

void handle_unknown_packet( Client* client );

void party_cmd_handler( Client* client, PKTBI_BF* msg );

void OnGuildButton( Client* client );
void OnQuestButton( Client* client );
void OnChatButton( Client* client );

void handle_bulletin_boards( Client* client, PKTBI_71* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_mode_set( Client* client, PKTBI_72* msg )
{
  if ( client->chr->warmode_wait > read_gameclock() )
  {
    send_move( client, client->chr );
    return;
  }
  else
  {
    client->chr->warmode_wait = read_gameclock() + settingsManager.combat_config.warmode_delay;
  }

  bool msg_warmode = msg->warmode ? true : false;

  // FIXME: Should reply with 0x77 packet!? (so says various docs!) [TJ]
  transmit( client, msg, sizeof *msg );

  client->chr->set_warmode( msg_warmode );
}

void handle_keep_alive( Network::Client* client, PKTBI_73* msg )
{
  transmit( client, msg, sizeof *msg );
}

void handle_rename_char( Client* client, PKTIN_75* msg )
{
  Mobile::Character* chr = find_character( cfBEu32( msg->serial ) );
  if ( chr != nullptr )
  {
    if ( client->chr->can_rename( chr ) )
    {
      msg->name[sizeof msg->name - 1] = '\0';
      // check for legal characters
      for ( char* p = msg->name; *p; p++ )
      {
        // only allow: a-z, A-Z & spaces
        if ( *p != ' ' && !isalpha( *p ) )
        {
          fmt::Writer tmp;
          tmp.Format( "Client#{} (account {}) attempted an invalid rename (packet 0x{:X}):\n{}\n" )
              << client->instance_
              << ( ( client->acct != nullptr ) ? client->acct->name() : "unknown" )
              << (int)msg->msgtype << msg->name;
          Clib::fdump( tmp, msg->name, static_cast<int>( strlen( msg->name ) ) );
          POLLOG_INFO << tmp.str();
          *p = '\0';
          send_sysmessage( client, "Invalid name!" );
          return;  // dave 12/26 if invalid name, do not apply to chr!
        }
      }
      chr->setname( msg->name );
    }
    else
    {
      send_sysmessage( client, "I can't rename that." );
    }
  }
  else
  {
    send_sysmessage( client, "I can't find that." );
  }
}

void handle_msg_B5( Client* client, PKTIN_B5* /*msg*/ )
{
  OnChatButton( client );
}

void handle_char_profile_request( Client* client, PKTBI_B8_IN* msg )
{
  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/charprofile", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    Mobile::Character* mobile;

    if ( msg->mode == msg->MODE_REQUEST )
    {
      mobile = system_find_mobile( cfBEu32( msg->profile_request.serial ) );
      if ( mobile == nullptr )
        return;
      client->chr->start_script( prog.get(), false, new Module::ECharacterRefObjImp( mobile ),
                                 new Bscript::BLong( msg->mode ), new Bscript::BLong( 0 ) );
    }
    else if ( msg->mode == msg->MODE_UPDATE )
    {
      mobile = system_find_mobile( cfBEu32( msg->profile_update.serial ) );
      if ( mobile == nullptr )
        return;
      u16* themsg = msg->profile_update.wtext;
      int intextlen = ( cfBEu16( msg->msglen ) - 12 ) / sizeof( msg->profile_update.wtext[0] );

      if ( intextlen < 0 )
        intextlen = 0;
      if ( intextlen > SPEECH_MAX_LEN )
        intextlen = SPEECH_MAX_LEN;
      std::string text = Bscript::String::fromUTF16( themsg, intextlen, true );
      client->chr->start_script( prog.get(), false, new Module::ECharacterRefObjImp( mobile ),
                                 new Bscript::BLong( msg->mode ), new Bscript::String( text ) );
    }
  }
}

void handle_msg_BB( Client* client, PKTBI_BB* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_client_version( Client* client, PKTBI_BD* msg )
{
  u16 len = cfBEu16( msg->msglen ) - 3;
  if ( len < 100 )
  {
    int c = 0;
    char ch;
    std::string ver2 = "";
    while ( c < len )
    {
      ch = msg->version[c];
      if ( ch == 0 )
        break;  // seems to be null-terminated
      ver2 += ch;
      ++c;
    }
    client->setversion( ver2 );

    VersionDetailStruct vers_det;
    client->itemizeclientversion( ver2, vers_det );
    client->setversiondetail( vers_det );

    if ( client->compareVersion( CLIENT_VER_70331 ) )
      client->setClientType( CLIENTTYPE_70331 );
    else if ( client->compareVersion( CLIENT_VER_70300 ) )
      client->setClientType( CLIENTTYPE_70300 );
    else if ( client->compareVersion( CLIENT_VER_70130 ) )
      client->setClientType( CLIENTTYPE_70130 );
    else if ( client->compareVersion( CLIENT_VER_7090 ) )
      client->setClientType( CLIENTTYPE_7090 );
    else if ( client->compareVersion( CLIENT_VER_7000 ) )
      client->setClientType( CLIENTTYPE_7000 );
    else if ( client->compareVersion( CLIENT_VER_60142 ) )
      client->setClientType( CLIENTTYPE_60142 );
    else if ( client->compareVersion( CLIENT_VER_6017 ) )  // Grid-loc support
      client->setClientType( CLIENTTYPE_6017 );
    else if ( client->compareVersion( CLIENT_VER_5020 ) )
      client->setClientType( CLIENTTYPE_5020 );
    else if ( client->compareVersion( CLIENT_VER_5000 ) )
      client->setClientType( CLIENTTYPE_5000 );
    else if ( client->compareVersion( CLIENT_VER_4070 ) )
      client->setClientType( CLIENTTYPE_4070 );
    else if ( client->compareVersion( CLIENT_VER_4000 ) )
      client->setClientType( CLIENTTYPE_4000 );

    if ( settingsManager.ssopt.core_sends_season )
      send_season_info(
          client );  // Scott 10/11/2007 added for login fixes and handling 1.x clients.
                     // Season info needs to check client version to keep from crashing 1.x
                     // version not set until shortly after login complete.
    // send_feature_enable(client); //dave commented out 8/21/03, unexpected problems with people
    // sending B9 via script with this too.
    if ( ( client->UOExpansionFlag & AOS ) )
    {
      send_object_cache( client, client->chr );
    }
  }
  else
  {
    POLLOG_INFO << "Suspect string length in PKTBI_BD packet: " << len << "\n";
  }
}

void ext_stats_in( Client* client, PKTBI_BF* msg )
{
  if ( settingsManager.ssopt.core_handled_locks )
  {
    const Mobile::Attribute* attrib = nullptr;
    switch ( msg->extstatin.stat )
    {
    case PKTBI_BF_1A::STAT_STR:
      attrib = gamestate.pAttrStrength;
      break;
    case PKTBI_BF_1A::STAT_DEX:
      attrib = gamestate.pAttrDexterity;
      break;
    case PKTBI_BF_1A::STAT_INT:
      attrib = gamestate.pAttrIntelligence;
      break;
    default:  // sent an illegal stat. Should report to console?
      return;
    }

    if ( attrib == nullptr )  // there's no attribute for this (?)
      return;

    u8 state = msg->extstatin.mode;
    if ( state > 2 )  // FIXME hard-coded value
      return;

    client->chr->attribute( attrib->attrid ).lock( state );
  }
}

void handle_msg_BF( Client* client, PKTBI_BF* msg )
{
  UObject* obj = nullptr;
  Multi::UMulti* multi = nullptr;
  Multi::UHouse* house = nullptr;
  switch ( cfBEu16( msg->subcmd ) )
  {
  case PKTBI_BF::TYPE_CLIENT_LANGUAGE:
    client->chr->uclang = Clib::strlowerASCII( msg->client_lang );
    break;
  case PKTBI_BF::TYPE_REQ_FULL_CUSTOM_HOUSE:
    if ( ( client->UOExpansionFlag & AOS ) == 0 )
      return;
    multi = system_find_multi( cfBEu32( msg->reqfullcustomhouse.house_serial ) );
    if ( multi != nullptr )
    {
      house = multi->as_house();
      if ( house != nullptr )
      {
        if ( client->UOExpansionFlag & AOS )
        {
          send_object_cache( client, (UObject*)( house ) );
        }
        // consider sending working design to certain players, to assist building, or GM help
        CustomHousesSendFull( house, client, Multi::HOUSE_DESIGN_CURRENT );
      }
    }
    break;
  case PKTBI_BF::TYPE_OBJECT_CACHE:
    if ( ( client->UOExpansionFlag & AOS ) == 0 )
      return;
    obj = system_find_object( cfBEu32( msg->objectcache.serial ) );
    if ( obj != nullptr )
    {
      SendAOSTooltip( client, obj );
    }
    break;
  case PKTBI_BF::TYPE_SESPAM:
    return;
    break;
  case PKTBI_BF::TYPE_SPELL_SELECT:
    do_cast( client, cfBEu16( msg->spellselect.selected_spell ) );
    break;
  case PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER:
    Module::character_race_changer_handler( client, msg );
    break;
  case PKTBI_BF::TYPE_PARTY_SYSTEM:
    party_cmd_handler( client, msg );
    break;
  case PKTBI_BF::TYPE_EXTENDED_STATS_IN:
    ext_stats_in( client, msg );
    break;
  case PKTBI_BF::TYPE_CLOSED_STATUS_GUMP:
    return;
    break;
  case PKTBI_BF::TYPE_SCREEN_SIZE:
    return;
    break;
  case PKTBI_BF::TYPE_TOGGLE_FLYING:
    if ( client->chr->race == Plib::RACE_GARGOYLE )
    {
      // FIXME: add checks if its possible to stand with new movemode
      client->chr->movemode = ( Plib::MOVEMODE )( client->chr->movemode ^ Plib::MOVEMODE_FLY );
      send_move_mobile_to_nearby_cansee( client->chr );
      send_goxyz( client, client->chr );
    }
    break;
  case PKTBI_BF::TYPE_CLIENTTYPE:
    client->UOExpansionFlagClient = ctBEu32( msg->clienttype.clientflag );
    break;
  case PKTBI_BF::TYPE_POPUP_MENU_REQUEST:
  {
    ref_ptr<Bscript::EScriptProgram> prog =
        find_script( "misc/popupmenu", true, Plib::systemstate.config.cache_interactive_scripts );
    if ( prog.get() == nullptr )
      break;
    u32 serial = cfBEu32( msg->serial_request_popup_menu );
    if ( IsCharacter( serial ) )
    {
      Pol::Mobile::Character* chr = system_find_mobile( serial );
      if ( chr == nullptr )
        break;
      client->chr->start_script( prog.get(), false, new Pol::Module::ECharacterRefObjImp( chr ) );
    }
    else
    {
      Pol::Items::Item* item = system_find_item( serial );
      if ( item == nullptr )
        break;
      client->chr->start_script( prog.get(), false, item->make_ref() );
    }
    break;
  }
  case PKTBI_BF::TYPE_POPUP_ENTRY_SELECT:
  {
    if ( client->chr->on_popup_menu_selection == nullptr )
    {
      POLLOG_INFO.Format( "{}/{} tried to use a popup menu, but none was active.\n" )
          << client->acct->name() << client->chr->name();
      break;
    }

    u32 serial = cfBEu32( msg->popupselect.serial );
    u16 id = cfBEu16( msg->popupselect.entry_tag );
    client->chr->on_popup_menu_selection( client, serial, id );
    break;
  }
  default:
    handle_unknown_packet( client );
  }
}

void handle_unknown_C4( Client* client, PKTOUT_C4* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_update_range_change( Client* client, PKTBI_C8* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_open_uo_store( Client* client, PKTIN_FA* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_update_view_public_house_content( Client* client, PKTIN_FB* /*msg*/ )
{
  handle_unknown_packet( client );
}

void handle_allnames( Client* client, PKTBI_98_IN* msg )
{
  u32 serial = cfBEu32( msg->serial );
  Mobile::Character* the_mob = find_character( serial );
  if ( the_mob != nullptr )
  {
    if ( !client->chr->is_visible_to_me( the_mob ) )
    {
      return;
    }
    if ( pol_distance( client->chr->x, client->chr->y, the_mob->x, the_mob->y ) > 20 )
    {
      return;
    }

    PktHelper::PacketOut<PktOut_98> msgOut;
    msgOut->WriteFlipped<u16>( 37u );  // static length
    msgOut->Write<u32>( the_mob->serial_ext );
    msgOut->Write( the_mob->name().c_str(), 30, false );
    msgOut.Send( client );
  }
  else
  {
    return;
  }
}

void handle_se_object_list( Client* client, PKTBI_D6_IN* msgin )
{
  UObject* obj = nullptr;
  int length = cfBEu16( msgin->msglen ) - 3;
  if ( length < 0 || ( length % 4 ) != 0 )
    return;
  int count = length / 4;

  for ( int i = 0; i < count; ++i )
  {
    obj = system_find_object( cfBEu32( msgin->serials[i].serial ) );
    if ( obj != nullptr )
      SendAOSTooltip( client, obj );
  }
}

void handle_ef_seed( Client* client, PKTIN_EF* msg )
{
  VersionDetailStruct detail;
  detail.major = cfBEu32( msg->ver_Major );
  detail.minor = cfBEu32( msg->ver_Minor );
  detail.rev = cfBEu32( msg->ver_Revision );
  detail.patch = cfBEu32( msg->ver_Patch );
  client->setversiondetail( detail );
  if ( client->compareVersion( CLIENT_VER_70331 ) )
    client->setClientType( CLIENTTYPE_70331 );
  else if ( client->compareVersion( CLIENT_VER_70300 ) )
    client->setClientType( CLIENTTYPE_70300 );
  else if ( client->compareVersion( CLIENT_VER_70130 ) )
    client->setClientType( CLIENTTYPE_70130 );
  else if ( client->compareVersion( CLIENT_VER_7090 ) )
    client->setClientType( CLIENTTYPE_7090 );
  else if ( client->compareVersion( CLIENT_VER_7000 ) )
    client->setClientType( CLIENTTYPE_7000 );
  else if ( client->compareVersion( CLIENT_VER_60142 ) )
    client->setClientType( CLIENTTYPE_60142 );
  else if ( client->compareVersion( CLIENT_VER_6017 ) )  // Grid-loc support
    client->setClientType( CLIENTTYPE_6017 );
  else if ( client->compareVersion( CLIENT_VER_5020 ) )
    client->setClientType( CLIENTTYPE_5020 );
  else if ( client->compareVersion( CLIENT_VER_5000 ) )
    client->setClientType( CLIENTTYPE_5000 );
  else if ( client->compareVersion( CLIENT_VER_4070 ) )
    client->setClientType( CLIENTTYPE_4070 );
  else if ( client->compareVersion( CLIENT_VER_4000 ) )
    client->setClientType( CLIENTTYPE_4000 );

  // detail->patch is since 5.0.7 always numeric, so no need to make it complicated
  OSTRINGSTREAM os;
  os << detail.major << "." << detail.minor << "." << detail.rev << "." << detail.patch;
  client->setversion( OSTRINGSTREAM_STR( os ) );
}

void handle_e1_clienttype( Client* client, PKTIN_E1* msg )
{
  switch ( cfBEu32( msg->clienttype ) )
  {
  case PKTIN_E1::CLIENTTYPE_KR:
    client->setClientType( CLIENTTYPE_UOKR );
    break;
  case PKTIN_E1::CLIENTTYPE_SA:
    client->setClientType( CLIENTTYPE_UOSA );
    break;
  default:
    INFO_PRINT << "Unknown client type send with packet 0xE1 : 0x"
               << fmt::hexu( static_cast<unsigned long>( cfBEu32( msg->clienttype ) ) ) << "\n";
    break;
  }
}


/**
 * Handler for a 0xD7 packet
 */
void handle_aos_commands( Client* client, PKTBI_D7* msg )
{
  // nullptr prevention, no need to disturb if client or character is not found
  if ( client == nullptr || client->chr == nullptr )
    return;

  /// Checks that serial written inside packet matches sending character's serial
  u32 serial = cfBEu32( msg->serial );
  if ( client && client->chr && client->chr->serial != serial )
  {
    INFO_PRINT << "Ignoring spoofed packet 0xD7 from character 0x"
               << fmt::hexu( client->chr->serial ) << " trying to spoof 0x" << fmt::hexu( serial )
               << "\n";
    return;
  }

  switch ( cfBEu16( msg->subcmd ) )
  {
  case PKTBI_D7::CUSTOM_HOUSE_BACKUP:
    Multi::CustomHousesBackup( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_RESTORE:
    Multi::CustomHousesRestore( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_COMMIT:
    Multi::CustomHousesCommit( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_ERASE:
    Multi::CustomHousesErase( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_ADD:
    Multi::CustomHousesAdd( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_QUIT:
    Multi::CustomHousesQuit( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_ADD_MULTI:
    Multi::CustomHousesAddMulti( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_SYNCH:
    Multi::CustomHousesSynch( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_CLEAR:
    Multi::CustomHousesClear( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_SELECT_FLOOR:
    Multi::CustomHousesSelectFloor( msg );
    break;

  case PKTBI_D7::CUSTOM_HOUSE_REVERT:
    Multi::CustomHousesRevert( msg );
    break;
  case PKTBI_D7::CUSTOM_HOUSE_SELECT_ROOF:
    Multi::CustomHousesRoofSelect( msg );
    break;
  case PKTBI_D7::CUSTOM_HOUSE_DELETE_ROOF:
    Multi::CustomHousesRoofRemove( msg );
    break;
  case PKTBI_D7::GUILD_BUTTON:
    OnGuildButton( client );
    break;
  case PKTBI_D7::QUEST_BUTTON:
    OnQuestButton( client );
    break;
  // missing combat book abilities
  default:
    handle_unknown_packet( client );
  }
}

void OnGuildButton( Client* client )
{
  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/guildbutton", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    client->chr->start_script( prog.get(), false );
  }
}

void OnQuestButton( Client* client )
{
  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/questbutton", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    client->chr->start_script( prog.get(), false );
  }
}

void OnChatButton( Client* client )
{
  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/chatbutton", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    client->chr->start_script( prog.get(), false );
  }
}
}  // namespace Core
}  // namespace Pol

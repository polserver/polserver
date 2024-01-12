#include "msghandl.h"

#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../getitem.h"
#include "../globals/network.h"
#include "client.h"
#include "pktboth.h"
#include "pktbothid.h"
#include "pktin.h"
#include "pktinid.h"
#include "pktout.h"
#include "pktoutid.h"

namespace Pol
{
namespace Network
{
void PacketRegistry::handle_msg( unsigned char msgid, Client* client, void* msg )
{
  MSG_HANDLER msghandler = find_handler( msgid, client );
  msghandler.func( client, msg );
}

MSG_HANDLER PacketRegistry::find_handler( unsigned char msgid, const Client* client )
{
  if ( client->might_use_v2_handler() && PacketRegistry::msglen_v2( msgid ) )
    return Core::networkManager.handler_v2[msgid];

  return Core::networkManager.handler[msgid];
}

PktHandlerFunc PacketRegistry::get_callback( unsigned char msgid,
                                             PacketVersion version /*= PacketVersion::V1*/ )
{
  if ( version == PacketVersion::V1 )
    return Core::networkManager.handler[msgid].func;

  return Core::networkManager.handler_v2[msgid].func;
}

int PacketRegistry::msglen( unsigned char msgid )
{
  return Core::networkManager.handler[msgid].msglen;
}

int PacketRegistry::msglen_v2( unsigned char msgid )
{
  return Core::networkManager.handler_v2[msgid].msglen;
}

void PacketRegistry::set_handler( unsigned char msgid, int len, PktHandlerFunc func,
                                  PacketVersion version /*= PacketVersion::V1*/ )
{
  passert( len != 0 );

  if ( version == PacketVersion::V1 )
  {
    Core::networkManager.handler[msgid].func = func;
    Core::networkManager.handler[msgid].msglen = len;
  }
  else
  {
    Core::networkManager.handler_v2[msgid].func = func;
    Core::networkManager.handler_v2[msgid].msglen = len;
  }
}

bool PacketRegistry::is_defined( unsigned char msgid )
{
  return Core::networkManager.handler[msgid].msglen ||
         Core::networkManager.handler_v2[msgid].msglen;
}

void PacketRegistry::handle_extended_cmd( Client* client, Core::PKTIN_12* msg )
{
  u16 msglen = cfBEu16( msg->msglen );

  if ( msglen > sizeof *msg )  // sanity-check the message.
    return;
  if ( msg->data[msglen - offsetof( Core::PKTIN_12, data ) - 1] !=
       '\0' )  // the string must be null-terminated.
    return;

  if ( Core::networkManager.ext_handler_table[msg->submsgtype].func )
    Core::networkManager.ext_handler_table[msg->submsgtype].func( client, msg );
}

void PacketRegistry::set_extended_handler( UEXTMSGID submsgtype,
                                           void ( *func )( Client* client, Core::PKTIN_12* msg ) )
{
  if ( Core::networkManager.ext_handler_table[submsgtype].func )
  {
    ERROR_PRINTLN( "Extended Message Handler {} ({:#X}) multiply defined.",
                   fmt::underlying( submsgtype ), fmt::underlying( submsgtype ) );
    throw std::runtime_error( "Extended Message Handler multiply defined." );
  }
  Core::networkManager.ext_handler_table[submsgtype].func = func;
}

// Preprocessor macros for registering message handlers:
//    example:
//               MESSAGE_HANDLER( PKTBI_2C, handle_resurrect_menu );
//
//  will register the fixed-length packet 2C with callback "handle_ressurect_menu"
#define MESSAGE_HANDLER( type, func )                                             \
  PacketRegistry::set_handler( type##_ID, sizeof( type ),                         \
                               reinterpret_cast<Network::PktHandlerFunc>( func ), \
                               Network::PacketVersion::V1 )

#define MESSAGE_HANDLER_VARLEN( type, func )                                                   \
  PacketRegistry::set_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc)func, \
                               Network::PacketVersion::V1 )

#define MESSAGE_HANDLER_V2( type, func )                                          \
  PacketRegistry::set_handler( type##_ID, sizeof( type ),                         \
                               reinterpret_cast<Network::PktHandlerFunc>( func ), \
                               Network::PacketVersion::V2 )

#define MESSAGE_HANDLER_VARLEN_V2( type, func )                                                \
  PacketRegistry::set_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (Network::PktHandlerFunc)func, \
                               Network::PacketVersion::V2 )

void PacketRegistry::initialize_msg_handlers()
{
  using namespace Core;
  MESSAGE_HANDLER( PKTIN_00, ClientCreateChar );
  MESSAGE_HANDLER( PKTIN_02, handle_walk );
  MESSAGE_HANDLER_VARLEN( PKTIN_03, SpeechHandler );
  MESSAGE_HANDLER( PKTIN_05, Mobile::handle_attack );
  MESSAGE_HANDLER( PKTIN_06, doubleclick );
  MESSAGE_HANDLER( PKTIN_07, GottenItem::handle );
  MESSAGE_HANDLER( PKTIN_08_V1, drop_item );
  MESSAGE_HANDLER_V2( PKTIN_08_V2, drop_item_v2 );
  MESSAGE_HANDLER( PKTIN_09, handle_singleclick );
  MESSAGE_HANDLER_VARLEN( PKTIN_12, PacketRegistry::handle_extended_cmd );
  MESSAGE_HANDLER( PKTIN_13, equip_item );
  MESSAGE_HANDLER( PKTBI_22_SYNC, handle_resync_request );
  MESSAGE_HANDLER( PKTBI_2C, Module::handle_resurrect_menu );
  MESSAGE_HANDLER( PKTIN_34, srequest );
  MESSAGE_HANDLER( PKTBI_3A_LOCKS, handle_skill_lock );
  MESSAGE_HANDLER_VARLEN( PKTBI_3B, Module::buyhandler );
  MESSAGE_HANDLER( PKTBI_56, handle_map_pin );
  MESSAGE_HANDLER( PKTIN_5D, char_select );
  MESSAGE_HANDLER_VARLEN( PKTBI_66, Module::read_book_page_handler );
  MESSAGE_HANDLER( PKTBI_6C, handle_target_cursor );
  MESSAGE_HANDLER_VARLEN( PKTBI_6F, handle_secure_trade_msg );
  MESSAGE_HANDLER_VARLEN( PKTBI_71, handle_bulletin_boards );
  MESSAGE_HANDLER( PKTBI_72, handle_mode_set );
  MESSAGE_HANDLER( PKTBI_73, handle_keep_alive );
  MESSAGE_HANDLER( PKTIN_75, handle_rename_char );
  MESSAGE_HANDLER( PKTIN_7D, handle_menu_selection );
  MESSAGE_HANDLER( PKTIN_80, loginserver_login );
  MESSAGE_HANDLER( PKTIN_83, handle_delete_character );
  MESSAGE_HANDLER( PKTIN_8D, ClientCreateCharKR );
  MESSAGE_HANDLER( PKTIN_91, login2 );
  MESSAGE_HANDLER( PKTBI_93, Module::open_book_handler );
  MESSAGE_HANDLER( PKTBI_95, Module::handle_selcolor );
  MESSAGE_HANDLER( PKTBI_98_IN, handle_allnames );
  MESSAGE_HANDLER_VARLEN( PKTBI_9A, handle_prompt );
  MESSAGE_HANDLER( PKTIN_9B, handle_help );
  MESSAGE_HANDLER_VARLEN( PKTIN_9F, Module::sellhandler );
  MESSAGE_HANDLER( PKTIN_A0, select_server );
  MESSAGE_HANDLER( PKTIN_A4, handle_A4 );
  MESSAGE_HANDLER( PKTIN_A7, handle_get_tip );
  MESSAGE_HANDLER_VARLEN( PKTIN_AC, Module::handle_textentry );
  MESSAGE_HANDLER_VARLEN( PKTIN_AD, UnicodeSpeechHandler );
  MESSAGE_HANDLER_VARLEN( PKTIN_B1, Module::gumpbutton_handler );
  MESSAGE_HANDLER( PKTIN_B5, handle_msg_B5 );
  MESSAGE_HANDLER( PKTIN_B6, handle_request_tooltip );
  MESSAGE_HANDLER_VARLEN( PKTBI_B8_IN, handle_char_profile_request );
  MESSAGE_HANDLER( PKTBI_BB, handle_msg_BB );
  MESSAGE_HANDLER_VARLEN( PKTBI_BD, handle_client_version );
  MESSAGE_HANDLER_VARLEN( PKTBI_BF, handle_msg_BF );
  MESSAGE_HANDLER_VARLEN( PKTBI_C2, handle_unicode_prompt );
  MESSAGE_HANDLER( PKTOUT_C4, handle_unknown_C4 );
  MESSAGE_HANDLER( PKTBI_C8, handle_update_range_change );
  MESSAGE_HANDLER_VARLEN( PKTBI_D6_IN, handle_se_object_list );
  MESSAGE_HANDLER_VARLEN( PKTBI_D7, handle_aos_commands );
  MESSAGE_HANDLER( PKTIN_D9, handle_D9 );
  MESSAGE_HANDLER( PKTIN_E1, handle_e1_clienttype );
  MESSAGE_HANDLER_VARLEN( PKTIN_E4, KR_Verifier_Response );
  MESSAGE_HANDLER( PKTIN_EF, handle_ef_seed );
  MESSAGE_HANDLER( PKTIN_F8, ClientCreateChar70160 );
  MESSAGE_HANDLER_VARLEN( PKTBI_F0, handle_krrios_packet );
  MESSAGE_HANDLER( PKTIN_FA, handle_open_uo_store );
  MESSAGE_HANDLER( PKTIN_FB, handle_update_view_public_house_content );

  initialize_extended_handlers();
}

void PacketRegistry::initialize_extended_handlers()
{
  using namespace Core;
  set_extended_handler( EXTMSGID_ACTION, handle_action );
  set_extended_handler( EXTMSGID_CASTSPELL1, handle_cast_spell );
  set_extended_handler( EXTMSGID_CASTSPELL2, handle_cast_spell );
  set_extended_handler( EXTMSGID_SPELLBOOK, handle_open_spellbook );
  set_extended_handler( EXTMSGID_SKILL, handle_use_skill );
}
}  // namespace Network
}  // namespace Pol

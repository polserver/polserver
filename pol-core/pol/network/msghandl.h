/** @file
 *
 * @par History
 * - 2009/08/03 MuadDib:   Renamed secondary handler class to *_V2 for naming convention
 */


#ifndef MSGHANDL_H
#define MSGHANDL_H

#include "pktboth.h"
#include "pktin.h"
#include "pktout.h"

namespace Pol
{
namespace Network
{
class Client;


enum class PacketVersion
{
  V1 = 1,  // The default, used by the earlier client versions
  V2 = 2,  // Used to handle packets which were redefined after 6.0.1.7
  Default = V1
};

using PktHandlerFunc = void ( * )( Client*, void* );

struct MSG_HANDLER
{
  int msglen;  // if 0, no message handler defined.
  PktHandlerFunc func;
};

// extended cmds (0x12)
enum UEXTMSGID
{
  EXTMSGID_SKILL = 0x24,       // Form: "SkillId parameter" (SkillId 0-based, param usu. 0)
  EXTMSGID_CASTSPELL1 = 0x27,  // Form: "SpellId SpellBookSerial"
  EXTMSGID_SPELLBOOK = 0x43,   // Form: (empty)
  EXTMSGID_CASTSPELL2 = 0x56,  // Form: "SpellId"
  EXTMSGID_OPENDOOR = 0x58,    // Form: (empty)
  EXTMSGID_ACTION = 0xC7       // Form: "action_name"
};
// Extended messages come in the form of a null-terminated string.  The
// "Extended Message" message handler validates that the string is in
// fact null terminated where it's supposed to be.  It does not currently
// verify that the string isn't terminated early also, or that the string
// only contains printables.
struct ExtMsgHandler
{
  void ( *func )( Client* client, Core::PKTIN_12* msg );
};

// Class for keeping the message handlers
class PacketRegistry
{
public:
  // Returns true if the message is defined in either original or v2 packet handlers
  static bool is_defined( unsigned char msgid );

  // Returns the message length (or 0 if undefined)
  static int msglen( unsigned char msgid );

  // Returns the message length (or 0 if undefined) for a v2 packet
  static int msglen_v2( unsigned char msgid );

  // Registers a handler for a certain message type
  static void set_handler( unsigned char msgid, int len, PktHandlerFunc func,
                           PacketVersion version = PacketVersion::V1 );

  // Returns the callback function for the handler of a certain message type
  static PktHandlerFunc get_callback( unsigned char msgid,
                                      PacketVersion version = PacketVersion::V1 );

  // Finds the appropriate handler for a message type according to client version
  static MSG_HANDLER find_handler( unsigned char msgid, const Client* client );

  // Handles the specific message type by invoking the callback according to client version
  static void handle_msg( unsigned char msgid, Client* client, void* data );

  // registers all pkts, called once on startup
  static void initialize_msg_handlers();

  // handler for pkt 12 to call sub handlers
  static void handle_extended_cmd( Client* client, Core::PKTIN_12* msg );

private:
  // registers all externed handlers (0x12)
  static void initialize_extended_handlers();
  static void set_extended_handler( UEXTMSGID submsgtype,
                                    void ( *func )( Client* client, Core::PKTIN_12* msg ) );
};


#define MSGLEN_2BYTELEN_DATA -2


}  // namespace Network
namespace Mobile
{
void handle_attack( Network::Client* client, Core::PKTIN_05* msg );
}
namespace Module
{
void handle_resurrect_menu( Network::Client* client, Core::PKTBI_2C* msg );
void handle_selcolor( Network::Client* client, Core::PKTBI_95* msg );
void open_book_handler( Network::Client* client, Core::PKTBI_93* msg );
void buyhandler( Network::Client* client, Core::PKTBI_3B* msg );
void sellhandler( Network::Client* client, Core::PKTIN_9F* msg );
void gumpbutton_handler( Network::Client* client, Core::PKTIN_B1* msg );
void handle_textentry( Network::Client* client, Core::PKTIN_AC* msg );
void read_book_page_handler( Network::Client* client, Core::PKTBI_66* msg );
}  // namespace Module
namespace Core
{
void ClientCreateChar( Network::Client* client, PKTIN_00* msg );
void ClientCreateCharKR( Network::Client* client, PKTIN_8D* msg );
void ClientCreateChar70160( Network::Client* client, PKTIN_F8* msg );
void doubleclick( Network::Client* client, PKTIN_06* msg );
void drop_item( Network::Client* client, PKTIN_08_V1* msg );
void drop_item_v2( Network::Client* client, PKTIN_08_V2* msg );
void equip_item( Network::Client* client, PKTIN_13* msg );
void handle_help( Network::Client* client, PKTIN_9B* msg );
void handle_skill_lock( Network::Client* client, PKTBI_3A_LOCKS* msg );
void srequest( Network::Client* client, PKTIN_34* msg );
void loginserver_login( Network::Client* client, PKTIN_80* msg );
void handle_A4( Network::Client* client, PKTIN_A4* msg );
void handle_D9( Network::Client* client, PKTIN_D9* msg );
void select_server( Network::Client* client, PKTIN_A0* msg );
void login2( Network::Client* client, PKTIN_91* msg );
void handle_delete_character( Network::Client* client, PKTIN_83* msg );
void handle_menu_selection( Network::Client* client, PKTIN_7D* msg );
void handle_mode_set( Network::Client* client, PKTBI_72* msg );
void handle_rename_char( Network::Client* client, PKTIN_75* msg );
void handle_msg_B5( Network::Client* client, PKTIN_B5* msg );
void handle_msg_BB( Network::Client* client, PKTBI_BB* msg );
void handle_unknown_C4( Network::Client* client, PKTOUT_C4* msg );
void handle_update_range_change( Network::Client* client, PKTBI_C8* msg );
void handle_allnames( Network::Client* client, PKTBI_98_IN* msg );
void handle_ef_seed( Network::Client* client, PKTIN_EF* msg );
void handle_e1_clienttype( Network::Client* client, PKTIN_E1* msg );
void handle_walk( Network::Client* client, PKTIN_02* msg02 );
void char_select( Network::Client* client, PKTIN_5D* msg );
void handle_resync_request( Network::Client* client, PKTBI_22_SYNC* msg );
void handle_keep_alive( Network::Client* client, PKTBI_73* msg );
void handle_singleclick( Network::Client* client, PKTIN_09* msg );
void handle_target_cursor( Network::Client* client, PKTBI_6C* msg );
void handle_get_tip( Network::Client* client, PKTIN_A7* msg );
void handle_request_tooltip( Network::Client* client, PKTIN_B6* msgin );
void handle_map_pin( Network::Client* client, PKTBI_56* msg );
void handle_unicode_prompt( Network::Client* client, Core::PKTBI_C2* msg );
void handle_secure_trade_msg( Network::Client* client, PKTBI_6F* msg );
void handle_prompt( Network::Client* client, PKTBI_9A* msg );
void KR_Verifier_Response( Network::Client* client, PKTIN_E4* msg );
void handle_bulletin_boards( Network::Client* client, PKTBI_71* msg );
void handle_char_profile_request( Network::Client* client, PKTBI_B8_IN* msg );
void handle_client_version( Network::Client* client, PKTBI_BD* msg );
void handle_msg_BF( Network::Client* client, PKTBI_BF* msg );
void handle_se_object_list( Network::Client* client, PKTBI_D6_IN* msgin );
void handle_aos_commands( Network::Client* client, PKTBI_D7* msg );
void SpeechHandler( Network::Client* client, PKTIN_03* mymsg );
void UnicodeSpeechHandler( Network::Client* client, PKTIN_AD* msgin );

void handle_action( Network::Client* client, PKTIN_12* cmd );
void handle_cast_spell( Network::Client* client, PKTIN_12* msg );
void handle_open_spellbook( Network::Client* client, PKTIN_12* msg );
void handle_use_skill( Network::Client* client, PKTIN_12* msg );
void handle_krrios_packet( Network::Client* client, PKTBI_F0* msg );
void handle_open_uo_store( Network::Client* client, PKTIN_FA* msg );
void handle_update_view_public_house_content( Network::Client* client, PKTIN_FB* msg );
}  // namespace Core
}  // namespace Pol
#endif

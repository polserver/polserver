/** @file
 *
 * @par History
 * - 2006/05/07 Shinigami: SendAOSTooltip - will now send merchant_description() if needed
 * - 2009/07/26 MuadDib:   updates for new Packet Refactoring
 * - 2009/07/31 Turley:    added send_fight_occuring() for packet 0x2F
 * - 2009/08/01 MuadDib:   Removed send_tech_stuff(), unused and obsolete.
 * - 2009/08/09 MuadDib:   Refactor of Packet 0x25 for naming convention
 * - 2009/09/22 Turley:    Added DamagePacket support
 */


#ifndef __UFUNC_H
#define __UFUNC_H

#include <stddef.h>

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include "base/position.h"
#include "core.h"
#include "network/packets.h"

namespace Pol
{
namespace Network
{
class Client;
class RemoveObjectPkt;
}  // namespace Network
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Multi
{
class UMulti;
}
namespace Realms
{
class Realm;
}

namespace Core
{
class Menu;
class UContainer;
class UObject;

void SetCurrentItemSerialNumber( u32 serial );
void SetCurrentCharSerialNumber( u32 serial );
u32 GetNextSerialNumber();
u32 GetCurrentItemSerialNumber();
u32 GetCurrentCharSerialNumber();
u32 UseCharSerialNumber( u32 serial );

u32 GetNewItemSerialNumber();
u32 UseItemSerialNumber( u32 serial );

void send_login_error( Network::Client* client, unsigned char reason );

bool inrange( const Mobile::Character* c1, const Mobile::Character* c2 );
bool inrange( const Mobile::Character* c1, const UObject* obj );
bool inrange( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 );

bool inrangex( const Mobile::Character* c1, const Mobile::Character* c2, int maxdist );
bool inrangex( const UObject* c1, unsigned short x, unsigned short y, int maxdist );

bool multi_inrange( const Mobile::Character* c1, const Multi::UMulti* obj );
bool multi_inrange( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 );

unsigned short pol_distance( const Mobile::Character* c1, const UObject* obj );
unsigned short pol_distance( unsigned short x1, unsigned short y1, unsigned short x2,
                             unsigned short y2 );

bool in_say_range( const Mobile::Character* c1, const Mobile::Character* c2 );
bool in_yell_range( const Mobile::Character* c1, const Mobile::Character* c2 );
bool in_whisper_range( const Mobile::Character* c1, const Mobile::Character* c2 );

void send_owncreate( Network::Client* client, const Mobile::Character* chr );
void send_owncreate( Network::Client* client, const Mobile::Character* chr,
                     Network::PktOut_78* owncreate );
void build_owncreate( const Mobile::Character* chr, Network::PktOut_78* msg );

void send_item( Network::Client* client, const Items::Item* item );
void send_corpse( Network::Client* client, const Items::Item* item );
void send_full_corpse( Network::Client* client, const Items::Item* item );

void send_wornitem( Network::Client* client, const Mobile::Character* chr,
                    const Items::Item* item );

void send_move( Network::Client* client, const Mobile::Character* chr );
void send_move( Network::Client* client, const Mobile::Character* chr,
                Network::PktOut_77* movebuffer );
void send_objdesc( Network::Client* client, Items::Item* item );

void send_poisonhealthbar( Network::Client* client, const Mobile::Character* chr );
void send_invulhealthbar( Network::Client* client, const Mobile::Character* chr );

void send_item_to_inrange( const Items::Item* item );
void update_item_to_inrange( const Items::Item* item );
void send_put_in_container( Network::Client* client, const Items::Item* item );
void send_put_in_container_to_inrange( const Items::Item* item );
void send_wornitem_to_inrange( const Mobile::Character* chr, const Items::Item* item );
void update_wornitem_to_inrange( const Mobile::Character* chr, const Items::Item* item );
void send_corpse_equip_inrange( const Items::Item* item );

void send_midi( Network::Client* client, unsigned short midi );
Mobile::Character* find_character( u32 serial );

UObject* find_toplevel_object( u32 serial );
void setrealm( Items::Item* item, void* arg );
void setrealmif( Items::Item* item, void* arg );

// TODO: 9 global functions for remove pkt???
void send_remove_character( Network::Client* client, const Mobile::Character* chr,
                            Network::RemoveObjectPkt& pkt );
void send_remove_character( Network::Client* client, const Mobile::Character* chr );
void send_remove_object_if_inrange( Network::Client* client, const Items::Item* obj );
void send_remove_object_to_inrange( const UObject* centerObject );
void send_remove_object( Network::Client* client, const UObject* item,
                         Network::RemoveObjectPkt& pkt );
void send_remove_object( Network::Client* client, const UObject* obj );
void send_remove_character_to_nearby( const Mobile::Character* chr );
void send_remove_character_to_nearby_cantsee( const Mobile::Character* chr );
void remove_objects_inrange( Network::Client* client );


void send_goxyz( Network::Client* client, const Mobile::Character* chr );
void send_light( Network::Client* client, int lightlevel );
void send_weather( Network::Client* client, unsigned char type, unsigned char severity,
                   unsigned char aux );
void send_mode( Network::Client* client );
void send_item_move_failure( Network::Client* client, u8 reason );
u16 convert_objtype_to_spellnum( u32 objtype, int school );

void send_stamina_level( Network::Client* client );
void send_mana_level( Network::Client* client );

UContainer* find_legal_container( const Mobile::Character* chr, u32 serial );
bool is_a_parent( const Items::Item* item, u32 serial );
void play_sound_effect( const UObject* center, u16 effect );
void play_sound_effect_private( const UObject* center, u16 effect, Mobile::Character* forchr );
void play_sound_effect_xyz( const Pos4d& center, u16 effect );
void play_lightning_bolt_effect( const UObject* center );
void play_moving_effect( const UObject* src, const UObject* dst, u16 effect, u8 speed, u8 loop,
                         u8 explode );
void play_moving_effect2( const Pos3d& src, const Pos3d& dst, u16 effect, u8 speed, u8 loop,
                          u8 explode, Realms::Realm* realm );
void play_object_centered_effect( const UObject* center, u16 effect, u8 speed, u8 loop );

void play_stationary_effect( const Pos4d& pos, u16 effect, u8 speed, u8 loop, u8 explode );

void play_stationary_effect_ex( const Pos4d& pos, u16 effect, u8 speed, u8 duration, u32 hue,
                                u32 render, u16 effect3d );
void play_object_centered_effect_ex( const UObject* center, u16 effect, u8 speed, u8 duration,
                                     u32 hue, u32 render, u8 layer, u16 effect3d );
void play_moving_effect_ex( const UObject* src, const UObject* dst, u16 effect, u8 speed,
                            u8 duration, u32 hue, u32 render, u8 direction, u8 explode,
                            u16 effect3d, u16 effect3dexplode, u16 effect3dsound );
void play_moving_effect2_ex( const Pos3d& src, const Pos3d& dst, Realms::Realm* realm, u16 effect,
                             u8 speed, u8 duration, u32 hue, u32 render, u8 direction, u8 explode,
                             u16 effect3d, u16 effect3dexplode, u16 effect3dsound );


// find_legal_item: search worn items, including backpack recursively, and
// items on the ground, recursively, for an item of a given serial.
Items::Item* find_legal_item( const Mobile::Character* chr, u32 serial, bool* additlegal = nullptr,
                              bool* isRemoteContainer = nullptr );

Items::Item* find_snoopable_item( u32 serial, Mobile::Character** powner = nullptr );

void send_sysmessage( Network::Client* client, const char* text,
                      unsigned short font = Plib::DEFAULT_TEXT_FONT,
                      unsigned short color = Plib::DEFAULT_TEXT_COLOR );
void send_sysmessage_unicode( Network::Client* client, const std::string& text,
                              const std::string& lang,
                              unsigned short font = Plib::DEFAULT_TEXT_FONT,
                              unsigned short color = Plib::DEFAULT_TEXT_COLOR );
void broadcast( const char* text, unsigned short font = Plib::DEFAULT_TEXT_FONT,
                unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                unsigned short requiredCmdLevel = Plib::DEFAULT_TEXT_REQUIREDCMD );
void broadcast_unicode( const std::string& text, const std::string& lang,
                        unsigned short font = Plib::DEFAULT_TEXT_FONT,
                        unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                        unsigned short requiredCmdLevel = Plib::DEFAULT_TEXT_REQUIREDCMD );
bool say_above( const UObject* obj, const char* text, unsigned short font = Plib::DEFAULT_TEXT_FONT,
                unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                unsigned int journal_print = JOURNAL_PRINT_NAME );
bool say_above_unicode( const UObject* obj, const std::string& text, const std::string& lang,
                        unsigned short font = Plib::DEFAULT_TEXT_FONT,
                        unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                        unsigned int journal_print = JOURNAL_PRINT_NAME );

bool private_say_above( Mobile::Character* chr, const UObject* obj, const char* text,
                        unsigned short font = Plib::DEFAULT_TEXT_FONT,
                        unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                        unsigned int journal_print = JOURNAL_PRINT_NAME );
bool private_say_above_unicode( Mobile::Character* chr, const UObject* obj, const std::string& text,
                                const std::string& lang,
                                unsigned short font = Plib::DEFAULT_TEXT_FONT,
                                unsigned short color = Plib::DEFAULT_TEXT_COLOR,
                                unsigned int journal_print = JOURNAL_PRINT_NAME );

Items::Item* copy_item( const Items::Item* src_item );
void update_all_weatherregions();

bool send_menu( Network::Client* client, Menu* menu );
void send_death_message( Mobile::Character* chr_died, Items::Item* corpse );
void send_container_contents( Network::Client* client, const UContainer& cont );

void send_char_data( Network::Client* client, Mobile::Character* chr );

void transmit_to_inrange( const UObject* center, const void* msg, unsigned msglen );
void transmit_to_others_inrange( Mobile::Character* center, const void* msg, unsigned msglen );

void destroy_item( Items::Item* item );

void move_item( Items::Item* item, Core::UFACING facing );
void move_item( Items::Item* item, unsigned short newx, unsigned short newy, signed char newz,
                Realms::Realm* oldrealm );
void move_item( Items::Item* item, const Core::Pos4d& newpos );

void send_char_if_newly_inrange( Mobile::Character* chr, Network::Client* client );
void send_item_if_newly_inrange( Items::Item* item, Network::Client* client );
void send_objects_newly_inrange( Network::Client* client );
void send_objects_newly_inrange_on_boat( Network::Client* client, u32 serial );

void send_create_mobile_if_nearby_cansee( Network::Client* client, const Mobile::Character* chr );
void send_create_mobile_to_nearby_cansee( const Mobile::Character* chr );

void send_move_mobile_to_nearby_cansee( const Mobile::Character* chr );

void send_remove( Network::Client& client, UObject& to_remove );

void send_remove_character_to_nearby_cansee( const Mobile::Character* chr );

void send_open_gump( Network::Client* client, const UContainer& cont );

void send_multis_newly_inrange( Multi::UMulti* multi, Network::Client* client );

void register_with_supporting_multi( Items::Item* item );

Mobile::Character* UpdateCharacterWeight( Items::Item* item );
void UpdateCharacterOnDestroyItem( Items::Item* item );
bool clientHasCharacter( Network::Client* c );
void login_complete( Network::Client* c );
void send_feature_enable( Network::Client* client );
void send_realm_change( Network::Client* client, Realms::Realm* realm );
void send_map_difs( Network::Client* client );
void send_season_info( Network::Client* client );
void send_new_subserver( Network::Client* client );
void send_fight_occuring( Network::Client* client, Mobile::Character* opponent );
void send_damage( Mobile::Character* attacker, Mobile::Character* defender, u16 damage );
void sendCharProfile( Mobile::Character* chr, Mobile::Character* of_who, const std::string& title,
                      const std::string& utext, const std::string& etext );

void send_buff_message( Mobile::Character* chr, u16 icon, bool show, u16 duration = 0,
                        u32 cl_name = 0, u32 cl_descr = 0,
                        const std::string& arguments = std::string() );
}  // namespace Core
}  // namespace Pol
#endif

/*
History
=======
2006/05/07 Shinigami: SendAOSTooltip - will now send merchant_description() if needed
2009/07/26 MuadDib:   updates for new Packet Refactoring
2009/07/31 Turley:    added send_fight_occuring() for packet 0x2F
2009/08/01 MuadDib:   Removed send_tech_stuff(), unused and obsolete.
2009/08/09 MuadDib:   Refactor of Packet 0x25 for naming convention
2009/09/22 Turley:    Added DamagePacket support

Notes
=======

*/

#ifndef __UFUNC_H
#define __UFUNC_H

#include <stddef.h>

#include "../clib/rawtypes.h"

#include "uconst.h"
#include "pktout.h"
#include "network/packets.h"

#include "core.h"
namespace Pol {
  namespace Network {
	class Client;
  }
  namespace Mobile {
	class Character;
  }
  namespace Items {
	class Item;
  }
  namespace Multi {
	class UMulti;
  }
  namespace Plib {
	class Realm;
  }

  namespace Core {
	class UContainer;
	class NPC;
	class UObject;
	struct USTRUCT_TILE;
	class Menu;
	enum UFACING;


	const u32 CHARACTERSERIAL_START = 0x00000001Lu;
	const u32 CHARACTERSERIAL_END = 0x3FFFFFFFLu;
	const u32 ITEMSERIAL_START = 0x40000000Lu;
	const u32 ITEMSERIAL_END = 0x7FffFFffLu;

	extern u32 itemserialnumber;
	extern u32 nonsaveditemserialnumber;
	extern u32 charserialnumber;

	void SetCurrentItemSerialNumber( u32 serial );
	void SetCurrentCharSerialNumber( u32 serial );
	u32 GetNextSerialNumber();
	u32 GetCurrentItemSerialNumber();
	u32 GetCurrentCharSerialNumber();
	u32 UseCharSerialNumber( u32 serial );

	u32 GetNewItemSerialNumber();
	u32 UseItemSerialNumber( u32 serial );

	void send_login_error( Network::Client *client, unsigned char reason );

	bool inrange( const Mobile::Character* c1, const Mobile::Character* c2 );
	bool inrange( const Mobile::Character* c1, const UObject* obj );
	bool inrange( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 );

	bool inrangex( const Mobile::Character* c1, const Mobile::Character* c2, int maxdist );
	bool inrangex( const UObject *c1, unsigned short x, unsigned short y, int maxdist );

	bool multi_inrange( const Mobile::Character *c1, const Multi::UMulti *obj );
	bool multi_inrange( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 );

	unsigned short pol_distance( const Mobile::Character* c1, const UObject* obj );
	unsigned short pol_distance( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 );

	bool in_say_range( const Mobile::Character *c1, const Mobile::Character *c2 );
	bool in_yell_range( const Mobile::Character *c1, const Mobile::Character *c2 );
	bool in_whisper_range( const Mobile::Character *c1, const Mobile::Character *c2 );

	void send_owncreate( Network::Client *client, const Mobile::Character *chr );
	void send_owncreate( Network::Client *client, const Mobile::Character *chr, Network::PktOut_78* owncreate, Network::PktOut_17* poisonbuffer, Network::PktOut_17* invulbuffer );
	void build_owncreate( const Mobile::Character *chr, Network::PktOut_78* msg );

	void send_item( Network::Client *client, const Items::Item *item );
	void send_wornitem( Network::Client *client, const Mobile::Character *chr, const Items::Item *item );

	void send_move( Network::Client *client, const Mobile::Character *chr );
	void send_move( Network::Client *client, const Mobile::Character *chr, Network::PktOut_77* movebuffer, Network::PktOut_17* poisonbuffer, Network::PktOut_17* invulbuffer );
	void build_send_move( const Mobile::Character *chr, Network::PktOut_77* msg );
	void send_objdesc( Network::Client *client, Items::Item *item );

	void send_poisonhealthbar( Network::Client *client, const Mobile::Character *chr );
	void send_invulhealthbar( Network::Client* client, const Mobile::Character *chr );
	void build_poisonhealthbar( const Mobile::Character *chr, Network::PktOut_17* msg );
	void build_invulhealthbar( const Mobile::Character *chr, Network::PktOut_17* msg );

	void send_item_to_inrange( const Items::Item *item );
	void update_item_to_inrange( const Items::Item* item );
	void send_put_in_container( Network::Client* client, const Items::Item* item );
	void send_put_in_container_to_inrange( const Items::Item *item );
	void send_wornitem_to_inrange( const Mobile::Character *chr, const Items::Item *item );
	void update_wornitem_to_inrange( const Mobile::Character *chr, const Items::Item *item );

	void send_midi( Network::Client* client, unsigned short midi );
	Mobile::Character *find_character( u32 serial );

	UObject* find_toplevel_object( u32 serial );
	void setrealm( Items::Item* item, void* arg );
	void setrealmif( Items::Item* item, void* arg );

	void send_remove_character( Network::Client *client, const Mobile::Character *chr, Network::PktOut_1D* buffer = nullptr, bool build = true );
	void send_remove_object_if_inrange( Network::Client *client, const Items::Item *obj );
	void send_remove_object_to_inrange( const UObject *centerObject );
    void send_remove_object( Network::Client *client, Network::PktOut_1D* buffer );
    void send_remove_object( Network::Client *client, const UObject *item, Network::PktOut_1D* buffer );
    void send_remove_object( Network::Client *client, const UObject *obj );

	void send_remove_character_to_nearby( const Mobile::Character* chr );
	void send_remove_character_to_nearby_cantsee( const Mobile::Character* chr );
	void send_char_data_to_nearby_ghosts( Mobile::Character* chr );

	void send_goxyz( Network::Client *client, const Mobile::Character *chr );
	void send_light( Network::Client *client, int lightlevel );
	void send_weather( Network::Client *client, unsigned char type, unsigned char severity, unsigned char aux );
	void send_mode( Network::Client *client );
	void send_item_move_failure( Network::Client *client, u8 reason );
	u16 convert_objtype_to_spellnum( u32 objtype, int school );

	void send_stamina_level( Network::Client *client );
	void send_mana_level( Network::Client *client );

	UContainer *find_legal_container( const Mobile::Character *chr, u32 serial );
	bool is_a_parent( const Items::Item *item, u32 serial );
	void play_sound_effect( const UObject *center, u16 effect );
	void play_sound_effect_private( const UObject *center, u16 effect, Mobile::Character* forchr );
	void play_sound_effect_xyz( u16 cx, u16 cy, s8 cz, u16 effect, Plib::Realm* realm );
	void play_lightning_bolt_effect( const UObject* center );
	void play_moving_effect( const UObject *src, const UObject *dst,
							 u16 effect,
							 u8 speed,
							 u8 loop,
							 u8 explode );
	void play_moving_effect2( u16 xs, u16 ys, s8 zs,
							  u16 xd, u16 yd, s8 zd,
							  u16 effect,
							  u8 speed,
							  u8 loop,
							  u8 explode,
							  Plib::Realm* realm );
	void play_object_centered_effect( const UObject* center,
									  u16 effect,
									  u8 speed,
									  u8 loop );

	void play_stationary_effect( u16 x, u16 y, s8 z, u16 effect, u8 speed, u8 loop, u8 explode, Plib::Realm* realm );

	void play_stationary_effect_ex( u16 x, u16 y, s8 z, Plib::Realm* realm, u16 effect, u8 speed, u8 duration, u32 hue,
									u32 render, u16 effect3d );
	void play_object_centered_effect_ex( const UObject* center, u16 effect, u8 speed, u8 duration, u32 hue,
										 u32 render, u8 layer, u16 effect3d );
	void play_moving_effect_ex( const UObject *src, const UObject *dst,
								u16 effect, u8 speed, u8 duration, u32 hue,
								u32 render, u8 direction, u8 explode, u16 effect3d, u16 effect3dexplode, u16 effect3dsound );
	void play_moving_effect2_ex( u16 xs, u16 ys, s8 zs,
								 u16 xd, u16 yd, s8 zd, Plib::Realm* realm,
								 u16 effect, u8 speed, u8 duration, u32 hue,
								 u32 render, u8 direction, u8 explode, u16 effect3d, u16 effect3dexplode, u16 effect3dsound );
	void partical_effect( Network::PktOut_C7* msg, u8 type, u32 srcserial, u32 dstserial,
						  u16 srcx, u16 srcy, s8 srcz,
						  u16 dstx, u16 dsty, s8 dstz,
						  u16 effect, u8 speed, u8 duration, u8 direction,
						  u8 explode, u32 hue, u32 render,
						  u16 effect3d, u16 effect3dexplode, u16 effect3dsound,
						  u32 itemid, u8 layer );


	// find_legal_item: search worn items, including backpack recursively, and
	// items on the ground, recursively, for an item of a given serial.
	Items::Item *find_legal_item( const Mobile::Character *chr, u32 serial, bool* additlegal = NULL, bool* isRemoteContainer = NULL );

	const unsigned short DEFAULT_TEXT_FONT = 3;
	const unsigned short DEFAULT_TEXT_COLOR = 0x3B2;

	void send_sysmessage( Network::Client *client,
						  const char *text,
						  unsigned short font = DEFAULT_TEXT_FONT,
						  unsigned short color = DEFAULT_TEXT_COLOR );
	void send_sysmessage( Network::Client *client,
						  const u16 *wtext, const char lang[4],
						  unsigned short font = DEFAULT_TEXT_FONT,
						  unsigned short color = DEFAULT_TEXT_COLOR );
	void broadcast( const char *text,
					unsigned short font = DEFAULT_TEXT_FONT,
					unsigned short color = DEFAULT_TEXT_COLOR );
	void broadcast( const u16 *wtext, const char lang[4],
					unsigned short font = DEFAULT_TEXT_FONT,
					unsigned short color = DEFAULT_TEXT_COLOR );
	bool say_above( const UObject* obj,
					const char* text,
					unsigned short font = DEFAULT_TEXT_FONT,
					unsigned short color = DEFAULT_TEXT_COLOR,
					unsigned int journal_print = JOURNAL_PRINT_NAME );
	bool say_above( const UObject* obj,
					const u16 *wtext, const char lang[4],
					unsigned short font = DEFAULT_TEXT_FONT,
					unsigned short color = DEFAULT_TEXT_COLOR,
					unsigned int journal_print = JOURNAL_PRINT_NAME );

	bool private_say_above( Mobile::Character* chr,
							const UObject* obj,
							const char *text,
							unsigned short font = DEFAULT_TEXT_FONT,
							unsigned short color = DEFAULT_TEXT_COLOR,
							unsigned int journal_print = JOURNAL_PRINT_NAME );
	bool private_say_above( Mobile::Character* chr,
							const UObject* obj,
							const u16 *wtext, const char lang[4],
							unsigned short font = DEFAULT_TEXT_FONT,
							unsigned short color = DEFAULT_TEXT_COLOR,
							unsigned int journal_print = JOURNAL_PRINT_NAME );

	Items::Item *copy_item( const Items::Item *src_item );
	void update_all_weatherregions();

	bool send_menu( Network::Client *client, Menu *menu );
	void send_death_message( Mobile::Character *chr_died, Items::Item *corpse );
	void send_container_contents( Network::Client *client, const UContainer& cont, bool show_invis = false );

	void send_char_data( Network::Client *client, Mobile::Character *chr );

	void for_nearby_npcs( void( *f )( NPC& npc, Mobile::Character *chr, const char *text, int textlen, u8 texttype ),
						  Mobile::Character *p_chr, const char *p_text, int p_textlen, u8 texttype );
	void for_nearby_npcs( void( *f )( NPC& npc, Mobile::Character *chr, const char *text, int textlen, u8 texttype, const u16 *wtext, const char lang[4], int wtextlen, Bscript::ObjArray* speechtokens ),
						  Mobile::Character *p_chr, const char *p_text, int p_textlen, u8 texttype, const u16 *p_wtext, const char p_lang[4], int p_wtextlen, Bscript::ObjArray* speechtokens = NULL );

	void transmit_to_inrange( const UObject* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR );
	void transmit_to_others_inrange( Mobile::Character* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR );

	void destroy_item( Items::Item* item );
	void world_delete( UObject* uobj );

	void move_item( Items::Item* item, UFACING facing );
	void move_item( Items::Item* item, unsigned short newx, unsigned short newy, signed char newz, Plib::Realm* oldrealm );

	void update_nearby_visible_chars( Network::Client* client ); 
	void send_char_if_newly_inrange( Mobile::Character *chr, Network::Client *client );
	void send_item_if_newly_inrange( Items::Item *item, Network::Client *client );
	void send_objects_newly_inrange( Network::Client* client );
	void send_objects_newly_inrange_on_boat( Network::Client* client, u32 serial );
	void remove_objects_inrange( Network::Client* client );

	void send_create_mobile_if_nearby_cansee( Network::Client* client, const Mobile::Character* chr );
	void send_create_mobile_to_nearby_cansee( const Mobile::Character* chr );

	void send_move_mobile_if_nearby_cansee( Network::Client* client, const Mobile::Character* chr );
	void send_move_mobile_to_nearby_cansee( const Mobile::Character* chr );

	void send_remove( Network::Client& client, UObject& to_remove );

	void send_remove_character_to_nearby_cansee( const Mobile::Character* chr );

	void send_open_gump( Network::Client *client, const UContainer& cont );

	void send_multis_newly_inrange( Multi::UMulti *multi, Network::Client *client );

	Mobile::Character* chr_from_wornitems( UContainer* wornitems );

	void register_with_supporting_multi( Items::Item* item );

	Mobile::Character* UpdateCharacterWeight( Items::Item* item );
	void UpdateCharacterOnDestroyItem( Items::Item* item );
	bool clientHasCharacter( Network::Client* c );
	void login_complete( Network::Client* c );
	void send_feature_enable( Network::Client* client );
	void send_realm_change( Network::Client* client, Plib::Realm* realm );
	void send_map_difs( Network::Client* client );
	void send_season_info( Network::Client* client );
	void send_new_subserver( Network::Client* client );
	void send_fight_occuring( Network::Client* client, Mobile::Character* opponent );
	void send_damage( Mobile::Character* attacker, Mobile::Character* defender, u16 damage );
	void send_damage_old( Network::Client* client, Mobile::Character* defender, u16 damage );
	void send_damage_new( Network::Client* client, Mobile::Character* defender, u16 damage );
	void sendCharProfile( Mobile::Character* chr, Mobile::Character* of_who, const char *title, const u16 *utext, const u16 *etext );
  }
}
#endif

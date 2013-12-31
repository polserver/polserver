/*
History
=======
2005/03/05 Shinigami: format_description -> ServSpecOpt UseAAnTileFlags to enable/disable "a"/"an" via Tiles.cfg in formatted Item Names
2005/04/03 Shinigami: send_feature_enable - added UOExpansionFlag for Samurai Empire
2005/08/29 Shinigami: ServSpecOpt UseAAnTileFlags renamed to UseTileFlagPrefix
2005/09/17 Shinigami: send_nametext - smaller bugfix in passert-check
2006/05/07 Shinigami: SendAOSTooltip - will now send merchant_description() if needed
2006/05/16 Shinigami: send_feature_enable - added UOExpansionFlag for Mondain's Legacy
2007/04/08 MuadDib:   Updated send_worn_item_to_inrange to create the message only
once and use the Transmit_to_inrange instead. Then uses
send_object_cache_to_inrange. Required a tooltips.* update.
2008/07/08 Turley:    get_flag1() changed to show WarMode of other player again
get_flag1_aos() removed
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/07/31 Turley:    added send_fight_occuring() for packet 0x2F
2009/08/01 MuadDib:   Removed send_tech_stuff(), unused and obsolete.
2009/08/09 MuadDib:   UpdateCharacterWeight() Rewritten to send stat msg intead of refreshar().
Refactor of Packet 0x25 for naming convention
2009/08/14 Turley:    PKTOUT_B9_V2 removed unk u16 and changed flag to u32
2009/09/03 MuadDib:   Relocation of account related cpp/h
Relocation of multi related cpp/h
2009/09/13 MuadDib:   Optimized send_remove_character_to_nearby_cansee, send_remove_character_to_nearby_cantsee, send_remove_character_to_nearby
to build packet and handle iter internally. Packet built just once this way, and sent to those who need it.
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
2009/09/22 MuadDib:   Adding resending of light level if override not in effect, to sending of season packet. Fixes EA client bug.
2009/09/22 Turley:    Added DamagePacket support
2009/10/07 Turley:    Fixed DestroyItem while in hand
2009/10/12 Turley:    whisper/yell/say-range ssopt definition
2009/12/02 Turley:    0xf3 packet support - Tomi
face support
2009/12/03 Turley:    added 0x17 packet everywhere only send if poisoned, fixed get_flag1 (problem with poisoned & flying)

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <assert.h>
#include <stddef.h>

#include "../clib/clib.h"
#include "../clib/endian.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"
#include "../clib/pkthelper.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../plib/mapcell.h"
#include "../plib/realm.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "item/equipmnt.h"
#include "fnsearch.h"
#include "layers.h"
#include "lightlvl.h"
#include "objtype.h"
#include "pktout.h"
#include "polcfg.h"
#include "polclass.h"
#include "realms.h"
#include "repsys.h"
#include "sockio.h"
#include "ssopt.h"
#include "statmsg.h"
#include "tmpvars.h"
#include "tooltips.h"
#include "uconst.h"
#include "ufunc.h"
#include "multi/multi.h"
#include "uobject.h"
#include "uoclient.h"
#include "uofile.h"
#include "ustruct.h"
#include "uvars.h"
#include "uworld.h"
#include "mdelta.h"
#include "multi/multi.h"
#include "../bscript/impstr.h"
#include "zone.h"
#include "miscrgn.h"
#include "network/cgdata.h"

#include "objecthash.h"
namespace Pol {
  namespace Core {
    using namespace Network;
    using namespace Mobile;
    using namespace Items;

	u32 itemserialnumber = ITEMSERIAL_START;
	u32 charserialnumber = CHARACTERSERIAL_START;

	//Dave added 3/9/3
	void SetCurrentItemSerialNumber( u32 serial )
	{
	  itemserialnumber = serial;
	}

	//Dave added 3/9/3
	void SetCurrentCharSerialNumber( u32 serial )
	{
	  charserialnumber = serial;
	}

	//Dave added 3/8/3
	u32 GetCurrentItemSerialNumber( void )
	{
	  return itemserialnumber;
	}

	//Dave added 3/8/3
	u32 GetCurrentCharSerialNumber( void )
	{
	  return charserialnumber;
	}

	//Dave changed 3/8/3 to use objecthash
	u32 GetNextSerialNumber( void )
	{
	  u32 nextserial = objecthash.GetNextUnusedCharSerial();
	  charserialnumber = nextserial;
	  return charserialnumber;
	}

	u32 UseCharSerialNumber( u32 serial )
	{
	  if ( serial > charserialnumber )
		charserialnumber = serial + 1;
	  return serial;
	}

	//Dave changed 3/8/3
	u32 UseItemSerialNumber( u32 serial )
	{
	  if ( serial > itemserialnumber )
		itemserialnumber = serial + 1;
	  return serial;
	}

	//Dave changed 3/8/3 to use objecthash
	u32 GetNewItemSerialNumber( void )
	{
	  u32 nextserial = objecthash.GetNextUnusedItemSerial();
	  itemserialnumber = nextserial;
	  return itemserialnumber;
	}

	void send_goxyz( Client *client, const Character *chr )
	{
	  PktHelper::PacketOut<PktOut_20> msg;
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( chr->graphic );
	  msg->offset++; //unk7
	  msg->WriteFlipped<u16>( chr->color );
	  msg->Write<u8>( chr->get_flag1( client ) );
	  msg->WriteFlipped<u16>( chr->x );
	  msg->WriteFlipped<u16>( chr->y );
	  msg->offset += 2; //unk15,16
	  msg->Write<u8>( static_cast<u8>( 0x80 | chr->facing ) ); // is it always right to set this flag?
	  msg->Write<s8>( chr->z );
	  msg.Send( client );

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->poisoned ) ) //if poisoned send 0x17 for newer clients
		send_poisonhealthbar( client, chr );

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->invul() ) ) //if invul send 0x17 for newer clients
		send_invulhealthbar( client, chr );
	}

	// Character chr has moved.  Tell a client about it.
	// FIXME: Use of this packet needs optimized. Other functions for
	// sending to in range etc, call this function for each client.
	// Even with the independant flags, we should be able to
	// optimize this out to reduce build amounts
	void send_move( Client *client, const Character *chr )
	{
	  PktHelper::PacketOut<PktOut_77> msg;
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( chr->graphic );
	  msg->WriteFlipped<u16>( chr->x );
	  msg->WriteFlipped<u16>( chr->y );
	  msg->Write<s8>( chr->z );
	  msg->Write<u8>( static_cast<u8>( ( chr->dir & 0x80 ) | chr->facing ) );// NOTE, this only includes mask 0x07 of the last MOVE message 
	  msg->WriteFlipped<u16>( chr->color );
	  msg->Write<u8>( chr->get_flag1( client ) );
	  msg->Write<u8>( chr->hilite_color_idx( client->chr ) );
	  msg.Send( client );

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->poisoned ) ) //if poisoned send 0x17 for newer clients
		send_poisonhealthbar( client, chr );

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->invul() ) ) //if invul send 0x17 for newer clients
		send_invulhealthbar( client, chr );
	}

	void send_move( Client *client, const Character *chr, PktOut_77* movebuffer, PktOut_17* poisonbuffer, PktOut_17* invulbuffer )
	{
	  movebuffer->offset = 15;
	  movebuffer->Write<u8>( chr->get_flag1( client ) );
	  movebuffer->Write<u8>( chr->hilite_color_idx( client->chr ) );
	  ADDTOSENDQUEUE( client, &movebuffer->buffer, movebuffer->offset );
	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->poisoned ) ) //if poisoned send 0x17 for newer clients
		ADDTOSENDQUEUE( client, &poisonbuffer->buffer, poisonbuffer->offset );
	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->invul() ) ) //if invul send 0x17 for newer clients
		ADDTOSENDQUEUE( client, &invulbuffer->buffer, invulbuffer->offset );
	}

	void build_send_move( const Character *chr, PktOut_77* msg )
	{
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( chr->graphic );
	  msg->WriteFlipped<u16>( chr->x );
	  msg->WriteFlipped<u16>( chr->y );
	  msg->Write<s8>( chr->z );
	  msg->Write<u8>( static_cast<u8>( ( chr->dir & 0x80 ) | chr->facing ) );// NOTE, this only includes mask 0x07 of the last MOVE message 
	  msg->WriteFlipped<u16>( chr->color );
	}

	void send_poisonhealthbar( Client *client, const Character *chr )
	{
	  PktHelper::PacketOut<PktOut_17> msg;
	  msg->WriteFlipped<u16>( static_cast<u16>( sizeof msg->buffer ) );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( 1 ); //unk
	  msg->WriteFlipped<u16>( 1 ); // 1 = Green, 2 = Yellow
	  msg->Write<u8>( ( chr->poisoned ) ? 1 : 0 ); //flag
	  msg.Send( client );
	}

	void send_invulhealthbar( Client *client, const Character *chr )
	{
	  PktHelper::PacketOut<PktOut_17> msg;
	  msg->WriteFlipped<u16>( static_cast<u16>( sizeof msg->buffer ) );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( 1 ); //unk
	  msg->WriteFlipped<u16>( 2 ); // 1 = Green, 2 = Yellow
	  msg->Write<u8>( ( chr->invul() ) ? 1 : 0 ); //flag
	  msg.Send( client );
	}

	void build_poisonhealthbar( const Character *chr, PktOut_17* msg )
	{
	  msg->WriteFlipped<u16>( static_cast<u16>( sizeof msg->buffer ) );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( 1 ); //unk
	  msg->WriteFlipped<u16>( 1 ); // 1 = Green, 2 = Yellow
	  msg->Write<u8>( ( chr->poisoned ) ? 1 : 0 ); //flag
	}

	void build_invulhealthbar( const Character *chr, PktOut_17* msg )
	{
	  msg->WriteFlipped<u16>( static_cast<u16>( sizeof msg->buffer ) );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( 1 ); //unk
	  msg->WriteFlipped<u16>( 2 ); // 1 = Green, 2 = Yellow
	  msg->Write<u8>( ( chr->invul() ) ? 1 : 0 ); //flag
	}

	void send_owncreate( Client *client, const Character *chr )
	{
	  PktHelper::PacketOut<PktOut_78> owncreate;
	  owncreate->offset += 2;
	  owncreate->Write<u32>( chr->serial_ext );
	  owncreate->WriteFlipped<u16>( chr->graphic );
	  owncreate->WriteFlipped<u16>( chr->x );
	  owncreate->WriteFlipped<u16>( chr->y );
	  owncreate->Write<s8>( chr->z );
	  owncreate->Write<u8>( chr->facing );
	  owncreate->WriteFlipped<u16>( chr->color );
	  owncreate->Write<u8>( chr->get_flag1( client ) );
	  owncreate->Write<u8>( chr->hilite_color_idx( client->chr ) );

	  for ( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
	  {
		const Item *item = chr->wornitem( layer );
		if ( item == NULL )
		  continue;

		// Dont send faces if older client or ssopt
		if ( ( layer == LAYER_FACE ) && ( ( ssopt.support_faces == 0 ) || ( ~client->ClientType & CLIENTTYPE_UOKR ) ) )
		  continue;

		if ( item->color )
		{
		  owncreate->Write<u32>( item->serial_ext );
		  owncreate->WriteFlipped<u16>( static_cast<u16>( 0x8000 | item->graphic ) );
		  owncreate->Write<u8>( static_cast<u8>( layer ) );
		  owncreate->WriteFlipped<u16>( item->color );
		}
		else
		{
		  owncreate->Write<u32>( item->serial_ext );
		  owncreate->WriteFlipped<u16>( item->graphic );
		  owncreate->Write<u8>( static_cast<u8>( layer ) );
		}
	  }
	  owncreate->offset += 4; //items nullterm
	  u16 len = owncreate->offset;
	  owncreate->offset = 1;
	  owncreate->WriteFlipped<u16>( len );

	  owncreate.Send( client, len );

	  if ( client->UOExpansionFlag & AOS )
	  {
		send_object_cache( client, dynamic_cast<const UObject*>( chr ) );
		// 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send revision
		for ( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
		  const Item *item = chr->wornitem( layer );
		  if ( item == NULL )
			continue;
		  if ( layer == LAYER_FACE )
			continue;
		  send_object_cache( client, dynamic_cast<const UObject*>( item ) );
		}
	  }

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->poisoned ) ) //if poisoned send 0x17 for newer clients
		send_poisonhealthbar( client, chr );

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->invul() ) ) //if invul send 0x17 for newer clients
		send_invulhealthbar( client, chr );
	}

	void build_owncreate( const Character *chr, PktOut_78* owncreate )
	{
	  owncreate->offset += 2;
	  owncreate->Write<u32>( chr->serial_ext );
	  owncreate->WriteFlipped<u16>( chr->graphic );
	  owncreate->WriteFlipped<u16>( chr->x );
	  owncreate->WriteFlipped<u16>( chr->y );
	  owncreate->Write<s8>( chr->z );
	  owncreate->Write<u8>( chr->facing );
	  owncreate->WriteFlipped<u16>( chr->color );//17
	}
	void send_owncreate( Client *client, const Character *chr, PktOut_78* owncreate, PktOut_17* poisonbuffer, PktOut_17* invulbuffer )
	{
	  owncreate->offset = 17;
	  owncreate->Write<u8>( chr->get_flag1( client ) );
	  owncreate->Write<u8>( chr->hilite_color_idx( client->chr ) );

	  for ( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
	  {
		const Item *item = chr->wornitem( layer );
		if ( item == NULL )
		  continue;

		// Dont send faces if older client or ssopt
		if ( ( layer == LAYER_FACE ) && ( ( ssopt.support_faces == 0 ) || ( ~client->ClientType & CLIENTTYPE_UOKR ) ) )
		  continue;

		if ( item->color )
		{
		  owncreate->Write<u32>( item->serial_ext );
		  owncreate->WriteFlipped<u16>( static_cast<u16>( 0x8000 | item->graphic ) );
		  owncreate->Write<u8>( static_cast<u8>( layer ) );
		  owncreate->WriteFlipped<u16>( item->color );
		}
		else
		{
		  owncreate->Write<u32>( item->serial_ext );
		  owncreate->WriteFlipped<u16>( item->graphic );
		  owncreate->Write<u8>( static_cast<u8>( layer ) );
		}
	  }
	  owncreate->offset += 4; //items nullterm
	  u16 len = owncreate->offset;
	  owncreate->offset = 1;
	  owncreate->WriteFlipped<u16>( len );

	  ADDTOSENDQUEUE( client, &owncreate->buffer, len );

	  if ( client->UOExpansionFlag & AOS )
	  {
		send_object_cache( client, dynamic_cast<const UObject*>( chr ) );
		// 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send revision
		for ( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
		  const Item *item = chr->wornitem( layer );
		  if ( item == NULL )
			continue;
		  if ( layer == LAYER_FACE )
			continue;
		  send_object_cache( client, dynamic_cast<const UObject*>( item ) );
		}
	  }

	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->poisoned ) ) //if poisoned send 0x17 for newer clients
		ADDTOSENDQUEUE( client, &poisonbuffer->buffer, poisonbuffer->offset );
	  if ( ( client->ClientType & CLIENTTYPE_UOKR ) && ( chr->invul() ) ) //if invul send 0x17 for newer clients
		ADDTOSENDQUEUE( client, &invulbuffer->buffer, invulbuffer->offset );
	}

	void send_remove_character( Client *client, const Character *chr, PktOut_1D* buffer, bool build )
	{
	  if ( !client->ready )     /* if a client is just connecting, don't bother him. */
		return;

	  /* Don't remove myself */
	  if ( client->chr == chr )
		return;

      if ( buffer == nullptr )
      {
        PktHelper::PacketOut<PktOut_1D> msgremove;
        msgremove->Write<u32>( chr->serial_ext );
        msgremove.Send( client );
      }
      else
      {
        if ( build )
        {
          buffer->offset = 1;
          buffer->Write<u32>( chr->serial_ext );
        }

        ADDTOSENDQUEUE( client, &buffer->buffer, buffer->offset );
      }
	}

	void send_remove_character_to_nearby( const Character* chr )
	{
	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( chr->serial_ext );
      ForEachPlayerInVisualRange( chr, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( zonechr == chr )
          return;
        msgremove.Send( zonechr->client );
      } );
	}

	void send_remove_character_to_nearby_cantsee( const Character* chr )
	{
	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( chr->serial_ext );

      ForEachPlayerInVisualRange( chr, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( zonechr == chr )
          return;
        if ( !zonechr->is_visible_to_me( chr ) )
          msgremove.Send( zonechr->client );
      } );
	}

    void send_remove_character_to_nearby_cansee( const Character* chr )
	{
	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( chr->serial_ext );
      ForEachPlayerInVisualRange( chr, [&]( Character* _chr )
      {
        if ( _chr->has_active_client() &&
             _chr != chr &&
             _chr->is_visible_to_me( chr ) )
             msgremove.Send( _chr->client );
      } );
	}

	void send_remove_object_if_inrange( Client *client, const Item *item )
	{
	  if ( !client->ready )     /* if a client is just connecting, don't bother him. */
		return;

	  if ( !inrange( client->chr, item ) )
		return;

	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( item->serial_ext );
	  msgremove.Send( client );
	}

    void send_remove_object( Client* client, const UObject *object )
    {
      if ( client == NULL || !client->ready )
        return;
      PktHelper::PacketOut<PktOut_1D> msgremove;
      msgremove->Write<u32>( object->serial_ext );
      msgremove.Send( client );
    }

	void send_remove_object_to_inrange( const UObject *centerObject )
	{
	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( centerObject->serial_ext );

      ForEachPlayerInVisualRange( centerObject, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        // FIXME need to check character's additional_legal_items.
        msgremove.Send( zonechr->client );
      } );
	}

    void send_remove_object( Client *client, PktOut_1D* buffer )
    {
      if ( client == NULL || !client->ready )     /* if a client is just connecting, don't bother him. */
        return;
      ADDTOSENDQUEUE( client, &buffer->buffer, buffer->offset );
    }

    void send_remove_object( Client *client, const UObject *item, PktOut_1D* buffer )
	{
      if ( !client->ready )     /* if a client is just connecting, don't bother him. */
        return;
      buffer->offset = 1;
      buffer->Write<u32>( item->serial_ext );
      ADDTOSENDQUEUE( client, &buffer->buffer, buffer->offset );
	}

	bool inrangex( const Character *c1, const Character *c2, int maxdist )
	{
	  return ( ( c1->realm == c2->realm ) &&
			   ( abs( c1->x - c2->x ) <= maxdist ) &&
			   ( abs( c1->y - c2->y ) <= maxdist ) );
	}

	bool inrangex( const UObject *c1, unsigned short x, unsigned short y, int maxdist )
	{
	  return ( ( abs( c1->x - x ) <= maxdist ) &&
			   ( abs( c1->y - y ) <= maxdist ) );
	}

	bool inrange( const UObject *c1, unsigned short x, unsigned short y )
	{
	  return ( ( abs( c1->x - x ) <= RANGE_VISUAL ) &&
			   ( abs( c1->y - y ) <= RANGE_VISUAL ) );
	}

    bool inrange( const Mobile::Character *c1, const Mobile::Character *c2 )
	{
	  // note, these are unsigned.  abs converts to signed, so everything _should_ be okay.
	  return ( ( c1->realm == c2->realm ) &&
			   ( abs( c1->x - c2->x ) <= RANGE_VISUAL ) &&
			   ( abs( c1->y - c2->y ) <= RANGE_VISUAL ) );
	}

    bool inrange( const Mobile::Character *c1, const UObject *obj )
	{
	  obj = obj->toplevel_owner();

	  return ( ( c1->realm == obj->realm ) &&
			   ( abs( c1->x - obj->x ) <= RANGE_VISUAL ) &&
			   ( abs( c1->y - obj->y ) <= RANGE_VISUAL ) );
	}

    bool multi_inrange( const Mobile::Character *c1, const Multi::UMulti *obj )
	{
	  return ( ( c1->realm == obj->realm ) &&
			   ( abs( c1->x - obj->x ) <= RANGE_VISUAL_LARGE_BUILDINGS ) &&
			   ( abs( c1->y - obj->y ) <= RANGE_VISUAL_LARGE_BUILDINGS ) );
	}

	unsigned short pol_distance( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 )
	{
	  int xd = abs( x1 - x2 );
	  int yd = abs( y1 - y2 );
	  if ( xd > yd )
		return static_cast<unsigned short>( xd );
	  else
		return static_cast<unsigned short>( yd );
	}

    unsigned short pol_distance( const Mobile::Character* c1, const UObject* obj )
	{
	  obj = obj->toplevel_owner();

	  int xd = abs( c1->x - obj->x );
	  int yd = abs( c1->y - obj->y );
	  if ( xd > yd )
		return static_cast<unsigned short>( xd );
	  else
		return static_cast<unsigned short>( yd );
	}

	bool in_say_range( const Character *c1, const Character *c2 )
	{
	  return inrangex( c1, c2, ssopt.speech_range );
	}
	bool in_yell_range( const Character *c1, const Character *c2 )
	{
	  return inrangex( c1, c2, ssopt.yell_range );
	}
	bool in_whisper_range( const Character *c1, const Character *c2 )
	{
	  return inrangex( c1, c2, ssopt.whisper_range );
	}

	bool inrange( unsigned short x1, unsigned short y1,
				  unsigned short x2, unsigned short y2 )
	{
	  return ( ( abs( x1 - x2 ) <= RANGE_VISUAL ) &&
			   ( abs( y1 - y2 ) <= RANGE_VISUAL ) );
	}

	bool multi_inrange( unsigned short x1, unsigned short y1,
						unsigned short x2, unsigned short y2 )
	{
	  return ( ( abs( x1 - x2 ) <= RANGE_VISUAL_LARGE_BUILDINGS ) &&
			   ( abs( y1 - y2 ) <= RANGE_VISUAL_LARGE_BUILDINGS ) );
	}

	void send_put_in_container( Client* client, const Item* item )
	{
	  PktHelper::PacketOut<PktOut_25> msg;
	  msg->Write<u32>( item->serial_ext );
	  msg->WriteFlipped<u16>( item->graphic );
	  msg->offset++; //unk7 layer?
	  msg->WriteFlipped<u16>( item->get_senditem_amount() );
	  msg->WriteFlipped<u16>( item->x );
	  msg->WriteFlipped<u16>( item->y );
	  if ( client->ClientType & CLIENTTYPE_6017 )
		msg->Write<u8>( item->slot_index() );
	  msg->Write<u32>( item->container->serial_ext );
	  msg->WriteFlipped<u16>( item->color );
	  msg.Send( client );

	  if ( client->UOExpansionFlag & AOS )
		send_object_cache( client, dynamic_cast<const UObject*>( item ) );
	}

	void send_put_in_container_to_inrange( const Item *item )
	{
	  // FIXME there HAS to be a better, more efficient way than this, without
	  // building these with pointer and NULL to check. Cuz that method requires
	  // recast and delete. Ewwww.
	  PktHelper::PacketOut<PktOut_25> legacy_buffer;
	  PktHelper::PacketOut<PktOut_25> slot_buffer;

      // FIXME mightsee also checks remote containers thus the ForEachPlayer functions cannot be used
	  for ( auto &client2 : clients )
	  {
		if ( !client2->ready )
		  continue;
		// FIXME need to check character's additional_legal_items.
		// looks like inrange should be a Character member function.
		if ( client2->chr->mightsee( item->container ) )
		{
		  // FIXME if the container has an owner, and I'm not it, don't tell me?
		  if ( client2->ClientType & CLIENTTYPE_6017 )
		  {
			if ( slot_buffer->offset == 1 )
			{
			  slot_buffer->Write<u32>( item->serial_ext );
			  slot_buffer->WriteFlipped<u16>( item->graphic );
			  slot_buffer->offset++; //unk7 layer?
			  slot_buffer->WriteFlipped<u16>( item->get_senditem_amount() );
			  slot_buffer->WriteFlipped<u16>( item->x );
			  slot_buffer->WriteFlipped<u16>( item->y );
			  slot_buffer->Write<u8>( item->slot_index() );
			  slot_buffer->Write<u32>( item->container->serial_ext );
			  slot_buffer->WriteFlipped<u16>( item->color );
			}
			slot_buffer.Send( client2 );
		  }
		  else
		  {
			if ( legacy_buffer->offset == 1 )
			{
			  legacy_buffer->Write<u32>( item->serial_ext );
			  legacy_buffer->WriteFlipped<u16>( item->graphic );
			  legacy_buffer->offset++; //unk7 layer?
			  legacy_buffer->WriteFlipped<u16>( item->get_senditem_amount() );
			  legacy_buffer->WriteFlipped<u16>( item->x );
			  legacy_buffer->WriteFlipped<u16>( item->y );
			  legacy_buffer->Write<u32>( item->container->serial_ext );
			  legacy_buffer->WriteFlipped<u16>( item->color );
			}
			legacy_buffer.Send( client2 );
		  }
		  if ( client2->UOExpansionFlag & AOS )
		  {
			send_object_cache( client2, dynamic_cast<const UObject*>( item ) );
		  }
		}
	  }
	}

	// FIXME it would be better to compose this message once and
	// send to multiple clients.
	// Uh, WTF.  Looks like we have to send a full "container contents"
	// message, just to get the clothes on the corpse - without an
	// 'open gump' message of course
	void send_corpse_items( Client *client, const Item *item )
	{
	  const UContainer *cont = static_cast<const UContainer *>( item );

	  PktHelper::PacketOut<PktOut_89> msg;
	  msg->offset += 2;
	  msg->Write<u32>( item->serial_ext );

	  int n_layers_found = 0;
	  for ( const auto &item2 : *cont )
	  {
		if ( item2->layer == 0 )
		  continue;

		// FIXME it looks like it's possible for a corpse to have more than NUM_LAYERS items that specify a layer.
		// probably by dropping something onto a corpse
		if ( n_layers_found >= NUM_LAYERS )
		{
		  static u32 last_serial = 0;
		  if ( item->serial != last_serial )
		  {
			Clib::Log2( "Too many items specify a layer on corpse 0x%lx\n", item->serial );
			last_serial = item->serial;
		  }
		  break;
		}
		msg->Write<u8>( item2->layer );
		msg->Write<u32>( item2->serial_ext );
		n_layers_found++;
	  }
	  passert_always( n_layers_found <= NUM_LAYERS );
	  msg->offset += 1; // nullterm byte
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );

	  msg.Send( client, len );

	  send_container_contents( client, *cont, true );
	}

	// Item::sendto( Client* ) ??
	void send_item( Client *client, const Item *item )
	{
	  // printf( "Sending Item %08.08lX to Character %08.08lX\n", 
	  //         item->serial, client->chr->serial );

	  if ( item->invisible() && !client->chr->can_seeinvisitems() )
	  {
		send_remove_object( client, item );
		return;
	  }

	  u8 flags = 0;
	  if ( client->chr->can_move( item ) )
		flags |= ITEM_FLAG_FORCE_MOVABLE;
	  if ( client->ClientType & CLIENTTYPE_7000 )
	  {
		// Client >= 7.0.0.0 ( SA )
		PktHelper::PacketOut<PktOut_F3> msg;
		msg->WriteFlipped<u16>( 0x1 );
		msg->offset++; // datatype
		msg->Write<u32>( item->serial_ext );
		msg->WriteFlipped<u16>( item->graphic );
		msg->Write<u8>( 0 );
		msg->WriteFlipped<u16>( item->get_senditem_amount() );
		msg->WriteFlipped<u16>( item->get_senditem_amount() );
		msg->WriteFlipped<u16>( item->x );
		msg->WriteFlipped<u16>( item->y );
		msg->Write<s8>( item->z );
		msg->Write<u8>( item->facing );
		msg->WriteFlipped<u16>( item->color );
		msg->Write<u8>( flags );
		if ( client->ClientType & CLIENTTYPE_7090 )
		  msg->offset += 2;
		msg.Send( client );
	  }
	  else
	  {
		PktHelper::PacketOut<PktOut_1A> msg;
		// transmit item info
		msg->offset += 2;
		// If the 0x80000000 is left out, the item won't show up. 
		msg->WriteFlipped<u32>( static_cast<u32>( 0x80000000 | item->serial ) ); // bit 0x80000000 enables piles
		msg->WriteFlipped<u16>( item->graphic );
		msg->WriteFlipped<u16>( item->get_senditem_amount() );
		if ( item->facing == 0 )
		{
		  msg->WriteFlipped<u16>( item->x );
		  // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
		  msg->WriteFlipped<u16>( static_cast<u16>( 0xC000 | item->y ) ); // dyeable and moveable?
		}
		else
		{
		  msg->WriteFlipped<u16>( static_cast<u16>( 0x8000 | item->x ) );
		  // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
		  msg->WriteFlipped<u16>( static_cast<u16>( 0xC000 | item->y ) ); // dyeable and moveable?
		  msg->Write<u8>( item->facing );
		}
		msg->Write<s8>( item->z );
		msg->WriteFlipped<u16>( item->color );
		msg->Write<u8>( flags );
		u16 len = msg->offset;
		msg->offset = 1;
		msg->WriteFlipped<u16>( len );
		msg.Send( client, len );
	  }

	  // if the item is a corpse, transmit items contained by it
	  if ( item->objtype_ == UOBJ_CORPSE )
	  {
		send_corpse_items( client, item );
	  }

	  if ( client->UOExpansionFlag & AOS )
	  {
		send_object_cache( client, item );
		return;
	  }
	}

	/* Tell all clients new information about an item */
	// FIXME OPTIMIZE: OMG, EVERY TIME IT CALLS SEND_ITEM!
	void send_item_to_inrange( const Item *item )
	{
	  // FIXME could use transmit_to_inrange, almost.
	  // (Character-specific flags, like can_move(), make it so we can't)
	  // However, could build main part of packet before for/loop, then
	  // adjust per client. Would this be a better solution?

      ForEachPlayerInVisualRange( item, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        send_item( zonechr->client, item );
      } );
	}


	void update_item_to_inrange( const Item* item )
	{
	  if ( item->container != NULL )
	  {
		if ( IsCharacter( item->container->serial ) )
		{
		  // this may not be the right thing in all cases.
		  // specifically, handle_dye used to not ever do send_wornitem.
		  // FIXME way, way inefficient, but nontrivial.
		  Character* chr = find_character( item->container->serial );
		  if ( chr )
		  {
			update_wornitem_to_inrange( chr, item );
		  }
		  else
            Clib::Log( "Ack! update_item_to_inrange: character 0x%lx doesn't exist!\n",
			item->container->serial );
		}
		else
		{
		  send_put_in_container_to_inrange( item );
		}
	  }
	  else
	  {
		send_item_to_inrange( item );
	  }
	}

	void send_light( Client *client, int lightlevel )
	{
	  if ( VALID_LIGHTLEVEL( lightlevel ) )
	  {
		PktHelper::PacketOut<PktOut_4F> msg;
		msg->Write<u8>( static_cast<u8>( lightlevel ) );
		msg.Send( client );
	  }
	}

	void send_weather( Client* client, u8 type, u8 severity, u8 aux )
	{
	  PktHelper::PacketOut<PktOut_65> msg;
	  msg->Write<u8>( type );
	  msg->Write<u8>( severity );
	  msg->Write<u8>( aux );
	  msg.Send( client );
	}

	/* send_char_data: called once for each client when a new character enters
	   the world. */
	void send_char_data( Client *client, Character *chr )
	{
	  if ( !client->ready )
		return;

	  if ( !client->chr->is_visible_to_me( chr ) )
		return;

	  if ( inrange( client->chr, chr ) )
	  {
		send_owncreate( client, chr );
	  }
	}

	/* send_client_char_data: called once for each character when a client
	   logs on.  If in range, tell the client about each character. */
	void send_client_char_data( Character *chr, Client *client )
	{
	  // Don't tell a client about its own character.
	  if ( client->chr == chr )
		return;

	  if ( !client->chr->is_visible_to_me( chr ) )
		return;

	  if ( inrange( client->chr, chr ) )
	  {
		send_owncreate( client, chr );
	  }
	}

	void send_item_move_failure( Network::Client *client, u8 reason )
	{
	  PktHelper::PacketOut<PktOut_27> msg;
	  msg->Write<u8>( reason );
	  msg.Send( client );
	}

	void send_wornitem( Client *client, const Character *chr, const Item *item )
	{
	  PktHelper::PacketOut<PktOut_2E> msg;
	  msg->Write<u32>( item->serial_ext );
	  msg->WriteFlipped<u16>( item->graphic );
	  msg->offset++; //unk7
	  msg->Write<u8>( item->layer );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( item->color );
	  msg.Send( client );

	  if ( client->UOExpansionFlag & AOS )
	  {
		send_object_cache( client, dynamic_cast<const UObject*>( item ) );
	  }
	}

	void send_wornitem_to_inrange( const Character *chr, const Item *item )
	{
	  PktHelper::PacketOut<PktOut_2E> msg;
	  msg->Write<u32>( item->serial_ext );
	  msg->WriteFlipped<u16>( item->graphic );
	  msg->offset++; //unk7
	  msg->Write<u8>( item->layer );
	  msg->Write<u32>( chr->serial_ext );
	  msg->WriteFlipped<u16>( item->color );
	  transmit_to_inrange( item, &msg->buffer, msg->offset, false, false );
	  send_object_cache_to_inrange( dynamic_cast<const UObject*>( item ) );
	}

	// This used when item already worn and graphic/color changed. Deletes the item
	// at client and then sends the new information.
	void update_wornitem_to_inrange( const Character *chr, const Item *item )
	{
	  if ( chr != NULL )
	  {
		PktHelper::PacketOut<PktOut_1D> msgremove;
		msgremove->Write<u32>( item->serial_ext );
		transmit_to_inrange( item, &msgremove->buffer, msgremove->offset, false, false );

		PktHelper::PacketOut<PktOut_2E> msg;
		msg->Write<u32>( item->serial_ext );
		msg->WriteFlipped<u16>( item->graphic );
		msg->offset++; //unk7
		msg->Write<u8>( item->layer );
		msg->Write<u32>( chr->serial_ext );
		msg->WriteFlipped<u16>( item->color );
		transmit_to_inrange( item, &msg->buffer, msg->offset, false, false );

		send_object_cache_to_inrange( dynamic_cast<const UObject*>( item ) );
	  }
	}

	// does 'item' have a parent with serial 'serial'?
	bool is_a_parent( const Item *item, u32 serial )
	{
	  while ( item->container != NULL )
	  {
		// UNTESTED
		item = item->container;
		if ( item->serial == serial )
		  return true;
	  }
	  return false;
	}



	// search for a container that this character can legally act upon
	// - remove items, insert items, etc.
	UContainer *find_legal_container( const Character *chr, u32 serial )
	{
	  UContainer *cont;
	  cont = chr->backpack();
	  if ( cont )
	  {
		if ( serial == cont->serial )
		  return cont;
		// not the main backpack, look for subpacks.
		cont = cont->find_container( serial );
		if ( cont )
		  return cont;
	  }

	  // 4/2007 - MuadDib
	  // Wasn't in backpack, check wornitems
	  cont = NULL;
	  Item *worn_item = chr->find_wornitem( serial );
	  if ( worn_item != NULL && worn_item->script_isa( POLCLASS_CONTAINER ) )
	  {
		// Ignore these layers explicitly. Backpack especially since it was
		// already checked above.
		if ( worn_item->layer != LAYER_HAIR && worn_item->layer != LAYER_FACE && worn_item->layer != LAYER_BEARD
			 && worn_item->layer != LAYER_BACKPACK && worn_item->layer != LAYER_MOUNT )
		{
		  UContainer* worn_cont = static_cast<UContainer*>( worn_item );
		  if ( worn_cont != NULL )
			return worn_cont;
		}
	  }


	  // not in the backpack, or in a subpack.  check global items and subpacks.
	  // FIXME doesn't check range?
	  unsigned short wxL, wyL, wxH, wyH;
	  zone_convert_clip( chr->x - 8, chr->y - 8, chr->realm, wxL, wyL );
	  zone_convert_clip( chr->x + 8, chr->y + 8, chr->realm, wxH, wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          for ( auto &item : chr->realm->zone[wx][wy].items )
		  {
			if ( item->isa( UObject::CLASS_CONTAINER ) )
			{
			  cont = (UContainer *)item;
			  if ( serial == cont->serial )
				return cont;
			  cont = cont->find_container( serial );
			  if ( cont )
				return cont;
			}
		  }
		}
	  }

	  Item* item = chr->search_remote_containers( serial, NULL /* don't care if it's a remote container */ );
	  if ( item != NULL && item->isa( UObject::CLASS_CONTAINER ) )
		return static_cast<UContainer*>( item );
	  else
		return NULL;
	}

	// assume if you pass additlegal or isRemoteContainer, you init to false
	Item *find_legal_item( const Character *chr, u32 serial, bool* additlegal, bool* isRemoteContainer )
	{
	  UContainer *backpack = chr->backpack();
	  if ( backpack != NULL && backpack->serial == serial )
		return backpack;

	  // check worn items
	  // 04/2007 - MuadDib Added:
	  // find_wornitem will now check inside containers listed in layers
	  // for normal items now also. This will allow for quivers
	  // in wornitems, handbags, pockets, whatever people want,
	  // to find stuff as a legal item to the character. Treats it
	  // just like the backpack, without making it specific like
	  // a bankbox or backpack.
	  Item *item = chr->find_wornitem( serial );
	  if ( item != NULL )
		return item;

	  if ( backpack != NULL )
	  {
		item = backpack->find( serial );
		if ( item != NULL )
		  return item;
	  }

	  // check items on the ground
	  unsigned short wxL, wyL, wxH, wyH;
	  zone_convert_clip( chr->x - 8, chr->y - 8, chr->realm, wxL, wyL );
	  zone_convert_clip( chr->x + 8, chr->y + 8, chr->realm, wxH, wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          for ( auto &_item : chr->realm->zone[wx][wy].items )
		  {
			if ( !inrange( chr, _item ) )
			  continue;
			if ( _item->serial == serial )
			{
			  passert_always( _item->container == NULL );
			  return _item;
			}
			if ( _item->isa( UObject::CLASS_CONTAINER ) )
			{
			  _item = ( (UContainer *)_item )->find( serial );
			  if ( _item != NULL )
				return _item;
			}
		  }
		}
	  }

	  if ( additlegal != NULL )
		*additlegal = true;
	  return chr->search_remote_containers( serial, isRemoteContainer );
	}

	void play_sound_effect( const UObject *center, u16 effect )
	{
	  PktHelper::PacketOut<PktOut_54> msg;
	  msg->Write<u8>( static_cast<u8>( PKTOUT_54_FLAG_SINGLEPLAY ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( effect - 1 ) ); // SOUND_EFFECT_XX, see sfx.h
	  msg->offset += 2; //volume
	  //msg->WriteFlipped(static_cast<u16>(0));
	  msg->WriteFlipped<u16>( center->x );
	  msg->WriteFlipped<u16>( center->y );
	  msg->offset += 2;
	  //msg->WriteFlipped(z);
	  // FIXME hearing range check perhaps?
	  transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	}

	void play_sound_effect_xyz( u16 cx, u16 cy, s8 cz, u16 effect, Plib::Realm* realm )
	{
	  PktHelper::PacketOut<PktOut_54> msg;
	  msg->Write<u8>( static_cast<u8>( PKTOUT_54_FLAG_SINGLEPLAY ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( effect - 1 ) ); // SOUND_EFFECT_XX, see sfx.h
	  msg->offset += 2; //volume
	  //msg->WriteFlipped(static_cast<u16>(0));
	  msg->WriteFlipped<u16>( cx );
	  msg->WriteFlipped<u16>( cy );
	  msg->WriteFlipped<s16>( static_cast<s16>( cz ) );

      ForEachPlayerInRange( cx, cy, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
	}

	void play_sound_effect_private( const UObject *center, u16 effect, Character* forchr )
	{
	  if ( forchr->client )
	  {
		PktHelper::PacketOut<PktOut_54> msg;
		msg->Write<u8>( static_cast<u8>( PKTOUT_54_FLAG_SINGLEPLAY ) );
		msg->WriteFlipped<u16>( static_cast<u16>( effect - 1 ) ); // SOUND_EFFECT_XX, see sfx.h
		msg->offset += 2; //volume
		//msg->WriteFlipped(static_cast<u16>(0));
		msg->WriteFlipped<u16>( center->x );
		msg->WriteFlipped<u16>( center->y );
		msg->offset += 2;
		//msg->WriteFlipped(z);
		msg.Send( forchr->client );
	  }
	}

	void play_moving_effect( const UObject *src, const UObject *dst,
							 u16 effect,
							 u8 speed,
							 u8 loop,
							 u8 explode )
	{
	  PktHelper::PacketOut<PktOut_70> msg;
	  msg->Write<u8>( static_cast<u8>( EFFECT_TYPE_MOVING ) );
	  msg->Write<u32>( src->serial_ext );
	  msg->Write<u32>( dst->serial_ext );
	  msg->WriteFlipped<u16>( effect );
	  msg->WriteFlipped<u16>( src->x );
	  msg->WriteFlipped<u16>( src->y );
	  msg->Write<s8>( static_cast<s8>( src->z + src->height ) );
	  msg->WriteFlipped<u16>( dst->x );
	  msg->WriteFlipped<u16>( dst->y );
	  msg->Write<s8>( static_cast<s8>( dst->z + dst->height ) );
	  msg->Write<u8>( speed );
	  msg->Write<u8>( loop );
	  msg->offset += 3; //unk24,unk25,unk26
	  msg->Write<u8>( explode );

      ForEachPlayerInVisualRange( src->toplevel_owner(), [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
      ForEachPlayerInVisualRange( dst->toplevel_owner(), [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange( zonechr, src ) ) // send to char only in range of dst
          msg.Send( zonechr->client );
      } );
	}

	void play_moving_effect2( u16 xs, u16 ys, s8 zs,
							  u16 xd, u16 yd, s8 zd,
							  u16 effect,
							  u8 speed,
							  u8 loop,
							  u8 explode,
							  Plib::Realm* realm )
	{
	  PktHelper::PacketOut<PktOut_70> msg;
	  msg->Write<u8>( static_cast<u8>( EFFECT_TYPE_MOVING ) );
	  msg->offset += 8; // src+dst serial
	  msg->WriteFlipped<u16>( effect );
	  msg->WriteFlipped<u16>( xs );
	  msg->WriteFlipped<u16>( ys );
	  msg->Write<s8>( zs );
	  msg->WriteFlipped<u16>( xd );
	  msg->WriteFlipped<u16>( yd );
	  msg->Write<s8>( zd );
	  msg->Write<u8>( speed );
	  msg->Write<u8>( loop );
	  msg->offset += 3; //unk24,unk25,unk26
	  msg->Write<u8>( explode );

      ForEachPlayerInRange( xs, ys, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
      ForEachPlayerInRange( xd, yd, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange( zonechr, xs, ys ) ) // send to chrs only in range of dest
          msg.Send( zonechr->client );
      } );
	}


	void play_lightning_bolt_effect( const UObject* center )
	{
	  PktHelper::PacketOut<PktOut_70> msg;
	  msg->Write<u8>( static_cast<u8>( EFFECT_TYPE_LIGHTNING_BOLT ) );
	  msg->Write<u32>( center->serial_ext );
	  msg->offset += 6; // dst serial + effect
	  msg->WriteFlipped<u16>( center->x );
	  msg->WriteFlipped<u16>( center->y );
	  msg->Write<s8>( center->z );
	  msg->offset += 11;
	  transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	}

	void play_object_centered_effect( const UObject* center,
									  u16 effect,
									  u8 speed,
									  u8 loop )
	{
	  PktHelper::PacketOut<PktOut_70> msg;
	  msg->Write<u8>( static_cast<u8>( EFFECT_TYPE_FOLLOW_OBJ ) );
	  msg->Write<u32>( center->serial_ext );
	  msg->offset += 4; // dst serial
	  msg->WriteFlipped<u16>( effect );
	  msg->WriteFlipped<u16>( center->x );
	  msg->WriteFlipped<u16>( center->y );
	  msg->Write<s8>( center->z );
	  msg->offset += 5; //dst x,y,z
	  msg->Write<u8>( speed );
	  msg->Write<u8>( loop );
	  msg->offset += 4; //unk24,unk25,unk26,explode
	  transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	}

    void play_stationary_effect( u16 x, u16 y, s8 z, u16 effect, u8 speed, u8 loop, u8 explode, Plib::Realm* realm )
	{
	  PktHelper::PacketOut<PktOut_70> msg;
	  msg->Write<u8>( static_cast<u8>( EFFECT_TYPE_STATIONARY_XYZ ) );
	  msg->offset += 8; // src,dst serial
	  msg->WriteFlipped<u16>( effect );
	  msg->WriteFlipped<u16>( x );
	  msg->WriteFlipped<u16>( y );
	  msg->Write<s8>( z );
	  msg->offset += 5; //dst x,y,z
	  msg->Write<u8>( speed );
	  msg->Write<u8>( loop );
	  msg->offset += 2; //unk24,unk25
	  msg->Write<u8>( static_cast<u8>( 1 ) ); // this is right for teleport, anyway
	  msg->Write<u8>( explode );

      ForEachPlayerInRange( x, y, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
	}

    void play_stationary_effect_ex( u16 x, u16 y, s8 z, Plib::Realm* realm, u16 effect, u8 speed, u8 duration, u32 hue,
									u32 render, u16 effect3d )
	{
	  PktHelper::PacketOut<PktOut_C7> msg;
	  partical_effect( msg.Get(), PKTOUT_C0::EFFECT_FIXEDXYZ,
					   0, 0, x, y, z, x, y, z,
					   effect, speed, duration, 1, 0,
					   hue, render, effect3d, 1, 0,
					   0, 0xFF );

      ForEachPlayerInRange( x, y, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
	}

	void play_object_centered_effect_ex( const UObject* center, u16 effect, u8 speed, u8 duration, u32 hue,
										 u32 render, u8 layer, u16 effect3d )
	{
	  PktHelper::PacketOut<PktOut_C7> msg;
	  partical_effect( msg.Get(), PKTOUT_C0::EFFECT_FIXEDFROM,
					   center->serial_ext, center->serial_ext,
					   center->x, center->y, center->z,
					   center->x, center->y, center->z,
					   effect, speed, duration, 1, 0,
					   hue, render, effect3d, 1, 0,
					   center->serial_ext, layer );

	  transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	}

	void play_moving_effect_ex( const UObject *src, const UObject *dst,
								u16 effect, u8 speed, u8 duration, u32 hue,
								u32 render, u8 direction, u8 explode,
								u16 effect3d, u16 effect3dexplode, u16 effect3dsound )
	{
	  PktHelper::PacketOut<PktOut_C7> msg;
	  partical_effect( msg.Get(), PKTOUT_C0::EFFECT_MOVING,
					   src->serial_ext, dst->serial_ext,
					   src->x, src->y, src->z + src->height,
					   dst->x, dst->y, dst->z + dst->height,
					   effect, speed, duration, direction, explode, hue, render,
					   effect3d, effect3dexplode, effect3dsound,
					   0, 0xFF );

      ForEachPlayerInVisualRange( src, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
      ForEachPlayerInVisualRange( dst, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange(zonechr, src ) ) // send to chrs only in range of dst
          msg.Send( zonechr->client );
      } );
	}

	void play_moving_effect2_ex( u16 xs, u16 ys, s8 zs,
                                 u16 xd, u16 yd, s8 zd, Plib::Realm* realm,
								 u16 effect, u8 speed, u8 duration, u32 hue,
								 u32 render, u8 direction, u8 explode,
								 u16 effect3d, u16 effect3dexplode, u16 effect3dsound )
	{
	  PktHelper::PacketOut<PktOut_C7> msg;
	  partical_effect( msg.Get(), PKTOUT_C0::EFFECT_MOVING,
					   0, 0, xs, ys, zs, xd, yd, zd,
					   effect, speed, duration, direction, explode, hue, render,
					   effect3d, effect3dexplode, effect3dsound,
					   0, 0xFF );

      ForEachPlayerInRange( xs, ys, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        msg.Send( zonechr->client );
      } );
      ForEachPlayerInRange( xd, yd, realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange(zonechr, xs, ys ) ) // send to chrs only in range of dst
          msg.Send( zonechr->client );
      } );
	}

	// Central function to build 0xC7 packet
	void partical_effect( PktOut_C7* msg, u8 type, u32 srcserial, u32 dstserial,
						  u16 srcx, u16 srcy, s8 srcz,
						  u16 dstx, u16 dsty, s8 dstz,
						  u16 effect, u8 speed, u8 duration, u8 direction,
						  u8 explode, u32 hue, u32 render,
						  u16 effect3d, u16 effect3dexplode, u16 effect3dsound,
						  u32 itemid, u8 layer )
	{
	  //C0 part
	  msg->Write<u8>( type );
	  msg->Write<u32>( srcserial );
	  msg->Write<u32>( dstserial );
	  msg->WriteFlipped<u16>( effect );
	  msg->WriteFlipped<u16>( srcx );
	  msg->WriteFlipped<u16>( srcy );
	  msg->Write<s8>( srcz );
	  msg->WriteFlipped<u16>( dstx );
	  msg->WriteFlipped<u16>( dsty );
	  msg->Write<s8>( dstz );
	  msg->Write<u8>( speed );
	  msg->Write<u8>( duration );
	  msg->offset += 2; // u16 unk
	  msg->Write<u8>( direction );
	  msg->Write<u8>( explode );
	  msg->WriteFlipped<u32>( hue );
	  msg->WriteFlipped<u32>( render );
	  // C7 part
	  msg->WriteFlipped<u16>( effect3d );   //see particleffect subdir
	  msg->WriteFlipped<u16>( effect3dexplode ); //0 if no explosion
	  msg->WriteFlipped<u16>( effect3dsound ); //for moving effects, 0 otherwise
	  msg->Write<u32>( itemid ); //if target is item (type 2), 0 otherwise 
	  msg->Write<u8>( layer ); //(of the character, e.g left hand, right hand, � 0-5,7, 0xff: moving effect or target is no char) 
	  msg->offset += 2; // u16 unk_effect
	}

	// System message -- message in lower left corner
	void send_sysmessage( Network::Client *client, const char *text, unsigned short font, unsigned short color )
	{
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( strlen( text ) + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN + 1;

	  msg->offset += 2;
	  msg->Write<u32>( static_cast<u32>( 0x01010101 ) );
	  msg->Write<u16>( static_cast<u16>( 0x0101 ) );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  msg->Write( "System", 30 );
	  msg->Write( text, textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( client, len );
	}

	// Unicode System message -- message in lower left corner
    void send_sysmessage( Network::Client *client,
						  const u16 *wtext, const char lang[4],
						  unsigned short font, unsigned short color )
	{
	  unsigned textlen = 0;
	  //textlen = wcslen((const wchar_t*)wtext) + 1;
	  while ( wtext[textlen] != L'\0' )
		++textlen;
	  if ( textlen > ( SPEECH_MAX_LEN ) )  // FIXME need to handle this better second msg?
		textlen = ( SPEECH_MAX_LEN );

	  PktHelper::PacketOut<PktOut_AE> msg;
	  msg->offset += 2;
	  msg->Write<u32>( static_cast<u32>( 0x01010101 ) );
	  msg->Write<u16>( static_cast<u16>( 0x0101 ) );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  msg->Write( lang, 4 );
	  msg->Write( "System", 30 );
	  msg->WriteFlipped( &wtext[0], static_cast<u16>( textlen ) );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( client, len );
	}

    void send_sysmessage( Network::Client *client, const std::string& text, unsigned short font, unsigned short color )
	{
	  send_sysmessage( client, text.c_str(), font, color );
	}

    void send_sysmessage( Network::Client *client, const std::wstring& wtext, const char lang[4], unsigned short font, unsigned short color )
	{
	  using std::wstring;
	  u16 uctext[SPEECH_MAX_LEN + 1];
	  for ( size_t i = 0; i < wtext.length(); i++ )
		uctext[i] = static_cast<u16>( wtext[i] );
	  uctext[wtext.length()] = 0x00;
	  send_sysmessage( client, uctext, lang, font, color );
	}

	void broadcast( const char *text, unsigned short font, unsigned short color )
	{
	  for ( auto &client : clients )
	  {
		if ( !client->ready )
		  continue;

		send_sysmessage( client, text, font, color );
	  }
	}

	void broadcast( const u16 *wtext, const char lang[4],
					unsigned short font, unsigned short color )
	{
      for ( auto &client : clients )
	  {
		if ( !client->ready )
		  continue;

		send_sysmessage( client, wtext, lang, font, color );
	  }
	}

	void send_nametext( Client *client, const Character *chr, const std::string& str )
	{
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( str.length() + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )
		textlen = SPEECH_MAX_LEN + 1;

	  msg->offset += 2;
	  msg->Write<u32>( chr->serial_ext );
	  msg->Write<u16>( static_cast<u16>( 0x0101 ) );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_YOU_SEE ) );
	  msg->WriteFlipped<u16>( chr->name_color( client->chr ) ); // 0x03B2
	  msg->WriteFlipped<u16>( static_cast<u16>( 3 ) );
	  msg->Write( str.c_str(), 30 );
	  msg->Write( str.c_str(), textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( client, len );
	}

	bool say_above( const UObject* obj,
					const char *text,
					unsigned short font,
					unsigned short color,
					unsigned int journal_print )
	{
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( strlen( text ) + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN + 1;

	  msg->offset += 2;
	  msg->Write<u32>( obj->serial_ext );
	  msg->WriteFlipped<u16>( obj->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  switch ( journal_print )
	  {
		case JOURNAL_PRINT_YOU_SEE:
		  msg->Write( "You see", 30 );
		  break;
		case JOURNAL_PRINT_NAME:
		default:
		  msg->Write( obj->description().c_str(), 30 );
		  break;
	  }
	  msg->Write( text, textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  // todo: only send to those that I'm visible to.
	  transmit_to_inrange( obj, &msg->buffer, len, false, false );
	  return true;
	}

	bool say_above( const UObject* obj,
					const u16 *wtext,
					const char lang[4],
					unsigned short font,
					unsigned short color,
					unsigned int journal_print )
	{
	  unsigned textlen = 0;
	  //textlen = wcslen((const wchar_t*)wtext) + 1;
	  while ( wtext[textlen] != L'\0' )
		++textlen;
	  if ( textlen > ( SPEECH_MAX_LEN ) )  // FIXME need to handle this better second msg?
		textlen = ( SPEECH_MAX_LEN );

	  PktHelper::PacketOut<PktOut_AE> msg;
	  msg->offset += 2;
	  msg->Write<u32>( obj->serial_ext );
	  msg->WriteFlipped<u16>( obj->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  msg->Write( lang, 4 );
	  switch ( journal_print )
	  {
		case JOURNAL_PRINT_YOU_SEE:
		  msg->Write( "You see", 30 );
		  break;
		case JOURNAL_PRINT_NAME:
		default:
		  msg->Write( obj->description().c_str(), 30 );
		  break;
	  }
	  msg->WriteFlipped( &wtext[0], static_cast<u16>( textlen ) );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  // todo: only send to those that I'm visible to.
	  transmit_to_inrange( obj, &msg->buffer, len, false, false );
	  return true;
	}

	bool private_say_above( Character* chr,
							const UObject* obj,
							const char *text,
							unsigned short font,
							unsigned short color,
							unsigned int journal_print )
	{
	  if ( chr->client == NULL )
		return false;
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( strlen( text ) + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN + 1;

	  msg->offset += 2;
	  msg->Write<u32>( obj->serial_ext );
	  msg->WriteFlipped<u16>( obj->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  switch ( journal_print )
	  {
		case JOURNAL_PRINT_YOU_SEE:
		  msg->Write( "You see", 30 );
		  break;
		case JOURNAL_PRINT_NAME:
		default:
		  msg->Write( obj->description().c_str(), 30 );
		  break;
	  }
	  msg->Write( text, textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( chr->client, len );
	  return true;
	}

	bool private_say_above( Character* chr,
							const UObject* obj,
							const u16 *wtext,
							const char lang[4],
							unsigned short font,
							unsigned short color,
							unsigned int journal_print )
	{
	  unsigned textlen = 0;
	  //textlen = wcslen((const wchar_t*)wtext) + 1;
	  while ( wtext[textlen] != L'\0' )
		++textlen;
	  if ( textlen > ( SPEECH_MAX_LEN ) )  // FIXME need to handle this better second msg?
		textlen = ( SPEECH_MAX_LEN );
	  if ( chr->client == NULL )
		return false;

	  PktHelper::PacketOut<PktOut_AE> msg;
	  msg->offset += 2;
	  msg->Write<u32>( obj->serial_ext );
	  msg->WriteFlipped<u16>( obj->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( font );
	  msg->Write( lang, 4 );
	  switch ( journal_print )
	  {
		case JOURNAL_PRINT_YOU_SEE:
		  msg->Write( "You see", 30 );
		  break;
		case JOURNAL_PRINT_NAME:
		default:
		  msg->Write( obj->description().c_str(), 30 );
		  break;
	  }
	  msg->WriteFlipped( &wtext[0], static_cast<u16>( textlen ) );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( chr->client, len );
	  return true;
	}

	bool private_say_above_ex( Character* chr,
							   const UObject* obj,
							   const char *text,
							   unsigned short color )
	{
	  if ( chr->client == NULL )
		return false;
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( strlen( text ) + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN + 1;

	  msg->offset += 2;
	  msg->Write<u32>( obj->serial_ext );
	  msg->WriteFlipped<u16>( obj->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_NORMAL ) );
	  msg->WriteFlipped<u16>( color );
	  msg->WriteFlipped<u16>( static_cast<u16>( 3 ) );
	  msg->Write( obj->description().c_str(), 30 );
	  msg->Write( text, textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( chr->client, len );
	  return true;
	}

	void send_objdesc( Client *client, Item *item )
	{
	  PktHelper::PacketOut<PktOut_1C> msg;
	  u16 textlen = static_cast<u16>( item->description().length() + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN + 1;
	  msg->offset += 2;
	  msg->Write<u32>( item->serial_ext );
	  msg->WriteFlipped<u16>( item->graphic );
	  msg->Write<u8>( static_cast<u8>( TEXTTYPE_YOU_SEE ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( 0x03B2 ) );
	  msg->WriteFlipped<u16>( static_cast<u16>( 3 ) );
	  msg->Write( "System", 30 );
	  msg->Write( item->description().c_str(), textlen );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( client, len );
	}

	void send_stamina_level( Client *client )
	{
	  Character *chr = client->chr;

	  PktHelper::PacketOut<PktOut_A3> msg;
	  msg->Write<u32>( chr->serial_ext );

	  if ( uoclient_general.stamina.any )
	  {
		int v = chr->vital( uoclient_general.mana.id ).maximum_ones();
		if ( v > 0xFFFF )
		  v = 0xFFFF;
		msg->WriteFlipped<u16>( static_cast<u16>( v ) );

		v = chr->vital( uoclient_general.mana.id ).current_ones();
		if ( v > 0xFFFF )
		  v = 0xFFFF;
		msg->WriteFlipped<u16>( static_cast<u16>( v ) );
	  }
	  else
	  {
		msg->offset += 4;
	  }
	  msg.Send( client );
	}

	void send_mana_level( Client *client )
	{
	  Character *chr = client->chr;

	  PktHelper::PacketOut<PktOut_A2> msg;
	  msg->Write<u32>( chr->serial_ext );

	  if ( uoclient_general.mana.any )
	  {
		int v = chr->vital( uoclient_general.mana.id ).maximum_ones();
		if ( v > 0xFFFF )
		  v = 0xFFFF;
		msg->WriteFlipped<u16>( static_cast<u16>( v ) );

		v = chr->vital( uoclient_general.mana.id ).current_ones();
		if ( v > 0xFFFF )
		  v = 0xFFFF;
		msg->WriteFlipped<u16>( static_cast<u16>( v ) );
	  }
	  else
	  {
		msg->offset += 4;
	  }
	  msg.Send( client );
	}

	void send_death_message( Character *chr_died, Item *corpse )
	{
	  PktHelper::PacketOut<PktOut_AF> msg;
	  msg->Write<u32>( chr_died->serial_ext );
	  msg->Write<u32>( corpse->serial_ext );
	  msg->offset += 4; // u32 unk4_zero

      ForEachPlayerInVisualRange( corpse, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( zonechr== chr_died )
          return;
        msg.Send( zonechr->client );
      } );
	}

	void transmit_to_inrange( const UObject* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR )
	{
      ForEachPlayerInVisualRange( center, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        Client* client = zonechr->client;
        if ( is_6017 && ( !( client->ClientType & CLIENTTYPE_6017 ) ) )
          return;
        if ( is_UOKR && ( !( client->ClientType & CLIENTTYPE_UOKR ) ) )
          return;
        ADDTOSENDQUEUE( client, msg, msglen );
      } );
	}

	void transmit_to_others_inrange( Character* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR )
	{
      ForEachPlayerInVisualRange( center, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        Client* client = zonechr->client;
        if ( is_6017 && ( !( client->ClientType & CLIENTTYPE_6017 ) ) )
          return;
        if ( is_UOKR && ( !( client->ClientType & CLIENTTYPE_UOKR ) ) )
          return;
        if ( zonechr == center )
          return;
        ADDTOSENDQUEUE( client, msg, msglen );
      } );
	}

	//DAVE made heavy changes to this 11/17 for speed.
	Character* chr_from_wornitems( UContainer* wornitems )
	{
	  Character* owner = wornitems->get_chr_owner();
	  if ( owner != NULL )
		return owner;
	  else
		return NULL; //fixed 3/8/3
	}

	void destroy_item( Item* item )
	{
	  if ( item->serial == 0 )
	  {
		Clib::Log( "destroy %s: %s, orphan! (old serial: 0x%lx)\n",
			 item->description().c_str(),
			 item->classname(),
			 cfBEu32( item->serial_ext ) );
	  }

	  if ( item->serial != 0 )
	  {
		/*
			cout << "destroy " << item->description() << ": "
			<< item->classname() << " " <<  item
			<< ", serial=" << hexint(item->serial) << endl;
			*/
		item->set_dirty();

        PktHelper::PacketOut<PktOut_1D> msgremove;
        msgremove->Write<u32>( item->serial_ext );
        ForEachPlayerInVisualRange( item, [&]( Mobile::Character *zonechr )
        {
          send_remove_object( zonechr->client, msgremove.Get() );
        } );
		if ( item->container == NULL ) // on ground, easy.
		{
		  if ( !item->is_gotten() ) // and not in hand
			remove_item_from_world( item );
		}
		else
		{
		  item->extricate();
		}
		item->destroy();
	  }
	}

	void setrealm( Item* item, void* arg )
	{
      Plib::Realm* realm = static_cast<Plib::Realm*>( arg );
	  item->realm = realm;
	}

	void setrealmif( Item* item, void* arg )
	{
      Plib::Realm* realm = static_cast<Plib::Realm*>( arg );
	  if ( item->realm == realm )
		item->realm = realm->baserealm;
	}

	void world_delete( UObject* uobj )
	{
	  // std::cout << "world_delete(0x" << hex << uobj->serial << dec << ")" << endl;
	  uobj->destroy();
	  /* moved to UObject::destroy()
		  uobj->serial = 0;
		  uobj->serial_ext = 0;

		  passert( uobj->count() >= 1 );

		  ref_ptr<UObject> ref( uobj );

		  uobj->release();
		  */
	}

	void subtract_amount_from_item( Item* item, unsigned short amount )
	{
	  if ( amount >= item->getamount() )
	  {
		destroy_item( item );
		return; //destroy_item will update character weight if item is carried.
	  }
	  else
	  {
		item->subamount( amount );
		update_item_to_inrange( item );
	  }
	  //DAVE added this 11/17: if in a Character's pack, update weight.
	  UpdateCharacterWeight( item );
	}



	void move_item( Item* item, UFACING facing )
	{
	  u16 oldx = item->x;
	  u16 oldy = item->y;

	  item->x += move_delta[facing].xmove;
	  item->y += move_delta[facing].ymove;

	  item->restart_decay_timer();
	  MoveItemWorldPosition( oldx, oldy, item, NULL );

	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( item->serial_ext );

      ForEachPlayerInVisualRange( item, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        send_item( zonechr->client, item );
      } );
      ForEachPlayerInRange( oldx, oldy, item->realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange(zonechr, item ) )// not in range.  If old loc was in range, send a delete.
          msgremove.Send( zonechr->client );
      } );
	}

	// FIXME: this is called from some places where the item didn't used
	// to be on the ground - in a container, say.
	// FIXME OPTIMIZE: Core is building the packet in send_item for every single client
	// that needs to get it. There should be a better method for this. Such as, a function
	// to run all the checks after building the packet here, then send as it needs to.
	void move_item( Item* item, unsigned short newx, unsigned short newy, signed char newz, Plib::Realm* oldrealm )
	{
	  item->set_dirty();

	  u16 oldx = item->x;
	  u16 oldy = item->y;

	  item->x = newx;
	  item->y = newy;
	  item->z = newz;

	  item->restart_decay_timer();
	  MoveItemWorldPosition( oldx, oldy, item, oldrealm );

	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( item->serial_ext );

      ForEachPlayerInVisualRange( item, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        send_item( zonechr->client, item );
      } );
      ForEachPlayerInRange( oldx, oldy, oldrealm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange( zonechr, item ) )// not in range.  If old loc was in range, send a delete.
          msgremove.Send( zonechr->client );
      } );
	}

	void move_boat_item( Item* item, unsigned short newx, unsigned short newy, signed char newz, Plib::Realm* oldrealm )
	{
	  u16 oldx = item->x;
	  u16 oldy = item->y;

	  item->x = newx;
	  item->y = newy;
	  item->z = newz;

	  MoveItemWorldPosition( oldx, oldy, item, oldrealm );

	  PktHelper::PacketOut<PktOut_1A> msg;
	  msg->offset += 2;
	  msg->Write<u32>( item->serial_ext );
	  msg->WriteFlipped<u16>( item->graphic );
	  msg->WriteFlipped<u16>( item->x );
	  msg->WriteFlipped<u16>( item->y );
	  msg->Write<s8>( item->z );
	  u16 len1A = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len1A );

	  // Client >= 7.0.0.0 ( SA )
	  PktHelper::PacketOut<PktOut_F3> msg2;
	  msg2->WriteFlipped<u16>( static_cast<u16>( 0x1 ) );
	  msg2->offset++; // datatype
	  msg2->Write<u32>( item->serial_ext );
	  msg2->WriteFlipped<u16>( item->graphic );
	  msg2->Write<u8>( static_cast<u8>( 0 ) );
	  msg2->WriteFlipped<u16>( static_cast<u16>( 0x1 ) );
	  msg2->WriteFlipped<u16>( static_cast<u16>( 0x1 ) );
	  msg2->WriteFlipped<u16>( item->x );
	  msg2->WriteFlipped<u16>( item->y );
	  msg2->Write<s8>( item->z );
	  msg2->Write<u8>( item->facing ); //facing
	  msg2->WriteFlipped<u16>( item->color );
	  msg2->offset++; //flags

	  // Client >= 7.0.9.0 ( HSA )
	  PktHelper::PacketOut<PktOut_F3> msg3;
	  memcpy( &msg3->buffer, &msg2->buffer, sizeof msg3->buffer );
	  msg3->offset = 26; //unk short at the end

	  PktHelper::PacketOut<PktOut_1D> msgremove;
	  msgremove->Write<u32>( item->serial_ext );

      ForEachPlayerInVisualRange( item, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        Client* client = zonechr->client;
        if ( client->ClientType & CLIENTTYPE_7090 )
          msg3.Send( client );
        else if ( client->ClientType & CLIENTTYPE_7000 )
          msg2.Send( client );
        else
          msg.Send( client, len1A );
      } );
      ForEachPlayerInRange( oldx, oldy, item->realm, RANGE_VISUAL, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( !inrange( zonechr, item ) )// not in range.  If old loc was in range, send a delete.
          msgremove.Send( zonechr->client );
      } );
	}


	void send_multi( Client* client, const Multi::UMulti* multi )
	{
	  if ( client->ClientType & CLIENTTYPE_7000 )
	  {
		PktHelper::PacketOut<PktOut_F3> msg;
		msg->WriteFlipped<u16>( static_cast<u16>( 0x1 ) );
		msg->Write<u8>( static_cast<u8>( 0x02 ) );
		msg->Write<u32>( multi->serial_ext );
		msg->WriteFlipped<u16>( multi->multidef().multiid );
		msg->offset++; //0;
		msg->WriteFlipped<u16>( static_cast<u16>( 0x1 ) ); //amount
		msg->WriteFlipped<u16>( static_cast<u16>( 0x1 ) ); //amount2
		msg->WriteFlipped<u16>( multi->x );
		msg->WriteFlipped<u16>( multi->y );
		msg->Write<s8>( multi->z );
		msg->offset++; // u8 facing
		msg->WriteFlipped<u16>( multi->color ); // u16 color
		msg->offset++; // u8 flags
		if ( client->ClientType & CLIENTTYPE_7090 )
		  msg->offset += 2;
		msg.Send( client );
	  }
	  else
	  {
		PktHelper::PacketOut<PktOut_1A> msg;
		msg->offset += 2;
		msg->Write<u32>( multi->serial_ext );
		u16 graphic = multi->multidef().multiid | 0x4000;
		msg->WriteFlipped<u16>( graphic );
		msg->WriteFlipped<u16>( multi->x );
		msg->WriteFlipped<u16>( multi->y );
		msg->Write<s8>( multi->z );
		u16 len = msg->offset;
		msg->offset = 1;
		msg->WriteFlipped<u16>( len );
		msg.Send( client, len );
	  }
	}

    void send_multi_to_inrange( const Multi::UMulti* multi )
	{
	  // FIXME could use transmit_to_inrange, almost.
	  // (Character-specific flags, like can_move(), make it so we can't)
	  // However, could build main part of packet before for/loop, then
	  // adjust per client. Would this be a better solution?
      ForEachPlayerInVisualRange( multi, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client( ) )
          return;
        send_multi( zonechr->client, multi );
      } );
	}



	void update_lightregion( Client* client, LightRegion* lightregion )
	{
	  if ( !client->ready )
		return;

	  client->chr->check_light_region_change();
	}

	void SetRegionLightLevel( LightRegion* lightregion, int lightlevel )
	{
	  lightregion->lightlevel = lightlevel;
	  PktHelper::PacketOut<PktOut_4F> msg;
	  msg->Write<u8>( static_cast<u8>( lightlevel ) );
	  for ( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	  {
		Client *client = *itr;
		if ( !client->ready )
		  continue;

		if ( client->chr->lightoverride_until < read_gameclock() && client->chr->lightoverride_until != ~0u )
		{
		  client->chr->lightoverride_until = 0;
		  client->chr->lightoverride = -1;
		}

		if ( client->gd->weather_region &&
			 client->gd->weather_region->lightoverride != -1 &&
			 client->chr->lightoverride == -1 )
			 continue;

		int newlightlevel;
		if ( client->chr->lightoverride != -1 )
		  newlightlevel = client->chr->lightoverride;
		else
		{
		  //dave 12-22 check for no regions
		  LightRegion* light_region = lightdef->getregion( client->chr->x, client->chr->y, client->chr->realm );
		  if ( light_region != NULL )
			newlightlevel = light_region->lightlevel;
		  else
			newlightlevel = ssopt.default_light_level;
		}

		if ( newlightlevel != client->gd->lightlevel )
		{
		  if ( VALID_LIGHTLEVEL( newlightlevel ) )
		  {
			if ( newlightlevel != lightlevel )
			{
			  msg->offset = 1;
			  msg->Write<u8>( static_cast<u8>( newlightlevel ) );
			  msg.Send( client );
			  msg->offset = 1;
			  msg->Write<u8>( static_cast<u8>( lightlevel ) );
			}
			else
			  msg.Send( client );
		  }
		  client->gd->lightlevel = newlightlevel;
		}
	  }
	}

	void update_weatherregion( Client* client, WeatherRegion* weatherregion )
	{
	  if ( !client->ready )
		return;

	  if ( client->gd->weather_region == weatherregion )
	  {
		// client->gd->weather_region = NULL;  //dave commented this out 5/26/03, causing no processing to happen in following function, added force bool instead.
		client->chr->check_weather_region_change( true );
		client->chr->check_light_region_change();
	  }
	}

	void SetRegionWeatherLevel( WeatherRegion* weatherregion,
								unsigned type,
								unsigned severity,
								unsigned aux,
								int lightoverride )
	{
	  weatherregion->weathertype = static_cast<unsigned char>( type );
	  weatherregion->severity = static_cast<unsigned char>( severity );
	  weatherregion->aux = static_cast<unsigned char>( aux );
	  weatherregion->lightoverride = lightoverride;

	  Clib::ForEach( clients, update_weatherregion, weatherregion );
	}

	void check_weather_change( Client* client )
	{
	  if ( !client->ready )
		return;

	  client->chr->check_weather_region_change();
	  client->chr->check_light_region_change();
	}

	void update_all_weatherregions()
	{
	  Clib::ForEach( clients, check_weather_change );
	}

	/* there are four forms of 'name' in objinfo:
		name					 (normal)
		name%s					 (percent followed by plural-part, then null-term)
		name%s%					 (percent followed by plural-part, then percent, then more)
		wheat shea%ves/f%		 ( '%', plural part, '/', single part, '%', rest )
		Some examples:
		pil%es/e% of hides
		banana%s%
		feather%s
		Known bugs:
		1 gold coin displays as "gold coin".  There must be a bit somewhere
		that I just don't understand yet.
		*/
	string format_description( unsigned int polflags, const string& descdef, unsigned short amount, const std::string suffix )
	{
	  string desc;

	  if ( amount != 1 )
	  {
		char s[15];
		sprintf( s, "%hu ", amount );
		desc = s;
	  }
	  else
	  if ( ssopt.use_tile_flag_prefix )
	  {
		if ( polflags & Plib::FLAG::DESC_PREPEND_AN )
		{
		  desc = "an ";
		}
        else if ( polflags & Plib::FLAG::DESC_PREPEND_A )
		{
		  desc = "a ";
		}
	  }

	  // might want to consider strchr'ing here,  
	  //   if not found, strcpy/return
	  //   if found, memcpy up to strchr result, then continue as below.
	  const char *src = descdef.c_str();
	  int singular = ( amount == 1 );
	  int plural_handled = 0;
	  int phase = 0; /* 0= first part, 1=plural part, 2=singular part, 3=rest */
	  char ch;
	  while ( '\0' != ( ch = *src ) )
	  {
		if ( phase == 0 )
		{
		  if ( ch == '%' )
		  {
			plural_handled = 1;
			phase = 1;
		  }
		  else
		  {
			desc += ch;
		  }
		}
		else if ( phase == 1 )
		{
		  if ( ch == '%' )
			phase = 3;
		  else if ( ch == '/' )
			phase = 2;
		  else if ( !singular )
			desc += ch;
		}
		else if ( phase == 2 )
		{
		  if ( ch == '%' )
			phase = 3;
		  else if ( singular )
			desc += ch;
		}
		// if phase == 3 that means there are more words to come, 
		// lets loop through them to support singular/plural stuff in more than just the first word of the desc.
		else
		{
		  desc += ch;
		  phase = 0;
		}
		++src;
	  }

	  if ( !singular && !plural_handled )
		desc += 's';

	  if ( !suffix.empty() )
		desc += " " + suffix;

	  return desc;
	}

	void send_midi( Client* client, u16 midi )
	{
	  PktHelper::PacketOut<PktOut_6D> msg;
	  msg->WriteFlipped<u16>( midi );
	  msg.Send( client );
	  // cout << "Setting midi for " << client->chr->name() << " to " << midi << endl;
	}

	void register_with_supporting_multi( Item* item )
	{
	  if ( item->container == NULL )
	  {
		Multi::UMulti* multi = item->realm->find_supporting_multi( item->x, item->y, item->z );
		if ( multi )
		  multi->register_object( item );
	  }
	}


	void send_create_mobile_if_nearby_cansee( Client* client, const Character* chr )
	{
	  if ( client->ready &&                // must be logged into game
		   inrange( client->chr, chr ) &&
		   client->chr != chr &&
		   client->chr->is_visible_to_me( chr ) )
	  {
		send_owncreate( client, chr );
	  }
	}

	void send_move_mobile_if_nearby_cansee( Client* client, const Character* chr )
	{
	  if ( client->ready &&                // must be logged into game
		   inrange( client->chr, chr ) &&
		   client->chr != chr &&
		   client->chr->is_visible_to_me( chr ) )
	  {
		send_move( client, chr );
	  }
	}

	void send_create_mobile_to_nearby_cansee( const Character* chr )
	{
      ForEachPlayerInVisualRange( chr, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( zonechr == chr )
          return;
        if ( zonechr->is_visible_to_me( chr ) )
          send_owncreate( zonechr->client, chr );
      } );
	}

	void send_move_mobile_to_nearby_cansee( const Character* chr )
	{
      ForEachPlayerInVisualRange( chr, [&]( Character *zonechr )
      {
        if ( !zonechr->has_active_client() )
          return;
        if ( zonechr == chr )
          return;
        if ( zonechr->is_visible_to_me( chr ) )
          send_move( zonechr->client, chr );
      } );
	}

	Character* UpdateCharacterWeight( Item* item )
	{
	  Character* chr_owner = item->GetCharacterOwner();
	  if ( chr_owner != NULL && chr_owner->client != NULL )
	  {
		send_full_statmsg( chr_owner->client, chr_owner );
		return chr_owner;
	  }
	  return NULL;
	}

	void UpdateCharacterOnDestroyItem( Item* item )
	{
	  Character* chr_owner = item->GetCharacterOwner();
	  if ( chr_owner != NULL )
	  {
		if ( item->layer && chr_owner->is_equipped( item ) )
		{
		  PktHelper::PacketOut<PktOut_1D> msgremove;
		  msgremove->Write<u32>( item->serial_ext );
		  transmit_to_inrange( item, &msgremove->buffer, msgremove->offset, false, false );
		}
	  }
	}

	//Dave added this 12/1/02
	bool clientHasCharacter( Client* c )
	{
	  return ( c->chr != NULL );
	}

	void login_complete( Client* c )
	{
	  PktHelper::PacketOut<PktOut_55> msg;
	  msg.Send( c );
	}

	void send_feature_enable( Client* client )
	{
	  u32 clientflag = 0;
	  switch ( client->acct->uo_expansion_flag() )
	  {
		case HSA:
		  clientflag = 0x387DF;
		  client->UOExpansionFlag = HSA | SA | KR | ML | SE | AOS; // HSA needs SA- KR- ML- SE- and AOS- features (and used checks) too
		  break;
		case SA:
		  clientflag = 0x187DF;
		  client->UOExpansionFlag = SA | KR | ML | SE | AOS; // SA needs KR- ML- SE- and AOS- features (and used checks) too
		  break;
		case KR:
		  clientflag = 0x86DB;
		  client->UOExpansionFlag = KR | ML | SE | AOS; // KR needs ML- SE- and AOS-features (and used checks) too
		  break;
		case ML:
		  clientflag = 0x80DB;
		  client->UOExpansionFlag = ML | SE | AOS; // ML needs SE- and AOS-features (and used checks) too
		  break;
		case SE:
		  clientflag = 0x805B;
		  client->UOExpansionFlag = SE | AOS; // SE needs AOS-features (and used checks) too
		  break;
		case AOS:
		  clientflag = 0x801B;
		  client->UOExpansionFlag = AOS;
		  break;
		case LBR:
		  clientflag = 0x0002;
		  client->UOExpansionFlag = LBR;
		  break;
		case T2A:
		  clientflag = 0x0001;
		  client->UOExpansionFlag = T2A;
		  break;
		default:
		  break;
	  }

	  // Change flag according to the number of CharacterSlots
	  if ( client->UOExpansionFlag & AOS )
	  {
		if ( config.character_slots == 7 )
		{
		  clientflag |= 0x1000; // 7th & 6th character flag (B9 Packet)
		  clientflag &= ~0x0004; // Disable Third Dawn?
		}
		else if ( config.character_slots == 6 )
		{
		  clientflag |= 0x0020; // 6th character flag (B9 Packet)
		  clientflag &= ~0x0004;
		}
	  }

	  // Roleplay faces?
	  if ( client->UOExpansionFlag & KR )
	  {
		if ( ssopt.support_faces == 2 )
		  clientflag |= 0x2000;
	  }

	  PktHelper::PacketOut<PktOut_B9> msg;
	  if ( client->ClientType & CLIENTTYPE_60142 )
		msg->WriteFlipped<u32>( clientflag );
	  else
		msg->WriteFlipped<u16>( static_cast<u16>( clientflag ) );
	  msg.Send( client );
	}

	void send_realm_change( Client* client, Plib::Realm* realm )
	{
	  PktHelper::PacketOut<PktOut_BF_Sub8> msg;
	  msg->WriteFlipped<u16>( static_cast<u16>( 6 ) );
	  msg->offset += 2; //sub
	  msg->Write<u8>( static_cast<u8>( realm->getUOMapID() ) );
	  msg.Send( client );
	}

	void send_map_difs( Client* client )
	{
	  PktHelper::PacketOut<PktOut_BF_Sub18> msg;
	  msg->offset += 4; //len+sub
	  msg->WriteFlipped<u32>( baserealm_count );
	  for ( unsigned int i = 0; i < baserealm_count; i++ )
	  {
		msg->WriteFlipped<u32>( Realms->at( i )->getNumStaticPatches() );
		msg->WriteFlipped<u32>( Realms->at( i )->getNumMapPatches() );
	  }
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( client, len );
	}

	// FIXME : Works, except for Login. Added length check as to not mess with 1.x clients
	//         during login. Added send_season_info to handle_client_version to make up.
	void send_season_info( Client* client )
	{
	  if ( client->getversiondetail().major >= 1 )
	  {
		PktHelper::PacketOut<PktOut_BC> msg;
		msg->Write<u8>( static_cast<u8>( client->chr->realm->season() ) );
		msg->Write<u8>( static_cast<u8>( PKTOUT_BC::PLAYSOUND_YES ) );
		msg.Send( client );

		// Sending Season info resets light level in client, this fixes it during login
		if ( client->gd->weather_region != NULL &&
			 client->gd->weather_region->lightoverride != -1 &&
			 client->chr->lightoverride == -1 )
		{
		  send_light( client, client->gd->weather_region->lightoverride );
		}
	  }
	}

	//assumes new realm has been set on client->chr
	void send_new_subserver( Client* client )
	{
	  PktHelper::PacketOut<PktOut_76> msg;
	  msg->WriteFlipped<u16>( client->chr->x );
	  msg->WriteFlipped<u16>( client->chr->y );
	  msg->WriteFlipped<u16>( static_cast<u16>( client->chr->z ) );
	  msg->offset += 5; //unk0,x1,y2
	  msg->WriteFlipped<u16>( client->chr->realm->width() );
	  msg->WriteFlipped<u16>( client->chr->realm->height() );
	  msg.Send( client );
	}

	void send_fight_occuring( Client* client, Character* opponent )
	{
	  PktHelper::PacketOut<PktOut_2F> msg;
	  msg->offset++; //zero1
	  msg->Write<u32>( client->chr->serial_ext );
	  msg->Write<u32>( opponent->serial_ext );
	  msg.Send( client );
	}

	void send_damage( Character* attacker, Character* defender, u16 damage )
	{
	  if ( attacker->client != NULL )
	  {
		if ( attacker->client->ClientType & CLIENTTYPE_4070 )
		  send_damage_new( attacker->client, defender, damage );
		else
		  send_damage_old( attacker->client, defender, damage );
	  }
	  if ( ( defender->client != NULL ) && ( attacker != defender ) )
	  {
		if ( defender->client->ClientType & CLIENTTYPE_4070 )
		  send_damage_new( defender->client, defender, damage );
		else
		  send_damage_old( defender->client, defender, damage );
	  }
	}

	void send_damage_new( Client* client, Character* defender, u16 damage )
	{
	  PktHelper::PacketOut<PktOut_0B> msg;
	  msg->Write<u32>( defender->serial_ext );
	  msg->WriteFlipped<u16>( damage );
	  msg.Send( client );
	}

	void send_damage_old( Client* client, Character* defender, u16 damage )
	{
	  PktHelper::PacketOut<PktOut_BF_Sub22> msg;
	  msg->WriteFlipped<u16>( static_cast<u16>( 11 ) );
	  msg->offset += 2; //sub
	  msg->Write<u8>( static_cast<u8>( 1 ) );
	  msg->Write<u32>( defender->serial_ext );
	  if ( damage > 0xFF )
		msg->Write<u8>( static_cast<u8>( 0xFF ) );
	  else
		msg->Write<u8>( static_cast<u8>( damage ) );
	  msg.Send( client );
	}

	void sendCharProfile( Character* chr, Character* of_who, const char *title, const u16 *utext, const u16 *etext )
	{
	  PktHelper::PacketOut<PktOut_B8> msg;

	  size_t newulen = 0, newelen = 0, titlelen;

	  while ( utext[newulen] != L'\0' )
		++newulen;

	  while ( etext[newelen] != L'\0' )
		++newelen;

	  titlelen = strlen( title );

	  // Check Lengths

	  if ( titlelen > SPEECH_MAX_LEN )
		titlelen = SPEECH_MAX_LEN;
	  if ( newulen > SPEECH_MAX_LEN )
		newulen = SPEECH_MAX_LEN;
	  if ( newelen > SPEECH_MAX_LEN )
		newelen = SPEECH_MAX_LEN;

	  // Build Packet
	  msg->offset += 2;
	  msg->Write<u32>( of_who->serial_ext );
	  msg->Write( title, static_cast<u16>( titlelen + 1 ) );
	  msg->WriteFlipped( utext, static_cast<u16>( newulen ) );
	  msg->WriteFlipped( etext, static_cast<u16>( newelen ) );
	  u16 len = msg->offset;
	  msg->offset = 1;
	  msg->WriteFlipped<u16>( len );
	  msg.Send( chr->client, len );
	}
  }
}
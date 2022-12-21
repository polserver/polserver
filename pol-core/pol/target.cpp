/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Relocation of multi related cpp/h
 */


#include "target.h"

#include <stddef.h>

#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "multi/multi.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "polclass.h"
#include "realms/realm.h"
#include "regions/guardrgn.h"
#include "systems/suspiciousacts.h"
#include "ufunc.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
void handle_target_cursor( Network::Client* client, PKTBI_6C* msg )
{
  Mobile::Character* targetter = client->chr;

  if ( !targetter->tcursor2 )
  {
    SuspiciousActs::OutOfSequenceCursor( client );
    return;
  }
  targetter->tcursor2->handle_target_cursor( targetter, msg );
}

/*
Target Cursor messages come in different forms:
1) Item selected:
'create': 0
target_cursor_serial: serial of cursor
unk6: 0
selected_serial: serial of selected item
x: x-pos of selected item
y: y-pos of selected item
unk15: 0
z: z-pos of selected item
objtype: objtype of selected item
2) Character selected:
same as for (1)
3) Cursor Cancelled:
create is 0
target_cursor_serial: serial of cursor
unk6: 0
selected_serial: 0
x: 0xffff
y: 0xffff
unk15: 0
z: 0
objtype: 0
4) Static targetted:
create is 1
target_cursor_serial: serial of cursor
unk6: 0
selected_serial: 0
x: x-pos
y: y-pos
unk15: 0
z: z-pos
objtype: objtype of item
5) Backpack on a paperdoll selected
create is 0
target_cursor_serial: serial of cursor
cursor_type: 0
selected_serial: serial of backpack
x: 0xffff
y: 0xffff
unk15: 0
z: 0
objtype: objtype of backpack
*/

/*
struct {
void (*func)(Client *client, PKTBI_6C *msg);
} target_handlers[ MAX_CURSORS ];
*/

TargetCursor::TargetCursor( bool inform_on_cancel ) : inform_on_cancel_( inform_on_cancel )
{
  cursorid_ = gamestate.target_cursors._cursorid_count++;
}

// FIXME we need something like is done for item menus, where we check
// to make sure that he client has any business using this menu - in this
// case, we need to make sure the client is supposed to have an active cursor.

bool TargetCursor::send_object_cursor( Network::Client* client, PKTBI_6C::CURSOR_TYPE crstype )
{
  if ( !client->chr->target_cursor_busy() )
  {
    Network::PktHelper::PacketOut<Network::PktOut_6C> msg;
    msg->Write<u8>( PKTBI_6C::UNK1_00 );
    msg->WriteFlipped<u32>( cursorid_ );
    msg->Write<u8>( crstype );
    // rest 0
    msg.Send( client, sizeof msg->buffer );
    client->chr->tcursor2 = this;
    return true;
  }
  else
  {
    return false;
  }
}

void TargetCursor::cancel( Mobile::Character* chr )
{
  chr->tcursor2 = nullptr;
  if ( inform_on_cancel_ )
    on_target_cursor( chr, nullptr );
}

void TargetCursor::handle_target_cursor( Mobile::Character* chr, PKTBI_6C* msg )
{
  u32 target_cursor_serial = cfBEu32( msg->target_cursor_serial );
  if ( target_cursor_serial != cursorid_ )
  {
    if ( target_cursor_serial != 0 )
      SuspiciousActs::OutOfSequenceCursor( chr->client );
    cancel( chr );
    return;
  }
  if ( msg->selected_serial != 0 )  // targetted something
  {
    if ( chr->dead() )  // but is dead
    {
      if ( chr->client != nullptr )
        send_sysmessage( chr->client, "I am dead and cannot do that." );
      cancel( chr );
      return;
    }

    if ( ( chr->frozen() || chr->paralyzed() ) && !chr->can_freemove() )
    {
      if ( chr->client != nullptr )
      {
        if ( chr->frozen() )
          private_say_above( chr, chr, "I am frozen and cannot do that." );
        else if ( chr->paralyzed() )
          private_say_above( chr, chr, "I am paralyzed and cannot do that." );
      }
      cancel( chr );
      return;
    }

    u32 selected_serial = cfBEu32( msg->selected_serial );
    UObject* obj = system_find_object( selected_serial );
    if ( obj != nullptr && obj->script_isa( POLCLASS_MOBILE ) && !obj->script_isa( POLCLASS_NPC ) )
    {
      Mobile::Character* targeted = find_character( selected_serial );
      // check for when char is not logged on
      if ( targeted != nullptr )
      {
        if ( !chr->is_visible_to_me( targeted ) )
        {
          cancel( chr );
          return;
        }

        if ( msg->cursor_type == 1 )
        {
          if ( ( JusticeRegion::RunNoCombatCheck( chr->client ) == true ) ||
               ( JusticeRegion::RunNoCombatCheck( targeted->client ) == true ) )
          {
            send_sysmessage( chr->client, "Combat is not allowed in this area." );
            cancel( chr );
            return;
          }
        }
      }
    }
  }

  if ( msg->x != 0xffff || msg->selected_serial != 0 )
  {
    chr->tcursor2 = nullptr;
    on_target_cursor( chr, msg );
  }
  else
    cancel( chr );
}

FullMsgTargetCursor::FullMsgTargetCursor( void ( *func )( Mobile::Character*, PKTBI_6C* ) )
    : TargetCursor( false /* don't inform on cancel */ ), func( func )
{
}

void FullMsgTargetCursor::on_target_cursor( Mobile::Character* chr, PKTBI_6C* msg )
{
  ( *func )( chr, msg );
}


/******************************************************/
LosCheckedTargetCursor::LosCheckedTargetCursor( void ( *func )( Mobile::Character*, UObject* ),
                                                bool inform_on_cancel, bool allow_nonlocal )
    : TargetCursor( inform_on_cancel ), allow_nonlocal_( allow_nonlocal ), func( func )
{
}

void LosCheckedTargetCursor::on_target_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
{
  if ( msgin == nullptr )
  {
    ( *func )( chr, nullptr );
    return;
  }

  u32 selected_serial = cfBEu32( msgin->selected_serial );

  UObject* uobj = find_toplevel_object( selected_serial );
  // FIXME inefficient, but neither works well by itself.
  bool additlegal = false;
  if ( uobj == nullptr )
    uobj = find_legal_item( chr, selected_serial, &additlegal );

  if ( uobj == nullptr )
    uobj = system_find_multi( selected_serial );

  if ( allow_nonlocal_ && uobj == nullptr )
  {
    uobj = find_snoopable_item( selected_serial );
    additlegal = false;
  }

  if ( uobj == nullptr )
  {
    if ( chr->client != nullptr )
      send_sysmessage( chr->client, "What you selected does not seem to exist." );
    if ( inform_on_cancel_ )
      ( *func )( chr, nullptr );
    return;
  }

  if ( !additlegal && !chr->realm()->has_los( *chr, *uobj->toplevel_owner() ) )
  {
    if ( chr->client != nullptr )
      send_sysmessage( chr->client, "That is not within your line of sight." );
    if ( inform_on_cancel_ )
      ( *func )( chr, nullptr );
    return;
  }

  ( *func )( chr, uobj );
}
/******************************************************/


/******************************************************/
NoLosCheckedTargetCursor::NoLosCheckedTargetCursor( void ( *func )( Mobile::Character*, UObject* ),
                                                    bool inform_on_cancel, bool allow_nonlocal )
    : TargetCursor( inform_on_cancel ), allow_nonlocal_( allow_nonlocal ), func( func )
{
}

void NoLosCheckedTargetCursor::on_target_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
{
  if ( msgin == nullptr )
  {
    ( *func )( chr, nullptr );
    return;
  }

  u32 selected_serial = cfBEu32( msgin->selected_serial );

  UObject* uobj = find_toplevel_object( selected_serial );
  // FIXME inefficient, but neither works well by itself.
  bool additlegal = false;
  if ( uobj == nullptr )
    uobj = find_legal_item( chr, selected_serial, &additlegal );

  if ( uobj == nullptr )
    uobj = system_find_multi( selected_serial );

  if ( allow_nonlocal_ && uobj == nullptr )
    uobj = find_snoopable_item( selected_serial );

  if ( uobj == nullptr )
  {
    if ( chr->client != nullptr )
      send_sysmessage( chr->client, "What you selected does not seem to exist." );
    if ( inform_on_cancel_ )
      ( *func )( chr, nullptr );
    return;
  }

  ( *func )( chr, uobj );
}
/******************************************************/


LosCheckedCoordCursor::LosCheckedCoordCursor( void ( *func )( Mobile::Character*, PKTBI_6C* ),
                                              bool inform_on_cancel )
    : TargetCursor( inform_on_cancel ), func_( func )
{
}

bool LosCheckedCoordCursor::send_coord_cursor( Network::Client* client )
{
  if ( !client->chr->target_cursor_busy() )
  {
    Network::PktHelper::PacketOut<Network::PktOut_6C> msg;
    msg->Write<u8>( PKTBI_6C::UNK1_01 );
    msg->WriteFlipped<u32>( cursorid_ );
    msg->Write<u8>( PKTBI_6C::CURSOR_TYPE_NEUTRAL );
    // rest 0
    msg.Send( client, sizeof msg->buffer );
    client->chr->tcursor2 = this;
    return true;
  }
  else
  {
    return false;
  }
}

void LosCheckedCoordCursor::on_target_cursor( Mobile::Character* chr, PKTBI_6C* msg )
{
  ( *func_ )( chr, msg );
}


MultiPlacementCursor::MultiPlacementCursor( void ( *func )( Mobile::Character*, PKTBI_6C* ) )
    : TargetCursor( true ), func_( func )
{
}

void MultiPlacementCursor::send_placemulti( Network::Client* client, unsigned int objtype,
                                            int flags, s16 xoffset, s16 yoffset, u32 hue )
{
  Network::PktHelper::PacketOut<Network::PktOut_99> msg;
  msg->Write<u8>( 0x1u );
  msg->WriteFlipped<u32>( cursorid_ );
  msg->offset += 12;  // 12x u8 unk
  u16 multiid = Items::find_multidesc( objtype ).multiid;
  multiid +=
      static_cast<u16>( ( flags & Multi::CRMULTI_FACING_MASK ) >> Multi::CRMULTI_FACING_SHIFT );
  msg->WriteFlipped<u16>( multiid );
  msg->WriteFlipped<s16>( xoffset );
  msg->WriteFlipped<s16>( yoffset );
  msg->offset += 2;  // u16 maybe_zoffset
  if ( client->ClientType & Network::CLIENTTYPE_7090 )
    msg->WriteFlipped<u32>( hue );
  msg.Send( client );
  client->chr->tcursor2 = this;
}

void MultiPlacementCursor::on_target_cursor( Mobile::Character* chr, PKTBI_6C* msg )
{
  ( *func_ )( chr, msg );
}


NoLosCharacterCursor::NoLosCharacterCursor( void ( *func )( Mobile::Character*,
                                                            Mobile::Character* ) )
    : TargetCursor( false /* don't inform on cancel */ ), func( func )
{
}

void NoLosCharacterCursor::on_target_cursor( Mobile::Character* targetter, PKTBI_6C* msgin )
{
  if ( msgin == nullptr )
    return;
  u32 selected_serial = cfBEu32( msgin->selected_serial );
  Mobile::Character* chr = find_character( selected_serial );
  if ( chr != nullptr )
    ( *func )( targetter, chr );
}

NoLosUObjectCursor::NoLosUObjectCursor( void ( *func )( Mobile::Character*, UObject* ),
                                        bool inform_on_cancel )
    : TargetCursor( inform_on_cancel ), func( func )
{
}

void NoLosUObjectCursor::on_target_cursor( Mobile::Character* targetter, PKTBI_6C* msgin )
{
  if ( msgin == nullptr )
  {
    ( *func )( targetter, nullptr );
    return;
  }
  u32 selected_serial = cfBEu32( msgin->selected_serial );

  UObject* obj = system_find_object( selected_serial );
  if ( obj )
  {
    ( *func )( targetter, obj );
  }
  else if ( inform_on_cancel_ )
  {
    ( *func )( targetter, nullptr );
  }
}

Cursors::Cursors()
    : _cursorid_count( 1 ),  // NOTE: the id is 1-based (seems that the stealth client has problem
                             // with serial==0)
      los_checked_script_cursor( Module::handle_script_cursor, true ),
      nolos_checked_script_cursor( Module::handle_script_cursor, true ),
      los_checked_allow_nonlocal_script_cursor( Module::handle_script_cursor, true, true ),
      nolos_checked_allow_nonlocal_script_cursor( Module::handle_script_cursor, true, true ),

      add_member_cursor( handle_add_member_cursor ),
      remove_member_cursor( handle_remove_member_cursor ),
      ident_cursor( handle_ident_cursor ),
      script_cursor2( Module::handle_coord_cursor, true ),
      multi_placement_cursor( Module::handle_coord_cursor ),
      repdata_cursor( show_repdata ),
      startlog_cursor( start_packetlog ),
      stoplog_cursor( stop_packetlog )
{
}
}  // namespace Core
}  // namespace Pol

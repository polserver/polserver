/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Relocation of multi related cpp/h

Notes
=======

*/

#include "clib/stl_inc.h"

#include <stdio.h>

#include "clib/endian.h"
#include "clib/logfile.h"

#include "plib/realm.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "fnsearch.h"
#include "item/itemdesc.h"
#include "los.h"
#include "msghandl.h"
#include "objtype.h"
#include "pktboth.h"
#include "realms.h"
#include "ufunc.h"
#include "uvars.h"
#include "multi/multi.h"

#include "target.h"
#include "polclass.h"

#include "guardrgn.h"
#include "network/cgdata.h"

#define MAX_CURSORS 500

TargetCursor* target_cursors[ MAX_CURSORS ];
static u32 cursorid_count;

void handle_target_cursor( Client *client, PKTBI_6C *msg )
{
	Character* targetter = client->chr;
	u32 target_cursor_serial = cfBEu32( msg->target_cursor_serial );
	
		// does target cursor even exist?
	if (target_cursor_serial >= cursorid_count)
	{
		return;
	}

	TargetCursor* tcursor = target_cursors[ target_cursor_serial ];
	if (tcursor != targetter->tcursor2)
	{
		cerr << targetter->acct->name() << "/" << targetter->name() << " used out of sequence cursor." << endl;
		Log( "%s/%s used out-of-sequence target cursor\n", targetter->acct->name(), targetter->name().c_str() );

		targetter->tcursor2 = NULL;
		return;
	}
	
	targetter->tcursor2 = NULL;
	tcursor->handle_target_cursor( targetter, msg );
}

MESSAGE_HANDLER( PKTBI_6C, handle_target_cursor );
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

TargetCursor::TargetCursor( bool inform_on_cancel ) :
	inform_on_cancel_( inform_on_cancel )
{
	if (cursorid_count >= MAX_CURSORS)
	{
		throw runtime_error( "Too many targetting cursors!" );
	}

	cursorid_ = cursorid_count;

	target_cursors[ cursorid_ ] = this;

	cursorid_count++;
}

// FIXME we need something like is done for item menus, where we check
// to make sure that he client has any business using this menu - in this
// case, we need to make sure the client is supposed to have an active cursor.

bool TargetCursor::send_object_cursor( Client *client, 
									   PKTBI_6C::CURSOR_TYPE crstype )
{
	if (!client->chr->target_cursor_busy())
	{
		static PKTBI_6C msg;
		msg.msgtype = PKTBI_6C_ID;
		msg.unk1 = PKTBI_6C::UNK1_00;
		msg.target_cursor_serial = ctBEu32( cursorid_ );
		msg.cursor_type = static_cast<u8>(crstype); 
		client->transmit( &msg, sizeof msg );
		client->chr->tcursor2 = this;
		return true;
	}
	else
	{
		return false;
	}
}

void TargetCursor::cancel( Character* chr )
{
	chr->tcursor2 = NULL;
	if (inform_on_cancel_)
		on_target_cursor( chr, NULL );
}

void TargetCursor::handle_target_cursor( Character* chr, PKTBI_6C* msg )
{
	// printf( "Targetting Cursor\n" );
	
	if (msg->selected_serial != 0)	 // targetted something
	{

		if (chr->dead())						// but is dead
		{
			if (chr->client != NULL) send_sysmessage( chr->client, "I am dead and cannot do that." );
			cancel( chr );
			return;
		}
		
		if ( (chr->frozen() || chr->paralyzed()) && !chr->setting_enabled("freemove") )
		{
			if ( chr->client != NULL )
				private_say_above( chr, chr, "I am frozen and cannot do that." );
			cancel(chr);
			return;
		}

		JusticeRegion* cur_justice_region = chr->client->gd->justice_region;
		u32 selected_serial = cfBEu32( msg->selected_serial );
		UObject* obj = system_find_object( selected_serial );
		if ( obj != NULL && obj->script_isa(POLCLASS_MOBILE) && 
			!obj->script_isa(POLCLASS_NPC) )
		{
			Character* targeted = find_character( selected_serial );
			// check for when char is not logged on
			if( targeted != NULL )
			{
				if( !chr->is_visible_to_me(targeted) )
				{
					cancel(chr);
					return;
				}

				if( msg->cursor_type == 1 )
				{
					JusticeRegion* cur_justice_region_t = targeted->client->gd->justice_region;
					if( (cur_justice_region->RunNoCombatCheck(chr->client) == true) 
						|| (cur_justice_region_t->RunNoCombatCheck(targeted->client) == true) )
					{
						send_sysmessage( chr->client, "Combat is not allowed in this area." );
						cancel(chr);
						return;
					}
				}
			}
		}
	}

	if (msg->x != 0xffff || msg->selected_serial != 0)
		on_target_cursor( chr, msg );
	else 
		cancel( chr );
}

FullMsgTargetCursor::FullMsgTargetCursor( void (*func)(Character*, PKTBI_6C*) ) :
	TargetCursor( false /* don't inform on cancel */ ),
	func(func)
{
}

void FullMsgTargetCursor::on_target_cursor( Character* chr, PKTBI_6C* msg )
{
	(*func)(chr, msg);
}



/******************************************************/
LosCheckedTargetCursor::LosCheckedTargetCursor( 
								   void (*func)(Character*, UObject*),
								   bool inform_on_cancel ) :
	TargetCursor( inform_on_cancel ),
	func(func)
{
}

void LosCheckedTargetCursor::on_target_cursor( Character* chr, PKTBI_6C* msgin )
{
	if (msgin == NULL)
	{
		(*func)(chr,NULL);
		return;
	}

	u32 selected_serial = cfBEu32( msgin->selected_serial );

	// printf( "  LOS Checked targetting: selected_serial: %lx\n", selected_serial );
	
	UObject* uobj = find_toplevel_object( selected_serial );
			// FIXME inefficient, but neither works well by itself.
	bool additlegal = false;
	if (uobj == NULL)
		uobj = find_legal_item( chr, selected_serial, &additlegal );
	
	if (uobj == NULL)
		uobj = system_find_multi( selected_serial );
	
	if (uobj == NULL)
	{
		if (chr->client != NULL) send_sysmessage( chr->client, "What you selected does not seem to exist." );
		if (inform_on_cancel_)
			(*func)(chr,NULL);
		return;
	}
 
	if (!additlegal && !chr->realm->has_los( *chr, *uobj->toplevel_owner() ))
	{
		if (chr->client != NULL) send_sysmessage( chr->client, "That is not within your line of sight." );
		if (inform_on_cancel_)
			(*func)(chr,NULL);
		return;
	}

	(*func)(chr, uobj);
}
/******************************************************/



/******************************************************/
NoLosCheckedTargetCursor::NoLosCheckedTargetCursor( 
								   void (*func)(Character*, UObject*),
								   bool inform_on_cancel ) :
	TargetCursor( inform_on_cancel ),
	func(func)
{
}

void NoLosCheckedTargetCursor::on_target_cursor( Character* chr, PKTBI_6C* msgin )
{
	if (msgin == NULL)
	{
		(*func)(chr,NULL);
		return;
	}

	u32 selected_serial = cfBEu32( msgin->selected_serial );

	// printf( "  LOS Checked targetting: selected_serial: %lx\n", selected_serial );
	
	UObject* uobj = find_toplevel_object( selected_serial );
			// FIXME inefficient, but neither works well by itself.
	bool additlegal = false;
	if (uobj == NULL)
		uobj = find_legal_item( chr, selected_serial, &additlegal );
	
	if (uobj == NULL)
		uobj = system_find_multi( selected_serial );
	
	if (uobj == NULL)
	{
		if (chr->client != NULL) send_sysmessage( chr->client, "What you selected does not seem to exist." );
		if (inform_on_cancel_)
			(*func)(chr,NULL);
		return;
	}

	(*func)(chr, uobj);
}
/******************************************************/








LosCheckedCoordCursor::LosCheckedCoordCursor( 
							void (*func)(Character*,PKTBI_6C*),
							bool inform_on_cancel ) :
	TargetCursor( inform_on_cancel ),
	func_(func)
{
}

bool LosCheckedCoordCursor::send_coord_cursor( Client* client )
{
	if (!client->chr->target_cursor_busy())
	{
		static PKTBI_6C msg;
		msg.msgtype = PKTBI_6C_ID;
		msg.unk1 = PKTBI_6C::UNK1_01;
		msg.target_cursor_serial = ctBEu32( cursorid_ );	
		msg.cursor_type = PKTBI_6C::CURSOR_TYPE_NEUTRAL;
		client->transmit( &msg, sizeof msg );
		client->chr->tcursor2 = this;
		return true;
	}
	else
	{
		return false;
	}

}

void LosCheckedCoordCursor::on_target_cursor( Character* chr, PKTBI_6C* msg )
{
	(*func_)(chr,msg);
}



MultiPlacementCursor::MultiPlacementCursor( void (*func)(Character*, PKTBI_6C*) ) :
	TargetCursor( true ),
	func_(func)
{
}

void MultiPlacementCursor::send_placemulti( Client* client, unsigned short objtype, long flags, s16 xoffset, s16 yoffset )
{
	PKTBI_99 msg;
	memset(&msg,0,sizeof msg);
	msg.msgtype = PKTBI_99_ID;
	msg.unk1_01 = 0x01;
	msg.deed = ctBEu32(cursorid_);
	u16 multiid = find_multidesc(objtype).multiid;
	multiid += static_cast<u16>((flags & CRMULTI_FACING_MASK) >> CRMULTI_FACING_SHIFT);
	msg.graphic = ctBEu16( multiid );
	msg.xoffset = ctBEu16( xoffset );
	msg.yoffset = ctBEu16( yoffset );
	client->transmit( &msg, sizeof msg );
	client->chr->tcursor2 = this;
}

void MultiPlacementCursor::on_target_cursor( Character* chr, PKTBI_6C* msg )
{
	(*func_)(chr,msg);
}




NoLosCharacterCursor::NoLosCharacterCursor( void (*func)(Character*, Character*) ) :
	TargetCursor( false /* don't inform on cancel */ ),
	func(func)
{
}

void NoLosCharacterCursor::on_target_cursor( Character* targetter, PKTBI_6C* msgin )
{
	if (msgin == NULL)
		return;
	u32 selected_serial = cfBEu32( msgin->selected_serial );
	Character* chr = find_character( selected_serial );
	if (chr != NULL)
		(*func)(targetter, chr);
}

NoLosUObjectCursor::NoLosUObjectCursor( void (*func)(Character*, UObject*),
										bool inform_on_cancel) :
	TargetCursor(inform_on_cancel),
	func(func)
{
}

void NoLosUObjectCursor::on_target_cursor( Character* targetter, PKTBI_6C* msgin )
{
	if (msgin == NULL)
	{
		(*func)(targetter,NULL);
		return;
	}
	u32 selected_serial = cfBEu32( msgin->selected_serial );

	UObject* obj = system_find_object( selected_serial );
	if (obj)
	{
		(*func)(targetter, obj);
	}
	else if (inform_on_cancel_)
	{ 
		(*func)(targetter, NULL);
	}
}

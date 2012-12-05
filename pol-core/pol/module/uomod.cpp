/*
History
=======
2005/03/02 Shinigami: internal_MoveItem - item took from container don't had the correct realm
2005/04/02 Shinigami: mf_CreateItemCopyAtLocation - added realm param
2005/04/28 Shinigami: mf_EnumerateItemsInContainer/enumerate_contents - added flag to list content of locked container too
2005/04/28 Shinigami: added mf_SecureTradeWin - to init secure trade via script over long distances
					  added mf_MoveItemToSecureTradeWin - to move item to secure trade window via script
2005/04/31 Shinigami: mf_EnumerateItemsInContainer - error message added
2005/06/01 Shinigami: added mf_Attach - to attach a Script to a Character
					  mf_MoveItem - stupid non-realm anti-crash bugfix (e.g. for intrinsic weapons without realm)
					  added mf_ListStaticsAtLocation - list static Items at location
					  added mf_ListStaticsNearLocation - list static Items around location
					  added mf_GetStandingLayers - get layer a mobile can stand
2005/06/06 Shinigami: mf_ListStatics* will return multi Items too / flags added
2005/07/04 Shinigami: mf_ListStatics* constants renamed
					  added Z to mf_ListStatics*Location - better specify what u want to get
					  modified Z handling of mf_ListItems*Location* and mf_ListMobilesNearLocation*
					  better specify what u want to get
					  added realm-based coord check to mf_ListItems*Location*,
					  mf_List*InBox and mf_ListMobilesNearLocation*
					  added mf_ListStaticsInBox - list static items in box
2005/07/07 Shinigami: realm-based coord check in mf_List*InBox disabled
2005/09/02 Shinigami: mf_Attach - smaller bug which allowed u to attach more than one Script to a Character
2005/09/03 Shinigami: mf_FindPath - added Flags to support non-blocking doors
2005/09/23 Shinigami: added mf_SendStatus - to send full status packet to support extensions
2005/12/06 MuadDib:   Rewrote realm handling for Move* and internal_move_item. New realm is
					  to be passed to function, and oldrealm handling is done within the function.
					  Container's still save and check oldrealm as before to update clients.
2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
2006/05/10 Shinigami: mf_ListMobilesNearLocationEx - just a logical mismatch
2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
2006/06/08 Shinigami: started to add FindPath logging
2006/09/17 Shinigami: mf_SendEvent() will return error "Event queue is full, discarding event"
2006/11/25 Shinigami: mf_GetWorldHeight() fixed bug because z was'n initialized
2007/05/03 Turley:	added mf_GetRegionNameAtLocation - get name of justice region
2007/05/04 Shinigami: added mf_GetRegionName - get name of justice region by objref
2007/05/07 Shinigami: fixed a crash in mf_GetRegionName using NPCs; added TopLevelOwner
2008/07/08 Turley:	Added mf_IsStackable - Is item stackable?
2008/07/21 Mehdorn:   mf_ReserveItem() will return 2 if Item is already reserved by me (instead of 1)
2009/07/27 MuadDib:   Packet Struct Refactoring
2009/08/08 MuadDib:   mf_SetRawSkill(),  mf_GetRawSkill(),  mf_ApplyRawDamage(), mf_GameStat(), 
					  mf_AwardRawPoints(), old replace_properties(), mf_GetSkill() cleaned out.
2009/08/25 Shinigami: STLport-5.2.1 fix: additional parentheses in mf_ListMultisInBox
2009/09/03 MuadDib:	  Changes for account related source file relocation
					  Changes for multi related source file relocation
2009/09/14 MuadDib:   Slot support added to creation/move to container.
2009/09/15 MuadDib:   Multi registration/unregistration support added.
2009/10/22 Turley:	added CanWalk()
2009/11/19 Turley:	added flag param to UpdateMobile controls if packet 0x78 or 0x77 should be send - Tomi
2009/12/02 Turley:	added config.max_tile_id - Tomi
2009/12/17 Turley:	CloseWindow( character, type, object ) - Tomi
2010/01/15 Turley:	(Tomi) season stuff
					  (Tomi) SpeakPowerWords font and color params
2011/11/12 Tomi:	  added extobj.mount

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#endif


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib.h"
#include "../../clib/endian.h"
#include "../../clib/esignal.h"
#include "../../clib/logfile.h"
#include "../../clib/mlog.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../clib/unicode.h"
#include "../../clib/weakptr.h"

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/dict.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/impstr.h"
#include "../../bscript/token.h"

#include "../../plib/mapcell.h"
#include "../../plib/mapshape.h"
#include "../../plib/maptile.h"
#include "../../plib/realm.h"

#include "../network/client.h"
#include "../network/clienttransmit.h"

#include "../action.h"
#include "../cfgrepos.h"
#include "../core.h"
#include "../extobj.h"
#include "../eventid.h"
#include "../fnsearch.h"
#include "../guardrgn.h"
#include "../item/itemdesc.h"
#include "../listenpt.h"
#include "../los.h"
#include "../mdelta.h"
#include "../menu.h"
#include "../mobile/charactr.h"
#include "../multi/boat.h"
#include "../multi/multidef.h"
#include "../network/cgdata.h"
#include "../network/packets.h"
#include "../npc.h"
#include "../objecthash.h"
#include "../objtype.h"
#include "../pktboth.h"
#include "../pktin.h"
#include "../polcfg.h"
#include "../polclass.h"
#include "../polsig.h"
#include "../poltype.h"
#include "../profile.h"
#include "../realms.h"
#include "../savedata.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../skilladv.h"
#include "../spells.h"
#include "../ssopt.h"
#include "../target.h"
#include "../udatfile.h"
#include "../ufunc.h"
#include "../umanip.h"
#include "../uoexec.h"
#include "../uofile.h"
#include "../uopathnode.h"
#include "../uoscrobj.h"
#include "../ustruct.h"
#include "../uvars.h"
#include "../uworld.h"
#include "cfgmod.h"
#include "osmod.h"
#include "uomod.h"

#define CONST_DEFAULT_ZRANGE 19

class EMenuObjImp : public BApplicObj< Menu >
{
public:
	EMenuObjImp( const Menu& m ) : BApplicObj<Menu>(&menu_type, m) {}
	virtual const char* typeOf() const { return "MenuRef"; }
	virtual int typeOfInt() const { return OTMenuRef; }
	virtual BObjectImp* copy() const { return new EMenuObjImp(value()); }
};


UOExecutorModule::UOExecutorModule(UOExecutor& exec) :
	ExecutorModule("UO", exec),
	uoexec( exec ),
	target_cursor_chr(NULL),
	menu_selection_chr(NULL),
	prompt_chr(NULL),
	gump_chr(NULL),
	textentry_chr(NULL),
	resurrect_chr(NULL),
	selcolor_chr(NULL),
	target_options(0),
	attached_chr_(NULL),
	attached_npc_(NULL),
	controller_(NULL),
	reserved_items_(),
	registered_for_speech_events(false)
{
}

UOExecutorModule::~UOExecutorModule()
{
	while (!reserved_items_.empty())
	{
		Item* item = reserved_items_.back().get();
		item->inuse( false );
		reserved_items_.pop_back();
	}

	if (target_cursor_chr != NULL)
	{
		// CHECKME can we cancel the cursor request?
		target_cursor_chr->client->gd->target_cursor_uoemod = NULL;
		target_cursor_chr = NULL;
	}
	if (menu_selection_chr != NULL)
	{
		menu_selection_chr->client->gd->menu_selection_uoemod = NULL;
		menu_selection_chr = NULL;
	}
	if (prompt_chr != NULL)
	{
		prompt_chr->client->gd->prompt_uoemod = NULL;
		prompt_chr = NULL;
	}
	if (gump_chr != NULL)
	{
		gump_chr->client->gd->remove_gumpmod( this );
		// gump_chr->client->gd->gump_uoemod = NULL;
		gump_chr = NULL;
	}
	if (textentry_chr != NULL)
	{
		textentry_chr->client->gd->textentry_uoemod = NULL;
		textentry_chr = NULL;
	}
	if (resurrect_chr != NULL)
	{
		resurrect_chr->client->gd->resurrect_uoemod = NULL;
		resurrect_chr = NULL;
	}
	if (selcolor_chr != NULL)
	{
		selcolor_chr->client->gd->selcolor_uoemod = NULL;
		selcolor_chr = NULL;
	}
	if (attached_chr_ != NULL)
	{
		passert( attached_chr_->script_ex == &uoexec );
		attached_chr_->script_ex = NULL;
		attached_chr_ = NULL;
	}
	if (registered_for_speech_events)
	{
		deregister_from_speech_events( &uoexec );
	}
}

BObjectImp* UOExecutorModule::mf_Attach(/* Character */)
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (attached_chr_ == NULL)
		{
			if (chr->script_ex == NULL)
			{
				attached_chr_ = chr;
				attached_chr_->script_ex = &uoexec;

				return new BLong(1);
			}
			else
				return new BError( "Another script still attached." );
		}
		else
			return new BError( "Another character still attached." );
	}
	else
		return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_Detach()
{
	if (attached_chr_ != NULL)
	{
		passert( attached_chr_->script_ex == &uoexec );
		attached_chr_->script_ex = NULL;
		attached_chr_ = NULL;
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

static bool item_create_params_ok( u32 objtype, int amount )
{
	return (objtype >= UOBJ_ITEM__LOWEST && objtype <= config.max_objtype) &&
		   amount > 0 &&
		   amount <= 60000L;
}

BObjectImp* _create_item_in_container(
									   UContainer* cont,
									   const ItemDesc* descriptor,
									   unsigned short amount,
									   bool force_stacking,
									   UOExecutorModule* uoemod )
{
	if ((tile_flags( descriptor->graphic ) & FLAG::STACKABLE) || force_stacking)
	{
		for( UContainer::const_iterator itr = cont->begin(); itr != cont->end(); ++itr )
		{
			Item *item = GET_ITEM_PTR( itr );
			ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends

			if (item->objtype_ == descriptor->objtype &&
				!item->newbie() &&
				item->color == descriptor->color && //dave added 5/11/3, only add to existing stack if is default color
				item->has_only_default_cprops(descriptor) &&	   //dave added 5/11/3, only add to existing stack if default cprops
				(!item->inuse() || (uoemod && uoemod->is_reserved_to_me(item))) &&
				item->can_add_to_self(amount))
			{

				//DAVE added this 11/17, call can/onInsert scripts for this container
				Character* chr_owner = cont->GetCharacterOwner();
				if(chr_owner == NULL)
					chr_owner = uoemod->controller_.get();

				//If the can insert script fails for combining a stack, we'll let the create new item code below handle it
				// what if a cannInsert script modifies (adds to) the container we're iterating over? (they shouldn't do that)
				// FIXME oh my, this makes no sense.  'item' in this case is already in the container.
				if(!cont->can_insert_increase_stack( chr_owner, UContainer::MT_CORE_CREATED, item, amount, NULL ))
					continue;
				if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
				{
					return new BError( "Item was destroyed in CanInsert Script" );
				}
				#ifdef PERGON
				item->ct_merge_stacks_pergon( amount ); // Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
				#endif

				int newamount = item->getamount();
				newamount += amount;
				item->setamount( static_cast<unsigned short>(newamount) );

				update_item_to_inrange( item );
				UpdateCharacterWeight( item );

				// FIXME again, this makes no sense, item is already in the container.
				cont->on_insert_increase_stack( chr_owner, UContainer::MT_CORE_CREATED, item, amount );
				if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
				{
					return new BError( "Item was destroyed in OnInsert Script" );
				}


				return new EItemRefObjImp(item);
			}
		}
	}
	else if (amount != 1 && !force_stacking)
	{
		return new BError( "That item is not stackable.  Create one at a time." );
	}

	Item* item = Item::create( *descriptor );
	if (item != NULL)
	{
		ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends
		item->realm = cont->realm;
		item->setamount( amount );

		if (cont->can_add( *item ))
		{
			if (!descriptor->create_script.empty())
			{
				BObjectImp* res = run_script_to_completion( descriptor->create_script, new EItemRefObjImp( item ) );
				if (!res->isTrue())
				{
					item->destroy();
					return res;
				}
				else
				{
					BObject ob( res );
				}
				if (!cont->can_add( *item ))
				{							   // UNTESTED
					item->destroy();
					return new BError( "Couldn't add item to container after create script ran" );
				}
			}

			// run before owner is found. No need to find owner if not even able to use slots.
			u8 slotIndex = item->slot_index();
			if (!cont->can_add_to_slot(slotIndex))
			{
				item->destroy();
				return new BError( "No slots available in this container" );
			}
			if ( !item->slot_index(slotIndex) )
			{
				item->destroy();
				return new BError( "Couldn't set slot index on item" );
			}

			//DAVE added this 11/17, call can/onInsert scripts for this container
			Character* chr_owner = cont->GetCharacterOwner();
			if(chr_owner == NULL)
				chr_owner = uoemod->controller_.get();

			if(!cont->can_insert_add_item( chr_owner, UContainer::MT_CORE_CREATED, item ))
			{
				item->destroy();
				//FIXME: try to propogate error from returned result of the canInsert script
				return new BError( "Could not insert item into container." );
			}
			if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
			{
				return new BError( "Item was destroyed in CanInsert Script" );
			}

			cont->add_at_random_location( item );
			update_item_to_inrange( item );
			//DAVE added this 11/17, refresh owner's weight on item insert
			UpdateCharacterWeight( item );

			cont->on_insert_add_item( chr_owner, UContainer::MT_CORE_CREATED, item );
			if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
			{
				return new BError( "Item was destroyed in OnInsert Script" );
			}

			return new EItemRefObjImp( item );
		}
		else
		{
			item->destroy();
			return new BError( "That container is full" );
		}
	}
	else
	{
		return new BError( "Failed to create that item type" );
	}
}

BObjectImp* UOExecutorModule::mf_CreateItemInContainer()
{
	Item* item;
	const ItemDesc* descriptor;
	int amount;

	if (getItemParam( exec, 0, item ) &&
		getObjtypeParam( exec, 1, descriptor ) &&
		getParam( 2, amount ) &&
		item_create_params_ok( descriptor->objtype, amount ))
	{
		if (item->isa( UObject::CLASS_CONTAINER ))
		{
			return _create_item_in_container( static_cast<UContainer*>(item),
												descriptor,
												static_cast<unsigned short>(amount),
												false,
												this );
		}
		else
		{
			return new BError( "That is not a container" );
		}
	}
	else
	{
		return new BError( "A parameter was invalid" );
	}
}

BObjectImp* UOExecutorModule::mf_CreateItemInInventory()
{
	Item* item;
	const ItemDesc* descriptor;
	int amount;

	if (getItemParam( exec, 0, item ) &&
		getObjtypeParam( exec, 1, descriptor ) &&
		getParam( 2, amount ) &&
		item_create_params_ok( descriptor->objtype, amount ))
	{
		if (item->isa( UObject::CLASS_CONTAINER ))
		{
			return _create_item_in_container( static_cast<UContainer*>(item),
											  descriptor,
											  static_cast<unsigned short>(amount),
											  true,
											  this );
		}
		else
		{
			return new BError( "That is not a container" );
		}
	}
	else
	{
		return new BError( "A parameter was invalid" );
	}
}


BObjectImp* UOExecutorModule::broadcast()
{
	const char *text;
	unsigned short font;
	unsigned short color;
	text = exec.paramAsString(0);
	if (text &&
		getParam( 1, font ) && // todo: getFontParam
		getParam( 2, color ))   // todo: getColorParam
	{
		::broadcast( text, font, color );
		return new BLong( 1 );
	}
	else
	{
		return NULL;
	}
}

/* Special containers (specifically, bankboxes, but probably any other
   "invisible" accessible container) seem to work thus:
   They sit in layer 0x1D.  The player is told to "wear_item" the item,
   then the gump and container contents are sent.
   We'll put a reference to this item in the character's additional_legal_items
   container, which is flushed whenever he moves.
   It might be better to actually put it in that layer, because that way
   it implicitly is at the same location (location of the wornitems container)
*/
BObjectImp* UOExecutorModule::mf_SendOpenSpecialContainer()
{
	Character* chr;
	Item* item;
	if (!getCharacterParam( exec, 0, chr ) ||
		!getItemParam( exec, 1, item ))
	{
		return new BError( "Invalid parameter type" );
	}

	if (!chr->has_active_client())
	{
		return new BError( "No client attached" );
	}
	if (!item->isa( UObject::CLASS_CONTAINER ))
	{
		return new BError( "That isn't a container" );
	}

	u8 save_layer = item->layer;
	item->layer = LAYER_BANKBOX;
	send_wornitem( chr->client, chr, item );
	item->layer = save_layer;
	item->x = chr->x;
	item->y = chr->y;
	item->z = chr->z;
	item->double_click( chr->client );			  // open the container on the client's screen
	chr->add_remote_container( item );

	return new BLong(1);
}

BObjectImp* open_trade_window( Client* client, Character* dropon );
BObjectImp* UOExecutorModule::mf_SecureTradeWin()
{
	Character* chr;
	Character* chr2;
	if (!getCharacterParam( exec, 0, chr ) ||
		!getCharacterParam( exec, 1, chr2 ))
	{
		return new BError( "Invalid parameter type." );
	}

	if (chr == chr2)
	{
		return new BError( "You can't trade with yourself." );
	}

	if (!chr->has_active_client())
	{
		return new BError( "No client attached." );
	}
	if (!chr2->has_active_client())
	{
		return new BError( "No client attached." );
	}
	
	return open_trade_window( chr->client, chr2 );
}

void cancel_trade( Character* chr1 );
BObjectImp* UOExecutorModule::mf_CloseTradeWindow()
{
	Character* chr;
	if ( !getCharacterParam( exec, 0, chr ) )
	{
		return new BError( "Invalid parameter type." );
	}
	if ( !chr->trading_with )
		return new BError("Mobile is not currently trading with anyone.");
	
	cancel_trade(chr);
	return new BLong(1);
}

BObjectImp* UOExecutorModule::mf_SendViewContainer()
{
	Character* chr;
	Item* item;
	if (!getCharacterParam( exec, 0, chr ) ||
		!getItemParam( exec, 1, item ))
	{
		return new BError( "Invalid parameter type" );
	}

	if (!chr->has_active_client())
	{
		return new BError( "No client attached" );
	}
	if (!item->isa( UObject::CLASS_CONTAINER ))
	{
		return new BError( "That isn't a container" );
	}
	UContainer* cont = static_cast<UContainer*>(item);
	chr->client->pause();
	send_open_gump( chr->client, *cont );
	send_container_contents( chr->client, *cont );
	chr->client->restart();
	return new BLong(1);
}

BObjectImp* UOExecutorModule::mf_FindObjtypeInContainer()
{
	Item* item;
	unsigned int objtype;
	if (!getItemParam( exec, 0, item ) ||
		!getObjtypeParam( exec, 1, objtype ))
	{
		return new BError( "Invalid parameter type" );
	}
	if (!item->isa( UObject::CLASS_CONTAINER ))
	{
		return new BError( "That is not a container" );
	}

	UContainer* cont = static_cast<UContainer*>(item);
	Item* found = cont->find_toplevel_objtype( objtype );
	if (found == NULL)
		return new BError( "No items were found" );
	else
		return new EItemRefObjImp( found );
}


BObjectImp* UOExecutorModule::mf_SendSysMessage()
{
	Character* chr;
	const String* ptext;
	unsigned short font;
	unsigned short color;

	if (getCharacterParam( exec, 0, chr ) &&
		( (ptext = getStringParam( 1 )) != NULL) &&
		getParam( 2, font ) &&
		getParam( 3, color ))
	{
		if (chr->has_active_client())
		{
			send_sysmessage( chr->client, ptext->data(), font, color );
			return new BLong(1);
		}
		else
		{
			return new BError( "Mobile has no active client" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_PrintTextAbove()
{
	UObject* obj;
	const String* ptext;
	unsigned short font;
	unsigned short color;
	int journal_print;

	if (getUObjectParam( exec, 0, obj ) &&
		getStringParam( 1, ptext ) &&
		getParam( 2, font ) &&
		getParam( 3, color ) &&
		getParam( 4, journal_print) )
	{
		return new BLong( say_above( obj, ptext->data(), font, color, journal_print ) );
	}
	else
	{
		return new BError( "A parameter was invalid" );
	}
}
BObjectImp* UOExecutorModule::mf_PrivateTextAbove()
{
	Character* chr;
	UObject* obj;
	const String* ptext;
	unsigned short font;
	unsigned short color;
	int journal_print;

	if (getUObjectParam( exec, 0, obj ) &&
		getStringParam( 1, ptext ) &&
		getCharacterParam( exec, 2, chr ) &&
		getParam( 3, font ) &&
		getParam( 4, color ) &&
		getParam( 5, journal_print) )
	{
		return new BLong( private_say_above( chr, obj, ptext->data(), font, color, journal_print ) );
	}
	else
	{
		return new BError( "A parameter was invalid" );
	}
}

const int TGTOPT_CHECK_LOS	 = 0x0001;
const int TGTOPT_NOCHECK_LOS   = 0x0000;
const int TGTOPT_HARMFUL	   = 0x0002;
const int TGTOPT_HELPFUL	   = 0x0004;

// FIXME susceptible to out-of-sequence target cursors
void handle_script_cursor( Character* chr, UObject* obj )
{
	if (chr != NULL &&
		chr->client->gd->target_cursor_uoemod != NULL)
	{
		if (obj != NULL)
		{
			if (obj->ismobile())
			{
				Character* targetted_chr = static_cast<Character*>(obj);
				if (chr->client->gd->target_cursor_uoemod->target_options & TGTOPT_HARMFUL)
				{
					targetted_chr->inform_engaged( chr );
					chr->repsys_on_attack( targetted_chr );
				}
				else if (chr->client->gd->target_cursor_uoemod->target_options & TGTOPT_HELPFUL)
				{
					chr->repsys_on_help( targetted_chr );
				}
			}
			chr->client->gd->target_cursor_uoemod->uoexec.ValueStack.top().set( new BObject( obj->make_ref() ) );
		}
		// even on cancel, we wake the script up.
		chr->client->gd->target_cursor_uoemod->uoexec.os_module->revive();
		chr->client->gd->target_cursor_uoemod->target_cursor_chr = NULL;
		chr->client->gd->target_cursor_uoemod = NULL;
	}
}

LosCheckedTargetCursor los_checked_script_cursor( &handle_script_cursor, true );
NoLosCheckedTargetCursor nolos_checked_script_cursor( &handle_script_cursor, true );


BObjectImp* UOExecutorModule::mf_Target()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (chr->has_active_client())
		{
			if (!chr->target_cursor_busy())
			{
				if (!getParam( 1, target_options ))
					target_options = TGTOPT_CHECK_LOS;

				PKTBI_6C::CURSOR_TYPE crstype;

				if (target_options & TGTOPT_HARMFUL)
					crstype = PKTBI_6C::CURSOR_TYPE_HARMFUL;
				else if (target_options & TGTOPT_HELPFUL)
					crstype = PKTBI_6C::CURSOR_TYPE_HELPFUL;
				else
					crstype = PKTBI_6C::CURSOR_TYPE_NEUTRAL;

				if ((target_options & TGTOPT_CHECK_LOS) && !chr->ignores_line_of_sight())
					if (los_checked_script_cursor.send_object_cursor( chr->client, crstype ) )
					{
						chr->client->gd->target_cursor_uoemod = this;
						target_cursor_chr = chr;
						uoexec.os_module->suspend();
						return new BLong(0);
					}
					else
					{
						return new BError( "Client has an active target cursor" );
					}
				else
				{
					if ( nolos_checked_script_cursor.send_object_cursor( chr->client, crstype ) )
					{
						chr->client->gd->target_cursor_uoemod = this;
						target_cursor_chr = chr;
						uoexec.os_module->suspend();
						return new BLong(0);
					}
					else
					{
						return new BError( "Client has an active target cursor" );
					}
				}
				return new BLong(0);
			}
			else
			{
				return new BError( "Client busy with another target cursor" );
			}
		}
		else
		{
			return new BError( "No client connected" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_TargetCancel()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (chr->has_active_client())
		{
			if (chr->target_cursor_busy())
			{
				PktHelper::PacketOut<PktOut_6C> msg;
				msg->Write<u8>(static_cast<u8>(PKTBI_6C::UNK1_00));
				msg->offset+=4; // u32 target_cursor_serial
				msg->Write<u8>(static_cast<u8>(0x3));
				// rest 0
				msg.Send(chr->client, sizeof msg->buffer);
				return new BLong(0);
			}
			else
			{
				return new BError( "Client does not have an active target cursor" );
			}
		}
		else
		{
			return new BError( "No client connected" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

void handle_coord_cursor( Character* chr, PKTBI_6C* msg )
{
	if (chr != NULL &&
		chr->client->gd->target_cursor_uoemod != NULL)
	{
		if (msg != NULL)
		{
			BStruct* arr = new BStruct;
			arr->addMember( "x", new BLong( cfBEu16(msg->x) ) );
			arr->addMember( "y", new BLong( cfBEu16(msg->y) ) );
			arr->addMember( "z", new BLong( msg->z ) );
//			FIXME: Objtype CANNOT be trusted! Scripts must validate this, or, we must
//			validate right here. Should we check map/static, or let that reside
//			for scripts to run ListStatics? In theory, using Injection or similar,
//			you could mine where no mineable tiles are by faking objtype in packet
//			and still get the resources?
			arr->addMember( "objtype", new BLong( cfBEu16(msg->graphic) ) );

			u32 selected_serial = cfBEu32(msg->selected_serial);
			if (selected_serial)
			{
				UObject* obj = system_find_object( selected_serial );
				if (obj)
				{
					arr->addMember( obj->target_tag(), obj->make_ref() );
				}
			}

//			Never trust packet's objtype is the reason here. They should never
//			target on other realms. DUH!
			arr->addMember( "realm", new String( chr->realm->name() ) );

			UMulti* multi = chr->realm->find_supporting_multi( cfBEu16(msg->x), cfBEu16(msg->y), msg->z );
			if ( multi != NULL )
				arr->addMember( "multi", multi->make_ref() );

			chr->client->gd->target_cursor_uoemod->uoexec.ValueStack.top().set( new BObject( arr ) );

		}

		chr->client->gd->target_cursor_uoemod->uoexec.os_module->revive();
		chr->client->gd->target_cursor_uoemod->target_cursor_chr = NULL;
		chr->client->gd->target_cursor_uoemod = NULL;
	}
}

LosCheckedCoordCursor script_cursor2( &handle_coord_cursor, true );
BObjectImp* UOExecutorModule::mf_TargetCoordinates()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (chr->has_active_client())
		{
			if (!chr->target_cursor_busy())
			{
				if (script_cursor2.send_coord_cursor( chr->client ))
				{
					chr->client->gd->target_cursor_uoemod = this;
					target_cursor_chr = chr;
					uoexec.os_module->suspend();
					return new BLong(0);
				}
				else
				{
					return new BError( "Client has an active target cursor" );
				}

			}
			else
			{
				return new BError( "Client has an active target cursor" );
			}
		}
		else
		{
			return new BError( "Mobile has no active client" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

MultiPlacementCursor multi_placement_cursor( &handle_coord_cursor );
BObjectImp* UOExecutorModule::mf_TargetMultiPlacement()
{
	Character* chr;
	unsigned int objtype;
	int flags;
	int xoffset, yoffset;
	if (! (getCharacterParam( exec, 0, chr ) &&
		   getObjtypeParam( exec, 1, objtype ) &&
		   getParam(2, flags) &&
		   getParam(3, xoffset) &&
		   getParam(4, yoffset)) )
	{
		return new BError( "Invalid parameter type" );
	}


	if (!chr->has_active_client())
	{
		return new BError( "No client attached" );
	}

	if (chr->target_cursor_busy())
	{
		return new BError( "Client busy with another target cursor" );
	}

	if (find_itemdesc(objtype).type != ItemDesc::BOATDESC &&
		find_itemdesc(objtype).type != ItemDesc::HOUSEDESC)
	{
		return new BError( "Object Type is out of range for Multis" );
	}

	chr->client->gd->target_cursor_uoemod = this;
	target_cursor_chr = chr;
	multi_placement_cursor.send_placemulti( chr->client, objtype, flags, (s16)xoffset, (s16)yoffset );
	uoexec.os_module->suspend();
	return new BLong(0);
}

BObjectImp* UOExecutorModule::mf_GetObjType()
{
	Item* item;
	Character* chr;

	if (getItemParam( exec, 0, item ))
	{
		return new BLong( item->objtype_ );
	}
	else if (getCharacterParam( exec, 0, chr ))
	{
		return new BLong( chr->objtype_ );
	}
	else
	{
		return new BLong(0);
	}
}

// FIXME character needs an Accessible that takes an Item*
BObjectImp* UOExecutorModule::mf_Accessible()
{
	Character* chr;
	Item* item;
	if (getCharacterParam( exec, 0, chr ) &&
		getItemParam( exec, 1, item ))
	{
		if (find_legal_item( chr, item->serial ) != NULL)
			return new BLong(1);
		else
			return new BLong(0);
	}
	return new BLong(0);
}

BObjectImp* UOExecutorModule::mf_GetAmount()
{
	Item* item;

	if (getItemParam( exec, 0, item ))
	{
		return new BLong(item->getamount());
	}
	else
	{
		return new BLong(0);
	}
}


// FIXME, copy-pasted from NPC, copy-pasted to CreateItemInContainer
BObjectImp* UOExecutorModule::mf_CreateItemInBackpack()
{
	Character* chr;
	const ItemDesc* descriptor;
	unsigned short amount;

	if (getCharacterParam( exec, 0, chr ) &&
		getObjtypeParam( exec, 1, descriptor ) &&
		getParam( 2, amount ) &&
		item_create_params_ok( descriptor->objtype, amount ))
	{
		UContainer* backpack = chr->backpack();
		if (backpack != NULL)
		{
			return _create_item_in_container( backpack, descriptor, amount, false, this );
		}
		else
		{
			return new BError( "Character has no backpack." );
		}
	}
	else
	{
		return new BError( "A parameter was invalid." );
	}
}

BObjectImp* _complete_create_item_at_location( Item* item, unsigned short x, unsigned short y, short z, Realm* realm )
{
	//Dave moved these 3 lines up here 12/20 cuz x,y,z was uninit in the createscript.
	item->x = x;
	item->y = y;
	item->z = static_cast<signed char>(z);
	item->realm = realm;

	// ITEMDESCTODO: use the original descriptor
	const ItemDesc& id = find_itemdesc( item->objtype_ );
	if (!id.create_script.empty())
	{
		BObjectImp* res = run_script_to_completion( id.create_script, new EItemRefObjImp( item ) );
		if (!res->isTrue())
		{
			item->destroy();
			return res;
		}
		else
		{
			BObject ob( res );
		}
	}

	update_item_to_inrange( item );
	add_item_to_world( item );
	register_with_supporting_multi( item );
	return new EItemRefObjImp( item );
}

BObjectImp* UOExecutorModule::mf_CreateItemAtLocation(/* x,y,z,objtype,amount,realm */)
{
	unsigned short x, y;
	short z;
	const ItemDesc* itemdesc;
	unsigned short amount;
	const String* strrealm;
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
		getObjtypeParam( exec, 3, itemdesc ) &&
		getParam( 4, amount, 1, 60000 ) &&
		getStringParam( 5, strrealm ) &&
		item_create_params_ok( itemdesc->objtype, amount ))
	{
		if (!(tile_flags( itemdesc->graphic ) & FLAG::STACKABLE) &&
			 (amount != 1))
		{
			return new BError( "That item is not stackable.  Create one at a time." );
		}

		Realm* realm = find_realm(strrealm->value());
		if(!realm)
			return new BError( "Realm not found" );

		if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for Realm");
		Item* item = Item::create( *itemdesc );
		if (item != NULL)
		{
			item->setamount( amount );
			return _complete_create_item_at_location( item, x, y, z, realm );
		}
		else
		{
			return new BError( "Unable to create item of objtype " + hexint(itemdesc->objtype) );
		}
	}
	return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_CreateItemCopyAtLocation(/* x,y,z,item,realm */)
{
	unsigned short x, y;
	short z;
	Item* origitem;
	const String* strrealm;
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
		getItemParam( exec, 3, origitem ) &&
		getStringParam( 4, strrealm ))
	{
		if(origitem->script_isa(POLCLASS_MULTI))
			return new BError( "This function does not work with Multi objects." );

		Realm* realm = find_realm(strrealm->value());
		if(!realm)
			return new BError( "Realm not found" );

		if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for Realm");
		Item* item = origitem->clone();
		if (item != NULL)
		{
			return _complete_create_item_at_location( item, x, y, z, realm );
		}
		else
		{
			return new BError( "Unable to clone item" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_CreateMultiAtLocation(/* x,y,z,objtype,flags,realm */)
{
	unsigned short x, y;
	short z;
	const ItemDesc* descriptor;
	int flags = 0;
	Realm* realm = find_realm(string("britannia"));
	if (! (getParam( 0, x ) &&
		   getParam( 1, y ) &&
		   getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX  ) &&
		   getObjtypeParam( exec, 3, descriptor )) )
	{
		return new BError( "Invalid parameter type" );
	}
	if (exec.hasParams(5))
	{
		if (!getParam(4,flags))
			return new BError( "Invalid parameter type" );
	}
	if (exec.hasParams(6))
	{
		const String* strrealm;
		if (!getStringParam(5, strrealm))
			return new BError( "Invalid parameter type" );
		realm = find_realm(strrealm->value());
		if(!realm)
			return new BError( "Realm not found" );
	}
	if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for Realm");
	if (descriptor->type != ItemDesc::BOATDESC &&
		descriptor->type != ItemDesc::HOUSEDESC)
	{
		return new BError( "That objtype is not a Multi" );
	}

	return UMulti::scripted_create( *descriptor,
									x,
									y,
									static_cast<signed char>(z),
									realm, flags );
}

void replace_properties( ConfigElem& elem, BStruct* custom )
{
	for( BStruct::Contents::const_iterator citr = custom->contents().begin(), end = custom->contents().end(); citr != end; ++citr )
	{
	  
		const string& name = (*citr).first;
		BObjectImp* ref = (*citr).second->impptr();

		if( name == "CProps" )
		{
			if (ref->isa( BObjectImp::OTDictionary ))
			{
				BDictionary* cpropdict = static_cast<BDictionary*>(ref);
				const BDictionary::Contents& cprop_cont = cpropdict->contents();
				BDictionary::Contents::const_iterator itr;
				for( itr = cprop_cont.begin(); itr != cprop_cont.end(); ++itr )
				{
					elem.add_prop("cprop", ((*itr).first->getStringRep() + "\t" + (*itr).second->impptr()->pack()).c_str() );
				}
			}
			else
			{
				throw runtime_error( "NPC override_properties: CProps must be a dictionary, but is: " + string( ref->typeOf() ) );
			}
		}
		else
		{
			elem.clear_prop( name.c_str() );
			elem.add_prop( name.c_str(), ref->getStringRep().c_str() );
		}
	}
}

bool FindNpcTemplate( const char *template_name,
					  ConfigFile& cfile,
					  ConfigElem& elem );
bool FindNpcTemplate( const char* template_name, ConfigElem& elem );

BObjectImp* UOExecutorModule::mf_CreateNpcFromTemplate()
{
	const String* tmplname;
	unsigned short x,y;
	short z;
	const String* strrealm;
	Realm* realm = find_realm(string("britannia"));

	if (!(getStringParam( 0, tmplname ) &&
		  getParam( 1, x  ) &&
		  getParam( 2, y  ) &&
		  getParam( 3, z, ZCOORD_MIN, ZCOORD_MAX  ) ))
	{
		return new BError( "Invalid parameter type" );
	}
	BObjectImp* imp = getParamImp( 4 );
	BStruct* custom_struct = NULL;
	if (imp->isa( BObjectImp::OTLong ))
	{
		custom_struct = NULL;
	}
	else if (imp->isa( BObjectImp::OTStruct ))
	{
		custom_struct = static_cast<BStruct*>(imp);
	}
	else
	{
		return new BError( string("Parameter 4 must be a Struct or Integer(0), got ") + BObjectImp::typestr( imp->type() ) );
	}
	if (exec.hasParams(6))
	{
		if(!getStringParam( 5, strrealm ))
			return new BError( "Realm not found" );
		realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for Realm");
	}
	//ConfigFile cfile;
	ConfigElem elem;
	START_PROFILECLOCK( npc_search );
	//bool found = FindNpcTemplate( tmplname->data(), cfile, elem );
	bool found = FindNpcTemplate( tmplname->data(), elem );
	STOP_PROFILECLOCK( npc_search );
	INC_PROFILEVAR( npc_searches );

	if (!found)
	{
		return new BError( "NPC template '" + tmplname->value() + "' not found" );
	}
	MOVEMODE movemode = Character::decode_movemode( elem.read_string( "MoveMode", "L" ) );

	short newz;
	UMulti* dummy_multi;
	Item* dummy_walkon;
	if (!realm->walkheight(x,y,z,&newz,&dummy_multi, &dummy_walkon, true, movemode))
	{
		return new BError( "Not a valid location for an NPC!" );
	}
	z = newz;


	NPCRef npc;
			// readProperties can throw, if stuff is missing.
	try {
		npc.set( new NPC( elem.remove_ushort( "OBJTYPE" ), elem ) );
		npc->set_dirty();
		elem.clear_prop( "Serial" );
		elem.clear_prop( "X" );
		elem.clear_prop( "Y" );
		elem.clear_prop( "Z" );

		elem.add_prop( "Serial", GetNextSerialNumber() );
		// FIXME sanity check
		elem.add_prop( "X", x );
		elem.add_prop( "Y", y );
		elem.add_prop( "Z", z );
		elem.add_prop( "Realm", realm->name().c_str() );
		if (custom_struct != NULL)
			replace_properties( elem, custom_struct );
		npc->readPropertiesForNewNPC( elem );

		////HASH
		objecthash.Insert(npc.get());
		////



		//characters.push_back( npc.get() );
		SetCharacterWorldPosition( npc.get() );

		ForEach( clients, send_char_data, static_cast<Character*>(npc.get()) );

		//dave added 2/3/3 send entered area events for npc create
		ForEachMobileInRange( x, y, realm, 32,
							  NpcPropagateMove, static_cast<Character*>(npc.get()) );
		// FIXME: Need to add Walkon checks for multi right here if type is house.
		if (dummy_multi)
		{
			dummy_multi->register_object( npc.get() );
			UHouse* this_house = dummy_multi->as_house();
			if ( this_house != NULL )
			{
				if ( npc->registered_house == 0 )
				{
					npc->registered_house = dummy_multi->serial;
					//cout << "walk on multi triggered" << endl;
					this_house->walk_on( npc.get() );
				}
			}
		}
		else
		{
			if ( npc->registered_house > 0 )
			{
				UMulti* multi = system_find_multi(npc->registered_house);
				if(multi != NULL)
				{
					UHouse* house = multi->as_house();
					if(house != NULL)
						house->unregister_object(npc.get());
				}
				npc->registered_house = 0;
			}
		}

		return new ECharacterRefObjImp( npc.get() );
	}
	catch( std::exception& ex )
	{
		if (npc.get() != NULL)
			npc->destroy();
		return new BError( "Exception detected trying to create npc from template '" + tmplname->value() + "': " + ex.what() );
	}
}


BObjectImp* UOExecutorModule::mf_SubtractAmount()
{
	Item* item;
	unsigned short amount;

	if (getItemParam( exec, 0, item ) &&
		getParam( 1, amount, 1, item->itemdesc().stack_limit )) 
	{
		if (item->inuse() && !is_reserved_to_me(item))
		{
			return new BError( "That item is being used." );
		}
		subtract_amount_from_item( item, amount );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_AddAmount()
{
	Item* item;
	unsigned short amount;

	if (getItemParam( exec, 0, item ) &&
		getParam( 1, amount, 1, MAX_STACK_ITEMS ))
	{
		if (item->inuse() && !is_reserved_to_me(item))
		{
			return new BError( "That item is being used." );
		}
		if (~tile_flags( item->graphic ) & FLAG::STACKABLE)
		{
			return new BError( "That item type is not stackable." );
		}
		if (!item->can_add_to_self(amount))
		{
			return new BError( "Can't add that much to that stack" );
		}

		unsigned short newamount = item->getamount();
		newamount += amount;
		item->setamount( newamount );
		update_item_to_inrange( item );

		//DAVE added this 12/05: if in a Character's pack, update weight.
		UpdateCharacterWeight(item);

		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_PerformAction()
{
	Character* chr;
	unsigned short actionval;
	unsigned short framecount, repeatcount; 
	unsigned short backward, repeatflag, delay;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, actionval ) &&
		getParam( 2, framecount) &&
		getParam( 3, repeatcount) &&
		getParam( 4, backward) &&
		getParam( 5, repeatflag) &&
		getParam( 6, delay))
	{
		UACTION action = static_cast<UACTION>(actionval);
		send_action_to_inrange( chr, action,
			framecount,
			repeatcount,
			static_cast<PKTOUT_6E::DIRECTION_FLAG>(backward),
			static_cast<PKTOUT_6E::REPEAT_FLAG>(repeatflag),
			static_cast<unsigned char>(delay));
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_PlaySoundEffect()
{
	UObject* chr;
	int effect;
	if (getUObjectParam( exec, 0, chr ) &&
		getParam( 1, effect ))
	{
		play_sound_effect( chr, static_cast<u16>(effect) );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_PlaySoundEffectPrivate()
{
	UObject* center;
	int effect;
	Character* forchr;
	if (getUObjectParam( exec, 0, center ) &&
		getParam( 1, effect ) &&
		getCharacterParam( exec, 2, forchr ) )
	{
		play_sound_effect_private( center, static_cast<u16>(effect), forchr );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_PlaySoundEffectXYZ()
{
	unsigned short cx,cy;
	short cz;
	int effect;
	const String* strrealm;
	if (getParam( 0, cx ) &&
		getParam( 1, cy ) &&
		getParam( 2, cz ) &&
		getParam( 3, effect ) &&
		getStringParam( 4, strrealm) )		
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(cx,cy,cz))
			return new BError("Invalid Coordinates for realm");
		play_sound_effect_xyz( cx,
						   cy,
						   static_cast<signed char>(cz),
						   static_cast<u16>(effect),
						   realm);
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_PlayMusic(/* char, music_id */)
{
	Character* chr;
	int musicid;
	if (getCharacterParam(exec, 0, chr) &&
		getParam( 1, musicid ))
	{
		send_midi(chr->client, static_cast<u16>(musicid));
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

void menu_selection_made( Client* client, MenuItem* mi, PKTIN_7D* msg )
{
	if (client != NULL)
	{
		Character* chr = client->chr;
		if (chr != NULL &&
			chr->client->gd->menu_selection_uoemod != NULL)
		{
			if (mi != NULL &&
				msg != NULL)
			{
				BStruct* selection = new BStruct;
						// FIXME should make sure objtype and choice are within valid range.
				selection->addMember( "objtype", new BLong( mi->objtype_ ) );
				selection->addMember( "graphic", new BLong( mi->graphic_ ) );
				selection->addMember( "index", new BLong( cfBEu16(msg->choice) ) ); // this has been validated
				selection->addMember( "color", new BLong( mi->color_ ) );
				chr->client->gd->menu_selection_uoemod->uoexec.ValueStack.top().set( new BObject( selection ) );
			}
			// 0 is already on the value stack, for the case of cancellation.
			chr->client->gd->menu_selection_uoemod->uoexec.os_module->revive();
			chr->client->gd->menu_selection_uoemod->menu_selection_chr = NULL;
			chr->client->gd->menu_selection_uoemod = NULL;
		}
	}
}

bool UOExecutorModule::getDynamicMenuParam( unsigned param, Menu*& menu )
{
	BApplicObjBase* aob = getApplicObjParam( param, &menu_type );
	if (aob != NULL)
	{
		EMenuObjImp* menu_imp = static_cast<EMenuObjImp*>(aob);
		menu = &menu_imp->value();
		return true;
	}
	else
	{
		return false;
	}
}

bool UOExecutorModule::getStaticOrDynamicMenuParam( unsigned param, Menu*& menu )
{
	BObjectImp* imp = getParamImp( param );
	if (imp->isa( BObjectImp::OTString ))
	{
		String* pmenuname = static_cast<String*>(imp);
		menu = find_menu( pmenuname->data() );
		return (menu != NULL);
	}
	else if (imp->isa( BObjectImp::OTApplicObj ))
	{
		BApplicObjBase* aob = static_cast<BApplicObjBase*>(imp);
		if (aob->object_type() == &menu_type)
		{
			EMenuObjImp* menu_imp = static_cast<EMenuObjImp*>(aob);
			menu = &menu_imp->value();
			return true;
		}
	}
	if (mlog.is_open())
		mlog << "SelectMenuItem: expected a menu name (static menu) or a CreateMenu() dynamic menu" << endl;
	return false;
}

BObjectImp* UOExecutorModule::mf_SelectMenuItem()
{
	Character* chr;
	Menu* menu;

	if (getCharacterParam( exec, 0, chr ) &&
		getStaticOrDynamicMenuParam( 1, menu ) &&
		(chr->client->gd->menu_selection_uoemod == NULL))
	{
		if (menu != NULL &&
			chr->has_active_client() &&
			!menu->menuitems_.empty())
		{
			if (send_menu( chr->client, menu ))
			{
				chr->menu = menu;
				chr->on_menu_selection = menu_selection_made;
				chr->client->gd->menu_selection_uoemod = this;
				menu_selection_chr = chr;
				uoexec.os_module->suspend();
				return new BLong(0);
			}
			else
			{
				return new BError( "Menu too large" );
			}
		}
		else
		{
			return new BError( "Client is busy, or menu is empty" );
		}
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

void append_objtypes( ObjArray* objarr, Menu* menu )
{
	for( unsigned i = 0; i < menu->menuitems_.size(); ++i )
	{
		MenuItem* mi = &menu->menuitems_[i];

		if (mi->submenu_id)
		{
// Code Analyze: Commented out and replaced with tmp_menu due to hiding
// menu passed to function. 
//			Menu* menu = find_menu( mi->submenu_id );
			Menu* tmp_menu = find_menu( mi->submenu_id );
			if (tmp_menu != NULL)
				append_objtypes( objarr, tmp_menu );
		}
		else
		{
			objarr->addElement( new BLong( mi->objtype_ ) );
		}
	}
}

BObjectImp* UOExecutorModule::mf_GetMenuObjTypes()
{
	Menu* menu;
	if (getStaticOrDynamicMenuParam( 0, menu ))
	{
		auto_ptr<ObjArray> objarr( new ObjArray );

		append_objtypes( objarr.get(), menu );

		return objarr.release();
	}
	return new BLong(0);
}

BObjectImp* UOExecutorModule::mf_ApplyConstraint()
{
	ObjArray* arr;
	StoredConfigFile* cfile;
	const String* propname_str;
	int amthave;

	if (getObjArrayParam( 0, arr ) &&
		getStoredConfigFileParam( *this, 1, cfile ) &&
		getStringParam( 2, propname_str ) &&
		getParam( 3, amthave ))
	{
		auto_ptr<ObjArray> newarr( new ObjArray );

		for( unsigned i = 0; i < arr->ref_arr.size(); i++ )
		{
			BObjectRef& ref = arr->ref_arr[ i ];

			BObject *bo = ref.get();
			if (bo == NULL)
				continue;
			if (!bo->isa( BObjectImp::OTLong ))
				continue;
			BLong* blong = static_cast<BLong*>(bo->impptr());
			unsigned int objtype = static_cast<u32>(blong->value());

			ref_ptr<StoredConfigElem> celem = cfile->findelem( objtype );
			if (celem.get() == NULL)
				continue;

			BObjectImp* propval = celem->getimp( propname_str->value() );
			if (propval == NULL)
				continue;
			if (!propval->isa( BObjectImp::OTLong))
				continue;
			BLong* amtreq = static_cast<BLong*>(propval);
			if (amtreq->value() > amthave)
				continue;
			newarr->addElement( new BLong( objtype ) );
		}

		return newarr.release();
	}

	return new UninitObject;
}

BObjectImp* UOExecutorModule::mf_CreateMenu()
{
	const String* title;
	if (getStringParam( 0, title ))
	{
		Menu temp;
		temp.menu_id = 0;
		strzcpy( temp.title, title->data(), sizeof temp.title );
		return new EMenuObjImp( temp );
	}
	return new BLong(0);
}

BObjectImp* UOExecutorModule::mf_AddMenuItem()
{
	Menu* menu;
	unsigned int objtype;
	const String* text;
	unsigned short color;

	if (getDynamicMenuParam( 0, menu ) &&
		getObjtypeParam( exec, 1, objtype ) &&
		getStringParam( 2, text ) &&
		getParam( 3, color ) )
	{
		menu->menuitems_.push_back( MenuItem() );
		MenuItem* mi = &menu->menuitems_.back();
		mi->objtype_ = objtype;
		mi->graphic_ = getgraphic( objtype );
		strzcpy( mi->title, text->data(), sizeof mi->title );
		mi->color_ = color & ssopt.item_color_mask;
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_GetObjProperty()
{
	UObject* uobj;
	const String* propname_str;
	if (getUObjectParam( exec, 0, uobj ) &&
		getStringParam( 1, propname_str ))
	{
		std::string val;
		if (uobj->getprop( propname_str->value(), val ))
		{
			return BObjectImp::unpack( val.c_str() );
		}
		else
		{
			return new BError( "Property not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_SetObjProperty()
{
	UObject* uobj;
	const String* propname_str;
	if (getUObjectParam( exec, 0, uobj ) &&
		getStringParam( 1, propname_str ))
	{
		BObjectImp* propval = getParamImp( 2 );
		uobj->setprop( propname_str->value(), propval->pack() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_EraseObjProperty()
{
	UObject* uobj;
	const String* propname_str;
	if (getUObjectParam( exec, 0, uobj ) &&
		getStringParam( 1, propname_str ))
	{
		uobj->eraseprop( propname_str->value() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}
BObjectImp* UOExecutorModule::mf_GetObjPropertyNames()
{
	UObject* uobj;
	if (getUObjectParam( exec, 0, uobj ))
	{
		vector<string> propnames;
		uobj->getpropnames( propnames );
		auto_ptr<ObjArray> arr (new ObjArray);
		for( unsigned i = 0; i < propnames.size(); ++i )
		{
			arr->addElement( new String(propnames[i]) );
		}
		return arr.release();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

#include "../gprops.h"
BObjectImp* UOExecutorModule::mf_GetGlobalProperty()
{
	const String* propname_str;
	if (getStringParam( 0, propname_str ))
	{
		std::string val;
		if (global_properties.getprop( propname_str->value(), val ))
		{
			return BObjectImp::unpack( val.c_str() );
		}
		else
		{
			return new BError( "Property not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_SetGlobalProperty()
{
	const String* propname_str;
	if (exec.getStringParam( 0, propname_str ))
	{
		BObjectImp* propval = exec.getParamImp( 1 );
		global_properties.setprop( propname_str->value(), propval->pack() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_EraseGlobalProperty()
{
	const String* propname_str;
	if (getStringParam( 0, propname_str ))
	{
		global_properties.eraseprop( propname_str->value() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_GetGlobalPropertyNames()
{
	vector<string> propnames;
	global_properties.getpropnames( propnames );
	auto_ptr<ObjArray> arr (new ObjArray);
	for( unsigned i = 0; i < propnames.size(); ++i )
	{
		arr->addElement( new String(propnames[i]) );
	}
	return arr.release();
}

BObjectImp* UOExecutorModule::mf_PlayMovingEffect()
{
	UObject* src;
	UObject* dst;
	unsigned short effect;
	int speed;
	int loop;
	int explode;
	if (getUObjectParam( exec, 0, src ) &&
		getUObjectParam( exec, 1, dst ) &&
		getParam( 2, effect ) &&
		getParam( 3, speed, UCHAR_MAX ) &&
		getParam( 4, loop, UCHAR_MAX ) &&
		getParam( 5, explode, UCHAR_MAX ))
	{
		if(src->realm != dst->realm)
			return new BError("Realms must match");
		play_moving_effect( src, dst, effect,
							static_cast<unsigned char>(speed),
							static_cast<unsigned char>(loop),
							static_cast<unsigned char>(explode));
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_PlayMovingEffectXyz()
{
	unsigned short sx, sy;
	unsigned short dx, dy;
	short sz, dz;

	unsigned short effect;
	int speed;
	int loop;
	int explode;
	const String* strrealm;

	if (getParam( 0, sx ) &&
		getParam( 1, sy ) &&
		getParam( 2, sz ) &&
		getParam( 3, dx ) &&
		getParam( 4, dy ) &&
		getParam( 5, dz ) &&
		getParam( 6, effect ) &&
		getParam( 7, speed, UCHAR_MAX ) &&
		getParam( 8, loop, UCHAR_MAX ) &&
		getParam( 9, explode, UCHAR_MAX ) &&
		getStringParam( 10, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(sx,sy,sz) || !realm->valid(dx,dy,dz))
			return new BError("Invalid Coordinates for realm");
		play_moving_effect2( sx,
							 sy,
							 static_cast<signed char>(sz),
							 dx,
							 dy,
							 static_cast<signed char>(dz),
							 effect,
							 static_cast<signed char>(speed),
							 static_cast<signed char>(loop),
							 static_cast<signed char>(explode),
							 realm);
		return new BLong(1);
	}
	else
	{
		return NULL;
	}
}

// FIXME add/test:stationary

BObjectImp* UOExecutorModule::mf_PlayObjectCenteredEffect()
{
	UObject* src;
	unsigned short effect;
	int speed;
	int loop;
	if (getUObjectParam( exec, 0, src ) &&
		getParam( 1, effect ) &&
		getParam( 2, speed, UCHAR_MAX ) &&
		getParam( 3, loop, UCHAR_MAX ))
	{
		play_object_centered_effect( src,
									 effect,
									 static_cast<unsigned char>(speed),
									 static_cast<unsigned char>(loop) );
		return new BLong(1);
	}
	else
	{
		return NULL;
	}
}

BObjectImp* UOExecutorModule::mf_PlayStationaryEffect()
{
	unsigned short x, y;
	short z;
	unsigned short effect;
	int speed, loop, explode;
	const String* strrealm;
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, effect ) &&
		getParam( 4, speed, UCHAR_MAX ) &&
		getParam( 5, loop, UCHAR_MAX ) &&
		getParam( 6, explode, UCHAR_MAX ) &&
		getStringParam( 7, strrealm))
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for realm");

		play_stationary_effect( x,
								y,
								static_cast<signed char>(z),
								effect,
								static_cast<unsigned char>(speed),
								static_cast<unsigned char>(loop),
								static_cast<unsigned char>(explode),
								realm );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}


BObjectImp* UOExecutorModule::mf_PlayMovingEffect_Ex()
{
	UObject* src;
	UObject* dst;
	unsigned short effect;
	int speed;
	int duration;
	int hue;
	int render;
	int direction;
	int explode;
	unsigned short effect3d;
	unsigned short effect3dexplode;
	unsigned short effect3dsound;

	if (getUObjectParam( exec, 0, src ) &&
		getUObjectParam( exec, 1, dst ) &&
		getParam( 2, effect ) &&
		getParam( 3, speed, UCHAR_MAX ) &&
		getParam( 4, duration, UCHAR_MAX ) &&
		getParam( 5, hue, INT_MAX ) &&
		getParam( 6, render, INT_MAX ) &&
		getParam( 7, direction, UCHAR_MAX ) &&
		getParam( 8, explode, UCHAR_MAX ) &&
		getParam( 9, effect3d ) &&
		getParam( 10, effect3dexplode ) &&
		getParam( 11, effect3dsound ))
	{
		if(src->realm != dst->realm)
			return new BError("Realms must match");
		play_moving_effect_ex( src, dst, effect,
							static_cast<unsigned char>(speed),
							static_cast<unsigned char>(duration),
							static_cast<unsigned int>(hue),
							static_cast<unsigned int>(render),
							static_cast<unsigned char>(direction),
							static_cast<unsigned char>(explode),
							effect3d,
							effect3dexplode,
							effect3dsound);
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_PlayMovingEffectXyz_Ex()
{
	unsigned short sx, sy;
	unsigned short dx, dy;
	short sz,dz;
	const String* strrealm;

	unsigned short effect;
	int speed;
	int duration;
	int hue;
	int render;
	int direction;
	int explode;
	unsigned short effect3d;
	unsigned short effect3dexplode;
	unsigned short effect3dsound;
	
	if (getParam( 0, sx ) &&
		getParam( 1, sy ) &&
		getParam( 2, sz ) &&
		getParam( 3, dx ) &&
		getParam( 4, dy ) &&
		getParam( 5, dz ) &&
		getStringParam( 6, strrealm) &&
		getParam( 7, effect ) &&
		getParam( 8, speed, UCHAR_MAX ) &&
		getParam( 9, duration, UCHAR_MAX ) &&
		getParam( 10, hue, INT_MAX ) &&
		getParam( 11, render, INT_MAX ) &&
		getParam( 12, direction, UCHAR_MAX ) &&
		getParam( 13, explode, UCHAR_MAX ) &&
		getParam( 14, effect3d ) &&
		getParam( 15, effect3dexplode ) &&
		getParam( 16, effect3dsound ))
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(sx,sy,sz) || !realm->valid(dx,dy,dz))
			return new BError("Invalid Coordinates for realm");
		play_moving_effect2_ex( sx,
							 sy,
							 static_cast<signed char>(sz),
							 dx,
							 dy,
							 static_cast<signed char>(dz),
							 realm,
							 effect,
							 static_cast<unsigned char>(speed),
							 static_cast<unsigned char>(duration),
							 static_cast<unsigned int>(hue),
							 static_cast<unsigned int>(render),
							 static_cast<unsigned char>(direction),
							 static_cast<unsigned char>(explode),
							 effect3d,
							 effect3dexplode,
							 effect3dsound);
		return new BLong(1);
	}
	else
	{
		return NULL;
	}
}

BObjectImp* UOExecutorModule::mf_PlayObjectCenteredEffect_Ex()
{
	UObject* src;
	unsigned short effect;
	int speed;
	int duration;
	int hue;
	int render;
	int layer;
	unsigned short effect3d;
	 
	if (getUObjectParam( exec, 0, src ) &&
		getParam( 1, effect ) &&
		getParam( 2, speed, UCHAR_MAX ) &&
		getParam( 3, duration, UCHAR_MAX ) &&
		getParam( 4, hue, INT_MAX ) &&
		getParam( 5, render, INT_MAX ) &&
		getParam( 6, layer, UCHAR_MAX ) &&
		getParam( 7, effect3d ))
	{
		play_object_centered_effect_ex( src,
										effect,
										static_cast<unsigned char>(speed),
										static_cast<unsigned char>(duration),
										static_cast<unsigned int>(hue),
										static_cast<unsigned int>(render),
										static_cast<unsigned char>(layer),
										effect3d );
		return new BLong(1);
	}
	else
	{
		return NULL;
	}
}

BObjectImp* UOExecutorModule::mf_PlayStationaryEffect_Ex()
{
	unsigned short x, y;
	short z;
	const String* strrealm;
	unsigned short effect;
	int speed;
	int duration;
	int hue;
	int render;
	unsigned short effect3d;
	
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getStringParam( 3, strrealm) &&
		getParam( 4, effect ) &&
		getParam( 5, speed, UCHAR_MAX ) &&
		getParam( 6, duration, UCHAR_MAX ) &&
		getParam( 7, hue, INT_MAX ) &&
		getParam( 8, render, INT_MAX ) &&
		getParam( 9, effect3d ))
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,z)) return new BError("Invalid Coordinates for realm");

		play_stationary_effect_ex( x,
								y,
								static_cast<signed char>(z),
								realm,
								effect,
								static_cast<unsigned char>(speed),
								static_cast<unsigned char>(duration),
								static_cast<unsigned int>(hue),
								static_cast<unsigned int>(render),
								effect3d);
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_PlayLightningBoltEffect()
{
	UObject* center;
	if (getUObjectParam( exec, 0, center ))
	{
		play_lightning_bolt_effect( center );
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_ListItemsNearLocation(/* x, y, z, range, realm */)
{
	unsigned short x, y;
	int z;
	short range;
	const String* strrealm;
	Realm* realm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getStringParam( 4, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );
		
		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneItems& witem = realm->zone[wx][wy].items;
				for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
				{
					Item* item = *itr;

					if ((abs(item->x - x) <= range) && (abs(item->y - y) <= range))
						if ((z == LIST_IGNORE_Z) || (abs(item->z - z) < CONST_DEFAULT_ZRANGE))
							newarr->addElement( item->make_ref() );
				}
			}
		}

		return newarr.release();
	}

	return NULL;
}

void UOExecutorModule::internal_InBoxAreaChecks(unsigned short &x1, unsigned short &y1, short &z1, unsigned short &x2, unsigned short &y2, short &z2, Realm* realm)
{
	if ( z1 < WORLD_MIN_Z )
		z1 = WORLD_MIN_Z;

	if( x2 >= realm->width() )
		x2 = (realm->width()-1);
	if ( y2 >= realm->height() )
		y2 = (realm->height()-1);
	if ( z2 > WORLD_MAX_Z )
		z2 = WORLD_MAX_Z;
}

BObjectImp* UOExecutorModule::mf_ListObjectsInBox(/* x1, y1, z1, x2, y2, z2, realm */)
{
	unsigned short x1, y1;
	short z1;
	unsigned short x2, y2;
	short z2;
	const String* strrealm;
	Realm* realm;

	if (!(getParam( 0, x1 ) &&
		  getParam( 1, y1 ) &&
		  getParam( 2, z1 ) &&
		  getParam( 3, x2 ) &&
		  getParam( 4, y2 ) &&
		  getParam( 5, z2 ) &&
		  getStringParam( 6, strrealm) ))
	{
		return new BError( "Invalid parameter" );
	}

	realm = find_realm(strrealm->value());
	if (!realm)
		return new BError("Realm not found");

	if (x1 > x2)
		swap(x1, x2);
	if (y1 > y2)
		swap(y1, y2);
	if (z1 > z2)
		swap(z1, z2);
	// Disabled again: ShardAdmins "loves" this "bug" :o/
	// if ((!realm->valid(x1, y1, z1)) || (!realm->valid(x2, y2, z2)))
	//	 return new BError("Invalid Coordinates for realm");
	internal_InBoxAreaChecks(x1, y1, z1, x2, y2, z2, realm);

	auto_ptr<ObjArray> newarr( new ObjArray );

	unsigned short wxL, wyL, wxH, wyH;
	zone_convert_clip( x1, y1, realm, wxL, wyL );
	zone_convert_clip( x2, y2, realm, wxH, wyH );

	for( unsigned short wx = wxL; wx <= wxH; ++wx )
	{
		for( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
			ZoneCharacters& wchr = realm->zone[wx][wy].characters;
			for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
			{
				Character* chr = *itr;
				if (chr->x >= x1 && chr->x <= x2 &&
					chr->y >= y1 && chr->y <= y2 &&
					chr->z >= z1 && chr->z <= z2)
				{
					newarr->addElement( chr->make_ref() );
				}
			}

			ZoneItems& witem = realm->zone[wx][wy].items;
			for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
			{
				Item* item = *itr;
				if (item->x >= x1 && item->x <= x2 &&
					item->y >= y1 && item->y <= y2 &&
					item->z >= z1 && item->z <= z2)
				{
					newarr->addElement( item->make_ref() );
				}
			}
		}
	}

	return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListMultisInBox(/* x1, y1, z1, x2, y2, z2, realm */)
{
	unsigned short x1, y1;
	short z1;
	unsigned short x2, y2;
	short z2;
	const String* strrealm;
	Realm* realm;

	if (!(getParam( 0, x1 ) &&
		  getParam( 1, y1 ) &&
		  getParam( 2, z1 ) &&
		  getParam( 3, x2 ) &&
		  getParam( 4, y2 ) &&
		  getParam( 5, z2 ) &&
		  getStringParam( 6, strrealm) ))
	{
		return new BError( "Invalid parameter" );
	}
	
	realm = find_realm(strrealm->value());
	if (!realm)
		return new BError("Realm not found");

	if (x1 > x2)
		swap(x1, x2);
	if (y1 > y2)
		swap(y1, y2);
	if (z1 > z2)
		swap(z1, z2);
	// Disabled again: ShardAdmins "loves" this "bug" :o/
	// if ((!realm->valid(x1, y1, z1)) || (!realm->valid(x2, y2, z2)))
	//	 return new BError("Invalid Coordinates for realm");
	internal_InBoxAreaChecks(x1, y1, z1, x2, y2, z2, realm);

	auto_ptr<ObjArray> newarr( new ObjArray );

	unsigned short wxL, wyL, wxH, wyH;
	zone_convert_clip( x1+MultiDef::global_minrx, y1+MultiDef::global_minry, realm, wxL, wyL );
	zone_convert_clip( x2+MultiDef::global_maxrx, y2+MultiDef::global_maxry, realm, wxH, wyH );

	// search for multis.  this is tricky, since the center might lie outside the box
	for( unsigned short wx = wxL; wx <= wxH; ++wx )
	{
		for( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
			ZoneMultis& multis = realm->zone[wx][wy].multis;
			for( ZoneMultis::const_iterator citr = multis.begin(), end = multis.end(); citr != end; ++citr )
			{
				UMulti* multi = *citr;
				const MultiDef& md = multi->multidef();
				if (multi->x + md.minrx > x2 || // east of the box
					multi->x + md.maxrx < x1 || // west of the box
					multi->y + md.minry > y2 || // south of the box
					multi->y + md.maxry < y1 || // north of the box
					multi->z + md.minrz > z2 || // above the box
					multi->z + md.maxrz < z1) // below the box
				{
					continue;
				}

				// some part of it is contained in the box.  Look at the individual statics, to see
				// if any of them lie within.
// Code Analyze: Commented and renaming due to C6246 Warning
//				for( MultiDef::Components::const_iterator citr_2 = md.components.begin(), end = md.components.end();
				for( MultiDef::Components::const_iterator citr_2 = md.components.begin(), end_2 = md.components.end();
					 citr_2 != end_2;
					 ++citr_2 )
				{
					const MULTI_ELEM* elem = (*citr_2).second;
					int absx = multi->x + elem->x;
					int absy = multi->y + elem->y;
					int absz = multi->z + elem->z;
					if (x1 <= absx && absx <= x2 &&
						y1 <= absy && absy <= y2)
					{
						// do Z checking
						int height = tileheight( getgraphic( elem->objtype) );
						int top = absz + height;

						if ((z1 <= absz && absz <= z2)  ||	// bottom point lies between
							(z1 <= top && top <= z2) ||	   // top lies between
							(top >= z2 && absz <= z1))		// spans
						{
							newarr->addElement( multi->make_ref() );
							break; // out of for
						}
					}
				}
			}
		}
	}

	return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListStaticsInBox(/* x1, y1, z1, x2, y2, z2, flags, realm */)
{
	unsigned short x1, y1;
	unsigned short x2, y2;
	short z1, z2;
	int flags;
	const String* strrealm;

	if (getParam( 0, x1 ) &&
		getParam( 1, y1 ) &&
		getParam( 2, z1 ) &&
		getParam( 3, x2 ) &&
		getParam( 4, y2 ) &&
		getParam( 5, z2 ) &&
		getParam( 6, flags ) &&
		getStringParam( 7, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (x1 > x2)
			swap(x1, x2);
		if (y1 > y2)
			swap(y1, y2);
		if (z1 > z2)
			swap(z1, z2);
		// Disabled again: ShardAdmins "loves" this "bug" :o/
		// if ((!realm->valid(x1, y1, z1)) || (!realm->valid(x2, y2, z2)))
		//	 return new BError("Invalid Coordinates for realm");
		internal_InBoxAreaChecks(x1, y1, z1, x2, y2, z2, realm);

		auto_ptr<ObjArray> newarr( new ObjArray );

		for( unsigned short wx = x1; wx <= x2; ++wx )
		{
			for( unsigned short wy = y1; wy <= y2; ++wy )
			{
				if (!( flags & ITEMS_IGNORE_STATICS ))
				{
					StaticEntryList slist;
					realm->getstatics( slist, wx, wy );

					for( unsigned i = 0; i < slist.size(); ++i )
					{
						if ((z1 <= slist[i].z) && (slist[i].z <= z2))
						{
							auto_ptr<BStruct> arr ( new BStruct );
							arr->addMember( "x", new BLong( wx ) );
							arr->addMember( "y", new BLong( wy ) );
							arr->addMember( "z", new BLong( slist[i].z ) );
							arr->addMember( "objtype", new BLong( slist[i].objtype ) );
							arr->addMember( "hue", new BLong( slist[i].hue ) );
							newarr->addElement( arr.release() );
						}
					}
				}

				if (!( flags & ITEMS_IGNORE_MULTIS ))
				{
					StaticList mlist;
					realm->readmultis( mlist, wx, wy );

					for( unsigned i = 0; i < mlist.size(); ++i )
					{
						if ((z1 <= mlist[i].z) && (mlist[i].z <= z2))
						{
							auto_ptr<BStruct> arr  ( new BStruct );
							arr->addMember( "x", new BLong( wx ) );
							arr->addMember( "y", new BLong( wy ) );
							arr->addMember( "z", new BLong( mlist[i].z ) );
							arr->addMember( "objtype", new BLong( mlist[i].graphic ) );
							newarr->addElement( arr.release() );
						}
					}
				}
			}
		}
		
		return newarr.release();
	}
	else
		return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_ListItemsNearLocationOfType(/* x, y, z, range, objtype, realm */)
{
	unsigned short x, y;
	int z, range;
	unsigned int objtype;
	const String* strrealm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getObjtypeParam( exec, 4, objtype ) &&
		getStringParam(5, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		auto_ptr<ObjArray> newarr( new ObjArray );

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );

		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneItems& witem = realm->zone[wx][wy].items;
				for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
				{
					Item* item = *itr;

					if ((item->objtype_ == objtype) && (abs(item->x - x) <= range) && (abs(item->y - y) <= range))
						if ((z == LIST_IGNORE_Z) || (abs(item->z - z) < CONST_DEFAULT_ZRANGE))
							newarr->addElement( item->make_ref() );
				}
			}
		}

		return newarr.release();
	}

	return NULL;
}


BObjectImp* UOExecutorModule::mf_ListItemsAtLocation(/* x, y, z, realm */)
{
	unsigned short x, y;
	int z;
	const String* strrealm;
	Realm* realm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getStringParam( 3, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wx, wy;
		zone_convert_clip( x, y, realm, wx, wy );
		ZoneItems& witem = realm->zone[wx][wy].items;
		
		for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
		{
			Item* item = *itr;

			if ((item->x == x) && (item->y == y))
				if ((z == LIST_IGNORE_Z) || (item->z == z))
					newarr->addElement( item->make_ref() );
		}

		return newarr.release();
	}

	return NULL;
}

BObjectImp* UOExecutorModule::mf_ListGhostsNearLocation()
{
	int x;
	int y;
	int z;
	int range;
	const String* strrealm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getStringParam( 4, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");

		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );
		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneCharacters& wchr = realm->zone[wx][wy].characters;

				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;

					if (chr->dead() &&
						(abs(chr->z - z) < CONST_DEFAULT_ZRANGE) &&
						(abs(chr->x - x) <= range) &&
						(abs(chr->y - y) <= range))
					{
						newarr->addElement( chr->make_ref() );
					}
				}
			}
		}

		return newarr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

const unsigned LMBLEX_FLAG_NORMAL = 0x01;
const unsigned LMBLEX_FLAG_HIDDEN = 0x02;
const unsigned LMBLEX_FLAG_DEAD   = 0x04;
const unsigned LMBLEX_FLAG_CONCEALED = 0x8;
const unsigned LMBLEX_FLAG_PLAYERS_ONLY = 0x10;
const unsigned LMBLEX_FLAG_NPC_ONLY = 0x20;

BObjectImp* UOExecutorModule::mf_ListMobilesNearLocationEx(/* x, y, z, range, flags, realm */)
{
	unsigned short x, y;
	int z, flags;
	short range;
	const String* strrealm;
	Realm* realm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getParam( 4, flags ) &&
		getStringParam(5, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		bool inc_normal = (flags & LMBLEX_FLAG_NORMAL)? true:false;
		bool inc_hidden = (flags & LMBLEX_FLAG_HIDDEN) ? true:false;
		bool inc_dead = (flags & LMBLEX_FLAG_DEAD)? true:false;
		bool inc_concealed = (flags & LMBLEX_FLAG_CONCEALED)? true:false;
		bool inc_players_only = (flags & LMBLEX_FLAG_PLAYERS_ONLY)? true:false;
		bool inc_npc_only = (flags & LMBLEX_FLAG_NPC_ONLY)? true:false;
		
		auto_ptr<ObjArray> newarr(new ObjArray);

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );

		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneCharacters& wchr = realm->zone[wx][wy].characters;

				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;

					if ( (inc_hidden && chr->hidden()) || (inc_dead   && chr->dead()) || (inc_concealed && chr->concealed()) || (inc_normal && !(chr->hidden()||chr->dead()||chr->concealed())) )
					{
						if ((abs(chr->x - x) <= range) && (abs(chr->y - y) <= range))
						{
							if ((z == LIST_IGNORE_Z) || (abs(chr->z - z) < CONST_DEFAULT_ZRANGE))
							{
								if (!inc_players_only && !inc_npc_only)
									newarr->addElement( chr->make_ref() );
								else if (inc_players_only && chr->client->ready)
									newarr->addElement( chr->make_ref() );
								else if (inc_npc_only && chr->script_isa(POLCLASS_NPC))
									newarr->addElement( chr->make_ref() );
							}
						}
					}
				}
			}
		}

		return newarr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ListMobilesNearLocation(/* x, y, z, range, realm */)
{
	unsigned short x, y;
	int z;
	short range;
	const String* strrealm;
	Realm* realm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getStringParam( 4, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );

		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneCharacters& wchr = realm->zone[wx][wy].characters;

				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;

					if ((!chr->concealed()) && (!chr->hidden()) && (!chr->dead()) &&
						(abs(chr->x - x) <= range) && (abs(chr->y - y) <= range))
						if ((z == LIST_IGNORE_Z) || (abs(chr->z - z) < CONST_DEFAULT_ZRANGE))
							newarr->addElement( chr->make_ref() );
				}
			}
		}

		return newarr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ListMobilesInLineOfSight()
{
	UObject* obj;
	int range;
	if (getUObjectParam( exec, 0, obj ) &&
		getParam( 1, range ))
	{
		obj = obj->toplevel_owner();
		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( obj->x - range, obj->y - range, obj->realm, wxL, wyL );
		zone_convert_clip( obj->x + range, obj->y + range, obj->realm, wxH, wyH );
		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneCharacters& wchr = obj->realm->zone[wx][wy].characters;

				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;
					if (chr->dead() || chr->hidden() || chr->concealed())
						continue;
					if (chr == obj)
						continue;
					if ((abs(chr->x - obj->x) <= range) &&
						(abs(chr->y - obj->y) <= range) &&
						(abs(chr->z - obj->z) < CONST_DEFAULT_ZRANGE))
					{
						if (obj->realm->has_los( *obj, *chr ))
						{
							newarr->addElement( chr->make_ref() );
						}
					}
				}
			}
		}
		return newarr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

// keep this in sync with UO.EM
const int LH_FLAG_LOS = 1;			 // only include those in LOS
const int LH_FLAG_INCLUDE_HIDDEN = 2;  // include hidden characters
BObjectImp* UOExecutorModule::mf_ListHostiles()
{
	Character* chr;
	int range;
	int flags;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, range ) &&
		getParam( 2, flags ))
	{
		auto_ptr<ObjArray> arr(new ObjArray);

		const Character::CharacterSet& cont = chr->hostiles();
		Character::CharacterSet::const_iterator itr = cont.begin(), end = cont.end();
		for( ; itr != end; ++itr )
		{
			Character* hostile = *itr;
			if (hostile->concealed())
				continue;
			if ((flags & LH_FLAG_LOS) && !chr->realm->has_los( *chr, *hostile ))
				continue;
			if ((~flags & LH_FLAG_INCLUDE_HIDDEN) && hostile->hidden())
				continue;
			if (!inrangex(chr,hostile,range))
				continue;
			arr->addElement( hostile->make_ref() );
		}

		return arr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_CheckLineOfSight()
{
	UObject* src;
	UObject* dst;
	if (getUObjectParam( exec, 0, src ) &&
		getUObjectParam( exec, 1, dst ))
	{
		return new BLong( src->realm->has_los( *src, *dst->toplevel_owner() ) );
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_CheckLosAt()
{
	UObject* src;
	unsigned short x, y;
	short z;
	if (getUObjectParam( exec, 0, src ) &&
		getParam( 1, x ) &&
		getParam( 2, y ) &&
		getParam( 3, z ))
	{
		if(!src->realm->valid(x,y,z)) return new BError("Invalid Coordinates for realm");
		LosObj att(*src);
		LosObj tgt(x,y,static_cast<signed char>(z));
		return new BLong( src->realm->has_los( att, tgt ) );
	}
	else
	{
		return NULL;
	}
}

BObjectImp* UOExecutorModule::mf_CheckLosBetween()
{
	unsigned short x1,x2;
	unsigned short y1,y2;
	short z1,z2;
	const String* strrealm;
	if (getParam( 0, x1 ) &&
		getParam( 1, y1 ) &&
		getParam( 2, z1 ) &&
		getParam( 3, x2 ) &&
		getParam( 4, y2 ) &&
		getParam( 5, z2 ) &&
		getStringParam( 6, strrealm ))
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm)
			return new BError( "Realm not found" );

		if ((!realm->valid(x1,y1,z1)) || (!realm->valid(x2,y2,z2)))
			return new BError("Invalid Coordinates for Realm");

		LosObj att(x1,y1,static_cast<signed char>(z1));
		LosObj tgt(x2,y2,static_cast<signed char>(z2));
		return new BLong( realm->has_los( att, tgt ) );
	}
	else
	{
		return NULL;
	}
}

BObjectImp* UOExecutorModule::mf_DestroyItem()
{
	Item* item;
	if (getItemParam( exec, 0, item ))
	{
		if ( item->inuse() )
		{
			if ( !is_reserved_to_me(item) )
			{
				return new BError( "That item is reserved." );
			}
			else if ( item->is_gotten() )
			{	
				return new BError( "That item is 'gotten'." );
			}
		}

		const ItemDesc& id = find_itemdesc( item->objtype_ );
		if (!id.destroy_script.empty())
		{
			BObjectImp* res = run_script_to_completion( id.destroy_script,
														new EItemRefObjImp( item ));
			if (res->isTrue())
			{				   // destruction is okay
				BObject ob( res );
			}
			else
			{
								// destruction isn't okay!
				return res;
			}
		}
		UpdateCharacterOnDestroyItem(item);
		UpdateCharacterWeight(item);
		destroy_item( item );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_SetName()
{
	UObject* obj;
	const String* name_str;

	if (getUObjectParam( exec, 0, obj ) &&
		getStringParam( 1, name_str ))
	{
		obj->setname( name_str->value() );
		return new BLong(1);
	}
	else
	{
		return new BLong(0);
	}
}

BObjectImp* UOExecutorModule::mf_GetPosition()
{
	UObject* obj;
	if (getUObjectParam( exec, 0, obj ))
	{
		auto_ptr<BStruct> arr (new BStruct);
		arr->addMember( "x", new BLong( obj->x ) );
		arr->addMember( "y", new BLong( obj->y ) );
		arr->addMember( "z", new BLong( obj->z ) );
		return arr.release();
	}
	else
	{
		return new BLong(0);
	}
}

void UContainer::enumerate_contents( ObjArray* arr, int flags )
{
	for( Contents::iterator itr = contents_.begin(), end = contents_.end(); itr != end; ++itr )
	{
		Item* item = GET_ITEM_PTR( itr );
		if( item ) //dave 1/1/03, wornitemscontainer can have null items!
		{
			arr->addElement( new EItemRefObjImp( item ) );
			// Austin 9-15-2006, added flag to not enumerate sub-containers.
			if ( !(flags & ENUMERATE_ROOT_ONLY) && (item->isa(CLASS_CONTAINER)) ) // FIXME check locks
			{
				UContainer* cont = static_cast<UContainer*>(item);
				if ( !cont->locked_ || (flags & ENUMERATE_IGNORE_LOCKED) )
					cont->enumerate_contents( arr, flags );
			}
		}
	}
}

BObjectImp* UOExecutorModule::mf_EnumerateItemsInContainer()
{
	Item* item;
	if (getItemParam( exec, 0, item ))
	{
		int flags = 0;
		if (exec.fparams.size() >= 2)
		{
			if (!getParam( 1, flags ))
				return new BError( "Invalid parameter type" );
		}

		if (item->isa( UObject::CLASS_CONTAINER ))
		{
			auto_ptr<ObjArray> newarr( new ObjArray );

			static_cast<UContainer*>(item)->enumerate_contents( newarr.get(), flags );

			return newarr.release();
		}

		return NULL;
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_EnumerateOnlineCharacters()
{
	auto_ptr<ObjArray> newarr( new ObjArray );

	for( Clients::const_iterator itr = clients.begin(), end=clients.end(); itr != end; ++itr )
	{
		if ((*itr)->chr != NULL)
		{
			newarr->addElement( new ECharacterRefObjImp( (*itr)->chr ) );
		}
	}

	return newarr.release();
}

BObjectImp* UOExecutorModule::mf_RegisterForSpeechEvents()
{
	UObject* center;
	int range;
	if (getUObjectParam( exec, 0, center ) &&
		getParam( 1, range ))
	{
		int flags = 0;
		if (exec.hasParams(3))
		{
			if (!getParam( 2, flags ))
				return new BError( "Invalid parameter type" );
		}
		if (!registered_for_speech_events)
		{
			register_for_speech_events( center, &uoexec, range, flags );
			registered_for_speech_events = true;
			return new BLong(1);
		}
		else
		{
			return new BError( "Already registered for speech events" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_EnableEvents()
{
	int eventmask;
	if (getParam( 0, eventmask ))
	{
		if (eventmask & (EVID_ENTEREDAREA|EVID_LEFTAREA|EVID_SPOKE))
		{
			unsigned short range;
			if (getParam( 1, range, 0, 32 ))
			{
				if (eventmask & (EVID_SPOKE))
					uoexec.speech_size = range;
				if (eventmask & (EVID_ENTEREDAREA|EVID_LEFTAREA))
					uoexec.area_size = range;
			}
			else
			{
				return NULL;
			}
		}
		uoexec.eventmask |= eventmask;
		return new BLong(uoexec.eventmask);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_DisableEvents()
{
	int eventmask;
	if (getParam( 0, eventmask ))
	{
		uoexec.eventmask &= ~eventmask;

		return new BLong(uoexec.eventmask);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_Resurrect()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (!chr->dead())
			return new BError( "That is not dead" );
		int flags = 0;
		if (exec.hasParams( 2 ))
		{
			if (!getParam( 1, flags ))
				return new BError( "Invalid parameter type" );
		}

		if (~flags & RESURRECT_FORCELOCATION)
		{
			// we want doors to block ghosts in this case.
			bool doors_block = ! (chr->graphic == UOBJ_GAMEMASTER ||
								  chr->cached_settings.ignoredoors);
			short newz;
			UMulti* supporting_multi;
			Item* walkon_item;
			if (!chr->realm->walkheight( chr->x, chr->y, chr->z, &newz, &supporting_multi, &walkon_item, doors_block, chr->movemode ))
			{
				return new BError( "That location is blocked" );
			}
		}

		chr->resurrect();
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_SystemFindObjectBySerial()
{
	int serial;
	if (getParam( 0, serial ))
	{
		int sysfind_flags = 0;
		if (exec.hasParams(2))
		{
			if (!getParam( 1, sysfind_flags ))
				return new BError( "Invalid parameter type" );
		}
		if (IsCharacter(serial))
		{
			Character* chr = system_find_mobile( serial );
			if (chr != NULL)
			{
				if (sysfind_flags & SYSFIND_SEARCH_OFFLINE_MOBILES)
					return new EOfflineCharacterRefObjImp(chr);
				else
					return new ECharacterRefObjImp( chr );
			}
			else
			{
				return new BError( "Character not found" );
			}
		}
		else
		{
			//dave changed this 3/8/3: objecthash (via system_find_item) will find any kind of item, so don't need system_find_multi here.
			Item* item = system_find_item( serial );

			if (item != NULL)
			{
				return item->make_ref();
			}

			return new BError( "Item not found." );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}


#include "../uimport.h"
#include "../gameclck.h"
void cancel_all_trades();
BObjectImp* UOExecutorModule::mf_SaveWorldState()
{
	update_gameclock();
	int flags = 0;
	if (exec.hasParams( 1 ))
	{
		if (!getParam( 0, flags ))
			return new BError( "Invalid parameter type" );
	}
	try
	{
		cancel_all_trades();

		PolClockPauser pauser;

		unsigned int dirty, clean, elapsed_ms;
		int res;
		if (flags & SAVE_INCREMENTAL)
		{
			res = save_incremental(dirty,clean,elapsed_ms);
		}
		else
		{
			res = write_data(dirty,clean,elapsed_ms);
		}
		if (res == 0)
		{
// Code Analyze: C6246
//			BStruct* res = new BStruct();
			BStruct* ret = new BStruct();
			ret->addMember( "DirtyObjects", new BLong( dirty ) );
			ret->addMember( "CleanObjects", new BLong( clean ) );
			ret->addMember( "ElapsedMilliseconds", new BLong( elapsed_ms ) );
			return ret;
		}
		else
		{
			return new BError( "pol.cfg has InhibitSaves=1" );
		}
	}
	catch( std::exception& ex )
	{
		Log( "Exception during world save! (%s)\n", ex.what() );
		return new BError( "Exception during world save" );
	}
}

#include "../zone.h"
#include "../lightlvl.h"
#include "../miscrgn.h"

BObjectImp* UOExecutorModule::mf_SetRegionLightLevel()
{
	const String* region_name_str;
	int lightlevel;
	if (! (getStringParam( 0, region_name_str ) &&
		   getParam( 1, lightlevel )) )
	{
		return new BError( "Invalid Parameter type" );
	}

	if (!VALID_LIGHTLEVEL(lightlevel))
	{
		return new BError( "Light Level is out of range" );
	}

	LightRegion* lightregion = lightdef->getregion( region_name_str->value() );
	if (lightregion == NULL)
	{
		return new BError( "Light region not found" );
	}

	SetRegionLightLevel( lightregion, lightlevel );
	return new BLong(1);
}
#include "../wthrtype.h"
BObjectImp* UOExecutorModule::mf_SetRegionWeatherLevel()
{
	const String* region_name_str;
	int type;
	int severity;
	int aux;
	int lightoverride;
	if (! (getStringParam( 0, region_name_str ) &&
		   getParam( 1, type ) &&
		   getParam( 2, severity ) &&
		   getParam( 3, aux ) &&
		   getParam( 4, lightoverride )) )
	{
		return new BError( "Invalid Parameter type" );
	}

	WeatherRegion* weatherregion = weatherdef->getregion( region_name_str->value() );
	if (weatherregion == NULL)
	{
		return new BError( "Weather region not found" );
	}

	SetRegionWeatherLevel( weatherregion, type, severity, aux, lightoverride );

	return new BLong(1);
}
BObjectImp* UOExecutorModule::mf_AssignRectToWeatherRegion()
{
	const String* region_name_str;
	unsigned short xwest, ynorth, xeast, ysouth;
	const String* strrealm;

	if (! (getStringParam( 0, region_name_str ) &&
		   getParam( 1, xwest ) &&
		   getParam( 2, ynorth ) &&
		   getParam( 3, xeast ) &&
		   getParam( 4, ysouth ) &&
		   getStringParam( 5, strrealm)) )
	{
		return new BError( "Invalid Parameter type" );
	}
	Realm* realm = find_realm(strrealm->value());
	if(!realm) return new BError("Realm not found");
	if(!realm->valid(xwest,ynorth,0)) return new BError("Invalid Coordinates for realm");
	if(!realm->valid(xeast,ysouth,0)) return new BError("Invalid Coordinates for realm");

	bool res = weatherdef->assign_zones_to_region( region_name_str->data(),
												  xwest, ynorth,
												  xeast, ysouth,
												  realm );
	if (res)
		return new BLong(1);
	else
		return new BError( "Weather region not found" );
}

BObjectImp* UOExecutorModule::mf_Distance()
{
	UObject* obj1;
	UObject* obj2;
	if (getUObjectParam( exec, 0, obj1 ) &&
		getUObjectParam( exec, 1, obj2 ))
	{
		const UObject* tobj1 = obj1->toplevel_owner();
		const UObject* tobj2 = obj2->toplevel_owner();
		int xd = tobj1->x - tobj2->x;
		int yd = tobj1->y - tobj2->y;
		if (xd < 0) xd = -xd;
		if (yd < 0) yd = -yd;
		if (xd > yd)
			return new BLong(xd);
		else
			return new BLong(yd);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_DistanceEuclidean()
{
	UObject* obj1;
	UObject* obj2;
	if (getUObjectParam( exec, 0, obj1 ) &&
		getUObjectParam( exec, 1, obj2 ))
	{
		const UObject* tobj1 = obj1->toplevel_owner();
		const UObject* tobj2 = obj2->toplevel_owner();
		return new Double(sqrt( pow( (double)(tobj1->x - tobj2->x), 2)
							  + pow( (double)(tobj1->y - tobj2->y), 2)));
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_CoordinateDistance()
{
	unsigned short x1, y1, x2, y2;
	if ( !(getParam(0, x1) && getParam(1, y1)
		&& getParam(2, x2) && getParam(3, y2)) )
	{
		return new BError("Invalid parameter type");
	}
	return new BLong(pol_distance(x1, y1, x2, y2));
}

BObjectImp* UOExecutorModule::mf_CoordinateDistanceEuclidean()
{
	unsigned short x1, y1, x2, y2;
	if ( !(getParam(0, x1) && getParam(1, y1)
		&& getParam(2, x2) && getParam(3, y2)) )
	{
		return new BError("Invalid parameter type");
	}
	return new Double(sqrt( pow( (double)(x1 - x2), 2)
						  + pow( (double)(y1 - y2), 2)));
}

BObjectImp* UOExecutorModule::mf_GetCoordsInLine()
{
	int x1, y1, x2, y2;
	if ( !(getParam(0, x1) && getParam(1, y1)
		&& getParam(2, x2) && getParam(3, y2)) )
	{
		return new BError("Invalid parameter type");
	}
	else if ( x1 == x2 && y1 == y2 )
	{	// Same exact coordinates ... just give them the coordinate back!
		ObjArray* coords = new ObjArray;
		BStruct* point = new BStruct;
		point->addMember("x", new BLong(x1));
		point->addMember("y", new BLong(y1));
		coords->addElement(point);
		return coords;
	}

	double dx = abs(x2-x1)+0.5;
	double dy = abs(y2-y1)+0.5;
	int vx = 0, vy = 0;

	if ( x2 > x1 )
		vx = 1;
	else if ( x2 < x1 )
		vx = -1;
	if ( y2 > y1 )
		vy = 1;
	else if ( y2 < y1 )
		vy = -1;
	
	auto_ptr<ObjArray> coords (new ObjArray);
	if ( dx >= dy )
	{
		dy = dy / dx;
		
		for ( int c = 0; c <= dx; c++ )
		{
			int point_x = x1 + (c * vx);
			
			double float_y = double(c) * double(vy) * dy;
			if ( float_y - floor(float_y) >= 0.5 )
				float_y = ceil(float_y);
			int point_y = int(float_y) + y1;
			
			auto_ptr<BStruct> point ( new BStruct );
			point->addMember("x", new BLong(point_x));
			point->addMember("y", new BLong(point_y));
			coords->addElement(point.release());
		}
	}
	else
	{
		dx = dx / dy;
		for ( int c = 0; c <= dy; c++ )
		{
			int point_y = y1 + (c * vy);
			
			double float_x = double(c) * double(vx) * dx;
			if ( float_x - floor(float_x) >= 0.5 )
				float_x = ceil(float_x);
			int point_x = int(float_x) + x1;
			
			auto_ptr<BStruct> point (new BStruct);
			point->addMember("x", new BLong(point_x));
			point->addMember("y", new BLong(point_y));
			coords->addElement(point.release());
		}
	}
	return coords.release();
}

BObjectImp* UOExecutorModule::mf_GetFacing()
{
	unsigned short from_x, from_y, to_x, to_y;
	if ( !(getParam(0, from_x) && getParam(1, from_y)
		&& getParam(2, to_x) && getParam(3, to_y)) )
	{
		return new BError("Invalid parameter type");
	}

	double x = to_x - from_x;
	double y = to_y - from_y;
	double pi = acos(double(-1));
	double r = sqrt(x*x + y*y);
	
	double angle = ((acos(x/r) * 180.0) / pi);
	
	double y_check = ((asin(y/r) * 180.0) / pi);
	if ( y_check < 0 )
	{
		angle = 360 - angle;
	}

	unsigned short facing = ((short(angle/40)+10)%8);

	return new BLong(facing);
}

// FIXME : Should we do an Orphan check here as well? Ugh.
void true_extricate( Item* item )
{
	ConstForEach( clients, send_remove_object_if_inrange, item );
	if (item->container != NULL)
	{
		item->extricate();
	}
	else
	{
		remove_item_from_world( item );
	}
}

BObjectImp* UOExecutorModule::mf_MoveItemToContainer()
{
	Item* item;
	Item* cont_item;
	int px;
	int py;
	int add_to_existing_stack;
	if ( !(getItemParam( exec, 0, item ) &&
		   getItemParam( exec, 1, cont_item ) &&
		   getParam( 2, px, -1, 65535 ) &&
		   getParam( 3, py, -1, 65535 ) &&
		   getParam( 4, add_to_existing_stack, 0, 1 )) )
	{
		return new BError( "Invalid parameter type" );
	}

	ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends
	if (!item->movable())
	{
		Character* chr = controller_.get();
		if (chr == NULL || !chr->can_move( item ))
			return new BError( "That is immobile" );
	}
	if (item->inuse() && !is_reserved_to_me(item))
	{
		return new BError( "That item is being used." );
	}


	if (!cont_item->isa( UObject::CLASS_CONTAINER ))
	{
		return new BError( "Non-container selected as target" );
	}
	UContainer* cont = static_cast<UContainer*>(cont_item);

	if (cont->serial == item->serial)
	{
		return new BError( "Can't put a container into itself" );
	}
	if (is_a_parent( cont, item->serial ))
	{
		return new BError( "Can't put a container into an item in itself" );
	}
	if (!cont->can_add( *item ))
	{
		return new BError( "Container is too full to add that" );
	}
	//DAVE added this 12/04, call can/onInsert & can/onRemove scripts for this container
	Character* chr_owner = cont->GetCharacterOwner();
	if(chr_owner == NULL)
		if(controller_.get() != NULL)
			chr_owner = controller_.get();

	//daved changed order 1/26/3 check canX scripts before onX scripts.
	UContainer* oldcont = item->container;
	Item* existing_stack = NULL;

	if( (oldcont != NULL) &&
		(!oldcont->check_can_remove_script( chr_owner, item, UContainer::MT_CORE_MOVED )) )
		return new BError( "Could not remove item from its container." );
	if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
	{
		return new BError( "Item was destroyed in CanRemove script" );
	}

	if ( add_to_existing_stack )
	{
		existing_stack = cont->find_addable_stack(item);
		if ( existing_stack != NULL )
		{
			if(!cont->can_insert_increase_stack( chr_owner, UContainer::MT_CORE_MOVED, existing_stack, item->getamount(), item ) )
				return new BError( "Could not add to existing stack" );
		}
		else
			return new BError( "There is no existing stack" );
	}
	else
	{
		if(!cont->can_insert_add_item( chr_owner, UContainer::MT_CORE_MOVED, item ))
			return new BError( "Could not insert item into container." );
	}

	if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
	{
		return new BError( "Item was destroyed in CanInsert Script" );
	}

	if (!item->check_unequiptest_scripts() || !item->check_unequip_script())
		return new BError( "Item cannot be unequipped" );
	if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
	{
		return new BError( "Item was destroyed in Equip Script" );
	}

	if(oldcont != NULL)
	{
		oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
		if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
		{
			return new BError( "Item was destroyed in OnRemove script" );
		}
	}

	if ( !add_to_existing_stack )
	{
		u8 slotIndex = item->slot_index();
		if (!cont->can_add_to_slot(slotIndex))
		{
			item->destroy();
			return new BError( "No slots available in new container" );
		}
		if ( !item->slot_index(slotIndex) )
		{
			item->destroy();
			return new BError( "Couldn't set slot index on item" );
		}

		short x = static_cast<short>(px);
		short y = static_cast<short>(py);
		if (/*x < 0 || y < 0 ||*/ !cont->is_legal_posn( item, x, y ))
		{
			u16 tx, ty;
			cont->get_random_location( &tx, &ty );
			x = tx;
			y = ty;
		}

		// item->set_dirty();

		true_extricate( item );

		item->x = x;
		item->y = y;
		item->z = 0;

		cont->add( item );
		update_item_to_inrange( item );
		//DAVE added this 11/17: if in a Character's pack, update weight.
		UpdateCharacterWeight(item);

		cont->on_insert_add_item( chr_owner, UContainer::MT_CORE_MOVED, item );
		if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
		{
			return new BError( "Item was destroyed in OnInsert script" );
		}
	}
	else
	{
		u16 amount = item->getamount();
		existing_stack->add_to_self(item);
		true_extricate( item );
		update_item_to_inrange( existing_stack );
		UpdateCharacterWeight( existing_stack );

		cont->on_insert_increase_stack( chr_owner, UContainer::MT_CORE_MOVED, existing_stack, amount );
	}

	return new BLong(1);
}

BObjectImp* place_item_in_secure_trade_container( Client* client, Item* item );
BObjectImp* UOExecutorModule::mf_MoveItemToSecureTradeWin()
{
	Item* item;
	Character* chr;
	if ( !(getItemParam( exec, 0, item ) &&
		   getCharacterParam( exec, 1, chr )) )
	{
		return new BError( "Invalid parameter type" );
	}
	
	ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends
	if (!item->movable())
	{
		Character* _chr = controller_.get();
		if (_chr == NULL || !_chr->can_move( item ))
			return new BError( "That is immobile" );
	}
	if (item->inuse() && !is_reserved_to_me(item))
	{
		return new BError( "That item is being used." );
	}

	//daved changed order 1/26/3 check canX scripts before onX scripts.
	UContainer* oldcont = item->container;

	//DAVE added this 12/04, call can/onInsert & can/onRemove scripts for this container
	Character* chr_owner = NULL;
	if (oldcont != NULL)
		chr_owner = oldcont->GetCharacterOwner();
	if(chr_owner == NULL)
		if(controller_.get() != NULL)
			chr_owner = controller_.get();

	if( (oldcont != NULL) &&
		(!oldcont->check_can_remove_script( chr_owner, item, UContainer::MT_CORE_MOVED )) )
		return new BError( "Could not remove item from its container." );
	if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
	{
		return new BError( "Item was destroyed in CanRemove script" );
	}

	if (!item->check_unequiptest_scripts() || !item->check_unequip_script())
		return new BError( "Item cannot be unequipped" );
	if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
	{
		return new BError( "Item was destroyed in Equip Script" );
	}

	if(oldcont != NULL)
	{
		oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
		if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
		{
			return new BError( "Item was destroyed in OnRemove script" );
		}
	}
	
	true_extricate( item );
	
	return place_item_in_secure_trade_container( chr->client, item );
}

BObjectImp* UOExecutorModule::mf_EquipItem()
{
	Character* chr;
	Item* item;
	if (getCharacterParam( exec, 0, chr ) &&
		getItemParam( exec, 1, item ))
	{
		ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends
		if (!item->movable())
		{
			Character*_chr = controller_.get();
			if (_chr == NULL || !_chr->can_move( item ))
				return new BError( "That is immobile" );
		}

		if (item->inuse() && !is_reserved_to_me(item))
		{
			return new BError( "That item is being used." );
		}

		if (!chr->equippable( item ) ||
			!item->check_equiptest_scripts( chr ))
		{
			return new BError( "That item is not equippable by that character" );
		}
		if(item->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
		{
			return new BError( "Item was destroyed in EquipTest script" );
		}

		if (item->objtype_ != extobj.mount)
		{
			item->layer = tilelayer( item->graphic );
		}
		else
		{
			item->layer = LAYER_MOUNT;
		}

		if (item->has_equip_script())
		{
			BObjectImp* res = item->run_equip_script( chr, false );
			if (!res->isTrue())
				return res;
			else
				BObject obj( res );
		}


		true_extricate( item );

		// at this point, 'item' is free - doesn't belong to the world, or a container.
		chr->equip( item );
		send_wornitem_to_inrange( chr, item );

		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_RestartScript()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		if (chr->isa( UObject::CLASS_NPC ))
		{
			NPC* npc = static_cast<NPC*>(chr);
			npc->restart_script();
			return new BLong(1);
		}
		else
		{
			return new BError( "RestartScript only operates on NPCs" );
		}
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

#include "../resource.h"

BObjectImp* UOExecutorModule::mf_GetHarvestDifficulty()
{
	const String* resource;
	xcoord x;
	ycoord y;
	unsigned short tiletype;
	const String* strrealm;

	if (getStringParam( 0, resource ) &&
		getParam( 1, x ) &&
		getParam( 2, y ) &&
		getParam( 3, tiletype ) &&
		getStringParam( 4, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

		return get_harvest_difficulty( resource->data(), x, y, realm, tiletype );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_HarvestResource()
{
	xcoord x;
	ycoord y;
	const String* resource;
	int b;
	int n;
	const String* strrealm;

	if (getStringParam( 0, resource ) &&
		getParam( 1, x ) &&
		getParam( 2, y ) &&
		getParam( 3, b ) &&
		getParam( 4, n ) &&
		getStringParam( 5, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

		if(b <= 0)
			return new BError("b must be >= 0");
		return harvest_resource( resource->data(), x, y, realm, b, n );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_GetRegionName(/* objref */)
{
	UObject* obj;

	if (getUObjectParam( exec, 0, obj ))
	{
		JusticeRegion* justice_region;
		if (obj->isa(UObject::CLASS_ITEM))
			obj = obj->toplevel_owner();

		if (obj->isa(UObject::CLASS_CHARACTER))
		{
			Character* chr = static_cast<Character*>(obj);

			if (chr->logged_in)
				justice_region = chr->client->gd->justice_region;
			else
				justice_region = justicedef->getregion( chr->x, chr->y, chr->realm );
		}
		else
			justice_region = justicedef->getregion( obj->x, obj->y, obj->realm );

		if (justice_region == NULL)
			return new BError("No Region defined at this Location");
		else
			return new String(justice_region->region_name());
	}
	else
		return new BError("Invalid parameter");
}

BObjectImp* UOExecutorModule::mf_GetRegionNameAtLocation(/* x, y, realm */)
{
	unsigned short x, y;
	const String* strrealm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getStringParam( 2, strrealm ))
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");
		if (!realm->valid(x,y,0))
			return new BError("Invalid Coordinates for realm");

		JusticeRegion* justice_region = justicedef->getregion( x, y, realm );
		if (justice_region == NULL)
			return new BError("No Region defined at this Location");
		else
			return new String(justice_region->region_name());
	}
	else
		return new BError("Invalid parameter");
}

BObjectImp* UOExecutorModule::mf_GetRegionString()
{
	const String* resource;
	unsigned short x,y;
	const String* propname;
	const String* strrealm;

	if (getStringParam( 0, resource ) &&
		getParam( 1, x ) &&
		getParam( 2, y ) &&
		getStringParam( 3, propname ) &&
		getStringParam( 4, strrealm ))
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

		return get_region_string( resource->data(), x, y, realm, propname->value() );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_GetRegionLightLevelAtLocation(/* x, y, realm */)
{
	unsigned short x, y;
	const String* strrealm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getStringParam( 2, strrealm ))
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");
		if (!realm->valid(x,y,0))
			return new BError("Invalid Coordinates for realm");
		LightRegion* light_region = lightdef->getregion( x, y, realm );
		int lightlevel;
		if( light_region != NULL)
			lightlevel = light_region->lightlevel;
		else
			lightlevel =  ssopt.default_light_level;
		return new BLong(lightlevel);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* equip_from_template( Character* chr, const char* template_name );
BObjectImp* UOExecutorModule::mf_EquipFromTemplate()
{
	Character* chr;
	const String* template_name;
	if (getCharacterParam( exec, 0, chr ) &&
		getStringParam( 1, template_name ))
	{
		return equip_from_template( chr, template_name->data() );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

// FIXME: Use a PrivUpdater here
BObjectImp* UOExecutorModule::mf_GrantPrivilege()
{
	Character* chr;
	const String* privstr;
	if (getCharacterParam( exec, 0, chr ) &&
		getStringParam( 1, privstr ))
	{
		chr->grant_privilege( privstr->data() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

// FIXME: Use a PrivUpdater here
BObjectImp* UOExecutorModule::mf_RevokePrivilege()
{
	Character* chr;
	const String* privstr;
	if (getCharacterParam( exec, 0, chr ) &&
		getStringParam( 1, privstr ))
	{
		chr->revoke_privilege( privstr->data() );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ReadGameClock()
{
	return new BLong( read_gameclock() );
}

unsigned char decode_xdigit( unsigned char ch )
{
	if (ch >= '0' && ch <= '9')
		ch -= '0';
	else if (ch >= 'A' && ch <= 'F')
		ch = ch - 'A' + 0xa;
	else if (ch >= 'a' && ch <= 'f')
		ch = ch - 'a' + 0xa;

	return ch;
}

BObjectImp* UOExecutorModule::mf_SendPacket()
{
	Character* chr;
	Client* client;
	const String* str;
	if (getCharacterOrClientParam( exec, 0, chr, client ) &&
		getStringParam( 1, str ))
	{
		if ( str->length() % 2 > 0 )
		{
			return new BError( "Invalid packet string length." );
		}
		PktHelper::PacketOut<EncryptedPktBuffer> buffer; // encryptedbuffer is the only one without getID buffer[0]
		unsigned char* buf = reinterpret_cast<unsigned char*>(buffer->getBuffer());
		const char*s = str->data();
		while (buffer->offset < 2000 && isxdigit( s[0] ) && isxdigit( s[1] ))
		{
			unsigned char ch;
			ch =  (decode_xdigit( s[0] ) << 4) | decode_xdigit( s[1] );
			*(buf++) = ch;
			buffer->offset++;
			s+=2;
		}
		if (chr != NULL)
		{
			if (chr->has_active_client())
			{
				//printf( "SendPacket() data: %d bytes\n", buflen );
				//fdump( stdout, buffer, buflen );
				buffer.Send(chr->client);
				return new BLong(1);
			}
			else
			{
				return new BError( "No client attached" );
			}
		}
		else if (client != NULL)
		{
			if (client->isConnected())
			{
				buffer.Send(client);
				return new BLong(1);
			}
			else
			{
				return new BError( "Client is disconnected" );
			}
		}
		else
		{
			return new BError( "Invalid parameter type" );
		}

	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_SendQuestArrow()
{
	Character* chr;
	int x,y;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, x, -1, 1000000 ) &&
		getParam( 2, y, -1, 1000000 ))  //max vaues checked below
	{
		PktHelper::PacketOut<PktOut_BA> msg;
		if ( x == -1 && y == -1 )
		{
			msg->Write<u8>(static_cast<u8>(PKTOUT_BA_ARROW_OFF));
			msg->offset+=4; // u16 x_tgt,y_tgt
		}
		else
		{
			if(!chr->realm->valid(static_cast<unsigned short>(x),static_cast<unsigned short>(y),0))
				return new BError("Invalid Coordinates for Realm");
			msg->Write<u8>(static_cast<u8>(PKTOUT_BA_ARROW_ON));
			msg->WriteFlipped<u16>(static_cast<unsigned short>(x & 0xFFFF));
			msg->WriteFlipped<u16>(static_cast<unsigned short>(y & 0xFFFF));
		}
		if (!chr->has_active_client())
			return new BError( "No client attached" );
		msg.Send(chr->client);
		return new BLong( 1 );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ConsumeReagents()
{
	Character* chr;
	int spellid;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, spellid ))
	{
		if (!VALID_SPELL_ID(spellid))
		{
			return new BError( "Spell ID out of range" );
		}
		USpell* spell = spells2[ spellid ];
		if (spell == NULL)
		{
			return new BError( "No such spell" );
		}

		return new BLong( spell->consume_reagents( chr ) ? 1 : 0 );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_StartSpellEffect()
{
	Character* chr;
	int spellid;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, spellid ))
	{
		if (!VALID_SPELL_ID(spellid))
		{
			return new BError( "Spell ID out of range" );
		}
		USpell* spell = spells2[ spellid ];
		if (spell == NULL)
		{
			return new BError( "No such spell" );
		}

		spell->cast( chr );
		return new BLong( 1 );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}
BObjectImp* UOExecutorModule::mf_GetSpellDifficulty()
{
	int spellid;
	if (getParam( 0, spellid ))
	{
		if (!VALID_SPELL_ID(spellid))
		{
			return new BError( "Spell ID out of range" );
		}
		USpell* spell = spells2[ spellid ];
		if (spell == NULL)
		{
			return new BError( "No such spell" );
		}

		return new BLong( spell->difficulty() );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}
BObjectImp* UOExecutorModule::mf_SpeakPowerWords()
{
	Character* chr;
	int spellid;
	unsigned short font;
	unsigned short color;

	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, spellid ) &&
		getParam( 2, font ) &&
		getParam( 3, color ))
	{
		if (!VALID_SPELL_ID(spellid))
		{
			return new BError( "Spell ID out of range" );
		}
		USpell* spell = spells2[ spellid ];
		if (spell == NULL)
		{
			return new BError( "No such spell" );
		}

		spell->speak_power_words( chr, font, color );

		return new BLong( 1 );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ListEquippedItems()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		auto_ptr<ObjArray> arr (new ObjArray);
		for( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
			Item* item = chr->wornitem( layer );
			if (item != NULL)
			{
				arr->addElement( new EItemRefObjImp( item ) );
			}
		}
		return arr.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_GetEquipmentByLayer()
{
	Character* chr;
	int layer;
	if (getCharacterParam( exec, 0, chr ) &&
		getParam( 1, layer ))
	{
		if (layer < LOWEST_LAYER || layer > HIGHEST_LAYER)
		{
			return new BError( "Invalid layer" );
		}

		Item* item = chr->wornitem( layer );
		if (item == NULL)
		{
			return new BError( "Nothing equipped on that layer." );
		}
		else
		{
			return new EItemRefObjImp( item );
		}
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_DisconnectClient()
{
	Character* chr;
	Client* client;

	if (getCharacterOrClientParam( exec, 0, chr, client ))
	{
		if (chr != NULL)
		{
			if (!chr->has_active_client())
				return new BError( "No client attached" );

			client = chr->client;
		}

		if (client != NULL)
		{
			if (client->isConnected())
			{
				client->Disconnect();
				return new BLong(1);
			}
			else
				return new BError( "Client is disconnected" );
		}
		else
			return new BError( "Invalid parameter type" );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_GetMapInfo()
{
	xcoord x;
	ycoord y;
	const String* strrealm;

		// note that this uses WORLD_MAX_X, not WORLD_X,
		// because we can't read the outermost edge of the map
	if (getParam( 0, x ) && // FIXME realm size
		getParam( 1, y ) &&
		getStringParam(2, strrealm) ) // FIXME realm size
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for realm");

		MAPTILE_CELL cell = realm->getmaptile( static_cast<unsigned short>(x), static_cast<unsigned short>(y) );

		auto_ptr<BStruct> result (new BStruct);
		result->addMember( "z", new BLong( cell.z ) );
		result->addMember( "landtile", new BLong( cell.landtile ) );

		return result.release();
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}
BObjectImp* UOExecutorModule::mf_GetWorldHeight()
{
	unsigned short x, y;
	const String* strrealm;
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getStringParam( 2, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");

		short z = -255;
		if (realm->lowest_standheight( x, y, &z ))
			return new BLong(z);
		else
			return new BError( "Nowhere" );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_GetObjtypeByName()
{
	const String* namestr;
	if (getStringParam( 0, namestr ))
	{
		unsigned int objtype = get_objtype_byname( namestr->data() );
		if (objtype != 0)
			return new BLong( objtype );
		else
			return new BError( "No objtype by that name" );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_SendEvent()
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		BObjectImp* event = exec.getParamImp( 1 );
		if (event != NULL)
		{
			if (chr->isa( UObject::CLASS_NPC ))
			{
				NPC* npc = static_cast<NPC*>(chr);
				// event->add_ref(); // UNTESTED
				return npc->send_event( event->copy() );
			}
			else
			{
				return new BError( "That mobile is not an NPC" );
			}
		}
		else
		{
			return new BError( "Huh?  Not enough parameters" );
		}
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}
#include "../multi/house.h"
BObjectImp* UOExecutorModule::mf_DestroyMulti()
{
	UMulti* multi;
	if (getMultiParam( exec, 0, multi ))
	{
		const ItemDesc& id = find_itemdesc(multi->objtype_);
		if ( !id.destroy_script.empty() )
		{
			BObjectImp* res = run_script_to_completion(id.destroy_script, new EItemRefObjImp(multi));
			if ( !res->isTrue() )
			{				   // destruction is okay
				return res;
			}
		}

		UBoat* boat = multi->as_boat();
		if (boat != NULL)
		{
			return destroy_boat( boat );
		}
		UHouse* house = multi->as_house();
		if (house != NULL)
		{
			return destroy_house( house );
		}
		return new BError( "WTF!? Don't know what kind of multi that is!" );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}


BObjectImp* UOExecutorModule::mf_GetMultiDimensions()
{
	u16 multiid;
	if (getParam( 0, multiid ))
	{
		if(!MultiDefByMultiIDExists(multiid))
			return new BError("MultiID not found");

		const MultiDef& md = *MultiDefByMultiID(multiid);
		auto_ptr<BStruct> ret (new BStruct);
		ret->addMember( "xmin", new BLong( md.minrx ) );
		ret->addMember( "xmax", new BLong( md.maxrx ) );
		ret->addMember( "ymin", new BLong( md.minry ) );
		ret->addMember( "ymax", new BLong( md.maxry ) );
		return ret.release();
	}
	else
		return new BError("Invalid parameter");

}

BObjectImp* UOExecutorModule::mf_SetScriptController()
{
	Character* old_controller = controller_.get();
	BObjectImp* param0 = getParamImp( 0 );
	bool handled = false;

	if (param0->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(param0);
		if (lng->value() == 0)
		{
			controller_.clear();
			handled = true;
		}
	}

	if (!handled)
	{
		Character* chr;
		if (getCharacterParam( exec, 0, chr ))
			controller_.set(chr);
		else
			controller_.clear();
	}

	if (old_controller)
		return new ECharacterRefObjImp( old_controller );
	else
		return new BLong(0);
}


/*
BObjectImp* UOExecutorModule::mf_AssignMultiComponent()
{
	UMulti* multi;
	Item* item;
	if (getMultiParam( *this, 0, multi ) &&
		getItemParan( 1, item ))
	{
		UHouse* house = multi->as_house();
		if (house != NULL)
		{
			house->add_component( item );
			return new BLong(1);
		}
		else
		{
			return new BError( "AssignMultiComponent only functions on houses" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}
*/

BObjectImp* UOExecutorModule::mf_GetStandingHeight()
{
	unsigned short x, y;
	short z;
	const String* strrealm;
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getStringParam( 3, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x,y,z)) return new BError("Coordinates Invalid for Realm");
		short newz;
		UMulti* multi;
		Item* walkon;
		if (realm->lowest_walkheight( x, y, z, &newz, &multi, &walkon, true, MOVEMODE_LAND ))
		{
			auto_ptr<BStruct> arr (new BStruct);
			arr->addMember( "z", new BLong( newz ) );
			if (multi != NULL)
				arr->addMember( "multi", new EMultiRefObjImp( multi ) );
			return arr.release();
		}
		else
		{
			return new BError( "Can't stand there" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_GetStandingLayers(/* x, y, flags, realm */)
{
	unsigned short x, y;
	int flags;
	const String* strrealm;
	
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, flags ) &&
		getStringParam( 3, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");
		
		if (!realm->valid(x, y, 0))
			return new BError("Coordinates Invalid for Realm");
		
		auto_ptr<ObjArray> newarr( new ObjArray );

		MapShapeList mlist;
		realm->readmultis( mlist, x, y, flags );
		realm->getmapshapes( mlist, x, y, flags );

		for( unsigned i = 0; i < mlist.size(); ++i )
		{
			auto_ptr<BStruct> arr  ( new BStruct );
			
			if ( mlist[i].flags & (FLAG::MOVELAND | FLAG::MOVESEA) )
				arr->addMember( "z", new BLong( mlist[i].z + 1 ) );
			else
				arr->addMember( "z", new BLong( mlist[i].z ) );
			
			arr->addMember( "height", new BLong( mlist[i].height ) );
			arr->addMember( "flags", new BLong( mlist[i].flags ) );
			newarr->addElement( arr.release() );
		}
		
		return newarr.release();
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_ReserveItem()
{
	Item* item;
	if (getItemParam( exec, 0, item ))
	{
		if (item->inuse())
		{
			if (is_reserved_to_me( item ))
				return new BLong( 2 );
			else
				return new BError( "That item is already being used." );
		}
		item->inuse( true );
		reserved_items_.push_back( ItemRef(item) );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* UOExecutorModule::mf_ReleaseItem()
{
	Item* item;
	if (getItemParam( exec, 0, item ))
	{
		if (item->inuse())
		{
			for( unsigned i = 0; i < reserved_items_.size(); ++i )
			{
				if (reserved_items_[i].get() == item)
				{
					item->inuse( false );
					reserved_items_[i] = reserved_items_.back();
					reserved_items_.pop_back();
					return new BLong(1);
				}
			}
			return new BError( "That item is not reserved by this script." );
		}
		else
		{
			return new BError( "That item is not reserved." );
		}
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

void send_skillmsg( Client *client, const Character *chr );

BObjectImp* UOExecutorModule::mf_SendSkillWindow()
{
	Character *towhom, *forwhom;
	if (getCharacterParam( exec, 0, towhom ) &&
		getCharacterParam( exec, 1, forwhom ))
	{
		if (towhom->has_active_client())
		{
			send_skillmsg( towhom->client, forwhom );
			return new BLong(1);
		}
		else
		{
			return new BError( "No client attached" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

void send_paperdoll( Client *client, Character *chr );

BObjectImp* UOExecutorModule::mf_OpenPaperdoll()
{
	Character *towhom, *forwhom;
	if (getCharacterParam( exec, 0, towhom ) &&
		getCharacterParam( exec, 1, forwhom ))
	{
		if (towhom->has_active_client())
		{
			send_paperdoll( towhom->client, forwhom );
			return new BLong(1);
		}
		else
		{
			return new BError( "No client attached" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

// TODO: a FindSubstance call that does the 'find' portion below, but returns an
// array of the stacks.  All returned items would be marked 'inuse'.
// Then, make ConsumeSubstance able to use such an array of owned items.

BObjectImp* UOExecutorModule::mf_ConsumeSubstance()
{
	Item* cont_item;
	unsigned int objtype;
	int amount;
	if (getItemParam( exec, 0, cont_item ) &&
		getObjtypeParam( exec, 1, objtype ) &&
		getParam( 2, amount ))
	{
		if (!cont_item->isa( UObject::CLASS_CONTAINER ))
			return new BError( "That is not a container" );
		if (amount < 0)
			return new BError( "Amount cannot be negative" );

		UContainer* cont = static_cast<UContainer*>(cont_item);
		int amthave = cont->find_sumof_objtype_noninuse(objtype);
		if (amthave < amount)
			return new BError( "Not enough of that substance in container" );

		cont->consume_sumof_objtype_noninuse( objtype, amount );

		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

bool UOExecutorModule::is_reserved_to_me( Item* item )
{
	for( unsigned i = 0; i < reserved_items_.size(); ++i )
	{
		if (reserved_items_[i].get() == item)
			return true;
	}
	return false;
}

BObjectImp* UOExecutorModule::mf_Shutdown()
{
	exit_signalled = 1;
#ifndef _WIN32
	// the catch_signals_thread (actually main) sits with sigwait(),
	// so it won't wake up except by being signalled.
	signal_catch_thread();
#endif
	return new BLong(1);
}

string get_textcmd_help( Character* chr, const char* cmd );
BObjectImp* UOExecutorModule::mf_GetCommandHelp()
{
	Character* chr;
	const String* cmd;
	if (getCharacterParam( exec, 0, chr ) &&
		getStringParam( 1, cmd ))
	{
		string help = get_textcmd_help( chr, cmd->value().c_str() );
		if (!help.empty())
		{
			return new String( help );
		}
		else
		{
			return new BError( "No help for that command found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

void send_tip( Client* client, const std::string& tiptext );
BObjectImp* UOExecutorModule::mf_SendStringAsTipWindow()
{
	Character* chr;
	const String* str;
	if (getCharacterParam( exec, 0, chr ) &&
		getStringParam( 1, str ))
	{
		if (chr->has_active_client())
		{
			send_tip( chr->client, str->value() );
			return new BLong(1);
		}
		else
		{
			return new BError( "No client attached" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}

}

BObjectImp* UOExecutorModule::mf_ListItemsNearLocationWithFlag(/* x, y, z, range, flags, realm */) //DAVE
{
	unsigned short x, y;
	short range;
	int z, flags;
	const String* strrealm;
	Realm* realm;

	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getParam( 4, flags ) &&
		getStringParam( 5, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );

		unsigned short wxL, wyL, wxH, wyH;
		zone_convert_clip( x - range, y - range, realm, wxL, wyL );
		zone_convert_clip( x + range, y + range, realm, wxH, wyH );

		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				ZoneItems& witem = realm->zone[wx][wy].items;
				for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
				{
					Item* item = *itr;

					if( (tile_uoflags(item->graphic) & flags) )
					{
						if ((abs(item->x - x) <= range) && (abs(item->y - y) <= range))
							if ((z == LIST_IGNORE_Z) || (abs(item->z - z) < CONST_DEFAULT_ZRANGE))
								newarr->addElement( new EItemRefObjImp( item ) );
					}
				}
			}
		}
		return newarr.release();
	}

	return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_ListStaticsAtLocation(/* x, y, z, flags, realm */)
{
	unsigned short x, y;
	int z, flags;
	const String* strrealm;
	
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, flags ) &&
		getStringParam( 4, strrealm) )
	{
		Realm* realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );

		if (!( flags & ITEMS_IGNORE_STATICS ))
		{
			StaticEntryList slist;
			realm->getstatics( slist, x, y );

			for( unsigned i = 0; i < slist.size(); ++i )
			{
				if ((z == LIST_IGNORE_Z) || (slist[i].z == z))
				{
					auto_ptr<BStruct> arr (new BStruct);
					arr->addMember( "x", new BLong( x ) );
					arr->addMember( "y", new BLong( y ) );
					arr->addMember( "z", new BLong( slist[i].z ) );
					arr->addMember( "objtype", new BLong( slist[i].objtype ) );
					arr->addMember( "hue", new BLong( slist[i].hue ) );
					newarr->addElement( arr.release() );
				}
			}
		}

		if (!( flags & ITEMS_IGNORE_MULTIS ))
		{
			StaticList mlist;
			realm->readmultis( mlist, x, y );

			for( unsigned i = 0; i < mlist.size(); ++i )
			{
				if ((z == LIST_IGNORE_Z) || (mlist[i].z == z))
				{
					auto_ptr<BStruct> arr ( new BStruct );
					arr->addMember( "x", new BLong( x ) );
					arr->addMember( "y", new BLong( y ) );
					arr->addMember( "z", new BLong( mlist[i].z ) );
					arr->addMember( "objtype", new BLong( mlist[i].graphic ) );
					newarr->addElement( arr.release() );
				}
			}
		}

		return newarr.release();
	}
	else
		return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_ListStaticsNearLocation(/* x, y, z, range, flags, realm */)
{
	unsigned short x, y;
	int z, flags;
	short range;
	const String* strrealm;
	Realm* realm;
	
	if (getParam( 0, x ) &&
		getParam( 1, y ) &&
		getParam( 2, z ) &&
		getParam( 3, range ) &&
		getParam( 4, flags ) &&
		getStringParam( 5, strrealm) )
	{
		realm = find_realm(strrealm->value());
		if (!realm)
			return new BError("Realm not found");

		if (z == LIST_IGNORE_Z)
		{
			if (!realm->valid(x, y, 0))
				return new BError("Invalid Coordinates for realm");
		}
		else
		{
			if (!realm->valid(x, y, static_cast<short>(z)))
				return new BError("Invalid Coordinates for realm");
		}

		auto_ptr<ObjArray> newarr( new ObjArray );
		
		short wxL, wyL, wxH, wyH;
		wxL = x - range;
		if (wxL < 0)
			wxL = 0;
		wyL = y - range;
		if (wyL < 0)
			wyL = 0;
		wxH = x + range;
		if (wxH > realm->width()-1)
			wxH = realm->width()-1;
		wyH = y + range;
		if (wyH > realm->height()-1)
			wyH = realm->height()-1;
		
		for( unsigned short wx = wxL; wx <= wxH; ++wx )
		{
			for( unsigned short wy = wyL; wy <= wyH; ++wy )
			{
				if (!( flags & ITEMS_IGNORE_STATICS ))
				{
					StaticEntryList slist;
					realm->getstatics( slist, wx, wy );

					for( unsigned i = 0; i < slist.size(); ++i )
					{
						if ((z == LIST_IGNORE_Z) || (abs(slist[i].z - z) < CONST_DEFAULT_ZRANGE))
						{
							auto_ptr<BStruct> arr ( new BStruct );
							arr->addMember( "x", new BLong( wx ) );
							arr->addMember( "y", new BLong( wy ) );
							arr->addMember( "z", new BLong( slist[i].z ) );
							arr->addMember( "objtype", new BLong( slist[i].objtype ) );
							arr->addMember( "hue", new BLong( slist[i].hue ) );
							newarr->addElement( arr.release() );
						}
					}
				}

				if (!( flags & ITEMS_IGNORE_MULTIS ))
				{
					StaticList mlist;
					realm->readmultis( mlist, wx, wy );

					for( unsigned i = 0; i < mlist.size(); ++i )
					{
						if ((z == LIST_IGNORE_Z) || (abs(mlist[i].z - z) < CONST_DEFAULT_ZRANGE))
						{
							auto_ptr<BStruct> arr ( new BStruct );
							arr->addMember( "x", new BLong( wx ) );
							arr->addMember( "y", new BLong( wy ) );
							arr->addMember( "z", new BLong( mlist[i].z ) );
							arr->addMember( "objtype", new BLong( mlist[i].graphic ) );
							newarr->addElement( arr.release() );
						}
					}
				}
			}
		}
		
		return newarr.release();
	}
	else
		return new BError( "Invalid parameter" );
}

//  Birdy :  (Notes on Pathfinding)
//
//  This implimentation of pathfinding is actually from a very nice stl based A*
//  implimentation.  It will, within reason, succeed in finding a path in a wide
//  variety of circumstance, including traversing stairs and multiple level
//  structures.
//
//  Unfortunately, it, or my POL support classes implimenting it, seems to at times
//  generate corrupted results in finding a path.  These corruptions take the form
//  of basically looping within the result list, or linking in non-closed list nodes
//  which may be on the open list and generate more looping or which may have even
//  been deleted by the algorithm, generally causing an illegal reference and crash.
//
//  Thus far, the above senerio has only been replicated, though fairly reliably, in
//  cases where there are many many(about 100) npc's trying to pathfind at the same time,
//  in close proximity to one another.
//
//  "Looping" will, of course, cause the shard to hang while the pathfinding routine
//  tries to build the child list for the solution set.  An illegal reference will
//  cause the shard to crash most likely.  Neither is tollerable of course, so I have
//  put in two safeguards against this.
//
//  Safeguard #1) As we go through the nodes in the solution, I make sure each of them
//				is on the Closed List.  If not, the pathfind errors out with a
//				"Solution Corrupt!" error.  This is an attempt to short circuit any
//				solution containing erroneous nodes in it.
//
//  Safeguard #2) I keep a vector of the nodes in the solution as we go through them.
//				Before adding each node to the vector, I search that vector for that
//				node.  If that vector already contains the node, the pathfind errors
//				out with a "Solution Corrupt!" error.  THis is an attempt to catch
//				the cases where Closed List nodes have looped for some reason.
//
//  These two safeguards should not truly be necessary for this algorithm, and take up
//  space and time to guard against.  Thus, finding out why these problems are occurring
//  in the first place would be great, as we could fix that instead and remove these
//  checks.  If necessary to live with long term, then the solution vector should probably
//  be made into a hash table for quick lookups.
//
//  I have also implimented a sort blocking list that is supposed to represent those
//  spots that mobiles occupy and are thus not walkable on.  This is only maintained if
//  the mobilesBlock parameter passed from escript is true.  It would probably be good
//  to make this a hash table.  It may be said that it is a good idea to pre-process all
//  blocking spots on the map, but this would hinder the ability to manage 3d shifts, or
//  probably be prohibitive if an attempt were made to check for traversal at all possible
//  various z's.  So it may be that checking such at the time of the search for the path
//  is ultimately the best means of handling this.
//
//  Other details that may be nice to have here might be a method for interacting with
//  escript, letting escript tell the pathfinder to ignore doors, and then the pathfinder
//  putting in the solution list an indication to escript that at the given node a door
//  was encountered, so that AI can open the door before attempting to traverse that
//  path.  Providing the user the ability to define an array of objects which are to be
//  considered as blocking, or an array of objects which are to be ignored even if they
//  are blocking might be nice as well.  The former could be easily implimented by
//  adding to the mobile's blocking list.  The latter may be more difficult to do
//  considering if the item blocks, the walk function will not let you traverse it, and
//  it is the walk function that we use to accurately replicate the ability of the
//  mobile to traverse from one place to the next.  Additional functionality of some
//  sort may be necessary to do this, or making a copy of the walk function and putting
//  in a special walk function strictly for pathfinding which can take this array into
//  account.
//
//  For now, however, these concerns are all unaddressed.  I wish to see if this
//  function works and is useful, and also would prefer to find some way to track down
//  the corruption of the solution nodes before adding more complexity to the problem.
//
//  Other than the below function, the following files will be added to CVS to support
//  it in the src module.  They are :
//
//  stlastar.h  --  virtually untouched by me really, it is the original author's
//				  implimentation, pretty nicely done and documented if you are
//				  interested in learning about A*.
//
//  fsa.h	   --  a "fixed size allocation" module which I toy with enabling and
//				  disabling.  Using it is supposed to get you some speed, but it
//				  limits your maps because it will only allocate a certain # of
//				  nodes at once, and after that, it will return out of memory.
//				  Presently, it is disabled, but will be submitted to CVS for
//				  completion in case we wish to use it later.
//
//  uopathnode.h -- this is stricly my work, love it or hate it, it's pretty quick
//				  and dirty, and can stand to be cleaned up, optimized, and so forth.
//				  I have the name field and function in there to allow for some
//				  debugging, though the character array could probably be removed to
//				  make the nodes smaller.  I didn't find it mattered particularly, since
//				  these puppies are created and destroyed at a pretty good clip.
//				  It is this class that encapsulates the necessary functionality to
//				  make the otherwise fairly generic stlastar class work.

typedef AStarSearch<UOPathState> UOSearch;

BObjectImp* UOExecutorModule::mf_FindPath()
{
	unsigned short x1, x2;
	unsigned short y1, y2;
	short z1, z2;

	if (getParam( 0, x1 ) &&
		getParam( 1, y1 ) &&
		getParam( 2, z1, WORLD_MIN_Z, WORLD_MAX_Z ) &&
		getParam( 3, x2) &&
		getParam( 4, y2) &&
		getParam( 5, z2, WORLD_MIN_Z, WORLD_MAX_Z ) )
	{
		if (pol_distance(x1, y1, x2, y2) > ssopt.max_pathfind_range)
			return new BError("Beyond Max Range.");
		
		const String* strrealm;
		short theSkirt;
		int flags;
		if(!getStringParam(6,strrealm))
			strrealm = new String("britannia");

		if (!getParam(7, flags))
			flags = FP_IGNORE_MOBILES;
		
		if (!getParam(8, theSkirt))
			theSkirt = 5;

		if (theSkirt < 0)
			theSkirt = 0;

		Realm* realm = find_realm(strrealm->value());
		if(!realm) return new BError("Realm not found");
		if(!realm->valid(x1,y1,z1)) return new BError("Start Coordinates Invalid for Realm");
		if(!realm->valid(x2,y2,z2)) return new BError("End Coordinates Invalid for Realm");
		UOSearch * astarsearch;
		unsigned int SearchState;
		astarsearch = new UOSearch;
		short xL, xH, yL, yH;

		if (x1 < x2)
		{
			xL = x1 - theSkirt;
			xH = x2 + theSkirt;
		}
		else
		{
			xH = x1 + theSkirt;
			xL = x2 - theSkirt;
		}

		if (y1 < y2)
		{
			yL = y1 - theSkirt;
			yH = y2 + theSkirt;
		}
		else
		{
			yH = y1 + theSkirt;
			yL = y2 - theSkirt;
		}

		if (xL < 0)
			xL = 0;
		if (yL < 0)
			yL = 0;
		if (xH >= realm->width())
			xH = realm->width() - 1;
		if (yH >= realm->height())
			yH = realm->height() - 1;

		if (config.loglevel >= 12)
		{
			Log( "[FindPath] Calling FindPath(%d, %d, %d, %d, %d, %d, %s, 0x%x, %d)\n",
			  x1, y1, z1, x2, y2, z2, strrealm->data(), flags, theSkirt);
			Log( "[FindPath]   search for Blockers inside %d %d %d %d\n", xL, yL, xH, yH);
		}

		AStarBlockers theBlockers(xL, xH, yL, yH);

		unsigned short wxL, wyL, wxH, wyH;
		if (!(flags & FP_IGNORE_MOBILES))
		{
			zone_convert_clip( xL, yL, realm, wxL, wyL );
			zone_convert_clip( xH, yH, realm, wxH, wyH );
			for( unsigned short wx = wxL; wx <= wxH; ++wx )
			{
				for( unsigned short wy = wyL; wy <= wyH; ++wy )
				{
					ZoneCharacters& wchr = realm->zone[wx][wy].characters;
					for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
					{
						Character* chr = *itr;
						theBlockers.AddBlocker(chr->x, chr->y, chr->z);

						if (config.loglevel >= 12)
							Log( "[FindPath]	 add Blocker %s at %d %d %d\n",
								chr->name().c_str(), chr->x, chr->y, chr->z);
					}
				}
			}
		}

		// passed via GetSuccessors to realm->walkheight
		bool doors_block = (flags & FP_IGNORE_DOORS) ? false : true;

		if (config.loglevel >= 12)
		{
			Log( "[FindPath]   use StartNode %d %d %d\n", x1, y1, z1);
			Log( "[FindPath]   use EndNode %d %d %d\n", x2, y2, z2);
		}

		// Create a start state
		UOPathState nodeStart( x1, y1, z1, realm, &theBlockers );
		// Define the goal state
		UOPathState nodeEnd( x2, y2, z2, realm, &theBlockers );
		// Set Start and goal states
		astarsearch->SetStartAndGoalStates( nodeStart, nodeEnd );
		do
		{
			SearchState = astarsearch->SearchStep(doors_block);
		}
		while( SearchState == UOSearch::SEARCH_STATE_SEARCHING );
		if( SearchState == UOSearch::SEARCH_STATE_SUCCEEDED )
		{
			UOPathState *node = astarsearch->GetSolutionStart();
			ObjArray* nodeArray = NULL;
			BStruct* nextStep = NULL;

			nodeArray = new ObjArray();
			while ((node = astarsearch->GetSolutionNext()) != NULL)
			{
				nextStep = new BStruct;
				nextStep->addMember("x", new BLong(node->x));
				nextStep->addMember("y", new BLong(node->y));
				nextStep->addMember("z", new BLong(node->z));
				nodeArray->addElement(nextStep);
			}
			astarsearch->FreeSolutionNodes();
			delete astarsearch;
			return nodeArray;
		}
		else if (SearchState == UOSearch::SEARCH_STATE_FAILED)
		{
			delete astarsearch;
			return new BError("Failed to find a path.");
		}
		else if (SearchState == UOSearch::SEARCH_STATE_OUT_OF_MEMORY)
		{
			delete astarsearch;
			return new BError("Out of memory.");
		}
		else if (SearchState == UOSearch::SEARCH_STATE_SOLUTION_CORRUPTED)
		{
			delete astarsearch;
			return new BError("Solution Corrupted!");
		}

		delete astarsearch;
		return new BError("Pathfind Error.");
	}
	else
	{
		return new BError("Invalid parameter");
	}
}


BObjectImp* UOExecutorModule::mf_UseItem()
{
	Item* item;
	Character* chr;

	if (getItemParam( exec, 0, item ) && getCharacterParam(exec, 1, chr))
	{

		const ItemDesc& itemdesc = find_itemdesc( item->objtype_ );

		if (itemdesc.requires_attention && (chr->skill_ex_active() ||
											chr->casting_spell()))
		{
			if (chr->client != NULL)
			{
				send_sysmessage( chr->client, "I am already doing something else." );
				return new BError("Character busy.");;
			}
		}

		if (itemdesc.requires_attention && chr->hidden())
			chr->unhide();

		ref_ptr<EScriptProgram> prog;

		std::string on_use_script = item->get_use_script_name();

		if (!on_use_script.empty())
		{
			ScriptDef sd( on_use_script, NULL, "" );
			prog = find_script2( sd,
								true, // complain if not found
								config.cache_interactive_scripts );
		}
		else if (!itemdesc.on_use_script.empty())
		{
			prog = find_script2( itemdesc.on_use_script,
							   true,
							   config.cache_interactive_scripts );
		}

		if (prog.get() != NULL)
		{
			if (chr->start_itemuse_script( prog.get(), item, itemdesc.requires_attention ))
				return new BLong(1);
			else
				return new BError("Failed to start script!");
			// else log the fact?
		}
		else
		{
			if (chr->client != NULL)
				item->builtin_on_use( chr->client );
			return new BLong(0);
		}
	}
	else
	{
		return new BError("Invalid parameter");
	}
}

BObjectImp* UOExecutorModule::mf_FindSubstance()
{
	UContainer::Contents substanceVector;

	Item* cont_item;
	unsigned int objtype;
	int amount;

	if (getItemParam( exec, 0, cont_item ) &&
		getObjtypeParam( exec, 1, objtype ) &&
		getParam( 2, amount ))
	{
		int makeInUseLong;
		bool makeInUse;
		int flags;
		if (!getParam(3, makeInUseLong))
			makeInUse = false;
		else
			makeInUse = (makeInUseLong ? true : false);
		if (!getParam(4, flags))
			flags = 0;

		if (!cont_item->isa( UObject::CLASS_CONTAINER ))
			return new BError( "That is not a container" );
		if (amount < 0)
			return new BError( "Amount cannot be negative" );

		UContainer* cont = static_cast<UContainer*>(cont_item);
		int amthave = cont->find_sumof_objtype_noninuse(objtype, amount, substanceVector, flags);
		if ( (amthave < amount) && ( !(flags & FINDSUBSTANCE_FIND_ALL) ) )
			return new BError( "Not enough of that substance in container" );
		else
		{
			auto_ptr<ObjArray> theArray ( new ObjArray() );
			Item * item;

			for( UContainer::Contents::const_iterator itr = substanceVector.begin(); itr != substanceVector.end(); ++itr )
			{
				item = GET_ITEM_PTR( itr );
				if (item != NULL)
				{
					if ((makeInUse) && (!item->inuse()))
					{
						item->inuse( true );
						reserved_items_.push_back( ItemRef(item) );
					}
					theArray->addElement( new EItemRefObjImp( item ) );
				}
			}
			return theArray.release();
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_IsStackable()
{
	Item* item1;
	Item* item2;

	if ( !(getItemParam( exec, 0, item1 ) &&
		   getItemParam( exec, 1, item2 )) )
	{
		return new BError( "Invalid parameter type" );
	}

	if (item1->objtype_ != item2->objtype_)
	  return new BError("Objtypes differs");
	if (!item1->stackable())
	  return new BError("That item type is not stackable.");
	
	if (item1->can_add_to_self(*item2))
	  return new BLong(1);
	else
	  return new BError("Failed to stack");
}

BObjectImp* UOExecutorModule::mf_UpdateMobile()
{
	Character* chr;
	int flags;

	if (getCharacterParam(exec, 0, chr) &&
		getParam(1, flags))
	{
		if (flags == 1)
		{
			if ((!chr->isa(UObject::CLASS_NPC)) && (chr->client))  // no npc and active client
				send_owncreate( chr->client, chr ); // inform self
			if ((chr->isa(UObject::CLASS_NPC)) || (chr->client))  // npc or active client
				send_create_mobile_to_nearby_cansee(chr); // inform other
			else
				return new BError("Mobile is offline");
		}
		else
		{
			if ((!chr->isa(UObject::CLASS_NPC)) && (chr->client))  // no npc and active client
				send_move( chr->client, chr ); // inform self
			if ((chr->isa(UObject::CLASS_NPC)) || (chr->client))  // npc or active client
				send_move_mobile_to_nearby_cansee(chr); // inform other
			else
				return new BError("Mobile is offline");
		}
		return new BLong(1);
	}
	return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_UpdateItem()
{
	Item* item;

	if(getItemParam(exec, 0, item))
	{
		send_item_to_inrange( item );
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

UFACING direction_toward( xcoord from_x, ycoord from_y, xcoord to_x, ycoord to_y );
BObjectImp* UOExecutorModule::mf_CanWalk(/*movemode, x1, y1, z1, x2_or_dir, y2 := -1, realm := DEF*/)
{
	xcoord x;
	ycoord y;
	zcoord z;
	int x2_or_dir, y2_;
	const String* realm_name;
	const String* movemode_name;

	if ((getStringParam(0, movemode_name)) &&
		(getParam(1,x)) &&
		(getParam(2,y)) &&
		(getParam(3,z)) &&
		(getParam(4,x2_or_dir)) &&
		(getParam(5,y2_)) &&
		(getStringParam(6, realm_name)))
	{
		MOVEMODE movemode = Character::decode_movemode(movemode_name->value());

		Realm* realm = find_realm(realm_name->value());
		if( !realm ) 
			return new BError("Realm not found.");
		else if( !realm->valid(x, y, z) ) 
			return new BError("Invalid coordinates for realm.");
		UFACING dir;
		if (y2_ == -1)
			dir=static_cast<UFACING>(x2_or_dir & 0x7);
		else
		{
			if( !realm->valid(static_cast<xcoord>(x2_or_dir), static_cast<ycoord>(y2_), 0) ) 
				return new BError("Invalid coordinates for realm.");
			dir=direction_toward( x,y,static_cast<xcoord>(x2_or_dir),static_cast<ycoord>(y2_));
		}

		if (dir & 1) // check if diagonal movement is allowed
		{
			short new_z;
			u8 tmp_facing = (dir+1) & 0x7;
			unsigned short tmp_newx = x + move_delta[ tmp_facing ].xmove;
			unsigned short tmp_newy = y + move_delta[ tmp_facing ].ymove;

			// needs to save because if only one direction is blocked, it shouldn't block ;)
			bool walk1 = realm->walkheight(tmp_newx,tmp_newy,z,&new_z,NULL,NULL,true,movemode,NULL);

			tmp_facing = (dir-1) & 0x7;
			tmp_newx = x + move_delta[ tmp_facing ].xmove;
			tmp_newy = y + move_delta[ tmp_facing ].ymove;

			if (!walk1 && !realm->walkheight(tmp_newx,tmp_newy,z,&new_z,NULL,NULL,true,movemode,NULL))
				return new BError("Cannot walk there");
		}

		unsigned short newx = x + move_delta[ dir ].xmove;
		unsigned short newy = y + move_delta[ dir ].ymove;
		short newz;

		if (!realm->walkheight(newx,newy,z,&newz,NULL,NULL,true,movemode,NULL))
			return new BError("Cannot walk there");

		return new BLong(newz);
	}
	else
		return new BError("Invalid parameter");
}

//FIXME move ufacing.h functions into *.cpp then move this function there
UFACING direction_toward( xcoord from_x, ycoord from_y, xcoord to_x, ycoord to_y )
{
	if (from_x < to_x)		// East to target
	{
		if (from_y < to_y)
			return FACING_SE;
		else if (from_y == to_y)
			return FACING_E;
		else /* from_y > to_y */
			return FACING_NE;
	}
	else if (from_x == to_x)
	{
		if (from_y < to_y)
			return FACING_S;
		else if (from_y > to_y)
			return FACING_N;
	}
	else /* from_x > to_x */  // West to target
	{
		if (from_y < to_y)
			return FACING_SW;
		else if (from_y == to_y)
			return FACING_W;
		else /* from_y > to_y */
			return FACING_NW;
	}
	return FACING_N;
}

BObjectImp* UOExecutorModule::mf_SendCharProfile(/*chr, of_who, title, uneditable_text := array, editable_text := array*/)
{
	Character *chr, *of_who;
	const String* title;
	ObjArray* uText;
	ObjArray* eText;

	if ( getCharacterParam(exec, 0, chr) && 
		 getCharacterParam(exec, 1, of_who) && 
		 getStringParam( 2, title ) &&
		 getObjArrayParam( 3, uText ) &&
		 getObjArrayParam( 4, eText ) )
	{
		if (chr->logged_in && of_who->logged_in)
		{
			// Get The Unicode message lengths and convert the arrays to UC
			u16 uwtext[ (SPEECH_MAX_LEN + 1) ];
			u16 ewtext[ (SPEECH_MAX_LEN + 1) ];

			size_t ulen = uText->ref_arr.size();
			if ( ulen > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );

			if(!convertArrayToUC(uText, uwtext, ulen))
				return new BError("Invalid parameter type");

			size_t elen = eText->ref_arr.size();
			if ( elen > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );

			if (!convertArrayToUC(eText, ewtext, elen))
				return new BError("Invalid parameter type");

			sendCharProfile( chr, of_who, title->data(), uwtext, ewtext );
			return new BLong(1);
		}
		else
			return new BError("Mobile must be online.");
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* UOExecutorModule::mf_SendOverallSeason(/*season_id, playsound := 1*/)
{
	int season_id, playsound;

	if ( getParam(0, season_id) &&
		 getParam(1, playsound))
	{
		if ( season_id < 0 || season_id > 4 )
			return new BError("Invalid season id");
			
		PktHelper::PacketOut<PktOut_BC> msg;
		msg->Write<u8>(static_cast<u8>(season_id));
		msg->Write<u8>(static_cast<u8>(playsound));

		for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
		{
			Client* client = *itr;
			if (!client->chr->logged_in || client->getversiondetail().major < 1)
				continue;
			msg.Send(client);
		}
		return new BLong(1);
	}
	else
		return new BError("Invalid parameter");
}

UOFunctionDef UOExecutorModule::function_table[] =
{
	{ "SendStatus",					&UOExecutorModule::mf_SendStatus },

	{ "SendCharacterRaceChanger",	&UOExecutorModule::mf_SendCharacterRaceChanger },
	{ "SendHousingTool",		&UOExecutorModule::mf_SendHousingTool },
	{ "MoveObjectToLocation",	&UOExecutorModule::mf_MoveObjectToLocation },
	{ "SendOpenBook",		   &UOExecutorModule::mf_SendOpenBook },
	{ "SelectColor",			&UOExecutorModule::mf_SelectColor },
	{ "AddAmount",			  &UOExecutorModule::mf_AddAmount },
	{ "SendViewContainer",	  &UOExecutorModule::mf_SendViewContainer },
	{ "SendInstaResDialog",	 &UOExecutorModule::mf_SendInstaResDialog },
	{ "SendStringAsTipWindow",  &UOExecutorModule::mf_SendStringAsTipWindow },
	{ "GetCommandHelp",		 &UOExecutorModule::mf_GetCommandHelp },
	{ "PlaySoundEffectPrivate", &UOExecutorModule::mf_PlaySoundEffectPrivate },
	{ "ConsumeSubstance",	   &UOExecutorModule::mf_ConsumeSubstance },
	{ "FindSubstance",		  &UOExecutorModule::mf_FindSubstance },
	{ "Shutdown",			   &UOExecutorModule::mf_Shutdown },
	{ "OpenPaperdoll",		  &UOExecutorModule::mf_OpenPaperdoll },
	{ "SendSkillWindow",		&UOExecutorModule::mf_SendSkillWindow },
	{ "ReserveItem",			&UOExecutorModule::mf_ReserveItem },
	{ "ReleaseItem",			&UOExecutorModule::mf_ReleaseItem },
	{ "GetStandingHeight",	  &UOExecutorModule::mf_GetStandingHeight },
	{ "GetStandingLayers",	  &UOExecutorModule::mf_GetStandingLayers },
	{ "AssignRectToWeatherRegion",  &UOExecutorModule::mf_AssignRectToWeatherRegion },
	{ "CreateAccount",		  &UOExecutorModule::mf_CreateAccount },
	{ "FindAccount",			&UOExecutorModule::mf_FindAccount },
	{ "ListAccounts",		   &UOExecutorModule::mf_ListAccounts },

	// { "AssignMultiComponent",   &UOExecutorModule::mf_AssignMultiComponent },
	{ "SetScriptController",	&UOExecutorModule::mf_SetScriptController },
	{ "PolCore",				&UOExecutorModule::mf_PolCore },
	{ "GetWorldHeight",		 &UOExecutorModule::mf_GetWorldHeight },
	{ "StartSpellEffect",	   &UOExecutorModule::mf_StartSpellEffect },
	{ "GetSpellDifficulty",	 &UOExecutorModule::mf_GetSpellDifficulty },
	{ "SpeakPowerWords",		&UOExecutorModule::mf_SpeakPowerWords },
	{ "GetMultiDimensions",	 &UOExecutorModule::mf_GetMultiDimensions },
	{ "DestroyMulti",		   &UOExecutorModule::mf_DestroyMulti },
	{ "SendTextEntryGump",	  &UOExecutorModule::mf_SendTextEntryGump },
	{ "SendDialogGump",		 &UOExecutorModule::mf_SendGumpMenu },
	{ "CloseGump",			  &UOExecutorModule::mf_CloseGump },
	{ "CloseWindow",			&UOExecutorModule::mf_CloseWindow },
	{ "SendEvent",			  &UOExecutorModule::mf_SendEvent },
	{ "PlayMovingEffectXyz",	&UOExecutorModule::mf_PlayMovingEffectXyz },
	{ "GetEquipmentByLayer",	&UOExecutorModule::mf_GetEquipmentByLayer },
	{ "GetObjtypeByName",	   &UOExecutorModule::mf_GetObjtypeByName },
	{ "ListHostiles",		   &UOExecutorModule::mf_ListHostiles },
	{ "DisconnectClient",	   &UOExecutorModule::mf_DisconnectClient },
	{ "GetRegionName",			&UOExecutorModule::mf_GetRegionName },
	{ "GetRegionNameAtLocation",&UOExecutorModule::mf_GetRegionNameAtLocation },
	{ "GetRegionLightLevelAtLocation", &UOExecutorModule::mf_GetRegionLightLevelAtLocation },
	{ "GetRegionString",		&UOExecutorModule::mf_GetRegionString },
	{ "PlayStationaryEffect",   &UOExecutorModule::mf_PlayStationaryEffect },
	{ "GetMapInfo",			 &UOExecutorModule::mf_GetMapInfo },
	{ "ListObjectsInBox",	   &UOExecutorModule::mf_ListObjectsInBox },
	{ "ListMultisInBox",		&UOExecutorModule::mf_ListMultisInBox },
	{ "ListStaticsInBox",	   &UOExecutorModule::mf_ListStaticsInBox },
	{ "ListEquippedItems",	  &UOExecutorModule::mf_ListEquippedItems },
	{ "ConsumeReagents",		&UOExecutorModule::mf_ConsumeReagents },
	{ "SendPacket",			 &UOExecutorModule::mf_SendPacket },
	{ "SendQuestArrow",		 &UOExecutorModule::mf_SendQuestArrow },
	{ "RequestInput",		   &UOExecutorModule::mf_PromptInput },
	{ "ReadGameClock",		  &UOExecutorModule::mf_ReadGameClock },
	{ "GrantPrivilege",		 &UOExecutorModule::mf_GrantPrivilege },
	{ "RevokePrivilege",		&UOExecutorModule::mf_RevokePrivilege },
	{ "EquipFromTemplate",	  &UOExecutorModule::mf_EquipFromTemplate },
	{ "GetHarvestDifficulty",   &UOExecutorModule::mf_GetHarvestDifficulty },
	{ "HarvestResource",		&UOExecutorModule::mf_HarvestResource },
	{ "RestartScript",		  &UOExecutorModule::mf_RestartScript },
	{ "EnableEvents",		   &UOExecutorModule::mf_EnableEvents },
	{ "DisableEvents",		  &UOExecutorModule::mf_DisableEvents },
	{ "EquipItem",			  &UOExecutorModule::mf_EquipItem },
	{ "MoveItemToContainer",	&UOExecutorModule::mf_MoveItemToContainer },
	{ "MoveItemToSecureTradeWin", &UOExecutorModule::mf_MoveItemToSecureTradeWin },
	{ "FindObjtypeInContainer", &UOExecutorModule::mf_FindObjtypeInContainer },
	{ "SendOpenSpecialContainer", &UOExecutorModule::mf_SendOpenSpecialContainer },
	{ "SecureTradeWin",		 &UOExecutorModule::mf_SecureTradeWin },
	{ "CloseTradeWindow",		&UOExecutorModule::mf_CloseTradeWindow },
	{ "SendBuyWindow",		  &UOExecutorModule::mf_SendBuyWindow },
	{ "SendSellWindow",		 &UOExecutorModule::mf_SendSellWindow },
	{ "CreateItemInContainer",  &UOExecutorModule::mf_CreateItemInContainer },
	{ "CreateItemInInventory",  &UOExecutorModule::mf_CreateItemInInventory },
	{ "ListMobilesNearLocationEx",	  &UOExecutorModule::mf_ListMobilesNearLocationEx },
	{ "SystemFindObjectBySerial",	   &UOExecutorModule::mf_SystemFindObjectBySerial },
	{ "ListItemsNearLocationOfType",	&UOExecutorModule::mf_ListItemsNearLocationOfType },
	{ "ListItemsNearLocationWithFlag", &UOExecutorModule::mf_ListItemsNearLocationWithFlag },
	{ "ListStaticsAtLocation",	 &UOExecutorModule::mf_ListStaticsAtLocation },
	{ "ListStaticsNearLocation",   &UOExecutorModule::mf_ListStaticsNearLocation },
	{ "ListGhostsNearLocation", &UOExecutorModule::mf_ListGhostsNearLocation },
	{ "ListMobilesInLineOfSight", &UOExecutorModule::mf_ListMobilesInLineOfSight },
	{ "Distance",			   &UOExecutorModule::mf_Distance },
	{ "CoordinateDistance",		&UOExecutorModule::mf_CoordinateDistance },
	{ "DistanceEuclidean",				&UOExecutorModule::mf_DistanceEuclidean },
	{ "CoordinateDistanceEuclidean",	&UOExecutorModule::mf_CoordinateDistanceEuclidean },
	{ "GetCoordsInLine",		&UOExecutorModule::mf_GetCoordsInLine },
	{ "GetFacing",				&UOExecutorModule::mf_GetFacing },
	{ "SetRegionLightLevel",	&UOExecutorModule::mf_SetRegionLightLevel },
	{ "SetRegionWeatherLevel",  &UOExecutorModule::mf_SetRegionWeatherLevel },
	{ "EraseObjProperty",	   &UOExecutorModule::mf_EraseObjProperty },
	{ "GetGlobalProperty",	  &UOExecutorModule::mf_GetGlobalProperty },
	{ "SetGlobalProperty",	  &UOExecutorModule::mf_SetGlobalProperty },
	{ "EraseGlobalProperty",	&UOExecutorModule::mf_EraseGlobalProperty },
	{ "GetGlobalPropertyNames", &UOExecutorModule::mf_GetGlobalPropertyNames },
	{ "SaveWorldState",		 &UOExecutorModule::mf_SaveWorldState },
	{ "CreateMultiAtLocation",  &UOExecutorModule::mf_CreateMultiAtLocation },
	{ "TargetMultiPlacement",   &UOExecutorModule::mf_TargetMultiPlacement },
	{ "Resurrect",			  &UOExecutorModule::mf_Resurrect },
	{ "CreateNpcFromTemplate",  &UOExecutorModule::mf_CreateNpcFromTemplate },
	{ "RegisterForSpeechEvents", &UOExecutorModule::mf_RegisterForSpeechEvents },
	{ "EnumerateOnlineCharacters", &UOExecutorModule::mf_EnumerateOnlineCharacters },
	{ "PrintTextAbove",		 &UOExecutorModule::mf_PrintTextAbove },
	{ "PrintTextAbovePrivate",  &UOExecutorModule::mf_PrivateTextAbove },

	{ "Accessible",			 &UOExecutorModule::mf_Accessible },
	{ "ApplyConstraint",		&UOExecutorModule::mf_ApplyConstraint },
	{ "Attach",				 &UOExecutorModule::mf_Attach },
	{ "broadcast",			  &UOExecutorModule::broadcast },
	{ "CheckLineOfSight",	   &UOExecutorModule::mf_CheckLineOfSight },
	{ "CheckLosAt",			 &UOExecutorModule::mf_CheckLosAt },
	
	{ "CreateItemInBackpack",   &UOExecutorModule::mf_CreateItemInBackpack },
	{ "CreateItemAtLocation",   &UOExecutorModule::mf_CreateItemAtLocation },

	{ "CreateItemCopyAtLocation",   &UOExecutorModule::mf_CreateItemCopyAtLocation },

	{ "DestroyItem",			&UOExecutorModule::mf_DestroyItem },
	{ "Detach",				 &UOExecutorModule::mf_Detach },
	{ "EnumerateItemsInContainer", &UOExecutorModule::mf_EnumerateItemsInContainer },
	{ "FindPath",			   &UOExecutorModule::mf_FindPath },
	{ "GetAmount",			  &UOExecutorModule::mf_GetAmount },
	{ "GetMenuObjTypes",		&UOExecutorModule::mf_GetMenuObjTypes },
	{ "GetObjProperty",		 &UOExecutorModule::mf_GetObjProperty },
	{ "GetObjPropertyNames",	&UOExecutorModule::mf_GetObjPropertyNames },
	{ "GetObjType",			 &UOExecutorModule::mf_GetObjType },
	{ "GetPosition",			&UOExecutorModule::mf_GetPosition },
	{ "IsStackable",			&UOExecutorModule::mf_IsStackable },
	{ "ListItemsAtLocation",	&UOExecutorModule::mf_ListItemsAtLocation },
	{ "ListItemsNearLocation",  &UOExecutorModule::mf_ListItemsNearLocation },
	{ "ListMobilesNearLocation", &UOExecutorModule::mf_ListMobilesNearLocation },
	{ "PerformAction",		  &UOExecutorModule::mf_PerformAction },
	{ "PlayLightningBoltEffect", &UOExecutorModule::mf_PlayLightningBoltEffect },
	{ "PlayMovingEffect",	   &UOExecutorModule::mf_PlayMovingEffect },
	{ "PlayObjectCenteredEffect", &UOExecutorModule::mf_PlayObjectCenteredEffect },
	{ "PlaySoundEffect",		&UOExecutorModule::mf_PlaySoundEffect },
	{ "PlaySoundEffectXYZ",	 &UOExecutorModule::mf_PlaySoundEffectXYZ },
	{ "PlayMusic",				&UOExecutorModule::mf_PlayMusic },
	{ "SelectMenuItem2",		&UOExecutorModule::mf_SelectMenuItem },
	{ "SendSysMessage",		 &UOExecutorModule::mf_SendSysMessage },
	{ "SetObjProperty",		 &UOExecutorModule::mf_SetObjProperty },
	{ "SetName",				&UOExecutorModule::mf_SetName },
	{ "SubtractAmount",		 &UOExecutorModule::mf_SubtractAmount },
	{ "Target",				 &UOExecutorModule::mf_Target },
	{ "TargetCoordinates",	  &UOExecutorModule::mf_TargetCoordinates },
	{ "CancelTarget",		   &UOExecutorModule::mf_TargetCancel },
	{ "UseItem",				&UOExecutorModule::mf_UseItem },

	{ "CreateMenu",			 &UOExecutorModule::mf_CreateMenu },
	{ "AddMenuItem",			&UOExecutorModule::mf_AddMenuItem },

	{ "PlayStationaryEffectEx", &UOExecutorModule::mf_PlayStationaryEffect_Ex },
	{ "PlayObjectCenteredEffectEx", &UOExecutorModule::mf_PlayObjectCenteredEffect_Ex },
	{ "PlayMovingEffectEx",	 &UOExecutorModule::mf_PlayMovingEffect_Ex },
	{ "PlayMovingEffectXyzEx",  &UOExecutorModule::mf_PlayMovingEffectXyz_Ex },

	{ "UpdateItem",				&UOExecutorModule::mf_UpdateItem },
	{ "UpdateMobile",		   &UOExecutorModule::mf_UpdateMobile },
	{ "CheckLosBetween",		&UOExecutorModule::mf_CheckLosBetween },
	{ "CanWalk",				&UOExecutorModule::mf_CanWalk },
	{ "SendCharProfile",		&UOExecutorModule::mf_SendCharProfile },
	{ "SendOverallSeason",		&UOExecutorModule::mf_SendOverallSeason }
};

typedef map< string, int, ci_cmp_pred > FuncIdxMap;
FuncIdxMap funcmap;
bool funcmap_init = false;

int UOExecutorModule::functionIndex( const char *name )
{
	if (!funcmap_init)
	{
		for( unsigned idx = 0; idx < arsize(function_table); idx++ )
		{
			funcmap[ function_table[idx].funcname ] = idx;
		}
		funcmap_init = true;
	}

	FuncIdxMap::iterator itr = funcmap.find( name );
	if (itr != funcmap.end())
		return (*itr).second;
	else
		return -1;
}

BObjectImp* UOExecutorModule::execFunc( unsigned funcidx )
{
	return callMemberFunction(*this, function_table[funcidx].fptr)();
};

std::string UOExecutorModule::functionName( unsigned idx )
{
	return function_table[idx].funcname;
}

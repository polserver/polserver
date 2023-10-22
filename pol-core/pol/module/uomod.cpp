/** @file
 *
 * @par History
 * - 2005/03/02 Shinigami: internal_MoveItem - item took from container don't had the correct realm
 * - 2005/04/02 Shinigami: mf_CreateItemCopyAtLocation - added realm param
 * - 2005/04/28 Shinigami: mf_EnumerateItemsInContainer/enumerate_contents - added flag to list
 * content of locked container too
 * - 2005/04/28 Shinigami: added mf_SecureTradeWin - to init secure trade via script over long
 * distances
 *                         added mf_MoveItemToSecureTradeWin - to move item to secure trade window
 * via script
 * - 2005/04/31 Shinigami: mf_EnumerateItemsInContainer - error message added
 * - 2005/06/01 Shinigami: added mf_Attach - to attach a Script to a Character
 *                         mf_MoveItem - stupid non-realm anti-crash bugfix (e.g. for intrinsic
 * weapons without realm)
 *                         added mf_ListStaticsAtLocation - list static Items at location
 *                         added mf_ListStaticsNearLocation - list static Items around location
 *                         added mf_GetStandingLayers - get layer a mobile can stand
 * - 2005/06/06 Shinigami: mf_ListStatics* will return multi Items too / flags added
 * - 2005/07/04 Shinigami: mf_ListStatics* constants renamed
 *                         added Z to mf_ListStatics*Location - better specify what u want to get
 *                         modified Z handling of mf_ListItems*Location* and
 * mf_ListMobilesNearLocation*
 *                         better specify what u want to get
 *                         added realm-based coord check to mf_ListItems*Location*,
 *                         mf_List*InBox and mf_ListMobilesNearLocation*
 *                         added mf_ListStaticsInBox - list static items in box
 * - 2005/07/07 Shinigami: realm-based coord check in mf_List*InBox disabled
 * - 2005/09/02 Shinigami: mf_Attach - smaller bug which allowed u to attach more than one Script to
 * a Character
 * - 2005/09/03 Shinigami: mf_FindPath - added Flags to support non-blocking doors
 * - 2005/09/23 Shinigami: added mf_SendStatus - to send full status packet to support extensions
 * - 2005/12/06 MuadDib:   Rewrote realm handling for Move* and internal_move_item. New realm is
 *                         to be passed to function, and oldrealm handling is done within the
 * function.
 *                         Container's still save and check oldrealm as before to update clients.
 * - 2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
 * - 2006/05/10 Shinigami: mf_ListMobilesNearLocationEx - just a logical mismatch
 * - 2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
 * - 2006/06/08 Shinigami: started to add FindPath logging
 * - 2006/09/17 Shinigami: mf_SendEvent() will return error "Event queue is full, discarding event"
 * - 2006/11/25 Shinigami: mf_GetWorldHeight() fixed bug because z was'n initialized
 * - 2007/05/03 Turley:    added mf_GetRegionNameAtLocation - get name of justice region
 * - 2007/05/04 Shinigami: added mf_GetRegionName - get name of justice region by objref
 * - 2007/05/07 Shinigami: fixed a crash in mf_GetRegionName using NPCs; added TopLevelOwner
 * - 2008/07/08 Turley:    Added mf_IsStackable - Is item stackable?
 * - 2008/07/21 Mehdorn:   mf_ReserveItem() will return 2 if Item is already reserved by me (instead
 * of 1)
 * - 2009/07/27 MuadDib:   Packet Struct Refactoring
 * - 2009/08/08 MuadDib:   mf_SetRawSkill(),  mf_GetRawSkill(),  mf_ApplyRawDamage(), mf_GameStat(),
 *                         mf_AwardRawPoints(), old replace_properties(), mf_GetSkill() cleaned out.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: additional parentheses in mf_ListMultisInBox
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/09/14 MuadDib:   Slot support added to creation/move to container.
 * - 2009/09/15 MuadDib:   Multi registration/unregistration support added.
 * - 2009/10/22 Turley:    added CanWalk()
 * - 2009/11/19 Turley:    added flag param to UpdateMobile controls if packet 0x78 or 0x77 should
 * be send - Tomi
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 * - 2009/12/17 Turley:    CloseWindow( character, type, object ) - Tomi
 * - 2010/01/15 Turley:    (Tomi) season stuff
 *                         (Tomi) SpeakPowerWords font and color params
 * - 2011/11/12 Tomi:      added extobj.mount
 */


#include "pol_global_config.h"

#include "uomod.h"
#include <cmath>
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/impstr.h"
#include "../../clib/cfgelem.h"
#include "../../clib/clib.h"
#include "../../clib/clib_endian.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../plib/clidata.h"
#include "../../plib/mapcell.h"
#include "../../plib/mapshape.h"
#include "../../plib/maptile.h"
#include "../../plib/staticblock.h"
#include "../../plib/stlastar.h"
#include "../../plib/systemstate.h"
#include "../../plib/uconst.h"
#include "../../plib/udatfile.h"
#include "../action.h"
#include "../cfgrepos.h"
#include "../containr.h"
#include "../core.h"
#include "../eventid.h"
#include "../fnsearch.h"
#include "../gameclck.h"
#include "../globals/object_storage.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../layers.h"
#include "../lightlvl.h"
#include "../listenpt.h"
#include "../los.h"
#include "../menu.h"
#include "../mobile/charactr.h"
#include "../mobile/npc.h"
#include "../mobile/ufacing.h"
#include "../multi/boat.h"
#include "../multi/house.h"
#include "../multi/multi.h"
#include "../multi/multidef.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktboth.h"
#include "../network/pktdef.h"
#include "../npctmpl.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../polclass.h"
#include "../polclock.h"
#include "../polobject.h"
#include "../polsig.h"
#include "../profile.h"
#include "../savedata.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../spells.h"
#include "../target.h"
#include "../ufunc.h"
#include "../uimport.h"
#include "../umanip.h"
#include "../uobject.h"
#include "../uoexec.h"
#include "../uopathnode.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "../wthrtype.h"
#include "cfgmod.h"
#include "realms/realm.h"
#include "realms/realms.h"
#include "regions/guardrgn.h"
#include "regions/miscrgn.h"
#include "regions/resource.h"

#include <module_defs/uo.h>

namespace Pol
{
namespace Core
{
void cancel_trade( Mobile::Character* chr1 );
void cancel_all_trades();
Bscript::BObjectImp* place_item_in_secure_trade_container( Network::Client* client,
                                                           Items::Item* item );
Bscript::BObjectImp* open_trade_window( Network::Client* client, Mobile::Character* dropon );
void send_tip( Network::Client* client, const std::string& tiptext );
std::string get_textcmd_help( Mobile::Character* chr, const std::string& cmd );
void send_paperdoll( Network::Client* client, Mobile::Character* chr );
void send_skillmsg( Network::Client* client, const Mobile::Character* chr );
Bscript::BObjectImp* equip_from_template( Mobile::Character* chr,
                                          const std::string& template_name );
}  // namespace Core
namespace Module
{
using namespace Bscript;
using namespace Items;
using namespace Mobile;
using namespace Core;

#define CONST_DEFAULT_ZRANGE 19

class EMenuObjImp final : public PolApplicObj<Menu>
{
public:
  EMenuObjImp( const Menu& m ) : PolApplicObj<Menu>( &menu_type, m ) {}
  virtual const char* typeOf() const override { return "MenuRef"; }
  virtual u8 typeOfInt() const override { return OTMenuRef; }
  virtual BObjectImp* copy() const override { return new EMenuObjImp( value() ); }
};


UOExecutorModule::UOExecutorModule( UOExecutor& exec )
    : TmplExecutorModule<UOExecutorModule, Core::PolModule>( exec ),
      target_cursor_chr( nullptr ),
      menu_selection_chr( nullptr ),
      popup_menu_selection_chr( nullptr ),
      popup_menu_selection_above( nullptr ),
      prompt_chr( nullptr ),
      gump_chr( nullptr ),
      textentry_chr( nullptr ),
      resurrect_chr( nullptr ),
      selcolor_chr( nullptr ),
      target_options( 0 ),
      attached_chr_( nullptr ),
      attached_npc_( nullptr ),
      attached_item_( nullptr ),
      controller_( nullptr ),
      reserved_items_(),
      registered_for_speech_events( false )
{
}

UOExecutorModule::~UOExecutorModule()
{
  auto& uoex = uoexec();
  while ( !reserved_items_.empty() )
  {
    Item* item = reserved_items_.back().get();
    item->inuse( false );
    reserved_items_.pop_back();
  }

  if ( target_cursor_chr != nullptr )
  {
    // CHECKME can we cancel the cursor request?
    if ( target_cursor_chr->client != nullptr && target_cursor_chr->client->gd != nullptr )
      target_cursor_chr->client->gd->target_cursor_uoemod = nullptr;
    target_cursor_chr = nullptr;
  }
  if ( menu_selection_chr != nullptr )
  {
    if ( menu_selection_chr->client != nullptr && menu_selection_chr->client->gd != nullptr )
      menu_selection_chr->client->gd->menu_selection_uoemod = nullptr;
    menu_selection_chr = nullptr;
  }
  if ( popup_menu_selection_chr != nullptr )
  {
    if ( popup_menu_selection_chr->client != nullptr &&
         popup_menu_selection_chr->client->gd != nullptr )
      popup_menu_selection_chr->client->gd->popup_menu_selection_uoemod = nullptr;
    popup_menu_selection_chr = nullptr;
    popup_menu_selection_above = nullptr;
  }
  if ( prompt_chr != nullptr )
  {
    if ( prompt_chr->client != nullptr && prompt_chr->client->gd != nullptr )
      prompt_chr->client->gd->prompt_uoemod = nullptr;
    prompt_chr = nullptr;
  }
  if ( gump_chr != nullptr )
  {
    if ( gump_chr->client != nullptr && gump_chr->client->gd != nullptr )
      gump_chr->client->gd->remove_gumpmods( this );
    gump_chr = nullptr;
  }
  if ( textentry_chr != nullptr )
  {
    if ( textentry_chr->client != nullptr && textentry_chr->client->gd != nullptr )
      textentry_chr->client->gd->textentry_uoemod = nullptr;
    textentry_chr = nullptr;
  }
  if ( resurrect_chr != nullptr )
  {
    if ( resurrect_chr->client != nullptr && resurrect_chr->client->gd != nullptr )
      resurrect_chr->client->gd->resurrect_uoemod = nullptr;
    resurrect_chr = nullptr;
  }
  if ( selcolor_chr != nullptr )
  {
    if ( selcolor_chr->client != nullptr && selcolor_chr->client->gd != nullptr )
      selcolor_chr->client->gd->selcolor_uoemod = nullptr;
    selcolor_chr = nullptr;
  }
  if ( attached_chr_ != nullptr )
  {
    passert( attached_chr_->script_ex == &uoex );
    attached_chr_->script_ex = nullptr;
    attached_chr_ = nullptr;
  }
  if ( attached_item_ )
  {
    attached_item_->process( nullptr );
    attached_item_ = nullptr;
  }
  if ( registered_for_speech_events )
  {
    ListenPoint::deregister_from_speech_events( &uoex );
  }
}

BObjectImp* UOExecutorModule::mf_Attach( /* Character */ )
{
  Character* chr;
  if ( getCharacterParam( 0, chr ) )
  {
    if ( attached_chr_ == nullptr )
    {
      if ( chr->script_ex == nullptr )
      {
        attached_chr_ = chr;
        attached_chr_->script_ex = &uoexec();

        return new BLong( 1 );
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
  if ( attached_chr_ != nullptr )
  {
    passert( attached_chr_->script_ex == &uoexec() );
    attached_chr_->script_ex = nullptr;
    attached_chr_ = nullptr;
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

static bool item_create_params_ok( u32 objtype, int amount )
{
  return ( objtype >= UOBJ_ITEM__LOWEST && objtype <= Plib::systemstate.config.max_objtype ) &&
         amount > 0 && amount <= 60000L;
}

BObjectImp* _create_item_in_container( UContainer* cont, const ItemDesc* descriptor,
                                       unsigned short amount, bool force_stacking,
                                       std::optional<Core::Pos2d> pos, UOExecutorModule* uoemod )
{
  if ( ( Plib::tile_flags( descriptor->graphic ) & Plib::FLAG::STACKABLE ) || force_stacking )
  {
    for ( UContainer::const_iterator itr = cont->begin(); itr != cont->end(); ++itr )
    {
      Item* item = *itr;
      ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function
                                // ends

      if ( item->objtype_ == descriptor->objtype && !item->newbie() &&
           item->insured() == descriptor->insured && item->cursed() == descriptor->cursed &&
           item->color ==
               descriptor
                   ->color &&  // dave added 5/11/3, only add to existing stack if is default color
           item->has_only_default_cprops(
               descriptor ) &&  // dave added 5/11/3, only add to existing stack if default cprops
           ( !item->inuse() || ( uoemod && uoemod->is_reserved_to_me( item ) ) ) &&
           item->can_add_to_self( amount, force_stacking ) )
      {
        // DAVE added this 11/17, call can/onInsert scripts for this container
        Character* chr_owner = cont->GetCharacterOwner();
        if ( chr_owner == nullptr )
          if ( uoemod != nullptr )
            chr_owner = uoemod->controller_.get();

        // If the can insert script fails for combining a stack, we'll let the create new item code
        // below handle it
        // what if a cannInsert script modifies (adds to) the container we're iterating over? (they
        // shouldn't do that)
        // FIXME oh my, this makes no sense.  'item' in this case is already in the container.
        if ( !cont->can_insert_increase_stack( chr_owner, UContainer::MT_CORE_CREATED, item, amount,
                                               nullptr ) )
          continue;
        if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
        {
          return new BError( "Item was destroyed in CanInsert Script" );
        }
#ifdef PERGON
        item->ct_merge_stacks_pergon(
            amount );  // Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
#endif

        int newamount = item->getamount();
        newamount += amount;
        item->setamount( static_cast<unsigned short>( newamount ) );

        update_item_to_inrange( item );
        UpdateCharacterWeight( item );

        // FIXME again, this makes no sense, item is already in the container.
        cont->on_insert_increase_stack( chr_owner, UContainer::MT_CORE_CREATED, item, amount );
        if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
        {
          return new BError( "Item was destroyed in OnInsert Script" );
        }


        return new EItemRefObjImp( item );
      }
    }
  }
  else if ( amount != 1 && !force_stacking )
  {
    return new BError( "That item is not stackable.  Create one at a time." );
  }

  Item* item = Item::create( *descriptor );
  if ( item != nullptr )
  {
    ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
    item->setposition( cont->pos() );
    item->setamount( amount );

    if ( cont->can_add( *item ) )
    {
      if ( !descriptor->create_script.empty() )
      {
        BObjectImp* res =
            run_script_to_completion( descriptor->create_script, new EItemRefObjImp( item ) );
        if ( !res->isTrue() )
        {
          item->destroy();
          return res;
        }
        else
        {
          BObject ob( res );
        }
        if ( !cont->can_add( *item ) )
        {  // UNTESTED
          item->destroy();
          return new BError( "Couldn't add item to container after create script ran" );
        }
      }

      // run before owner is found. No need to find owner if not even able to use slots.
      u8 slotIndex = item->slot_index();
      if ( !cont->can_add_to_slot( slotIndex ) )
      {
        item->destroy();
        return new BError( "No slots available in this container" );
      }
      if ( !item->slot_index( slotIndex ) )
      {
        item->destroy();
        return new BError( "Couldn't set slot index on item" );
      }

      // DAVE added this 11/17, call can/onInsert scripts for this container
      Character* chr_owner = cont->GetCharacterOwner();
      if ( chr_owner == nullptr )
        if ( uoemod != nullptr )
          chr_owner = uoemod->controller_.get();

      if ( !cont->can_insert_add_item( chr_owner, UContainer::MT_CORE_CREATED, item ) )
      {
        item->destroy();
        // FIXME: try to propogate error from returned result of the canInsert script
        return new BError( "Could not insert item into container." );
      }
      if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
      {
        return new BError( "Item was destroyed in CanInsert Script" );
      }

      if ( !pos || !cont->is_legal_posn( pos.value() ) )
        pos = cont->get_random_location();

      item->setposition( Core::Pos4d( pos.value(), 0, cont->realm() ) );  // TODO POS realm

      cont->add( item );

      update_item_to_inrange( item );
      // DAVE added this 11/17, refresh owner's weight on item insert
      UpdateCharacterWeight( item );

      cont->on_insert_add_item( chr_owner, UContainer::MT_CORE_CREATED, item );
      if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
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
  int px;
  int py;

  if ( getItemParam( 0, item ) && getObjtypeParam( 1, descriptor ) && getParam( 2, amount ) &&
       getParam( 3, px, -1, 65535 ) && getParam( 4, py, -1, 65535 ) &&
       item_create_params_ok( descriptor->objtype, amount ) )
  {
    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      std::optional<Core::Pos2d> pos;
      if ( px >= 0 && py >= 0 )
        pos = Core::Pos2d( static_cast<u16>( px ), static_cast<u16>( py ) );
      return _create_item_in_container( static_cast<UContainer*>( item ), descriptor,
                                        static_cast<unsigned short>( amount ), false, pos, this );
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
  int px;
  int py;

  if ( getItemParam( 0, item ) && getObjtypeParam( 1, descriptor ) && getParam( 2, amount ) &&
       getParam( 3, px, -1, 65535 ) && getParam( 4, py, -1, 65535 ) &&
       item_create_params_ok( descriptor->objtype, amount ) )
  {
    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      std::optional<Core::Pos2d> pos;
      if ( px >= 0 && py >= 0 )
        pos = Core::Pos2d( static_cast<u16>( px ), static_cast<u16>( py ) );
      return _create_item_in_container( static_cast<UContainer*>( item ), descriptor,
                                        static_cast<unsigned short>( amount ), true, pos, this );
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


BObjectImp* UOExecutorModule::mf_Broadcast()
{
  const char* text;
  unsigned short font;
  unsigned short color;
  unsigned short requiredCmdLevel;
  text = exec.paramAsString( 0 );
  if ( text && getParam( 1, font ) &&     // todo: getFontParam
       getParam( 2, color ) &&            // todo: getColorParam
       getParam( 3, requiredCmdLevel ) )  // todo: getRequiredCmdLevelParam
  {
    if ( !Bscript::String::hasUTF8Characters( text ) )
      Core::broadcast( text, font, color, requiredCmdLevel );
    else
      Core::broadcast_unicode( text, "ENU", font, color, requiredCmdLevel );
    return new BLong( 1 );
  }
  else
  {
    return nullptr;
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
  if ( !getCharacterParam( 0, chr ) || !getItemParam( 1, item ) )
  {
    return new BError( "Invalid parameter type" );
  }

  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached" );
  }
  if ( !item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    return new BError( "That isn't a container" );
  }

  u8 save_layer = item->layer;
  item->layer = LAYER_BANKBOX;
  send_wornitem( chr->client, chr, item );
  item->layer = save_layer;
  item->setposition( chr->pos() );
  item->double_click( chr->client );  // open the container on the client's screen
  chr->add_remote_container( item );

  return new BLong( 1 );
}


BObjectImp* UOExecutorModule::mf_SecureTradeWin()
{
  Character* chr;
  Character* chr2;
  if ( !getCharacterParam( 0, chr ) || !getCharacterParam( 1, chr2 ) )
  {
    return new BError( "Invalid parameter type." );
  }

  if ( chr == chr2 )
  {
    return new BError( "You can't trade with yourself." );
  }

  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached." );
  }
  if ( !chr2->has_active_client() )
  {
    return new BError( "No client attached." );
  }

  return Core::open_trade_window( chr->client, chr2 );
}

BObjectImp* UOExecutorModule::mf_CloseTradeWindow()
{
  Character* chr;
  if ( !getCharacterParam( 0, chr ) )
  {
    return new BError( "Invalid parameter type." );
  }
  if ( !chr->trading_with )
    return new BError( "Mobile is not currently trading with anyone." );

  Core::cancel_trade( chr );
  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_SendViewContainer()
{
  Character* chr;
  Item* item;
  if ( !getCharacterParam( 0, chr ) || !getItemParam( 1, item ) )
  {
    return new BError( "Invalid parameter type" );
  }

  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached" );
  }
  if ( !item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    return new BError( "That isn't a container" );
  }
  UContainer* cont = static_cast<UContainer*>( item );
  chr->client->pause();
  send_open_gump( chr->client, *cont );
  send_container_contents( chr->client, *cont );
  chr->client->restart();
  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_FindObjtypeInContainer()
{
  Item* item;
  unsigned int objtype;
  int flags;
  if ( !getItemParam( 0, item ) || !getObjtypeParam( 1, objtype ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( !item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    return new BError( "That is not a container" );
  }
  if ( !getParam( 2, flags ) )
    flags = 0;

  UContainer* cont = static_cast<UContainer*>( item );
  Item* found = cont->find_objtype( objtype, flags );
  if ( found == nullptr )
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

  if ( getCharacterParam( 0, chr ) && ( ( ptext = getStringParam( 1 ) ) != nullptr ) &&
       getParam( 2, font ) && getParam( 3, color ) )
  {
    if ( chr->has_active_client() )
    {
      if ( !ptext->hasUTF8Characters() )
        send_sysmessage( chr->client, ptext->data(), font, color );
      else
        Core::send_sysmessage_unicode( chr->client, ptext->value(), "ENU", font, color );
      return new BLong( 1 );
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

  if ( getUObjectParam( 0, obj ) && getStringParam( 1, ptext ) && getParam( 2, font ) &&
       getParam( 3, color ) && getParam( 4, journal_print ) )
  {
    if ( !ptext->hasUTF8Characters() )
      return new BLong( say_above( obj, ptext->data(), font, color, journal_print ) );
    else
      return new BLong(
          say_above_unicode( obj, ptext->value(), "ENU", font, color, journal_print ) );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}
BObjectImp* UOExecutorModule::mf_PrintTextAbovePrivate()
{
  Character* chr;
  UObject* obj;
  const String* ptext;
  unsigned short font;
  unsigned short color;
  int journal_print;

  if ( getUObjectParam( 0, obj ) && getStringParam( 1, ptext ) && getCharacterParam( 2, chr ) &&
       getParam( 3, font ) && getParam( 4, color ) && getParam( 5, journal_print ) )
  {
    if ( !ptext->hasUTF8Characters() )
      return new BLong( private_say_above( chr, obj, ptext->data(), font, color, journal_print ) );
    else
      return new BLong( private_say_above_unicode( chr, obj, ptext->value(), "ENU", font, color ) );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

// const int TGTOPT_NOCHECK_LOS = 0x0000; // currently unused
const int TGTOPT_CHECK_LOS = 0x0001;
const int TGTOPT_HARMFUL = 0x0002;
const int TGTOPT_HELPFUL = 0x0004;
const int TGTOPT_ALLOW_NONLOCAL = 0x0008;

void handle_script_cursor( Character* chr, UObject* obj )
{
  if ( chr != nullptr && chr->client->gd->target_cursor_uoemod != nullptr )
  {
    auto& uoex = chr->client->gd->target_cursor_uoemod->uoexec();
    if ( obj != nullptr )
    {
      if ( obj->ismobile() )
      {
        Character* targetted_chr = static_cast<Character*>( obj );
        if ( chr->client->gd->target_cursor_uoemod->target_options & TGTOPT_HARMFUL )
        {
          targetted_chr->inform_engaged( chr );
          chr->repsys_on_attack( targetted_chr );
        }
        else if ( chr->client->gd->target_cursor_uoemod->target_options & TGTOPT_HELPFUL )
        {
          chr->repsys_on_help( targetted_chr );
        }
      }
      uoex.ValueStack.back().set( new BObject( obj->make_ref() ) );
    }
    // even on cancel, we wake the script up.
    uoex.revive();
    chr->client->gd->target_cursor_uoemod->target_cursor_chr = nullptr;
    chr->client->gd->target_cursor_uoemod = nullptr;
  }
}


BObjectImp* UOExecutorModule::mf_Target()
{
  Character* chr;
  if ( !getCharacterParam( 0, chr ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( !chr->has_active_client() )
  {
    return new BError( "No client connected" );
  }
  if ( chr->target_cursor_busy() )
  {
    return new BError( "Client busy with another target cursor" );
  }

  if ( !getParam( 1, target_options ) )
    target_options = TGTOPT_CHECK_LOS;

  PKTBI_6C::CURSOR_TYPE crstype;

  if ( target_options & TGTOPT_HARMFUL )
    crstype = PKTBI_6C::CURSOR_TYPE_HARMFUL;
  else if ( target_options & TGTOPT_HELPFUL )
    crstype = PKTBI_6C::CURSOR_TYPE_HELPFUL;
  else
    crstype = PKTBI_6C::CURSOR_TYPE_NEUTRAL;

  if ( !uoexec().suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::Target():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  TargetCursor* tgt_cursor = nullptr;

  bool is_los_checked = ( target_options & TGTOPT_CHECK_LOS ) && !chr->ignores_line_of_sight();
  bool allow_nonlocal = ( target_options & TGTOPT_ALLOW_NONLOCAL );
  if ( is_los_checked )
  {
    if ( allow_nonlocal )
    {
      tgt_cursor = &gamestate.target_cursors.los_checked_allow_nonlocal_script_cursor;
    }
    else
    {
      tgt_cursor = &gamestate.target_cursors.los_checked_script_cursor;
    }
  }
  else
  {
    if ( allow_nonlocal )
    {
      tgt_cursor = &gamestate.target_cursors.nolos_checked_allow_nonlocal_script_cursor;
    }
    else
    {
      tgt_cursor = &gamestate.target_cursors.nolos_checked_script_cursor;
    }
  }

  tgt_cursor->send_object_cursor( chr->client, crstype );

  chr->client->gd->target_cursor_uoemod = this;
  target_cursor_chr = chr;

  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_CancelTarget()
{
  Character* chr;
  if ( getCharacterParam( 0, chr ) )
  {
    if ( chr->has_active_client() )
    {
      if ( chr->target_cursor_busy() )
      {
        Network::PktHelper::PacketOut<Network::PktOut_6C> msg;
        msg->Write<u8>( PKTBI_6C::UNK1_00 );
        msg->offset += 4;  // u32 target_cursor_serial
        msg->Write<u8>( 0x3u );
        // rest 0
        msg.Send( chr->client, sizeof msg->buffer );
        return new BLong( 0 );
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
  if ( chr != nullptr && chr->client->gd->target_cursor_uoemod != nullptr )
  {
    auto& uoex = chr->client->gd->target_cursor_uoemod->uoexec();
    if ( msg != nullptr )
    {
      BStruct* arr = new BStruct;
      arr->addMember( "x", new BLong( cfBEu16( msg->x ) ) );
      arr->addMember( "y", new BLong( cfBEu16( msg->y ) ) );
      arr->addMember( "z", new BLong( msg->z ) );
      //      FIXME: Objtype CANNOT be trusted! Scripts must validate this, or, we must
      //      validate right here. Should we check map/static, or let that reside
      //      for scripts to run ListStatics? In theory, using Injection or similar,
      //      you could mine where no mineable tiles are by faking objtype in packet
      //      and still get the resources?
      arr->addMember( "objtype", new BLong( cfBEu16( msg->graphic ) ) );

      u32 selected_serial = cfBEu32( msg->selected_serial );
      if ( selected_serial )
      {
        UObject* obj = system_find_object( selected_serial );
        if ( obj )
        {
          arr->addMember( obj->target_tag(), obj->make_ref() );
        }
      }

      //      Never trust packet's objtype is the reason here. They should never
      //      target on other realms. DUH!
      arr->addMember( "realm", new String( chr->realm()->name() ) );

      Multi::UMulti* multi =
          chr->realm()->find_supporting_multi( cfBEu16( msg->x ), cfBEu16( msg->y ), msg->z );
      if ( multi != nullptr )
        arr->addMember( "multi", multi->make_ref() );

      uoex.ValueStack.back().set( new BObject( arr ) );
    }

    uoex.revive();
    chr->client->gd->target_cursor_uoemod->target_cursor_chr = nullptr;
    chr->client->gd->target_cursor_uoemod = nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_TargetCoordinates()
{
  Character* chr;
  if ( !getCharacterParam( 0, chr ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( !chr->has_active_client() )
  {
    return new BError( "Mobile has no active client" );
  }
  if ( chr->target_cursor_busy() )
  {
    return new BError( "Client has an active target cursor" );
  }

  if ( !uoexec().suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::TargetCoordinates():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  gamestate.target_cursors.script_cursor2.send_coord_cursor( chr->client );
  chr->client->gd->target_cursor_uoemod = this;
  target_cursor_chr = chr;
  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_TargetMultiPlacement()
{
  Character* chr;
  unsigned int objtype, hue;
  int flags;
  int xoffset, yoffset;
  if ( !( getCharacterParam( 0, chr ) && getObjtypeParam( 1, objtype ) && getParam( 2, flags ) &&
          getParam( 3, xoffset ) && getParam( 4, yoffset ) && getParam( 5, hue ) ) )
  {
    return new BError( "Invalid parameter type" );
  }


  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached" );
  }

  if ( chr->target_cursor_busy() )
  {
    return new BError( "Client busy with another target cursor" );
  }

  if ( find_itemdesc( objtype ).type != ItemDesc::BOATDESC &&
       find_itemdesc( objtype ).type != ItemDesc::HOUSEDESC )
  {
    return new BError( "Object Type is out of range for Multis" );
  }

  if ( !uoexec().suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::TargetMultiPlacement():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  chr->client->gd->target_cursor_uoemod = this;
  target_cursor_chr = chr;

  gamestate.target_cursors.multi_placement_cursor.send_placemulti(
      chr->client, objtype, flags, (s16)xoffset, (s16)yoffset, hue );

  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_GetObjType()
{
  Item* item;
  Character* chr;

  if ( getItemParam( 0, item ) )
  {
    return new BLong( item->objtype_ );
  }
  else if ( getCharacterParam( 0, chr ) )
  {
    return new BLong( chr->objtype_ );
  }
  else
  {
    return new BLong( 0 );
  }
}

// FIXME character needs an Accessible that takes an Item*
BObjectImp* UOExecutorModule::mf_Accessible()
{
  Character* chr;
  Item* item;
  int range;

  if ( getCharacterParam( 0, chr ) && getItemParam( 1, item ) )
  {
    // Range defaults to -1 if it's not defined in the .em file (old scripts)
    // or the user provides a weird object.
    if ( exec.numParams() < 3 || !getParam( 2, range ) )
      range = -1;

    if ( chr->can_access( item, range ) )
      return new BLong( 1 );
    else
      return new BLong( 0 );
  }
  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_GetAmount()
{
  Item* item;

  if ( getItemParam( 0, item ) )
  {
    return new BLong( item->getamount() );
  }
  else
  {
    return new BLong( 0 );
  }
}


// FIXME, copy-pasted from NPC, copy-pasted to CreateItemInContainer
BObjectImp* UOExecutorModule::mf_CreateItemInBackpack()
{
  Character* chr;
  const ItemDesc* descriptor;
  unsigned short amount;
  int px;
  int py;

  if ( getCharacterParam( 0, chr ) && getObjtypeParam( 1, descriptor ) && getParam( 2, amount ) &&
       getParam( 3, px, -1, 65535 ) && getParam( 4, py, -1, 65535 ) &&
       item_create_params_ok( descriptor->objtype, amount ) )
  {
    UContainer* backpack = chr->backpack();
    if ( backpack != nullptr )
    {
      std::optional<Core::Pos2d> pos;
      if ( px >= 0 && py >= 0 )
        pos = Core::Pos2d( static_cast<u16>( px ), static_cast<u16>( py ) );
      return _create_item_in_container( backpack, descriptor, amount, false, pos, this );
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

BObjectImp* _complete_create_item_at_location( Item* item, unsigned short x, unsigned short y,
                                               short z, Realms::Realm* realm )
{
  item->setposition( Core::Pos4d( x, y, static_cast<signed char>( z ), realm ) );

  // ITEMDESCTODO: use the original descriptor
  const ItemDesc& id = find_itemdesc( item->objtype_ );
  if ( !id.create_script.empty() )
  {
    BObjectImp* res = run_script_to_completion( id.create_script, new EItemRefObjImp( item ) );
    if ( !res->isTrue() )
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

BObjectImp* UOExecutorModule::mf_CreateItemAtLocation( /* x,y,z,objtype,amount,realm */ )
{
  unsigned short x, y;
  short z;
  const ItemDesc* itemdesc;
  unsigned short amount;
  const String* strrealm;
  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
       getObjtypeParam( 3, itemdesc ) && getParam( 4, amount, 1, 60000 ) &&
       getStringParam( 5, strrealm ) && item_create_params_ok( itemdesc->objtype, amount ) )
  {
    if ( !( Plib::tile_flags( itemdesc->graphic ) & Plib::FLAG::STACKABLE ) && ( amount != 1 ) )
    {
      return new BError( "That item is not stackable.  Create one at a time." );
    }

    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( !realm->valid( x, y, z ) )
      return new BError( "Invalid Coordinates for Realm" );
    Item* item = Item::create( *itemdesc );
    if ( item != nullptr )
    {
      item->setamount( amount );
      return _complete_create_item_at_location( item, x, y, z, realm );
    }
    else
    {
      return new BError( "Unable to create item of objtype " + Clib::hexint( itemdesc->objtype ) );
    }
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_CreateItemCopyAtLocation( /* x,y,z,item,realm */ )
{
  unsigned short x, y;
  short z;
  Item* origitem;
  const String* strrealm;
  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
       getItemParam( 3, origitem ) && getStringParam( 4, strrealm ) )
  {
    if ( origitem->script_isa( POLCLASS_MULTI ) )
      return new BError( "This function does not work with Multi objects." );

    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( !realm->valid( x, y, z ) )
      return new BError( "Invalid Coordinates for Realm" );
    Item* item = origitem->clone();
    if ( item != nullptr )
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

BObjectImp* UOExecutorModule::mf_CreateMultiAtLocation( /* x,y,z,objtype,flags,realm */ )
{
  unsigned short x, y;
  short z;
  const ItemDesc* descriptor;
  int flags = 0;
  Realms::Realm* realm = find_realm( "britannia" );
  if ( !( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
          getObjtypeParam( 3, descriptor ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( exec.hasParams( 5 ) )
  {
    if ( !getParam( 4, flags ) )
      return new BError( "Invalid parameter type" );
  }
  if ( exec.hasParams( 6 ) )
  {
    const String* strrealm;
    if ( !getStringParam( 5, strrealm ) )
      return new BError( "Invalid parameter type" );
    realm = find_realm( strrealm->value() );
  }

  if ( !realm )
    return new BError( "Realm not found" );
  if ( !realm->valid( x, y, z ) )
    return new BError( "Invalid Coordinates for Realm" );
  if ( descriptor->type != ItemDesc::BOATDESC && descriptor->type != ItemDesc::HOUSEDESC )
  {
    return new BError( "That objtype is not a Multi" );
  }

  return Multi::UMulti::scripted_create( *descriptor, x, y, static_cast<signed char>( z ), realm,
                                         flags );
}

void replace_properties( Clib::ConfigElem& elem, BStruct* custom )
{
  for ( BStruct::Contents::const_iterator citr = custom->contents().begin(),
                                          end = custom->contents().end();
        citr != end; ++citr )
  {
    const std::string& name = ( *citr ).first;
    BObjectImp* ref = ( *citr ).second->impptr();

    if ( name == "CProps" )
    {
      if ( ref->isa( BObjectImp::OTDictionary ) )
      {
        BDictionary* cpropdict = static_cast<BDictionary*>( ref );
        const BDictionary::Contents& cprop_cont = cpropdict->contents();
        BDictionary::Contents::const_iterator itr;
        for ( itr = cprop_cont.begin(); itr != cprop_cont.end(); ++itr )
        {
          elem.add_prop( "cprop", ( ( *itr ).first->getStringRep() + "\t" +
                                    ( *itr ).second->impptr()->pack() ) );
        }
      }
      else
      {
        throw std::runtime_error( "NPC override_properties: CProps must be a dictionary, but is: " +
                                  std::string( ref->typeOf() ) );
      }
    }
    else
    {
      elem.clear_prop( name.c_str() );
      elem.add_prop( name, ref->getStringRep() );
    }
  }
}

BObjectImp* UOExecutorModule::mf_CreateNpcFromTemplate()
{
  const String* tmplname;
  unsigned short x, y;
  short z;
  const String* strrealm;
  int forceInt;
  bool forceLocation;
  Realms::Realm* realm = find_realm( "britannia" );

  if ( !( getStringParam( 0, tmplname ) && getParam( 1, x ) && getParam( 2, y ) &&
          getParam( 3, z, ZCOORD_MIN, ZCOORD_MAX ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  BObjectImp* imp = getParamImp( 4 );
  BStruct* custom_struct = nullptr;
  if ( imp->isa( BObjectImp::OTLong ) )
  {
    custom_struct = nullptr;
  }
  else if ( imp->isa( BObjectImp::OTStruct ) )
  {
    custom_struct = static_cast<BStruct*>( imp );
  }
  else
  {
    return new BError( std::string( "Parameter 4 must be a Struct or Integer(0), got " ) +
                       BObjectImp::typestr( imp->type() ) );
  }
  if ( exec.hasParams( 6 ) )
  {
    if ( !getStringParam( 5, strrealm ) )
      return new BError( "Realm not found" );
    realm = find_realm( strrealm->value() );
  }

  if ( !realm )
    return new BError( "Realm not found" );
  if ( !realm->valid( x, y, z ) )
    return new BError( "Invalid Coordinates for Realm" );

  if ( !getParam( 6, forceInt ) )
  {
    forceLocation = false;
  }
  else
  {
    forceLocation = forceInt ? true : false;
  }

  Clib::ConfigElem elem;
  START_PROFILECLOCK( npc_search );
  bool found = FindNpcTemplate( tmplname->data(), elem );
  STOP_PROFILECLOCK( npc_search );
  INC_PROFILEVAR( npc_searches );

  if ( !found )
  {
    return new BError( "NPC template '" + tmplname->value() + "' not found" );
  }
  Plib::MOVEMODE movemode = Character::decode_movemode( elem.read_string( "MoveMode", "L" ) );

  short newz;
  Multi::UMulti* dummy_multi = nullptr;
  Item* dummy_walkon = nullptr;
  if ( !realm->walkheight( x, y, z, &newz, &dummy_multi, &dummy_walkon, true, movemode ) )
  {
    if ( !forceLocation )
      return new BError( "Not a valid location for an NPC!" );
  }
  if ( !forceLocation )
    z = newz;


  NpcRef npc;
  // readProperties can throw, if stuff is missing.
  try
  {
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
    elem.add_prop( "Realm", realm->name() );
    if ( custom_struct != nullptr )
      replace_properties( elem, custom_struct );
    npc->readPropertiesForNewNPC( elem );
    ////HASH
    objStorageManager.objecthash.Insert( npc.get() );
    ////


    // characters.push_back( npc.get() );
    SetCharacterWorldPosition( npc.get(), Realms::WorldChangeReason::NpcCreate );
    WorldIterator<OnlinePlayerFilter>::InVisualRange(
        npc.get(), [&]( Character* zonechr ) { send_char_data( zonechr->client, npc.get() ); } );
    realm->notify_entered( *npc );
    // FIXME: Need to add Walkon checks for multi right here if type is house.
    if ( dummy_multi )
    {
      dummy_multi->register_object( npc.get() );
      Multi::UHouse* this_house = dummy_multi->as_house();
      if ( npc->registered_house == 0 )
      {
        npc->registered_house = dummy_multi->serial;

        if ( this_house != nullptr )
          this_house->walk_on( npc.get() );
      }
    }
    else
    {
      if ( npc->registered_house > 0 )
      {
        Multi::UMulti* multi = system_find_multi( npc->registered_house );
        if ( multi != nullptr )
        {
          multi->unregister_object( npc.get() );
        }
        npc->registered_house = 0;
      }
    }
    return new ECharacterRefObjImp( npc.get() );
  }
  catch ( std::exception& ex )
  {
    if ( npc.get() != nullptr )
      npc->destroy();
    return new BError( "Exception detected trying to create npc from template '" +
                       tmplname->value() + "': " + ex.what() );
  }
}


BObjectImp* UOExecutorModule::mf_SubtractAmount()
{
  Item* item;
  unsigned short amount;

  if ( getItemParam( 0, item ) && getParam( 1, amount, 1, item->itemdesc().stack_limit ) )
  {
    if ( item->has_gotten_by() )
      item->gotten_by()->clear_gotten_item();
    else if ( item->inuse() && !is_reserved_to_me( item ) )
      return new BError( "That item is being used." );
    subtract_amount_from_item( item, amount );
    return new BLong( 1 );
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

  if ( getItemParam( 0, item ) && getParam( 1, amount, 1, MAX_STACK_ITEMS ) )
  {
    if ( item->inuse() && !is_reserved_to_me( item ) )
    {
      return new BError( "That item is being used." );
    }
    if ( ~Plib::tile_flags( item->graphic ) & Plib::FLAG::STACKABLE )
    {
      return new BError( "That item type is not stackable." );
    }
    if ( !item->can_add_to_self( amount, false ) )
    {
      return new BError( "Can't add that much to that stack" );
    }

    unsigned short newamount = item->getamount();
    newamount += amount;
    item->setamount( newamount );
    update_item_to_inrange( item );

    // DAVE added this 12/05: if in a Character's pack, update weight.
    UpdateCharacterWeight( item );

    return new EItemRefObjImp( item );
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
  if ( getCharacterParam( 0, chr ) && getParam( 1, actionval ) && getParam( 2, framecount ) &&
       getParam( 3, repeatcount ) && getParam( 4, backward ) && getParam( 5, repeatflag ) &&
       getParam( 6, delay ) )
  {
    UACTION action = static_cast<UACTION>( actionval );
    send_action_to_inrange(
        chr, action, framecount, repeatcount, static_cast<DIRECTION_FLAG_OLD>( backward ),
        static_cast<REPEAT_FLAG_OLD>( repeatflag ), static_cast<unsigned char>( delay ) );
    return new BLong( 1 );
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
  if ( getUObjectParam( 0, chr ) && getParam( 1, effect ) )
  {
    play_sound_effect( chr, static_cast<u16>( effect ) );
    return new BLong( 1 );
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
  if ( getUObjectParam( 0, center ) && getParam( 1, effect ) && getCharacterParam( 2, forchr ) )
  {
    play_sound_effect_private( center, static_cast<u16>( effect ), forchr );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_PlaySoundEffectXYZ()
{
  Core::Pos4d center;
  int effect;
  if ( getPos4dParam( 0, 1, 2, 4, &center ) && getParam( 3, effect ) )
  {
    play_sound_effect_xyz( center, static_cast<u16>( effect ) );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_PlayMusic( /* char, music_id */ )
{
  Character* chr;
  int musicid;
  if ( getCharacterParam( 0, chr ) && getParam( 1, musicid ) )
  {
    send_midi( chr->client, static_cast<u16>( musicid ) );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

void menu_selection_made( Network::Client* client, MenuItem* mi, PKTIN_7D* msg )
{
  if ( client != nullptr )
  {
    Character* chr = client->chr;
    if ( chr != nullptr && chr->client->gd->menu_selection_uoemod != nullptr )
    {
      auto& uoex = chr->client->gd->menu_selection_uoemod->uoexec();
      if ( mi != nullptr && msg != nullptr )
      {
        BStruct* selection = new BStruct;
        // FIXME should make sure objtype and choice are within valid range.
        selection->addMember( "objtype", new BLong( mi->objtype_ ) );
        selection->addMember( "graphic", new BLong( mi->graphic_ ) );
        selection->addMember( "index",
                              new BLong( cfBEu16( msg->choice ) ) );  // this has been validated
        selection->addMember( "color", new BLong( mi->color_ ) );
        uoex.ValueStack.back().set( new BObject( selection ) );
      }
      // 0 is already on the value stack, for the case of cancellation.
      uoex.revive();
      chr->client->gd->menu_selection_uoemod->menu_selection_chr = nullptr;
      chr->client->gd->menu_selection_uoemod = nullptr;
    }
  }
}

bool UOExecutorModule::getDynamicMenuParam( unsigned param, Menu*& menu )
{
  BApplicObjBase* aob = getApplicObjParam( param, &menu_type );
  if ( aob != nullptr )
  {
    EMenuObjImp* menu_imp = static_cast<EMenuObjImp*>( aob );
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
  if ( imp->isa( BObjectImp::OTString ) )
  {
    String* pmenuname = static_cast<String*>( imp );
    menu = Menu::find_menu( pmenuname->data() );
    return ( menu != nullptr );
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = static_cast<BApplicObjBase*>( imp );
    if ( aob->object_type() == &menu_type )
    {
      EMenuObjImp* menu_imp = static_cast<EMenuObjImp*>( aob );
      menu = &menu_imp->value();
      return true;
    }
  }
  DEBUGLOG << "SelectMenuItem: expected a menu name (static menu) or a CreateMenu() dynamic menu\n";
  return false;
}

BObjectImp* UOExecutorModule::mf_SelectMenuItem2()
{
  Character* chr;
  Menu* menu;

  if ( !getCharacterParam( 0, chr ) || !getStaticOrDynamicMenuParam( 1, menu ) ||
       ( chr->client->gd->menu_selection_uoemod != nullptr ) )
  {
    return new BError( "Invalid parameter" );
  }

  if ( menu == nullptr || !chr->has_active_client() || menu->menuitems_.empty() )
  {
    return new BError( "Client is busy, or menu is empty" );
  }

  if ( !send_menu( chr->client, menu ) )
  {
    return new BError( "Menu too large" );
  }

  if ( !uoexec().suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SelectMenuItem():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  chr->menu = menu->getWeakPtr();
  chr->on_menu_selection = menu_selection_made;
  chr->client->gd->menu_selection_uoemod = this;
  menu_selection_chr = chr;

  return new BLong( 0 );
}

void append_objtypes( ObjArray* objarr, Menu* menu )
{
  for ( unsigned i = 0; i < menu->menuitems_.size(); ++i )
  {
    MenuItem* mi = &menu->menuitems_[i];

    if ( mi->submenu_id )
    {
      // Code Analyze: Commented out and replaced with tmp_menu due to hiding
      // menu passed to function.
      //      Menu* menu = find_menu( mi->submenu_id );
      Menu* tmp_menu = Menu::find_menu( mi->submenu_id );
      if ( tmp_menu != nullptr )
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
  if ( getStaticOrDynamicMenuParam( 0, menu ) )
  {
    std::unique_ptr<ObjArray> objarr( new ObjArray );

    append_objtypes( objarr.get(), menu );

    return objarr.release();
  }
  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_ApplyConstraint()
{
  ObjArray* arr;
  StoredConfigFile* cfile;
  const String* propname_str;
  int amthave;

  if ( getObjArrayParam( 0, arr ) && getStoredConfigFileParam( *this, 1, cfile ) &&
       getStringParam( 2, propname_str ) && getParam( 3, amthave ) )
  {
    std::unique_ptr<ObjArray> newarr( new ObjArray );

    for ( unsigned i = 0; i < arr->ref_arr.size(); i++ )
    {
      BObjectRef& ref = arr->ref_arr[i];

      BObject* bo = ref.get();
      if ( bo == nullptr )
        continue;
      if ( !bo->isa( BObjectImp::OTLong ) )
        continue;
      BLong* blong = static_cast<BLong*>( bo->impptr() );
      unsigned int objtype = static_cast<u32>( blong->value() );

      ref_ptr<StoredConfigElem> celem = cfile->findelem( objtype );
      if ( celem.get() == nullptr )
        continue;

      BObjectImp* propval = celem->getimp( propname_str->value() );
      if ( propval == nullptr )
        continue;
      if ( !propval->isa( BObjectImp::OTLong ) )
        continue;
      BLong* amtreq = static_cast<BLong*>( propval );
      if ( amtreq->value() > amthave )
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
  if ( getStringParam( 0, title ) )
  {
    Menu temp;
    temp.menu_id = 0;
    Clib::stracpy( temp.title, title->data(), sizeof temp.title );
    return new EMenuObjImp( temp );
  }
  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_AddMenuItem()
{
  Menu* menu;
  unsigned int objtype;
  const String* text;
  unsigned short color;

  if ( getDynamicMenuParam( 0, menu ) && getObjtypeParam( 1, objtype ) &&
       getStringParam( 2, text ) && getParam( 3, color ) )
  {
    menu->menuitems_.push_back( MenuItem() );
    MenuItem* mi = &menu->menuitems_.back();
    mi->objtype_ = objtype;
    mi->graphic_ = getgraphic( objtype );
    Clib::stracpy( mi->title, text->data(), sizeof mi->title );
    mi->color_ = color & settingsManager.ssopt.item_color_mask;
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_GetObjProperty()
{
  UObject* uobj;
  const String* propname_str;
  if ( getUObjectParam( 0, uobj ) && getStringParam( 1, propname_str ) )
  {
    std::string val;
    if ( uobj->getprop( propname_str->value(), val ) )
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
  if ( getUObjectParam( 0, uobj ) && getStringParam( 1, propname_str ) )
  {
    BObjectImp* propval = getParamImp( 2 );
    uobj->setprop( propname_str->value(), propval->pack() );
    return new BLong( 1 );
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
  if ( getUObjectParam( 0, uobj ) && getStringParam( 1, propname_str ) )
  {
    uobj->eraseprop( propname_str->value() );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
BObjectImp* UOExecutorModule::mf_GetObjPropertyNames()
{
  UObject* uobj;
  if ( getUObjectParam( 0, uobj ) )
  {
    std::vector<std::string> propnames;
    uobj->getpropnames( propnames );
    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( unsigned i = 0; i < propnames.size(); ++i )
    {
      arr->addElement( new String( propnames[i] ) );
    }
    return arr.release();
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


BObjectImp* UOExecutorModule::mf_GetGlobalProperty()
{
  const String* propname_str;
  if ( getStringParam( 0, propname_str ) )
  {
    std::string val;
    if ( gamestate.global_properties->getprop( propname_str->value(), val ) )
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
  if ( exec.getStringParam( 0, propname_str ) )
  {
    BObjectImp* propval = exec.getParamImp( 1 );
    gamestate.global_properties->setprop( propname_str->value(), propval->pack() );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_EraseGlobalProperty()
{
  const String* propname_str;
  if ( getStringParam( 0, propname_str ) )
  {
    gamestate.global_properties->eraseprop( propname_str->value() );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_GetGlobalPropertyNames()
{
  std::vector<std::string> propnames;
  gamestate.global_properties->getpropnames( propnames );
  std::unique_ptr<ObjArray> arr( new ObjArray );
  for ( unsigned i = 0; i < propnames.size(); ++i )
  {
    arr->addElement( new String( propnames[i] ) );
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
  if ( getUObjectParam( 0, src ) && getUObjectParam( 1, dst ) && getParam( 2, effect ) &&
       getParam( 3, speed, UCHAR_MAX ) && getParam( 4, loop, UCHAR_MAX ) &&
       getParam( 5, explode, UCHAR_MAX ) )
  {
    if ( src->realm() != dst->realm() )
      return new BError( "Realms must match" );
    play_moving_effect( src, dst, effect, static_cast<unsigned char>( speed ),
                        static_cast<unsigned char>( loop ), static_cast<unsigned char>( explode ) );
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_PlayMovingEffectXYZ()
{
  unsigned short effect;
  int speed;
  int loop;
  int explode;
  Realms::Realm* realm;
  Pos3d src, dst;
  if ( getRealmParam( 10, &realm ) && getPos3dParam( 0, 1, 2, &src, realm ) &&
       getPos3dParam( 3, 4, 5, &dst, realm ) && getParam( 6, effect ) &&
       getParam( 7, speed, UCHAR_MAX ) && getParam( 8, loop, UCHAR_MAX ) &&
       getParam( 9, explode, UCHAR_MAX ) )
  {
    play_moving_effect2( src, dst, effect, static_cast<signed char>( speed ),
                         static_cast<signed char>( loop ), static_cast<signed char>( explode ),
                         realm );
    return new BLong( 1 );
  }
  else
  {
    return nullptr;
  }
}

// FIXME add/test:stationary

BObjectImp* UOExecutorModule::mf_PlayObjectCenteredEffect()
{
  UObject* src;
  unsigned short effect;
  int speed;
  int loop;
  if ( getUObjectParam( 0, src ) && getParam( 1, effect ) && getParam( 2, speed, UCHAR_MAX ) &&
       getParam( 3, loop, UCHAR_MAX ) )
  {
    play_object_centered_effect( src, effect, static_cast<unsigned char>( speed ),
                                 static_cast<unsigned char>( loop ) );
    return new BLong( 1 );
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_PlayStationaryEffect()
{
  Pos4d pos;
  unsigned short effect;
  int speed, loop, explode;
  if ( getPos4dParam( 0, 1, 2, 7, &pos ) && getParam( 3, effect ) &&
       getParam( 4, speed, UCHAR_MAX ) && getParam( 5, loop, UCHAR_MAX ) &&
       getParam( 6, explode, UCHAR_MAX ) )
  {
    play_stationary_effect( pos, effect, static_cast<unsigned char>( speed ),
                            static_cast<unsigned char>( loop ),
                            static_cast<unsigned char>( explode ) );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}


BObjectImp* UOExecutorModule::mf_PlayMovingEffectEx()
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

  if ( getUObjectParam( 0, src ) && getUObjectParam( 1, dst ) && getParam( 2, effect ) &&
       getParam( 3, speed, UCHAR_MAX ) && getParam( 4, duration, UCHAR_MAX ) &&
       getParam( 5, hue, INT_MAX ) && getParam( 6, render, INT_MAX ) &&
       getParam( 7, direction, UCHAR_MAX ) && getParam( 8, explode, UCHAR_MAX ) &&
       getParam( 9, effect3d ) && getParam( 10, effect3dexplode ) && getParam( 11, effect3dsound ) )
  {
    if ( src->realm() != dst->realm() )
      return new BError( "Realms must match" );
    play_moving_effect_ex(
        src, dst, effect, static_cast<unsigned char>( speed ),
        static_cast<unsigned char>( duration ), static_cast<unsigned int>( hue ),
        static_cast<unsigned int>( render ), static_cast<unsigned char>( direction ),
        static_cast<unsigned char>( explode ), effect3d, effect3dexplode, effect3dsound );
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_PlayMovingEffectXYZEx()
{
  Pos3d src, dst;
  Realms::Realm* realm;

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

  if ( getRealmParam( 6, &realm ) && getPos3dParam( 0, 1, 2, &src, realm ) &&
       getPos3dParam( 3, 4, 5, &dst, realm ) && getParam( 7, effect ) &&
       getParam( 8, speed, UCHAR_MAX ) && getParam( 9, duration, UCHAR_MAX ) &&
       getParam( 10, hue, INT_MAX ) && getParam( 11, render, INT_MAX ) &&
       getParam( 12, direction, UCHAR_MAX ) && getParam( 13, explode, UCHAR_MAX ) &&
       getParam( 14, effect3d ) && getParam( 15, effect3dexplode ) &&
       getParam( 16, effect3dsound ) )
  {
    play_moving_effect2_ex(
        src, dst, realm, effect, static_cast<unsigned char>( speed ),
        static_cast<unsigned char>( duration ), static_cast<unsigned int>( hue ),
        static_cast<unsigned int>( render ), static_cast<unsigned char>( direction ),
        static_cast<unsigned char>( explode ), effect3d, effect3dexplode, effect3dsound );
    return new BLong( 1 );
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_PlayObjectCenteredEffectEx()
{
  UObject* src;
  unsigned short effect;
  int speed;
  int duration;
  int hue;
  int render;
  int layer;
  unsigned short effect3d;

  if ( getUObjectParam( 0, src ) && getParam( 1, effect ) && getParam( 2, speed, UCHAR_MAX ) &&
       getParam( 3, duration, UCHAR_MAX ) && getParam( 4, hue, INT_MAX ) &&
       getParam( 5, render, INT_MAX ) && getParam( 6, layer, UCHAR_MAX ) &&
       getParam( 7, effect3d ) )
  {
    play_object_centered_effect_ex(
        src, effect, static_cast<unsigned char>( speed ), static_cast<unsigned char>( duration ),
        static_cast<unsigned int>( hue ), static_cast<unsigned int>( render ),
        static_cast<unsigned char>( layer ), effect3d );
    return new BLong( 1 );
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_PlayStationaryEffectEx()
{
  Pos4d pos;
  unsigned short effect;
  int speed;
  int duration;
  int hue;
  int render;
  unsigned short effect3d;

  if ( getPos4dParam( 0, 1, 2, 3, &pos ) && getParam( 4, effect ) &&
       getParam( 5, speed, UCHAR_MAX ) && getParam( 6, duration, UCHAR_MAX ) &&
       getParam( 7, hue, INT_MAX ) && getParam( 8, render, INT_MAX ) && getParam( 9, effect3d ) )
  {
    play_stationary_effect_ex(
        pos, effect, static_cast<unsigned char>( speed ), static_cast<unsigned char>( duration ),
        static_cast<unsigned int>( hue ), static_cast<unsigned int>( render ), effect3d );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_PlayLightningBoltEffect()
{
  UObject* center;
  if ( getUObjectParam( 0, center ) )
  {
    play_lightning_bolt_effect( center );
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_ListItemsNearLocation( /* x, y, z, range, realm */ )
{
  unsigned short x, y;
  int z;
  short range;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getStringParam( 4, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<ItemFilter>::InRange(
        x, y, realm, range,
        [&]( Item* item )
        {
          if ( ( abs( item->x() - x ) <= range ) && ( abs( item->y() - y ) <= range ) )
          {
            if ( ( z == LIST_IGNORE_Z ) || ( abs( item->z() - z ) < CONST_DEFAULT_ZRANGE ) )
              newarr->addElement( item->make_ref() );
          }
        } );

    return newarr.release();
  }

  return nullptr;
}

Range3d UOExecutorModule::internal_InBoxAreaChecks( const Pos2d& p1, int z1, const Pos2d& p2,
                                                    int z2, Realms::Realm* realm )
{
  // no error checks for out of realm range on purpose
  if ( ( z1 > z2 ) && z1 != LIST_IGNORE_Z && z2 != LIST_IGNORE_Z )
    std::swap( z1, z2 );
  if ( z1 == LIST_IGNORE_Z )
    z1 = ZCOORD_MIN;
  if ( z2 == LIST_IGNORE_Z )
    z2 = ZCOORD_MAX;
  return Range3d( Pos3d( p1, Pos3d::clip_s8( z1 ) ), Pos3d( p2, Pos3d::clip_s8( z2 ) ), realm );
}

BObjectImp* UOExecutorModule::mf_ListObjectsInBox( /* x1, y1, z1, x2, y2, z2, realm */ )
{
  int z1, z2;
  Pos2d p1, p2;
  Realms::Realm* realm;

  if ( !( getPos2dParam( 0, 1, &p1 ) && getParam( 2, z1 ) && getPos2dParam( 3, 4, &p2 ) &&
          getParam( 5, z2 ) && getRealmParam( 6, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();

  std::unique_ptr<ObjArray> newarr( new ObjArray );
  WorldIterator<MobileFilter>::InBox( box.range(), realm,
                                      [&]( Mobile::Character* chr )
                                      {
                                        if ( chr->z() >= z1 && chr->z() <= z2 )
                                        {
                                          newarr->addElement( chr->make_ref() );
                                        }
                                      } );
  WorldIterator<ItemFilter>::InBox( box.range(), realm,
                                    [&]( Items::Item* item )
                                    {
                                      if ( item->z() >= z1 && item->z() <= z2 )
                                      {
                                        newarr->addElement( item->make_ref() );
                                      }
                                    } );

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListItemsInBoxOfObjType(
    /* objtype, x1, y1, z1, x2, y2, z2, realm */ )
{
  unsigned int objtype;
  int z1, z2;
  Pos2d p1, p2;
  Realms::Realm* realm;

  if ( !( getParam( 0, objtype ) && getPos2dParam( 1, 2, &p1 ) && getParam( 3, z1 ) &&
          getPos2dParam( 4, 5, &p2 ) && getParam( 6, z2 ) && getRealmParam( 7, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();

  std::unique_ptr<ObjArray> newarr( new ObjArray );
  WorldIterator<ItemFilter>::InBox(
      box.range(), realm,
      [&]( Items::Item* item )
      {
        if ( item->z() >= z1 && item->z() <= z2 && item->objtype_ == objtype )
        {
          newarr->addElement( item->make_ref() );
        }
      } );

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListObjectsInBoxOfClass(
    /* POL_Class, x1, y1, z1, x2, y2, z2, realm */ )
{
  unsigned int POL_Class;
  int z1, z2;
  Pos2d p1, p2;
  Realms::Realm* realm;

  if ( !( getParam( 0, POL_Class ) && getPos2dParam( 1, 2, &p1 ) && getParam( 3, z1 ) &&
          getPos2dParam( 4, 5, &p2 ) && getParam( 6, z2 ) && getRealmParam( 7, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();

  std::unique_ptr<ObjArray> newarr( new ObjArray );
  WorldIterator<MobileFilter>::InBox(
      box.range(), realm,
      [&]( Mobile::Character* chr )
      {
        if ( chr->z() >= z1 && chr->z() <= z2 && chr->script_isa( POL_Class ) )
        {
          newarr->addElement( chr->make_ref() );
        }
      } );
  WorldIterator<ItemFilter>::InBox(
      box.range(), realm,
      [&]( Items::Item* item )
      {
        if ( item->z() >= z1 && item->z() <= z2 && item->script_isa( POL_Class ) )
        {
          newarr->addElement( item->make_ref() );
        }
      } );

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListMobilesInBox( /* x1, y1, z1, x2, y2, z2, realm */ )
{
  int z1, z2;
  Pos2d p1, p2;
  Realms::Realm* realm;

  if ( !( getPos2dParam( 0, 1, &p1 ) && getParam( 2, z1 ) && getPos2dParam( 3, 4, &p2 ) &&
          getParam( 5, z2 ) && getRealmParam( 6, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();
  std::unique_ptr<ObjArray> newarr( new ObjArray );
  WorldIterator<MobileFilter>::InBox( box.range(), realm,
                                      [&]( Mobile::Character* chr )
                                      {
                                        if ( chr->z() >= z1 && chr->z() <= z2 )
                                        {
                                          newarr->addElement( chr->make_ref() );
                                        }
                                      } );

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListMultisInBox( /* x1, y1, z1, x2, y2, z2, realm */ )
{
  int z1, z2;
  Pos2d p1, p2;
  Realms::Realm* realm;

  if ( !( getPos2dParam( 0, 1, &p1 ) && getParam( 2, z1 ) && getPos2dParam( 3, 4, &p2 ) &&
          getParam( 5, z2 ) && getRealmParam( 6, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }

  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();

  std::unique_ptr<ObjArray> newarr( new ObjArray );

  // extend the coords to find the center item
  // but only as parameter for the filter function
  Range2d boxrange( box.nw() - gamestate.update_range, box.se() + gamestate.update_range, realm );

  // search for multis.  this is tricky, since the center might lie outside the box
  WorldIterator<MultiFilter>::InBox(
      boxrange, realm,
      [&]( Multi::UMulti* multi )
      {
        if ( !box.intersect( multi->current_box() ) )
          return;
        const Multi::MultiDef& md = multi->multidef();
        // some part of it is contained in the box.  Look at the
        // individual statics, to see if any of them lie within.
        for ( const auto& citr : md.components )
        {
          const Multi::MULTI_ELEM* elem = citr.second;
          Pos3d absp = multi->pos3d() + elem->relpos;
          if ( box.contains( absp.xy() ) )
          {
            // do Z checking
            int height = Plib::tileheight( getgraphic( elem->objtype ) );
            int top = absp.z() + height;

            if ( ( z1 <= absp.z() && absp.z() <= z2 ) ||  // bottom point lies between
                 ( z1 <= top && top <= z2 ) ||            // top lies between
                 ( top >= z2 && absp.z() <= z1 ) )        // spans
            {
              newarr->addElement( multi->make_ref() );
              break;  // out of for
            }
          }
        }
      } );

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListStaticsInBox( /* x1, y1, z1, x2, y2, z2, flags, realm */ )
{
  int z1, z2;
  Pos2d p1, p2;
  int flags;
  Realms::Realm* realm;

  if ( !( getPos2dParam( 0, 1, &p1 ) && getParam( 2, z1 ) && getPos2dParam( 3, 4, &p2 ) &&
          getParam( 5, z2 ) && getParam( 6, flags ) && getRealmParam( 7, &realm ) ) )
  {
    return new BError( "Invalid parameter" );
  }

  Range3d box = internal_InBoxAreaChecks( p1, z1, p2, z2, realm );
  z1 = box.nw_b().z();
  z2 = box.se_t().z();

  std::unique_ptr<ObjArray> newarr( new ObjArray );

  for ( const auto& pos : box.range() )
  {
    if ( !( flags & ITEMS_IGNORE_STATICS ) )
    {
      Plib::StaticEntryList slist;
      realm->getstatics( slist, pos );

      for ( unsigned i = 0; i < slist.size(); ++i )
      {
        if ( ( z1 <= slist[i].z ) && ( slist[i].z <= z2 ) )
        {
          std::unique_ptr<BStruct> arr( new BStruct );
          arr->addMember( "x", new BLong( pos.x() ) );
          arr->addMember( "y", new BLong( pos.y() ) );
          arr->addMember( "z", new BLong( slist[i].z ) );
          arr->addMember( "objtype", new BLong( slist[i].objtype ) );
          arr->addMember( "hue", new BLong( slist[i].hue ) );
          newarr->addElement( arr.release() );
        }
      }
    }

    if ( !( flags & ITEMS_IGNORE_MULTIS ) )
    {
      Plib::StaticList mlist;
      realm->readmultis( mlist, pos );

      for ( unsigned i = 0; i < mlist.size(); ++i )
      {
        if ( ( z1 <= mlist[i].z ) && ( mlist[i].z <= z2 ) )
        {
          std::unique_ptr<BStruct> arr( new BStruct );
          arr->addMember( "x", new BLong( pos.x() ) );
          arr->addMember( "y", new BLong( pos.y() ) );
          arr->addMember( "z", new BLong( mlist[i].z ) );
          arr->addMember( "objtype", new BLong( mlist[i].graphic ) );
          newarr->addElement( arr.release() );
        }
      }
    }
  }
  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_ListItemsNearLocationOfType( /* x, y, z, range, objtype, realm */ )
{
  unsigned short x, y;
  int z, range;
  unsigned int objtype;
  const String* strrealm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getObjtypeParam( 4, objtype ) && getStringParam( 5, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    std::unique_ptr<ObjArray> newarr( new ObjArray );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    WorldIterator<ItemFilter>::InRange(
        x, y, realm, range,
        [&]( Items::Item* item )
        {
          if ( ( item->objtype_ == objtype ) && ( abs( item->x() - x ) <= range ) &&
               ( abs( item->y() - y ) <= range ) )
          {
            if ( ( z == LIST_IGNORE_Z ) || ( abs( item->z() - z ) < CONST_DEFAULT_ZRANGE ) )
              newarr->addElement( item->make_ref() );
          }
        } );

    return newarr.release();
  }

  return nullptr;
}


BObjectImp* UOExecutorModule::mf_ListItemsAtLocation( /* x, y, z, realm */ )
{
  unsigned short x, y;
  int z;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getStringParam( 3, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<ItemFilter>::InRange( x, y, realm, 0,
                                        [&]( Items::Item* item )
                                        {
                                          if ( ( item->x() == x ) && ( item->y() == y ) )
                                          {
                                            if ( ( z == LIST_IGNORE_Z ) || ( item->z() == z ) )
                                              newarr->addElement( item->make_ref() );
                                          }
                                        } );

    return newarr.release();
  }

  return nullptr;
}

BObjectImp* UOExecutorModule::mf_ListGhostsNearLocation()
{
  u16 x;
  u16 y;
  int z;
  int range;
  const String* strrealm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getStringParam( 4, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<PlayerFilter>::InRange(
        x, y, realm, range,
        [&]( Mobile::Character* chr )
        {
          if ( chr->dead() && ( abs( chr->z() - z ) < CONST_DEFAULT_ZRANGE ) )
          {
            newarr->addElement( chr->make_ref() );
          }
        } );

    return newarr.release();
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

const unsigned LMBLEX_FLAG_NORMAL = 0x01;
const unsigned LMBLEX_FLAG_HIDDEN = 0x02;
const unsigned LMBLEX_FLAG_DEAD = 0x04;
const unsigned LMBLEX_FLAG_CONCEALED = 0x8;
const unsigned LMBLEX_FLAG_PLAYERS_ONLY = 0x10;
const unsigned LMBLEX_FLAG_NPC_ONLY = 0x20;

BObjectImp* UOExecutorModule::mf_ListMobilesNearLocationEx( /* x, y, z, range, flags, realm */ )
{
  unsigned short x, y;
  int z, flags;
  short range;
  const String* strrealm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getParam( 4, flags ) && getStringParam( 5, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    bool inc_normal = ( flags & LMBLEX_FLAG_NORMAL ) ? true : false;
    bool inc_hidden = ( flags & LMBLEX_FLAG_HIDDEN ) ? true : false;
    bool inc_dead = ( flags & LMBLEX_FLAG_DEAD ) ? true : false;
    bool inc_concealed = ( flags & LMBLEX_FLAG_CONCEALED ) ? true : false;
    bool inc_players_only = ( flags & LMBLEX_FLAG_PLAYERS_ONLY ) ? true : false;
    bool inc_npc_only = ( flags & LMBLEX_FLAG_NPC_ONLY ) ? true : false;

    std::unique_ptr<ObjArray> newarr( new ObjArray );

    auto fill_mobs = [&]( Mobile::Character* _chr )
    {
      if ( ( inc_hidden && _chr->hidden() ) || ( inc_dead && _chr->dead() ) ||
           ( inc_concealed && _chr->concealed() ) ||
           ( inc_normal && !( _chr->hidden() || _chr->dead() || _chr->concealed() ) ) )
      {
        if ( ( z == LIST_IGNORE_Z ) || ( abs( _chr->z() - z ) < CONST_DEFAULT_ZRANGE ) )
        {
          if ( !inc_players_only && !inc_npc_only )
            newarr->addElement( _chr->make_ref() );
          else if ( inc_players_only && _chr->client )
            newarr->addElement( _chr->make_ref() );
          else if ( inc_npc_only && _chr->isa( UOBJ_CLASS::CLASS_NPC ) )
            newarr->addElement( _chr->make_ref() );
        }
      }
    };
    if ( inc_players_only )
      WorldIterator<PlayerFilter>::InRange( x, y, realm, range, fill_mobs );
    else if ( inc_npc_only )
      WorldIterator<NPCFilter>::InRange( x, y, realm, range, fill_mobs );
    else
      WorldIterator<MobileFilter>::InRange( x, y, realm, range, fill_mobs );

    return newarr.release();
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_ListMobilesNearLocation( /* x, y, z, range, realm */ )
{
  unsigned short x, y;
  int z;
  short range;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getStringParam( 4, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<MobileFilter>::InRange(
        x, y, realm, range,
        [&]( Mobile::Character* chr )
        {
          if ( ( !chr->concealed() ) && ( !chr->hidden() ) && ( !chr->dead() ) )
            if ( ( z == LIST_IGNORE_Z ) || ( abs( chr->z() - z ) < CONST_DEFAULT_ZRANGE ) )
              newarr->addElement( chr->make_ref() );
        } );
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
  if ( getUObjectParam( 0, obj ) && getParam( 1, range ) )
  {
    obj = obj->toplevel_owner();
    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<MobileFilter>::InRange(
        obj->x(), obj->y(), obj->realm(), range,
        [&]( Mobile::Character* chr )
        {
          if ( chr->dead() || chr->hidden() || chr->concealed() )
            return;
          if ( chr == obj )
            return;
          if ( ( abs( chr->z() - obj->z() ) < CONST_DEFAULT_ZRANGE ) )
          {
            if ( obj->realm()->has_los( *obj, *chr ) )
            {
              newarr->addElement( chr->make_ref() );
            }
          }
        } );

    return newarr.release();
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_ListOfflineMobilesInRealm( /*realm*/ )
{
  const String* strrealm = nullptr;
  Realms::Realm* realm = nullptr;

  if ( getStringParam( 0, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    std::unique_ptr<ObjArray> newarr( new ObjArray() );

    for ( const auto& objitr : Pol::Core::objStorageManager.objecthash )
    {
      UObject* obj = objitr.second.get();
      if ( !obj->ismobile() || obj->isa( UOBJ_CLASS::CLASS_NPC ) )
        continue;

      Character* chr = static_cast<Character*>( obj );
      if ( chr->logged_in() || chr->realm() != realm || chr->orphan() )
        continue;

      newarr->addElement( new EOfflineCharacterRefObjImp( chr ) );
    }
    return newarr.release();
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

// keep this in sync with UO.EM
const int LH_FLAG_LOS = 1;             // only include those in LOS
const int LH_FLAG_INCLUDE_HIDDEN = 2;  // include hidden characters
BObjectImp* UOExecutorModule::mf_ListHostiles()
{
  Character* chr;
  int range;
  int flags;
  if ( getCharacterParam( 0, chr ) && getParam( 1, range ) && getParam( 2, flags ) )
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );

    const Character::CharacterSet& cont = chr->hostiles();
    Character::CharacterSet::const_iterator itr = cont.begin(), end = cont.end();
    for ( ; itr != end; ++itr )
    {
      Character* hostile = *itr;
      if ( hostile->concealed() )
        continue;
      if ( ( flags & LH_FLAG_LOS ) && !chr->realm()->has_los( *chr, *hostile ) )
        continue;
      if ( ( ~flags & LH_FLAG_INCLUDE_HIDDEN ) && hostile->hidden() )
        continue;
      if ( !inrangex( chr, hostile, range ) )
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
  if ( getUObjectParam( 0, src ) && getUObjectParam( 1, dst ) )
  {
    return new BLong( src->realm()->has_los( *src, *dst->toplevel_owner() ) );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_CheckLosAt()
{
  UObject* src;
  unsigned short x, y;
  short z;
  if ( getUObjectParam( 0, src ) && getParam( 1, x ) && getParam( 2, y ) && getParam( 3, z ) )
  {
    if ( !src->realm()->valid( x, y, z ) )
      return new BError( "Invalid Coordinates for realm" );
    LosObj tgt( x, y, static_cast<s8>( z ), src->realm() );
    return new BLong( src->realm()->has_los( *src, tgt ) );
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_CheckLosBetween()
{
  unsigned short x1, x2;
  unsigned short y1, y2;
  short z1, z2;
  const String* strrealm;
  if ( getParam( 0, x1 ) && getParam( 1, y1 ) && getParam( 2, z1 ) && getParam( 3, x2 ) &&
       getParam( 4, y2 ) && getParam( 5, z2 ) && getStringParam( 6, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( ( !realm->valid( x1, y1, z1 ) ) || ( !realm->valid( x2, y2, z2 ) ) )
      return new BError( "Invalid Coordinates for Realm" );

    LosObj att( x1, y1, static_cast<s8>( z1 ), realm );
    LosObj tgt( x2, y2, static_cast<s8>( z2 ), realm );
    return new BLong( realm->has_los( att, tgt ) );
  }
  else
  {
    return nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_DestroyItem()
{
  Item* item;
  if ( getItemParam( 0, item ) )
  {
    if ( item->has_gotten_by() )
      item->gotten_by()->clear_gotten_item();
    else if ( item->inuse() && !is_reserved_to_me( item ) )
      return new BError( "That item is being used." );
    else if ( item->script_isa( POLCLASS_MULTI ) )
      return new BError( "That item is a multi. Use uo::DestroyMulti instead." );

    const ItemDesc& id = find_itemdesc( item->objtype_ );
    if ( !id.destroy_script.empty() )
    {
      BObjectImp* res = run_script_to_completion( id.destroy_script, new EItemRefObjImp( item ) );
      if ( res->isTrue() )
      {  // destruction is okay
        BObject ob( res );
      }
      else
      {
        // destruction isn't okay!
        return res;
      }
    }
    UpdateCharacterOnDestroyItem( item );
    UpdateCharacterWeight( item );
    destroy_item( item );
    return new BLong( 1 );
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

  if ( getUObjectParam( 0, obj ) && getStringParam( 1, name_str ) )
  {
    obj->setname( name_str->value() );
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_GetPosition()
{
  UObject* obj;
  if ( getUObjectParam( 0, obj ) )
  {
    std::unique_ptr<BStruct> arr( new BStruct );
    arr->addMember( "x", new BLong( obj->x() ) );
    arr->addMember( "y", new BLong( obj->y() ) );
    arr->addMember( "z", new BLong( obj->z() ) );
    return arr.release();
  }
  else
  {
    return new BLong( 0 );
  }
}

BObjectImp* UOExecutorModule::mf_EnumerateItemsInContainer()
{
  Item* item;
  if ( getItemParam( 0, item ) )
  {
    int flags = 0;
    if ( exec.fparams.size() >= 2 )
    {
      if ( !getParam( 1, flags ) )
        return new BError( "Invalid parameter type" );
    }

    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      std::unique_ptr<ObjArray> newarr( new ObjArray );

      static_cast<UContainer*>( item )->enumerate_contents( newarr.get(), flags );

      return newarr.release();
    }

    return nullptr;
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_EnumerateOnlineCharacters()
{
  std::unique_ptr<ObjArray> newarr( new ObjArray );

  for ( Clients::const_iterator itr = networkManager.clients.begin(),
                                end = networkManager.clients.end();
        itr != end; ++itr )
  {
    if ( ( *itr )->chr != nullptr )
    {
      newarr->addElement( new ECharacterRefObjImp( ( *itr )->chr ) );
    }
  }

  return newarr.release();
}

BObjectImp* UOExecutorModule::mf_RegisterForSpeechEvents()
{
  UObject* center;
  int range;
  if ( getUObjectParam( 0, center ) && getParam( 1, range ) )
  {
    int flags = 0;
    if ( exec.hasParams( 3 ) )
    {
      if ( !getParam( 2, flags ) )
        return new BError( "Invalid parameter type" );
    }
    if ( !registered_for_speech_events )
    {
      ListenPoint::register_for_speech_events( center, &uoexec(), range, flags );
      registered_for_speech_events = true;
      return new BLong( 1 );
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
  if ( getParam( 0, eventmask ) )
  {
    auto& uoex = uoexec();
    if ( eventmask & ( EVID_ENTEREDAREA | EVID_LEFTAREA | EVID_SPOKE ) )
    {
      unsigned short range;
      if ( getParam( 1, range, 0, 32 ) )
      {
        if ( eventmask & ( EVID_SPOKE ) )
          uoex.speech_size = range;
        if ( eventmask & ( EVID_ENTEREDAREA | EVID_LEFTAREA ) )
          uoex.area_size = range;
      }
      else
      {
        return nullptr;
      }


      unsigned short evopts = 0;
      if ( exec.hasParams( 2 ) && getParam( 2, evopts ) )
      {
        if ( eventmask & ( EVID_ENTEREDAREA | EVID_LEFTAREA ) )
          uoex.area_mask = static_cast<unsigned char>( evopts & 255 );
      }
    }
    uoex.eventmask |= eventmask;
    return new BLong( uoex.eventmask );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_DisableEvents()
{
  int eventmask;
  if ( getParam( 0, eventmask ) )
  {
    auto& uoex = uoexec();
    uoex.eventmask &= ~eventmask;

    return new BLong( uoex.eventmask );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_Resurrect()
{
  Character* chr;
  if ( getCharacterParam( 0, chr ) )
  {
    if ( !chr->dead() )
      return new BError( "That is not dead" );
    int flags = 0;
    if ( exec.hasParams( 2 ) )
    {
      if ( !getParam( 1, flags ) )
        return new BError( "Invalid parameter type" );
    }

    if ( ~flags & RESURRECT_FORCELOCATION )
    {
      // we want doors to block ghosts in this case.
      bool doors_block = !( chr->graphic == UOBJ_GAMEMASTER ||
                            chr->cached_settings.get( PRIV_FLAGS::IGNORE_DOORS ) );
      short newz;
      Multi::UMulti* supporting_multi;
      Item* walkon_item;
      if ( !chr->realm()->walkheight( chr->x(), chr->y(), chr->z(), &newz, &supporting_multi,
                                      &walkon_item, doors_block, chr->movemode ) )
      {
        return new BError( "That location is blocked" );
      }
    }

    chr->resurrect();
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_SystemFindObjectBySerial()
{
  int serial;
  if ( getParam( 0, serial ) )
  {
    int sysfind_flags = 0;
    if ( exec.hasParams( 2 ) )
    {
      if ( !getParam( 1, sysfind_flags ) )
        return new BError( "Invalid parameter type" );
    }
    if ( IsCharacter( serial ) )
    {
      Character* chr = system_find_mobile( serial );
      if ( chr != nullptr )
      {
        if ( sysfind_flags & SYSFIND_SEARCH_OFFLINE_MOBILES )
          return new EOfflineCharacterRefObjImp( chr );
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
      // dave changed this 3/8/3: objecthash (via system_find_item) will find any kind of item, so
      // don't need system_find_multi here.
      Item* item = system_find_item( serial );

      if ( item != nullptr )
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

BObjectImp* UOExecutorModule::mf_SaveWorldState()
{
  update_gameclock();
  int flags = 0;
  if ( exec.hasParams( 1 ) )
  {
    if ( !getParam( 0, flags ) )
      return new BError( "Invalid parameter type" );
  }
  try
  {
    cancel_all_trades();

    PolClockPauser pauser;

    unsigned int dirty, clean;
    long long elapsed_ms;
    int res;
    if ( flags & SAVE_INCREMENTAL )
    {
      res = save_incremental( dirty, clean, elapsed_ms );
    }
    else
    {
      res = write_data( dirty, clean, elapsed_ms );
    }
    if ( res == 0 )
    {
      // Code Analyze: C6246
      //      BStruct* res = new BStruct();
      BStruct* ret = new BStruct();
      ret->addMember( "DirtyObjects", new BLong( dirty ) );
      ret->addMember( "CleanObjects", new BLong( clean ) );
      ret->addMember( "ElapsedMilliseconds", new BLong( static_cast<int>( elapsed_ms ) ) );
      return ret;
    }
    else
    {
      return new BError( "pol.cfg has InhibitSaves=1" );
    }
  }
  catch ( std::exception& ex )
  {
    POLLOG << "Exception during world save! (" << ex.what() << ")\n";
    return new BError( "Exception during world save" );
  }
}


BObjectImp* UOExecutorModule::mf_SetRegionLightLevel()
{
  const String* region_name_str;
  int lightlevel;
  if ( !( getStringParam( 0, region_name_str ) && getParam( 1, lightlevel ) ) )
  {
    return new BError( "Invalid Parameter type" );
  }

  if ( !VALID_LIGHTLEVEL( lightlevel ) )
  {
    return new BError( "Light Level is out of range" );
  }

  LightRegion* lightregion = gamestate.lightdef->getregion( region_name_str->value() );
  if ( lightregion == nullptr )
  {
    return new BError( "Light region not found" );
  }

  SetRegionLightLevel( lightregion, lightlevel );
  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_SetRegionWeatherLevel()
{
  const String* region_name_str;
  int type;
  int severity;
  int aux;
  int lightoverride;
  if ( !( getStringParam( 0, region_name_str ) && getParam( 1, type ) && getParam( 2, severity ) &&
          getParam( 3, aux ) && getParam( 4, lightoverride ) ) )
  {
    return new BError( "Invalid Parameter type" );
  }

  WeatherRegion* weatherregion = gamestate.weatherdef->getregion( region_name_str->value() );
  if ( weatherregion == nullptr )
  {
    return new BError( "Weather region not found" );
  }

  SetRegionWeatherLevel( weatherregion, type, severity, aux, lightoverride );

  return new BLong( 1 );
}
BObjectImp* UOExecutorModule::mf_AssignRectToWeatherRegion()
{
  const String* region_name_str;
  Pos2d nw, se;
  Realms::Realm* realm;

  if ( !( getStringParam( 0, region_name_str ) && getRealmParam( 5, &realm ) &&
          getPos2dParam( 1, 2, &nw, realm ) && getPos2dParam( 3, 4, &se, realm ) ) )
  {
    return new BError( "Invalid Parameter type" );
  }

  bool res = gamestate.weatherdef->assign_zones_to_region( region_name_str->data(),
                                                           Range2d( nw, se, nullptr ), realm );
  if ( res )
    return new BLong( 1 );
  else
    return new BError( "Weather region not found" );
}

BObjectImp* UOExecutorModule::mf_Distance()
{
  UObject* obj1;
  UObject* obj2;
  if ( getUObjectParam( 0, obj1 ) && getUObjectParam( 1, obj2 ) )
  {
    const UObject* tobj1 = obj1->toplevel_owner();
    const UObject* tobj2 = obj2->toplevel_owner();
    int xd = tobj1->x() - tobj2->x();
    int yd = tobj1->y() - tobj2->y();
    if ( xd < 0 )
      xd = -xd;
    if ( yd < 0 )
      yd = -yd;
    if ( xd > yd )
      return new BLong( xd );
    else
      return new BLong( yd );
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
  if ( getUObjectParam( 0, obj1 ) && getUObjectParam( 1, obj2 ) )
  {
    const UObject* tobj1 = obj1->toplevel_owner();
    const UObject* tobj2 = obj2->toplevel_owner();
    return new Double( sqrt( pow( (double)( tobj1->x() - tobj2->x() ), 2 ) +
                             pow( (double)( tobj1->y() - tobj2->y() ), 2 ) ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_CoordinateDistance()
{
  unsigned short x1, y1, x2, y2;
  if ( !( getParam( 0, x1 ) && getParam( 1, y1 ) && getParam( 2, x2 ) && getParam( 3, y2 ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  return new BLong( pol_distance( x1, y1, x2, y2 ) );
}

BObjectImp* UOExecutorModule::mf_CoordinateDistanceEuclidean()
{
  unsigned short x1, y1, x2, y2;
  if ( !( getParam( 0, x1 ) && getParam( 1, y1 ) && getParam( 2, x2 ) && getParam( 3, y2 ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  return new Double( sqrt( pow( (double)( x1 - x2 ), 2 ) + pow( (double)( y1 - y2 ), 2 ) ) );
}

BObjectImp* UOExecutorModule::mf_GetCoordsInLine()
{
  int x1, y1, x2, y2;
  if ( !( getParam( 0, x1 ) && getParam( 1, y1 ) && getParam( 2, x2 ) && getParam( 3, y2 ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  else if ( x1 == x2 && y1 == y2 )
  {  // Same exact coordinates ... just give them the coordinate back!
    ObjArray* coords = new ObjArray;
    BStruct* point = new BStruct;
    point->addMember( "x", new BLong( x1 ) );
    point->addMember( "y", new BLong( y1 ) );
    coords->addElement( point );
    return coords;
  }

  double dx = abs( x2 - x1 ) + 0.5;
  double dy = abs( y2 - y1 ) + 0.5;
  int vx = 0, vy = 0;

  if ( x2 > x1 )
    vx = 1;
  else if ( x2 < x1 )
    vx = -1;
  if ( y2 > y1 )
    vy = 1;
  else if ( y2 < y1 )
    vy = -1;

  std::unique_ptr<ObjArray> coords( new ObjArray );
  if ( dx >= dy )
  {
    dy = dy / dx;

    for ( int c = 0; c <= dx; c++ )
    {
      int point_x = x1 + ( c * vx );

      double float_y = double( c ) * double( vy ) * dy;
      if ( float_y - floor( float_y ) >= 0.5 )
        float_y = ceil( float_y );
      int point_y = int( float_y ) + y1;

      std::unique_ptr<BStruct> point( new BStruct );
      point->addMember( "x", new BLong( point_x ) );
      point->addMember( "y", new BLong( point_y ) );
      coords->addElement( point.release() );
    }
  }
  else
  {
    dx = dx / dy;
    for ( int c = 0; c <= dy; c++ )
    {
      int point_y = y1 + ( c * vy );

      double float_x = double( c ) * double( vx ) * dx;
      if ( float_x - floor( float_x ) >= 0.5 )
        float_x = ceil( float_x );
      int point_x = int( float_x ) + x1;

      std::unique_ptr<BStruct> point( new BStruct );
      point->addMember( "x", new BLong( point_x ) );
      point->addMember( "y", new BLong( point_y ) );
      coords->addElement( point.release() );
    }
  }
  return coords.release();
}

BObjectImp* UOExecutorModule::mf_GetFacing()
{
  unsigned short from_x, from_y, to_x, to_y;
  if ( !( getParam( 0, from_x ) && getParam( 1, from_y ) && getParam( 2, to_x ) &&
          getParam( 3, to_y ) ) )
  {
    return new BError( "Invalid parameter type" );
  }

  double x = to_x - from_x;
  double y = to_y - from_y;
  double pi = acos( double( -1 ) );
  double r = sqrt( x * x + y * y );

  double angle = ( ( acos( x / r ) * 180.0 ) / pi );

  double y_check = ( ( asin( y / r ) * 180.0 ) / pi );
  if ( y_check < 0 )
  {
    angle = 360 - angle;
  }

  unsigned short facing = ( ( short( angle / 40 ) + 10 ) % 8 );

  return new BLong( facing );
}

// FIXME : Should we do an Orphan check here as well? Ugh.
void true_extricate( Item* item )
{
  send_remove_object_to_inrange( item );
  if ( item->container != nullptr )
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
  if ( !( getItemParam( 0, item ) && getItemParam( 1, cont_item ) && getParam( 2, px, -1, 65535 ) &&
          getParam( 3, py, -1, 65535 ) && getParam( 4, add_to_existing_stack, 0, 2 ) ) )
  {
    return new BError( "Invalid parameter type" );
  }

  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  if ( !item->movable() )
  {
    Character* chr = controller_.get();
    if ( chr == nullptr || !chr->can_move( item ) )
      return new BError( "That is immobile" );
  }
  if ( item->inuse() && !is_reserved_to_me( item ) )
  {
    return new BError( "That item is being used." );
  }


  if ( !cont_item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    return new BError( "Non-container selected as target" );
  }
  UContainer* cont = static_cast<UContainer*>( cont_item );

  if ( cont->serial == item->serial )
  {
    return new BError( "Can't put a container into itself" );
  }
  if ( is_a_parent( cont, item->serial ) )
  {
    return new BError( "Can't put a container into an item in itself" );
  }
  if ( !cont->can_add( *item ) )
  {
    return new BError( "Container is too full to add that" );
  }
  // DAVE added this 12/04, call can/onInsert & can/onRemove scripts for this container
  Character* chr_owner = cont->GetCharacterOwner();
  if ( chr_owner == nullptr )
    if ( controller_.get() != nullptr )
      chr_owner = controller_.get();

  // daved changed order 1/26/3 check canX scripts before onX scripts.
  UContainer* oldcont = item->container;
  Item* existing_stack = nullptr;

  if ( ( oldcont != nullptr ) &&
       ( !oldcont->check_can_remove_script( chr_owner, item, UContainer::MT_CORE_MOVED ) ) )
    return new BError( "Could not remove item from its container." );
  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return new BError( "Item was destroyed in CanRemove script" );
  }

  if ( add_to_existing_stack )
  {
    existing_stack = cont->find_addable_stack( item );
    if ( existing_stack != nullptr )
    {
      if ( !cont->can_insert_increase_stack( chr_owner, UContainer::MT_CORE_MOVED, existing_stack,
                                             item->getamount(), item ) )
        return new BError( "Could not add to existing stack" );
    }
    else if ( add_to_existing_stack == 2 )
      add_to_existing_stack = 0;
    else
      return new BError( "There is no existing stack" );
  }

  if ( !add_to_existing_stack )
    if ( !cont->can_insert_add_item( chr_owner, UContainer::MT_CORE_MOVED, item ) )
      return new BError( "Could not insert item into container." );

  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return new BError( "Item was destroyed in CanInsert Script" );
  }

  if ( !item->check_unequiptest_scripts() || !item->check_unequip_script() )
    return new BError( "Item cannot be unequipped" );
  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return new BError( "Item was destroyed in Equip Script" );
  }

  if ( oldcont != nullptr )
  {
    oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in OnRemove script" );
    }
  }

  if ( !add_to_existing_stack )
  {
    u8 slotIndex = item->slot_index();
    if ( !cont->can_add_to_slot( slotIndex ) )
    {
      item->destroy();
      return new BError( "No slots available in new container" );
    }
    if ( !item->slot_index( slotIndex ) )
    {
      item->destroy();
      return new BError( "Couldn't set slot index on item" );
    }

    Core::Pos2d cntpos;
    if ( px < 0 || py < 0 )
      cntpos = cont->get_random_location();
    else
    {
      cntpos.x( static_cast<u16>( px ) ).y( static_cast<u16>( py ) );
      if ( !cont->is_legal_posn( cntpos ) )
        cntpos = cont->get_random_location();
    }

    true_extricate( item );

    item->setposition( Core::Pos4d( cntpos, 0, cont->realm() ) );  // TODO POS realm

    cont->add( item );
    update_item_to_inrange( item );
    // DAVE added this 11/17: if in a Character's pack, update weight.
    UpdateCharacterWeight( item );

    cont->on_insert_add_item( chr_owner, UContainer::MT_CORE_MOVED, item );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in OnInsert script" );
    }
  }
  else
  {
    u16 amount = item->getamount();
    true_extricate( item );
    existing_stack->add_to_self( item );
    update_item_to_inrange( existing_stack );
    UpdateCharacterWeight( existing_stack );

    cont->on_insert_increase_stack( chr_owner, UContainer::MT_CORE_MOVED, existing_stack, amount );
  }

  return new BLong( 1 );
}


BObjectImp* UOExecutorModule::mf_MoveItemToSecureTradeWin()
{
  Item* item;
  Character* chr;
  if ( !( getItemParam( 0, item ) && getCharacterParam( 1, chr ) ) )
  {
    return new BError( "Invalid parameter type" );
  }

  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  if ( !item->movable() )
  {
    Character* _chr = controller_.get();
    if ( _chr == nullptr || !_chr->can_move( item ) )
      return new BError( "That is immobile" );
  }
  if ( item->inuse() && !is_reserved_to_me( item ) )
  {
    return new BError( "That item is being used." );
  }

  // daved changed order 1/26/3 check canX scripts before onX scripts.
  UContainer* oldcont = item->container;

  // DAVE added this 12/04, call can/onInsert & can/onRemove scripts for this container
  Character* chr_owner = nullptr;
  if ( oldcont != nullptr )
    chr_owner = oldcont->GetCharacterOwner();
  if ( chr_owner == nullptr )
    if ( controller_.get() != nullptr )
      chr_owner = controller_.get();

  if ( ( oldcont != nullptr ) &&
       ( !oldcont->check_can_remove_script( chr_owner, item, UContainer::MT_CORE_MOVED ) ) )
    return new BError( "Could not remove item from its container." );
  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return new BError( "Item was destroyed in CanRemove script" );
  }

  if ( !item->check_unequiptest_scripts() || !item->check_unequip_script() )
    return new BError( "Item cannot be unequipped" );
  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return new BError( "Item was destroyed in Equip Script" );
  }

  if ( oldcont != nullptr )
  {
    oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
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
  if ( getCharacterParam( 0, chr ) && getItemParam( 1, item ) )
  {
    ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
    if ( !item->movable() )
    {
      Character* _chr = controller_.get();
      if ( _chr == nullptr || !_chr->can_move( item ) )
        return new BError( "That is immobile" );
    }

    if ( item->inuse() && !is_reserved_to_me( item ) )
    {
      return new BError( "That item is being used." );
    }

    if ( !chr->equippable( item ) || !item->check_equiptest_scripts( chr ) )
    {
      return new BError( "That item is not equippable by that character" );
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in EquipTest script" );
    }

    item->layer = Plib::tilelayer( item->graphic );

    if ( item->has_equip_script() )
    {
      BObjectImp* res = item->run_equip_script( chr, false );
      if ( !res->isTrue() )
        return res;
      else
        BObject obj( res );
    }


    true_extricate( item );

    // at this point, 'item' is free - doesn't belong to the world, or a container.
    chr->equip( item );
    send_wornitem_to_inrange( chr, item );

    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_RestartScript()
{
  Character* chr;
  if ( getCharacterParam( 0, chr ) )
  {
    if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) )
    {
      NPC* npc = static_cast<NPC*>( chr );
      npc->restart_script();
      return new BLong( 1 );
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


BObjectImp* UOExecutorModule::mf_GetHarvestDifficulty()
{
  const String* resource;
  unsigned short tiletype;
  Pos2d pos;
  Realms::Realm* realm;
  if ( getStringParam( 0, resource ) && getRealmParam( 4, &realm ) && getPos2dParam( 1, 2, &pos ) &&
       getParam( 3, tiletype ) )
  {
    return get_harvest_difficulty( resource->data(), Pos4d( pos, 0, realm ), tiletype );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_HarvestResource()
{
  Pos2d pos;
  Realms::Realm* realm;
  const String* resource;
  int b;
  int n;

  if ( getStringParam( 0, resource ) && getRealmParam( 5, &realm ) &&
       getPos2dParam( 1, 2, &pos, realm ) && getParam( 3, b ) && getParam( 4, n ) )
  {
    if ( b <= 0 )
      return new BError( "b must be >= 0" );
    return harvest_resource( resource->data(), Pos4d( pos, 0, realm ), b, n );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_GetRegionName( /* objref */ )
{
  UObject* obj;

  if ( getUObjectParam( 0, obj ) )
  {
    JusticeRegion* justice_region;
    if ( obj->isa( UOBJ_CLASS::CLASS_ITEM ) )
      obj = obj->toplevel_owner();

    if ( obj->isa( UOBJ_CLASS::CLASS_CHARACTER ) )
    {
      Character* chr = static_cast<Character*>( obj );

      if ( chr->logged_in() )
        justice_region = chr->client->gd->justice_region;
      else
        justice_region = gamestate.justicedef->getregion( chr->pos() );
    }
    else
      justice_region = gamestate.justicedef->getregion( obj->pos() );

    if ( justice_region == nullptr )
      return new BError( "No Region defined at this Location" );
    else
      return new String( justice_region->region_name() );
  }
  else
    return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_GetRegionNameAtLocation( /* x, y, realm */ )
{
  Pos2d pos;
  Realms::Realm* realm;
  if ( getRealmParam( 2, &realm ) && getPos2dParam( 0, 1, &pos ) )
  {
    JusticeRegion* justice_region = gamestate.justicedef->getregion( Pos4d( pos, 0, realm ) );
    if ( justice_region == nullptr )
      return new BError( "No Region defined at this Location" );
    else
      return new String( justice_region->region_name() );
  }
  else
    return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_GetRegionString()
{
  const String* resource;
  const String* propname;
  Pos2d pos;
  Realms::Realm* realm;
  if ( getStringParam( 0, resource ) && getRealmParam( 4, &realm ) && getPos2dParam( 1, 2, &pos ) &&
       getStringParam( 3, propname ) )
  {
    return get_region_string( resource->data(), Pos4d( pos, 0, realm ), propname->value() );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_GetRegionLightLevelAtLocation( /* x, y, realm */ )
{
  Pos2d pos;
  Realms::Realm* realm;

  if ( getRealmParam( 2, &realm ) && getPos2dParam( 0, 1, &pos ) )
  {
    LightRegion* light_region = gamestate.lightdef->getregion( Pos4d( pos, 0, realm ) );
    int lightlevel;
    if ( light_region != nullptr )
      lightlevel = light_region->lightlevel;
    else
      lightlevel = settingsManager.ssopt.default_light_level;
    return new BLong( lightlevel );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}


BObjectImp* UOExecutorModule::mf_EquipFromTemplate()
{
  Character* chr;
  const String* template_name;
  if ( getCharacterParam( 0, chr ) && getStringParam( 1, template_name ) )
  {
    return equip_from_template( chr, template_name->value() );
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
  if ( getCharacterParam( 0, chr ) && getStringParam( 1, privstr ) )
  {
    chr->grant_privilege( privstr->data() );
    return new BLong( 1 );
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
  if ( getCharacterParam( 0, chr ) && getStringParam( 1, privstr ) )
  {
    chr->revoke_privilege( privstr->data() );
    return new BLong( 1 );
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
  if ( ch >= '0' && ch <= '9' )
    ch -= '0';
  else if ( ch >= 'A' && ch <= 'F' )
    ch = ch - 'A' + 0xa;
  else if ( ch >= 'a' && ch <= 'f' )
    ch = ch - 'a' + 0xa;

  return ch;
}

BObjectImp* UOExecutorModule::mf_SendPacket()
{
  Character* chr;
  Network::Client* client;
  const String* str;
  if ( getCharacterOrClientParam( 0, chr, client ) && getStringParam( 1, str ) )
  {
    if ( str->length() % 2 > 0 )
    {
      return new BError( "Invalid packet string length." );
    }
    Network::PktHelper::PacketOut<Network::EncryptedPktBuffer>
        buffer;  // encryptedbuffer is the only one without getID buffer[0]
    unsigned char* buf = reinterpret_cast<unsigned char*>( buffer->getBuffer() );
    const char* s = str->data();
    while ( buffer->offset < 2000 && isxdigit( s[0] ) && isxdigit( s[1] ) )
    {
      unsigned char ch;
      ch = ( decode_xdigit( s[0] ) << 4 ) | decode_xdigit( s[1] );
      *( buf++ ) = ch;
      buffer->offset++;
      s += 2;
    }
    if ( chr != nullptr )
    {
      if ( chr->has_active_client() )
      {
        buffer.Send( chr->client );
        return new BLong( 1 );
      }
      else
      {
        return new BError( "No client attached" );
      }
    }
    else if ( client != nullptr )
    {
      if ( client->isConnected() )
      {
        buffer.Send( client );
        return new BLong( 1 );
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
  int x, y;
  u32 arrowid = 0;

  if ( getCharacterParam( 0, chr ) && getParam( 1, x, -1, 1000000 ) &&
       getParam( 2, y, -1, 1000000 ) )  // max values checked below
  {
    if ( !chr->has_active_client() )
      return new BError( "No client attached" );
    {
      int arrow_id;
      if ( exec.getParam( 3, arrow_id ) )
      {
        if ( arrow_id < 1 )
          return new BError( "ArrowID out of range" );
        arrowid = static_cast<u32>( arrow_id );
      }
      else
        arrowid = uoexec().pid();
    }
    bool usesNewPktSize = ( chr->client->ClientType & Network::CLIENTTYPE_7090 ) > 0;

    Network::PktHelper::PacketOut<Network::PktOut_BA> msg;
    if ( x == -1 && y == -1 )
    {
      msg->Write<u8>( PKTOUT_BA_ARROW_OFF );
      msg->offset += 4;  // u16 x_tgt,y_tgt
      if ( usesNewPktSize )
      {
        if ( !arrowid )
        {
          return new BError( "ArrowID must be supplied for cancelation." );
        }
        else
        {
          msg->Write<u32>( static_cast<u32>( arrowid & 0xFFFFFFFF ) );
        }
      }
    }
    else
    {
      if ( !chr->realm()->valid( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ),
                                 0 ) )
        return new BError( "Invalid Coordinates for Realm" );
      msg->Write<u8>( PKTOUT_BA_ARROW_ON );
      msg->WriteFlipped<u16>( static_cast<u16>( x & 0xFFFF ) );
      msg->WriteFlipped<u16>( static_cast<u16>( y & 0xFFFF ) );
      if ( usesNewPktSize )
        msg->Write<u32>( static_cast<u32>( arrowid & 0xFFFFFFFF ) );
    }
    msg.Send( chr->client );
    return new BLong( arrowid );
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
  if ( getCharacterParam( 0, chr ) && getParam( 1, spellid ) )
  {
    if ( !VALID_SPELL_ID( spellid ) )
    {
      return new BError( "Spell ID out of range" );
    }
    USpell* spell = gamestate.spells[spellid];
    if ( spell == nullptr )
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
  if ( getCharacterParam( 0, chr ) && getParam( 1, spellid ) )
  {
    if ( !VALID_SPELL_ID( spellid ) )
    {
      return new BError( "Spell ID out of range" );
    }
    USpell* spell = gamestate.spells[spellid];
    if ( spell == nullptr )
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
  if ( getParam( 0, spellid ) )
  {
    if ( !VALID_SPELL_ID( spellid ) )
    {
      return new BError( "Spell ID out of range" );
    }
    USpell* spell = gamestate.spells[spellid];
    if ( spell == nullptr )
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

  if ( getCharacterParam( 0, chr ) && getParam( 1, spellid ) && getParam( 2, font ) &&
       getParam( 3, color ) )
  {
    if ( !VALID_SPELL_ID( spellid ) )
    {
      return new BError( "Spell ID out of range" );
    }
    USpell* spell = gamestate.spells[spellid];
    if ( spell == nullptr )
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
  if ( getCharacterParam( 0, chr ) )
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
    {
      Item* item = chr->wornitem( layer );
      if ( item != nullptr )
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
  if ( getCharacterParam( 0, chr ) && getParam( 1, layer ) )
  {
    if ( layer < LOWEST_LAYER || layer > HIGHEST_LAYER )
    {
      return new BError( "Invalid layer" );
    }

    Item* item = chr->wornitem( layer );
    if ( item == nullptr )
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
  Network::Client* client;

  if ( getCharacterOrClientParam( 0, chr, client ) )
  {
    if ( chr != nullptr )
    {
      if ( !chr->has_active_client() )
        return new BError( "No client attached" );

      client = chr->client;
    }

    if ( client != nullptr )
    {
      if ( client->isConnected() )
      {
        client->Disconnect();
        return new BLong( 1 );
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
  if ( getParam( 0, x ) &&                                  // FIXME realm size
       getParam( 1, y ) && getStringParam( 2, strrealm ) )  // FIXME realm size
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );
    if ( !realm->valid( x, y, 0 ) )
      return new BError( "Invalid Coordinates for realm" );

    Plib::MAPTILE_CELL cell =
        realm->getmaptile( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ) );

    std::unique_ptr<BStruct> result( new BStruct );
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
  if ( getParam( 0, x ) && getParam( 1, y ) && getStringParam( 2, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );
    if ( !realm->valid( x, y, 0 ) )
      return new BError( "Invalid Coordinates for Realm" );

    short z = -255;
    if ( realm->lowest_standheight( x, y, &z ) )
      return new BLong( z );
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
  if ( getStringParam( 0, namestr ) )
  {
    unsigned int objtype = get_objtype_byname( namestr->data() );
    if ( objtype != 0 )
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
  if ( getCharacterParam( 0, chr ) )
  {
    BObjectImp* event = exec.getParamImp( 1 );
    if ( event != nullptr )
    {
      if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        NPC* npc = static_cast<NPC*>( chr );
        // event->add_ref(); // UNTESTED
        return npc->send_event_script( event->copy() );
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

BObjectImp* UOExecutorModule::mf_DestroyMulti()
{
  Multi::UMulti* multi;
  if ( getMultiParam( 0, multi ) )
  {
    const ItemDesc& id = find_itemdesc( multi->objtype_ );
    if ( !id.destroy_script.empty() )
    {
      BObjectImp* res = run_script_to_completion( id.destroy_script, new EItemRefObjImp( multi ) );
      if ( !res->isTrue() )
      {  // destruction is okay
        return res;
      }
    }

    Multi::UBoat* boat = multi->as_boat();
    if ( boat != nullptr )
    {
      return Multi::destroy_boat( boat );
    }
    Multi::UHouse* house = multi->as_house();
    if ( house != nullptr )
    {
      return Multi::destroy_house( house );
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
  if ( getParam( 0, multiid ) )
  {
    if ( !Multi::MultiDefByMultiIDExists( multiid ) )
      return new BError( "MultiID not found" );

    const Multi::MultiDef& md = *Multi::MultiDefByMultiID( multiid );
    std::unique_ptr<BStruct> ret( new BStruct );
    ret->addMember( "xmin", new BLong( md.minrxyz.x() ) );
    ret->addMember( "xmax", new BLong( md.maxrxyz.x() ) );
    ret->addMember( "ymin", new BLong( md.minrxyz.y() ) );
    ret->addMember( "ymax", new BLong( md.maxrxyz.y() ) );
    return ret.release();
  }
  else
    return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_SetScriptController()
{
  Character* old_controller = controller_.get();
  BObjectImp* param0 = getParamImp( 0 );
  bool handled = false;

  if ( param0->isa( BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( param0 );
    if ( lng->value() == 0 )
    {
      controller_.clear();
      handled = true;
    }
  }

  if ( !handled )
  {
    Character* chr;
    if ( getCharacterParam( 0, chr ) )
      controller_.set( chr );
    else
      controller_.clear();
  }

  if ( old_controller )
    return new ECharacterRefObjImp( old_controller );
  else
    return new BLong( 0 );
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
if (house != nullptr)
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
  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getStringParam( 3, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );
    if ( !realm->valid( x, y, z ) )
      return new BError( "Coordinates Invalid for Realm" );
    short newz;
    Multi::UMulti* multi;
    Item* walkon;
    if ( realm->lowest_walkheight( x, y, z, &newz, &multi, &walkon, true, Plib::MOVEMODE_LAND ) )
    {
      std::unique_ptr<BStruct> arr( new BStruct );
      arr->addMember( "z", new BLong( newz ) );
      if ( multi != nullptr )
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

BObjectImp* UOExecutorModule::mf_GetStandingLayers( /* x, y, flags, realm */ )
{
  unsigned short x, y;
  int flags;
  const String* strrealm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, flags ) &&
       getStringParam( 3, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( !realm->valid( x, y, 0 ) )
      return new BError( "Coordinates Invalid for Realm" );

    std::unique_ptr<ObjArray> newarr( new ObjArray );

    Plib::MapShapeList mlist;
    realm->readmultis( mlist, x, y, flags );
    realm->getmapshapes( mlist, x, y, flags );

    for ( unsigned i = 0; i < mlist.size(); ++i )
    {
      std::unique_ptr<BStruct> arr( new BStruct );

      if ( mlist[i].flags & ( Plib::FLAG::MOVELAND | Plib::FLAG::MOVESEA ) )
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
  if ( getItemParam( 0, item ) )
  {
    if ( item->inuse() )
    {
      if ( is_reserved_to_me( item ) )
        return new BLong( 2 );
      else
        return new BError( "That item is already being used." );
    }
    item->inuse( true );
    reserved_items_.push_back( ItemRef( item ) );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_ReleaseItem()
{
  Item* item;
  if ( getItemParam( 0, item ) )
  {
    if ( item->inuse() )
    {
      for ( unsigned i = 0; i < reserved_items_.size(); ++i )
      {
        if ( reserved_items_[i].get() == item )
        {
          item->inuse( false );
          reserved_items_[i] = reserved_items_.back();
          reserved_items_.pop_back();
          return new BLong( 1 );
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


BObjectImp* UOExecutorModule::mf_SendSkillWindow()
{
  Character *towhom, *forwhom;
  if ( getCharacterParam( 0, towhom ) && getCharacterParam( 1, forwhom ) )
  {
    if ( towhom->has_active_client() )
    {
      send_skillmsg( towhom->client, forwhom );
      return new BLong( 1 );
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


BObjectImp* UOExecutorModule::mf_OpenPaperdoll()
{
  Character *towhom, *forwhom;
  if ( getCharacterParam( 0, towhom ) && getCharacterParam( 1, forwhom ) )
  {
    if ( towhom->has_active_client() )
    {
      send_paperdoll( towhom->client, forwhom );
      return new BLong( 1 );
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
  if ( getItemParam( 0, cont_item ) && getObjtypeParam( 1, objtype ) && getParam( 2, amount ) )
  {
    if ( !cont_item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      return new BError( "That is not a container" );
    if ( amount < 0 )
      return new BError( "Amount cannot be negative" );

    UContainer* cont = static_cast<UContainer*>( cont_item );
    int amthave = cont->find_sumof_objtype_noninuse( objtype );
    if ( amthave < amount )
      return new BError( "Not enough of that substance in container" );

    cont->consume_sumof_objtype_noninuse( objtype, amount );

    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

bool UOExecutorModule::is_reserved_to_me( Item* item )
{
  for ( unsigned i = 0; i < reserved_items_.size(); ++i )
  {
    if ( reserved_items_[i].get() == item )
      return true;
  }
  return false;
}

BObjectImp* UOExecutorModule::mf_Shutdown()
{
  int exit_code = 0;

  if ( exec.hasParams( 1 ) )
  {
    getParam( 0, exit_code );
  }

  Clib::signal_exit( exit_code );
#ifndef _WIN32
  // the catch_signals_thread (actually main) sits with sigwait(),
  // so it won't wake up except by being signalled.
  signal_catch_thread();
#endif
  return new BLong( 1 );
}


BObjectImp* UOExecutorModule::mf_GetCommandHelp()
{
  Character* chr;
  const String* cmd;
  if ( getCharacterParam( 0, chr ) && getStringParam( 1, cmd ) )
  {
    std::string help = get_textcmd_help( chr, cmd->value() );
    if ( !help.empty() )
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


BObjectImp* UOExecutorModule::mf_SendStringAsTipWindow()
{
  Character* chr;
  const String* str;
  if ( getCharacterParam( 0, chr ) && getStringParam( 1, str ) )
  {
    if ( chr->has_active_client() )
    {
      send_tip( chr->client, str->value() );
      return new BLong( 1 );
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

BObjectImp* UOExecutorModule::mf_ListItemsNearLocationWithFlag(
    /* x, y, z, range, flags, realm */ )  // DAVE
{
  unsigned short x, y;
  short range;
  int z, flags;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getParam( 4, flags ) && getStringParam( 5, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );
    WorldIterator<ItemFilter>::InRange(
        x, y, realm, range,
        [&]( Item* item )
        {
          if ( ( Plib::tile_uoflags( item->graphic ) & flags ) )
          {
            if ( ( abs( item->x() - x ) <= range ) && ( abs( item->y() - y ) <= range ) )
            {
              if ( ( z == LIST_IGNORE_Z ) || ( abs( item->z() - z ) < CONST_DEFAULT_ZRANGE ) )
                newarr->addElement( new EItemRefObjImp( item ) );
            }
          }
        } );

    return newarr.release();
  }

  return new BError( "Invalid parameter" );
}

BObjectImp* UOExecutorModule::mf_ListStaticsAtLocation( /* x, y, z, flags, realm */ )
{
  unsigned short x, y;
  int z, flags;
  const String* strrealm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, flags ) &&
       getStringParam( 4, strrealm ) )
  {
    Realms::Realm* realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );

    if ( !( flags & ITEMS_IGNORE_STATICS ) )
    {
      Plib::StaticEntryList slist;
      realm->getstatics( slist, x, y );

      for ( unsigned i = 0; i < slist.size(); ++i )
      {
        if ( ( z == LIST_IGNORE_Z ) || ( slist[i].z == z ) )
        {
          std::unique_ptr<BStruct> arr( new BStruct );
          arr->addMember( "x", new BLong( x ) );
          arr->addMember( "y", new BLong( y ) );
          arr->addMember( "z", new BLong( slist[i].z ) );
          arr->addMember( "objtype", new BLong( slist[i].objtype ) );
          arr->addMember( "hue", new BLong( slist[i].hue ) );
          newarr->addElement( arr.release() );
        }
      }
    }

    if ( !( flags & ITEMS_IGNORE_MULTIS ) )
    {
      Plib::StaticList mlist;
      realm->readmultis( mlist, x, y );

      for ( unsigned i = 0; i < mlist.size(); ++i )
      {
        if ( ( z == LIST_IGNORE_Z ) || ( mlist[i].z == z ) )
        {
          std::unique_ptr<BStruct> arr( new BStruct );
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

BObjectImp* UOExecutorModule::mf_ListStaticsNearLocation( /* x, y, z, range, flags, realm */ )
{
  unsigned short x, y;
  int z, flags;
  short range;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getParam( 4, flags ) && getStringParam( 5, strrealm ) )
  {
    realm = find_realm( strrealm->value() );
    if ( !realm )
      return new BError( "Realm not found" );

    if ( z == LIST_IGNORE_Z )
    {
      if ( !realm->valid( x, y, 0 ) )
        return new BError( "Invalid Coordinates for realm" );
    }
    else
    {
      if ( !realm->valid( x, y, static_cast<short>( z ) ) )
        return new BError( "Invalid Coordinates for realm" );
    }

    std::unique_ptr<ObjArray> newarr( new ObjArray );

    short wxL, wyL, wxH, wyH;
    wxL = x - range;
    if ( wxL < 0 )
      wxL = 0;
    wyL = y - range;
    if ( wyL < 0 )
      wyL = 0;
    wxH = x + range;
    if ( wxH > realm->width() - 1 )
      wxH = realm->width() - 1;
    wyH = y + range;
    if ( wyH > realm->height() - 1 )
      wyH = realm->height() - 1;

    for ( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
      for ( unsigned short wy = wyL; wy <= wyH; ++wy )
      {
        if ( !( flags & ITEMS_IGNORE_STATICS ) )
        {
          Plib::StaticEntryList slist;
          realm->getstatics( slist, wx, wy );

          for ( unsigned i = 0; i < slist.size(); ++i )
          {
            if ( ( z == LIST_IGNORE_Z ) || ( abs( slist[i].z - z ) < CONST_DEFAULT_ZRANGE ) )
            {
              std::unique_ptr<BStruct> arr( new BStruct );
              arr->addMember( "x", new BLong( wx ) );
              arr->addMember( "y", new BLong( wy ) );
              arr->addMember( "z", new BLong( slist[i].z ) );
              arr->addMember( "objtype", new BLong( slist[i].objtype ) );
              arr->addMember( "hue", new BLong( slist[i].hue ) );
              newarr->addElement( arr.release() );
            }
          }
        }

        if ( !( flags & ITEMS_IGNORE_MULTIS ) )
        {
          Plib::StaticList mlist;
          realm->readmultis( mlist, wx, wy );

          for ( unsigned i = 0; i < mlist.size(); ++i )
          {
            if ( ( z == LIST_IGNORE_Z ) || ( abs( mlist[i].z - z ) < CONST_DEFAULT_ZRANGE ) )
            {
              std::unique_ptr<BStruct> arr( new BStruct );
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
//        is on the Closed List.  If not, the pathfind errors out with a
//        "Solution Corrupt!" error.  This is an attempt to short circuit any
//        solution containing erroneous nodes in it.
//
//  Safeguard #2) I keep a vector of the nodes in the solution as we go through them.
//        Before adding each node to the vector, I search that vector for that
//        node.  If that vector already contains the node, the pathfind errors
//        out with a "Solution Corrupt!" error.  THis is an attempt to catch
//        the cases where Closed List nodes have looped for some reason.
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
//          implimentation, pretty nicely done and documented if you are
//          interested in learning about A*.
//
//  fsa.h     --  a "fixed size allocation" module which I toy with enabling and
//          disabling.  Using it is supposed to get you some speed, but it
//          limits your maps because it will only allocate a certain # of
//          nodes at once, and after that, it will return out of memory.
//          Presently, it is disabled, but will be submitted to CVS for
//          completion in case we wish to use it later.
//
//  uopathnode.h -- this is stricly my work, love it or hate it, it's pretty quick
//          and dirty, and can stand to be cleaned up, optimized, and so forth.
//          I have the name field and function in there to allow for some
//          debugging, though the character array could probably be removed to
//          make the nodes smaller.  I didn't find it mattered particularly, since
//          these puppies are created and destroyed at a pretty good clip.
//          It is this class that encapsulates the necessary functionality to
//          make the otherwise fairly generic stlastar class work.

typedef Plib::AStarSearch<UOPathState> UOSearch;

BObjectImp* UOExecutorModule::mf_FindPath()
{
  Pos3d pos1, pos2;
  Realms::Realm* realm;
  const String* movemode_name;

  if ( !getPos3dParam( 0, 1, 2, &pos1 ) || !getPos3dParam( 3, 4, 5, &pos2 ) ||
       !getRealmParam( 6, &realm ) || !getStringParam( 9, movemode_name ) )
    return new BError( "Invalid parameter" );
  if ( !pos1.in_range( pos2, static_cast<u16>( settingsManager.ssopt.max_pathfind_range ) ) )
    return new BError( "Beyond Max Range." );

  short theSkirt;
  int flags;

  if ( !getParam( 7, flags ) )
    flags = FP_IGNORE_MOBILES;

  if ( !getParam( 8, theSkirt ) )
    theSkirt = 5;

  Plib::MOVEMODE movemode = Character::decode_movemode( movemode_name->value() );
  if ( movemode == Plib::MOVEMODE_NONE )
    return new BError( "Wrong movemode parameter" );

  if ( theSkirt < 0 )
    theSkirt = 0;

  if ( !realm->valid( pos1.xy() ) )
    return new BError( "Start Coordinates Invalid for Realm" );
  if ( !realm->valid( pos2.xy() ) )
    return new BError( "End Coordinates Invalid for Realm" );

  auto astarsearch = std::make_unique<UOSearch>();

  Range2d range( pos1.xy().min( pos2.xy() ) - Vec2d( theSkirt, theSkirt ),
                 pos1.xy().max( pos2.xy() ) + Vec2d( theSkirt, theSkirt ), realm );

  if ( Plib::systemstate.config.loglevel >= 12 )
  {
    POLLOG.Format( "[FindPath] Calling FindPath({}, {}, {}, 0x{:X}, {})\n" )
        << pos1 << pos2 << realm->name() << flags << theSkirt;
    POLLOG.Format( "[FindPath]   search for Blockers inside {}\n" ) << range;
  }

  bool doors_block = ( flags & FP_IGNORE_DOORS ) ? false : true;
  AStarParams params( range, doors_block, movemode, realm );

  if ( !( flags & FP_IGNORE_MOBILES ) )
  {
    WorldIterator<MobileFilter>::InBox( range, realm,
                                        [&]( Mobile::Character* chr )
                                        {
                                          params.AddBlocker( chr->pos3d() );

                                          if ( Plib::systemstate.config.loglevel >= 12 )
                                            POLLOG << "[FindPath]   add Blocker " << chr->name()
                                                   << " at " << chr->pos() << "\n";
                                        } );
  }

  if ( Plib::systemstate.config.loglevel >= 12 )
  {
    POLLOG.Format( "[FindPath]   use StartNode {}\n" ) << pos1;
    POLLOG.Format( "[FindPath]   use EndNode {}\n" ) << pos2;
  }

  // Create a start state
  UOPathState nodeStart( pos1, &params );
  // Define the goal state
  UOPathState nodeEnd( pos2, &params );
  // Set Start and goal states
  astarsearch->SetStartAndGoalStates( nodeStart, nodeEnd );
  unsigned int SearchState;
  do
  {
    SearchState = astarsearch->SearchStep();
  } while ( SearchState == UOSearch::SEARCH_STATE_SEARCHING );
  if ( SearchState == UOSearch::SEARCH_STATE_SUCCEEDED )
  {
    UOPathState* node = astarsearch->GetSolutionStart();

    auto nodeArray = std::make_unique<ObjArray>();
    while ( ( node = astarsearch->GetSolutionNext() ) != nullptr )
    {
      auto nextStep = std::make_unique<BStruct>();
      const auto& pos = node->position();
      nextStep->addMember( "x", new BLong( pos.x() ) );
      nextStep->addMember( "y", new BLong( pos.y() ) );
      nextStep->addMember( "z", new BLong( pos.z() ) );
      nodeArray->addElement( nextStep.release() );
    }
    astarsearch->FreeSolutionNodes();
    return nodeArray.release();
  }
  else if ( SearchState == UOSearch::SEARCH_STATE_FAILED )
  {
    return new BError( "Failed to find a path." );
  }
  else if ( SearchState == UOSearch::SEARCH_STATE_OUT_OF_MEMORY )
  {
    return new BError( "Out of memory." );
  }
  else if ( SearchState == UOSearch::SEARCH_STATE_SOLUTION_CORRUPTED )
  {
    return new BError( "Solution Corrupted!" );
  }

  return new BError( "Pathfind Error." );
}


BObjectImp* UOExecutorModule::mf_UseItem()
{
  Item* item;
  Character* chr;

  if ( getItemParam( 0, item ) && getCharacterParam( 1, chr ) )
  {
    const ItemDesc& itemdesc = find_itemdesc( item->objtype_ );

    if ( itemdesc.requires_attention && ( chr->skill_ex_active() || chr->casting_spell() ) )
    {
      if ( chr->client != nullptr )
      {
        send_sysmessage( chr->client, "I am already doing something else." );
        return new BError( "Character busy." );
        ;
      }
    }

    if ( itemdesc.requires_attention && chr->hidden() )
      chr->unhide();

    ref_ptr<EScriptProgram> prog;

    std::string on_use_script = item->get_use_script_name();

    if ( !on_use_script.empty() )
    {
      ScriptDef sd( on_use_script, nullptr, "" );
      prog = find_script2( sd,
                           true,  // complain if not found
                           Plib::systemstate.config.cache_interactive_scripts );
    }
    else if ( !itemdesc.on_use_script.empty() )
    {
      prog = find_script2( itemdesc.on_use_script, true,
                           Plib::systemstate.config.cache_interactive_scripts );
    }

    if ( prog.get() != nullptr )
    {
      if ( chr->start_itemuse_script( prog.get(), item, itemdesc.requires_attention ) )
        return new BLong( 1 );
      else
        return new BError( "Failed to start script!" );
      // else log the fact?
    }
    else
    {
      if ( chr->client != nullptr )
        item->builtin_on_use( chr->client );
      return new BLong( 0 );
    }
  }
  else
  {
    return new BError( "Invalid parameter" );
  }
}

BObjectImp* UOExecutorModule::mf_FindSubstance()
{
  UContainer::Contents substanceVector;

  Item* cont_item;
  unsigned int objtype;
  int amount;

  if ( getItemParam( 0, cont_item ) && getObjtypeParam( 1, objtype ) && getParam( 2, amount ) )
  {
    int makeInUseLong;
    bool makeInUse;
    int flags;
    if ( !getParam( 3, makeInUseLong ) )
      makeInUse = false;
    else
      makeInUse = ( makeInUseLong ? true : false );
    if ( !getParam( 4, flags ) )
      flags = 0;

    if ( !cont_item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      return new BError( "That is not a container" );
    if ( amount < 0 )
      return new BError( "Amount cannot be negative" );

    UContainer* cont = static_cast<UContainer*>( cont_item );
    int amthave = cont->find_sumof_objtype_noninuse( objtype, amount, substanceVector, flags );
    if ( ( amthave < amount ) && ( !( flags & FINDSUBSTANCE_FIND_ALL ) ) )
      return new BError( "Not enough of that substance in container" );
    else
    {
      std::unique_ptr<ObjArray> theArray( new ObjArray() );
      Item* item;

      for ( UContainer::Contents::const_iterator itr = substanceVector.begin();
            itr != substanceVector.end(); ++itr )
      {
        item = *itr;
        if ( item != nullptr )
        {
          if ( ( makeInUse ) && ( !item->inuse() ) )
          {
            item->inuse( true );
            reserved_items_.push_back( ItemRef( item ) );
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

  if ( !( getItemParam( 0, item1 ) && getItemParam( 1, item2 ) ) )
  {
    return new BError( "Invalid parameter type" );
  }

  if ( item1->objtype_ != item2->objtype_ )
    return new BError( "Objtypes differs" );
  if ( !item1->stackable() )
    return new BError( "That item type is not stackable." );

  if ( item1->can_add_to_self( *item2, false ) )
    return new BLong( 1 );
  else
    return new BError( "Failed to stack" );
}

BObjectImp* UOExecutorModule::mf_UpdateMobile()
{
  Character* chr;
  int flags;

  if ( getCharacterParam( 0, chr ) && getParam( 1, flags ) )
  {
    if ( flags == 1 )
    {
      if ( ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) ) && ( chr->client ) )  // no npc and active client
        send_owncreate( chr->client, chr );                             // inform self
      if ( ( chr->isa( UOBJ_CLASS::CLASS_NPC ) ) || ( chr->client ) )   // npc or active client
        send_create_mobile_to_nearby_cansee( chr );                     // inform other
      else
        return new BError( "Mobile is offline" );
    }
    else
    {
      if ( ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) ) && ( chr->client ) )  // no npc and active client
        send_move( chr->client, chr );                                  // inform self
      if ( ( chr->isa( UOBJ_CLASS::CLASS_NPC ) ) || ( chr->client ) )   // npc or active client
        send_move_mobile_to_nearby_cansee( chr );                       // inform other
      else
        return new BError( "Mobile is offline" );
    }
    return new BLong( 1 );
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_UpdateItem()
{
  Item* item;

  if ( getItemParam( 0, item ) )
  {
    send_item_to_inrange( item );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


BObjectImp* UOExecutorModule::mf_CanWalk(
    /*movemode, x1, y1, z1, x2_or_dir, y2 := -1, realm := DEF*/ )
{
  Pos4d p;
  int x2_or_dir, y2_;
  const String* movemode_name;

  if ( ( getStringParam( 0, movemode_name ) ) && ( getPos4dParam( 1, 2, 3, 6, &p ) ) &&
       ( getParam( 4, x2_or_dir ) ) && ( getParam( 5, y2_ ) ) )
  {
    Plib::MOVEMODE movemode = Character::decode_movemode( movemode_name->value() );

    Core::UFACING dir;
    if ( y2_ == -1 )
      dir = static_cast<Core::UFACING>( x2_or_dir & 0x7 );
    else
    {
      if ( !p.realm()->valid( static_cast<xcoord>( x2_or_dir ), static_cast<ycoord>( y2_ ), 0 ) )
        return new BError( "Invalid coordinates for realm." );

      dir = p.xy().direction_toward(
          Pos2d( static_cast<u16>( x2_or_dir ), static_cast<u16>( y2_ ) ) );
    }

    if ( dir & 1 )  // check if diagonal movement is allowed
    {
      short new_z;
      u8 tmp_facing = ( dir + 1 ) & 0x7;
      auto tmp_pos = p.move( static_cast<Core::UFACING>( tmp_facing ) );

      // needs to save because if only one direction is blocked, it shouldn't block ;)
      bool walk1 = p.realm()->walkheight( tmp_pos.x(), tmp_pos.y(), tmp_pos.z(), &new_z, nullptr,
                                          nullptr, true, movemode, nullptr );

      tmp_facing = ( dir - 1 ) & 0x7;
      tmp_pos = p.move( static_cast<Core::UFACING>( tmp_facing ) );

      if ( !walk1 && !p.realm()->walkheight( tmp_pos.x(), tmp_pos.y(), tmp_pos.z(), &new_z, nullptr,
                                             nullptr, true, movemode, nullptr ) )
        return new BError( "Cannot walk there" );
    }

    p.move_to( dir );
    short newz;

    if ( !p.realm()->walkheight( p.x(), p.y(), p.z(), &newz, nullptr, nullptr, true, movemode,
                                 nullptr ) )
      return new BError( "Cannot walk there" );

    return new BLong( newz );
  }
  else
    return new BError( "Invalid parameter" );
}


BObjectImp* UOExecutorModule::mf_SendCharProfile(
    /*chr, of_who, title, uneditable_text := array, editable_text := array*/ )
{
  Character *chr, *of_who;
  const String* title;
  const String* uText;
  const String* eText;

  if ( getCharacterParam( 0, chr ) && getCharacterParam( 1, of_who ) &&
       getStringParam( 2, title ) && getUnicodeStringParam( 3, uText ) &&
       getUnicodeStringParam( 4, eText ) )
  {
    if ( chr->logged_in() && of_who->logged_in() )
    {
      if ( uText->length() > SPEECH_MAX_LEN || eText->length() > SPEECH_MAX_LEN )
        return new BError( "Text exceeds maximum size." );

      sendCharProfile( chr, of_who, title->value(), uText->value(), eText->value() );
      return new BLong( 1 );
    }
    else
      return new BError( "Mobile must be online." );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* UOExecutorModule::mf_SendOverallSeason( /*season_id, playsound := 1*/ )
{
  int season_id, playsound;

  if ( getParam( 0, season_id ) && getParam( 1, playsound ) )
  {
    if ( season_id < 0 || season_id > 4 )
      return new BError( "Invalid season id" );

    Network::PktHelper::PacketOut<Network::PktOut_BC> msg;
    msg->Write<u8>( static_cast<u16>( season_id ) );
    msg->Write<u8>( static_cast<u16>( playsound ) );

    for ( Clients::iterator itr = networkManager.clients.begin(),
                            end = networkManager.clients.end();
          itr != end; ++itr )
    {
      Network::Client* client = *itr;
      if ( !client->chr || !client->chr->logged_in() || client->getversiondetail().major < 1 )
        continue;
      msg.Send( client );
    }
    return new BLong( 1 );
  }
  else
    return new BError( "Invalid parameter" );
}

// bresenham circle calculates the coords based on center coords and radius
BObjectImp* UOExecutorModule::mf_GetMidpointCircleCoords( /* xcenter, ycenter, radius */ )
{
  int xcenter, ycenter, radius;
  if ( !( getParam( 0, xcenter ) && getParam( 1, ycenter ) && getParam( 2, radius ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  std::unique_ptr<ObjArray> coords( new ObjArray );

  std::vector<std::tuple<int, int>> points;
  auto add_point = [&coords]( int x, int y )
  {
    std::unique_ptr<BStruct> point( new BStruct );
    point->addMember( "x", new BLong( x ) );
    point->addMember( "y", new BLong( y ) );
    coords->addElement( point.release() );
  };

  if ( radius == 0 )
  {
    add_point( xcenter, ycenter );
    return coords.release();
  }

  // inside of each quadrant the points are sorted,
  // store the quadrands in seperated vectors and merge them later
  // -> automatically sorted
  std::vector<std::tuple<int, int>> q1, q2, q3, q4;
  int x = -radius, y = 0, err = 2 - 2 * radius; /* II. Quadrant */
  do
  {
    q1.emplace_back( xcenter - x, ycenter + y ); /*   I. Quadrant */
    q2.emplace_back( xcenter - y, ycenter - x ); /*  II. Quadrant */
    q3.emplace_back( xcenter + x, ycenter - y ); /* III. Quadrant */
    q4.emplace_back( xcenter + y, ycenter + x ); /*  IV. Quadrant */
    radius = err;
    if ( radius <= y )
      err += ++y * 2 + 1;
    if ( radius > x || err > y )
      err += ++x * 2 + 1;
  } while ( x < 0 );

  for ( const auto& p : q1 )
    add_point( std::get<0>( p ), std::get<1>( p ) );
  for ( const auto& p : q2 )
    add_point( std::get<0>( p ), std::get<1>( p ) );
  for ( const auto& p : q3 )
    add_point( std::get<0>( p ), std::get<1>( p ) );
  for ( const auto& p : q4 )
    add_point( std::get<0>( p ), std::get<1>( p ) );

  return coords.release();
}

size_t UOExecutorModule::sizeEstimate() const
{
  size_t size = sizeof( *this );
  size += 3 * sizeof( Core::ItemRef* ) + reserved_items_.capacity() * sizeof( Core::ItemRef );
  return size;
}
}  // namespace Module
}  // namespace Pol

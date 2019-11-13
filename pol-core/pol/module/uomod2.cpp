/** @file
 *
 * @par History
 * - 2005/03/02 Shinigami: mf_MoveObjectToRealm - fixed item detection and added container handling
 * - 2005/06/11 Shinigami: added polcore().internal - just for internal Development (not documented)
 * - 2005/06/20 Shinigami: added memory log to polcore().internal (needs defined MEMORYLEAK)
 * - 2005/07/25 Shinigami: doubled Msg size in mf_SendGumpMenu to use larger Gumps
 * - 2005/10/16 Shinigami: added x- and y-offset to mf_SendGumpMenu
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/05/07 Shinigami: mf_SendBuyWindow & mf_SendSellWindow - added Flags to send Item
 * Description using AoS Tooltips
 * - 2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
 *                         added character_race_changer_handler()
 * - 2006/05/30 Shinigami: Changed params of character_race_changer_handler()
 *                         Fixed Bug with detection of Gump-Cancel in uo::SendCharacterRaceChanger()
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 * - 2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format
 * too (leak.log)
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Relocation of multi related cpp/h
 * - 2009/09/10 Turley:    CompressedGump support (Grin)
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/12/17 Turley:    CloseWindow( character, type, object ) - Tomi
 */


/*
    UOEMOD2.CPP - a nice place for the Buy/Sell Interface Functions
    */

#include "pol_global_config.h"

#include <ctype.h>
#include <stddef.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/eprog.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../clib/Program/ProgramConfig.h"
#include "../../clib/clib.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/make_unique.hpp"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../plib/pkg.h"
#include "../../plib/staticblock.h"
#include "../../plib/systemstate.h"
#include "../../plib/uconst.h"
#include "../accounts/account.h"
#include "../accounts/accounts.h"
#include "../accounts/acscrobj.h"
#include "../containr.h"
#include "../core.h"
#include "../exscrobj.h"
#include "../globals/memoryusage.h"
#include "../globals/script_internals.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../layers.h"
#include "../mobile/charactr.h"
#include "../mobile/npc.h"
#include "../multi/customhouses.h"
#include "../multi/house.h"
#include "../multi/multi.h"
#include "../multi/multidef.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packetinterface.h"
#include "../network/packets.h"
#include "../network/pktboth.h"
#include "../network/pktdef.h"
#include "../network/pktin.h"
#include "../objtype.h"
#include "../polclass.h"
#include "../profile.h"
#include "../realms/realm.h"
#include "../reftypes.h"
#include "../scrstore.h"
#include "../sngclick.h"
#include "../statmsg.h"
#include "../tooltips.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uoexec.h"
#include "../uoexhelp.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "uomod.h"

#ifdef MEMORYLEAK
#include "../../clib/opnew.h"
#endif


#ifdef USE_SYSTEM_ZLIB
#include <zlib.h>
#else
#include "../../../lib/zlib/zlib.h"
#endif

namespace Pol
{
namespace Core
{
bool validhair( u16 HairStyle );
bool validbeard( u16 BeardStyle );
}  // namespace Core
namespace Module
{
using namespace Bscript;
using namespace Network;
using namespace Mobile;
using namespace Items;
using namespace Core;

#define CONST_DEFAULT_ZRANGE 19

/*
0000: 74 02 70 40 29 ca d8 28  00 00 00 03 0b 53 65 77   t.p@)..( .....Sew
0010: 69 6e 67 20 6b 69 74 00  00 00 00 0d 09 53 63 69   ing kit. .....Sci
0020: 73 73 6f 72 73 00 00 00  00 09 0a 44 79 69 6e 67   ssors... ...Dying
0030: 20 74 75 62 00 00 00 00  09 05 44 79 65 73 00 00  tub.... ..Dyes..
0040: 00 00 1b 08 44 6f 75 62  6c 65 74 00 00 00 00 1a   ....Doub let.....
0050: 0c 53 68 6f 72 74 20 70  61 6e 74 73 00 00 00 00   .Short p ants....
0060: 37 0c 46 61 6e 63 79 20  73 68 69 72 74 00 00 00   7.Fancy  shirt...
0070: 00 26 0b 4c 6f 6e 67 20  70 61 6e 74 73 00 00 00   .&.Long  pants...
0080: 00 19 0c 46 61 6e 63 79  20 64 72 65 73 73 00 00   ...Fancy  dress..
0090: 00 00 39 0c 50 6c 61 69  6e 20 64 72 65 73 73 00   ..9.Plai n dress.
00a0: 00 00 00 21 06 53 6b 69  72 74 00 00 00 00 1f 05   ...!.Ski rt......
00b0: 4b 69 6c 74 00 00 00 00  1a 0b 48 61 6c 66 20 61   Kilt.... ..Half a
00c0: 70 72 6f 6e 00 00 00 00  49 05 52 6f 62 65 00 00   pron.... I.Robe..
00d0: 00 00 32 06 43 6c 6f 61  6b 00 00 00 00 32 06 43   ..2.Cloa k....2.C
00e0: 6c 6f 61 6b 00 00 00 00  1b 08 44 6f 75 62 6c 65   loak.... ..Double
00f0: 74 00 00 00 00 21 06 54  75 6e 69 63 00 00 00 00   t....!.T unic....
0100: 39 0c 4a 65 73 74 65 72  20 73 75 69 74 00 00 00   9.Jester  suit...
0110: 00 1f 0b 4a 65 73 74 65  72 20 68 61 74 00 00 00   ...Jeste r hat...
0120: 00 19 0b 46 6c 6f 70 70  79 20 68 61 74 00 00 00   ...Flopp y hat...
0130: 00 1a 0e 57 69 64 65 2d  62 72 69 6d 20 68 61 74   ...Wide- brim hat
0140: 00 00 00 00 1b 04 43 61  70 00 00 00 00 1a 0f 54   ......Ca p......T
0150: 61 6c 6c 20 73 74 72 61  77 20 68 61 74 00 00 00   all stra w hat...
0160: 00 19 0a 53 74 72 61 77  20 68 61 74 00 00 00 00   ...Straw  hat....
0170: 1e 0d 57 69 7a 61 72 64  27 73 20 68 61 74 00 00   ..Wizard 's hat..
0180: 00 00 1a 07 42 6f 6e 6e  65 74 00 00 00 00 1b 0e   ....Bonn et......
0190: 46 65 61 74 68 65 72 65  64 20 68 61 74 00 00 00   Feathere d hat...
01a0: 00 1a 0d 54 72 69 63 6f  72 6e 65 20 68 61 74 00   ...Trico rne hat.
01b0: 00 00 00 0e 08 42 61 6e  64 61 6e 61 00 00 00 00   .....Ban dana....
01c0: 0d 09 53 6b 75 6c 6c 63  61 70 00 00 00 00 02 0d   ..Skullc ap......
01d0: 46 6f 6c 64 65 64 20 63  6c 6f 74 68 00 00 00 00   Folded c loth....
01e0: 02 0d 46 6f 6c 64 65 64  20 63 6c 6f 74 68 00 00   ..Folded  cloth..
01f0: 00 00 02 0d 46 6f 6c 64  65 64 20 63 6c 6f 74 68   ....Fold ed cloth
0200: 00 00 00 00 02 0d 46 6f  6c 64 65 64 20 63 6c 6f   ......Fo lded clo
0210: 74 68 00 00 00 00 4a 0e  42 6f 6c 74 20 6f 66 20   th....J. Bolt of
0220: 63 6c 6f 74 68 00 00 00  00 4a 0e 42 6f 6c 74 20   cloth... .J.Bolt
0230: 6f 66 20 63 6c 6f 74 68  00 00 00 00 5b 0f 42 61   of cloth ....[.Ba
0240: 6c 65 20 6f 66 20 63 6f  74 74 6f 6e 00 00 00 00   le of co tton....
0250: 2d 0d 50 69 6c 65 20 6f  66 20 77 6f 6f 6c 00 00   -.Pile o f wool..
0260: 00 00 61 0c 46 6c 61 78  20 62 75 6e 64 6c 65 00   ..a.Flax  bundle.
*/
// msg 3C: be sure to not send the whole structure, just as many 'items' as you insert

bool send_vendorwindow_contents( Client* client, UContainer* for_sale, bool send_aos_tooltip )
{
  PktHelper::PacketOut<PktOut_74> msg;
  msg->offset += 2;  // msglen
  msg->Write<u32>( for_sale->serial_ext );
  msg->offset++;  // num_items
  u8 num_items = 0;
  // FIXME: ick! apparently we need to iterate backwards... WTF?
  for ( int i = for_sale->count() - 1; i >= 0; --i )
  {
    Item* item = ( *for_sale )[i];
    // const ItemDesc& id = find_itemdesc( item->objtype_ );
    std::string desc = item->merchant_description();
    size_t addlen = 5 + desc.size();
    if ( msg->offset + addlen > sizeof msg->buffer )
    {
      return false;
    }
    msg->WriteFlipped<u32>( item->sellprice() );
    msg->Write<u8>( desc.size() + 1 );  // Don't forget the nullptr
    msg->Write( desc.c_str(), static_cast<u16>( desc.size() + 1 ) );
    ++num_items;

    if ( send_aos_tooltip )
      SendAOSTooltip( client, item, true );
  }
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg->offset += 4;
  msg->Write<u8>( num_items );
  msg.Send( client, len );
  return true;
}

BObjectImp* UOExecutorModule::mf_SendBuyWindow( /* character, container, vendor, items, flags */ )
{
  Character *chr, *mrchnt;
  Item *item, *item2;
  NPC* merchant;
  int flags;
  UContainer *for_sale, *bought;
  unsigned char save_layer_one, save_layer_two;

  if ( getCharacterParam( exec, 0, chr ) && getItemParam( exec, 1, item ) &&
       getCharacterParam( exec, 2, mrchnt ) && getItemParam( exec, 3, item2 ) &&
       getParam( 4, flags ) )
  {
    if ( !chr->has_active_client() )
    {
      return new BError( "No client connected to character" );
    }

    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      for_sale = static_cast<UContainer*>( item );
    }
    else
    {
      return new BError( "Parameter 1 invalid" );
    }

    if ( mrchnt->isa( UOBJ_CLASS::CLASS_NPC ) )
    {
      merchant = static_cast<NPC*>( mrchnt );
    }
    else
    {
      return new BError( "Parameter 2 invalid" );
    }

    if ( item2->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      bought = static_cast<UContainer*>( item2 );
    }
    else
    {
      return new BError( "Parameter 3 invalid" );
    }

    //    say_above(merchant, "How may I help you?" );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }


  // try this
  save_layer_one = for_sale->layer;
  for_sale->layer = LAYER_VENDOR_FOR_SALE;
  send_wornitem( chr->client, merchant, for_sale );
  for_sale->layer = save_layer_one;
  for_sale->x = merchant->x;
  for_sale->y = merchant->y;
  for_sale->z = merchant->z;
  // chr->add_additional_legal_item( for_sale );

  save_layer_two = bought->layer;
  bought->layer = LAYER_VENDOR_PLAYER_ITEMS;
  send_wornitem( chr->client, merchant, bought );
  bought->layer = save_layer_two;
  bought->x = merchant->x;
  bought->y = merchant->y;
  bought->z = merchant->z;
  // chr->add_additional_legal_item( bought );

  bool send_aos_tooltip = flags & VENDOR_SEND_AOS_TOOLTIP ? true : false;

  chr->client->gd->vendor.set( merchant );
  chr->client->gd->vendor_for_sale.set( for_sale );
  chr->client->gd->vendor_bought.set( bought );

  chr->client->pause(); /* Prevent lag time between messages */

  // Send the 'for sale' container
  send_container_contents( chr->client, *for_sale );

  if ( !send_vendorwindow_contents( chr->client, for_sale, send_aos_tooltip ) )
  {
    chr->client->restart();
    return new BError( "Too much crap in vendor's inventory!" );
  }

  // Send the 'bought' container (?) (hmm, I think this is the player-sold items)
  send_container_contents( chr->client, *bought );

  if ( !send_vendorwindow_contents( chr->client, bought, send_aos_tooltip ) )
  {
    chr->client->restart();
    return new BError( "Too much crap in vendor's inventory!" );
  }

  // This looks good
  PktHelper::PacketOut<PktOut_24> open_window;
  open_window->Write<u32>( merchant->serial_ext );
  open_window->WriteFlipped<u16>( 0x0030u );  // FIXME: Serial of buy gump needs #define or enum?
  if ( chr->client->ClientType & CLIENTTYPE_7090 )
    open_window->WriteFlipped<u16>( 0x00u );
  open_window.Send( chr->client );

  // Tell the client how much gold the character has, I guess
  send_full_statmsg( chr->client, chr );

  chr->client->restart();

  return new BLong( 1 );
}


void send_clear_vendorwindow( Client* client, Character* vendor )
{
  PktHelper::PacketOut<PktOut_3B> msg;
  msg->WriteFlipped<u16>( sizeof msg->buffer );
  msg->Write<u32>( vendor->serial_ext );
  msg->Write<u8>( PKTBI_3B::STATUS_NOTHING_BOUGHT );
  msg.Send( client );
}
unsigned int calculate_cost( Character* /*vendor*/, UContainer* for_sale, UContainer* bought,
                             PKTBI_3B* msg )
{
  uint64_t amt = 0, prev_amt = 0;

  int nitems = ( cfBEu16( msg->msglen ) - offsetof( PKTBI_3B, items ) ) / sizeof msg->items[0];

  for ( int i = 0; i < nitems; ++i )
  {
    u32 serial = cfBEu32( msg->items[i].item_serial );
    Item* item = for_sale->find( serial );
    if ( item == nullptr )
    {
      item = bought->find( serial );
      if ( item == nullptr )
        continue;
    }
    // const ItemDesc& id = find_itemdesc(item->objtype_);
    amt += cfBEu16( msg->items[i].number_bought ) * static_cast<uint64_t>( item->sellprice() );

    if ( amt < prev_amt || amt > INT_MAX )
    {
      return INT_MAX + 1U;
    }

    prev_amt = amt;
  }
  return static_cast<uint32_t>( amt );
}

void oldBuyHandler( Client* client, PKTBI_3B* msg )
{
  // Close the gump

  if ( msg->status == PKTBI_3B::STATUS_NOTHING_BOUGHT )
    return;

  UContainer* backpack = client->chr->backpack();
  if ( backpack == nullptr )
    return;

  NPC* vendor = client->gd->vendor.get();

  if ( vendor == nullptr || vendor->orphan() || vendor->serial_ext != msg->vendor_serial )
  {
    return;
  }
  client->gd->vendor.clear();

  UContainer* for_sale = client->gd->vendor_for_sale.get();
  if ( for_sale == nullptr || for_sale->orphan() )
  {
    return;
  }
  client->gd->vendor_for_sale.clear();

  UContainer* vendor_bought = client->gd->vendor_bought.get();
  if ( vendor_bought == nullptr || vendor_bought->orphan() )
  {
    return;
  }
  client->gd->vendor_bought.clear();

  unsigned int total_cost = calculate_cost( vendor, for_sale, vendor_bought, msg );

  if ( total_cost > INT_MAX )
  {
    POLLOG_INFO.Format(
        "\nWarning: Character 0x{:X} tried to buy items with an overflow from vendor 0x{:X}.\n" )
        << client->chr->serial << vendor->serial;
    send_clear_vendorwindow( client, vendor );
    return;
  }

  if ( total_cost > client->chr->gold_carried() )
  {
    send_clear_vendorwindow( client, vendor );
    return;
  }

  unsigned int amount_spent = 0;

  // buy each item individually
  // note, we know the buyer can afford it all.
  // the question is, can it all fit in his backpack?
  int nitems = ( cfBEu16( msg->msglen ) - offsetof( PKTBI_3B, items ) ) / sizeof msg->items[0];

  bool from_bought;
  for ( int i = 0; i < nitems; ++i )
  {
    from_bought = false;
    u32 serial = cfBEu32( msg->items[i].item_serial );
    Item* fs_item = for_sale->find( serial );
    if ( fs_item == nullptr )
    {
      fs_item = vendor_bought->find( serial );
      if ( fs_item == nullptr )
        continue;
      from_bought = true;
    }
    unsigned short numleft = cfBEu16( msg->items[i].number_bought );
    if ( numleft > fs_item->getamount() )
      numleft = fs_item->getamount();

    // const ItemDesc& id = find_itemdesc( fs_item->objtype_ );
    while ( numleft )
    {
      unsigned short num;
      if ( fs_item == nullptr )
        break;
      if ( fs_item->stackable() )
      {
        num = numleft;
      }
      else
      {
        num = 1;
      }
      Item* tobuy = nullptr;
      if ( fs_item->amount_to_remove_is_partial( num ) )
      {
        tobuy = fs_item->remove_part_of_stack( num );
      }
      else
      {
        if ( from_bought )
          vendor_bought->remove( fs_item );
        else
          for_sale->remove( fs_item );
        tobuy = fs_item;
        fs_item = nullptr;
      }

      // move the whole item
      ItemRef itemref(
          tobuy );  // dave 1/28/3 prevent item from being destroyed before function ends
      Item* existing_stack;
      if ( tobuy->stackable() &&
           ( existing_stack = backpack->find_addable_stack( tobuy ) ) != nullptr )
      {
        // dave 1-14-3 check backpack's insert scripts before moving.
        if ( backpack->can_insert_increase_stack( client->chr, UContainer::MT_CORE_MOVED,
                                                  existing_stack, tobuy->getamount(), tobuy ) )
        {
          if ( tobuy->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
          {
            continue;
          }
        }
        else  // put the item back just as if the pack had too many/too heavy items.
        {
          numleft = 0;
          if ( fs_item )
            fs_item->add_to_self( tobuy );
          else
              // FIXME : Add Grid Index Default Location Checks here.
              // Remember, if index fails, move to the ground.
              if ( from_bought )
            vendor_bought->add( tobuy );
          else
            for_sale->add( tobuy );
          continue;
        }
        numleft -= num;
        amount_spent += tobuy->sellprice() * num;
        u16 amtadded = tobuy->getamount();
        existing_stack->add_to_self( tobuy );
        update_item_to_inrange( existing_stack );

        backpack->on_insert_increase_stack( client->chr, UContainer::MT_CORE_MOVED, existing_stack,
                                            amtadded );
      }
      else if ( backpack->can_add( *tobuy ) )
      {
        numleft -= num;

        // dave 12-20 check backpack's insert scripts before moving.
        bool canInsert =
            backpack->can_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, tobuy );
        if ( tobuy->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
        {
          continue;
        }

        if ( !canInsert )  // put the item back just as if the pack had too many/too heavy items.
        {
          numleft = 0;
          if ( fs_item )
            fs_item->add_to_self( tobuy );
          else
              // FIXME : Add Grid Index Default Location Checks here.
              // Remember, if index fails, move to the ground.
              if ( from_bought )
            vendor_bought->add( tobuy );
          else
            for_sale->add( tobuy );
          continue;
        }

        // FIXME : Add Grid Index Default Location Checks here.
        // Remember, if index fails, move to the ground.
        backpack->add_at_random_location( tobuy );
        update_item_to_inrange( tobuy );
        amount_spent += tobuy->sellprice() * num;

        backpack->on_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, tobuy );
      }
      else
      {
        numleft = 0;
        if ( fs_item )
          fs_item->add_to_self( tobuy );
        else
        {
          // FIXME : Add Grid Index Default Location Checks here.
          // Remember, if index fails, move to the ground.
          if ( from_bought )
            vendor_bought->add( tobuy );
          else
            for_sale->add( tobuy );
        }
      }
    }
  }

  client->chr->spend_gold( amount_spent );

  std::unique_ptr<SourcedEvent> sale_event( new SourcedEvent( EVID_MERCHANT_SOLD, client->chr ) );
  sale_event->addMember( "amount", new BLong( amount_spent ) );
  vendor->send_event( sale_event.release() );

  send_clear_vendorwindow( client, vendor );
}

void buyhandler( Client* client, PKTBI_3B* msg )
{
  if ( !settingsManager.ssopt.scripted_merchant_handlers )
  {
    oldBuyHandler( client, msg );
    return;
  }
  // Close the gump

  if ( msg->status == PKTBI_3B::STATUS_NOTHING_BOUGHT )
    return;

  UContainer* backpack = client->chr->backpack();
  if ( backpack == nullptr )
  {
    return;
  }

  NPC* vendor = client->gd->vendor.get();
  if ( vendor == nullptr || vendor->orphan() || vendor->serial_ext != msg->vendor_serial )
  {
    return;
  }
  client->gd->vendor.clear();

  UContainer* for_sale = client->gd->vendor_for_sale.get();
  if ( for_sale == nullptr || for_sale->orphan() )
  {
    return;
  }
  client->gd->vendor_for_sale.clear();
  UContainer* vendor_bought = client->gd->vendor_bought.get();
  if ( vendor_bought == nullptr || vendor_bought->orphan() )
  {
    return;
  }
  client->gd->vendor_bought.clear();

  // buy each item individually
  // the question is, can it all fit in his backpack?
  // But who cares ... let the scripter(s) handle it!
  int nitems = ( cfBEu16( msg->msglen ) - offsetof( PKTBI_3B, items ) ) / sizeof msg->items[0];

  std::unique_ptr<ObjArray> items_bought( new ObjArray );
  for ( int i = 0; i < nitems; ++i )
  {
    Item* fs_item = for_sale->find( cfBEu32( msg->items[i].item_serial ) );
    if ( fs_item == nullptr )
    {
      fs_item = vendor_bought->find( cfBEu32( msg->items[i].item_serial ) );
      if ( fs_item == nullptr )
        continue;
    }
    unsigned short numleft = cfBEu16( msg->items[i].number_bought );

    if ( numleft > fs_item->getamount() )
      numleft = fs_item->getamount();

    std::unique_ptr<BStruct> entry( new BStruct );
    entry->addMember( "item", fs_item->make_ref() );
    entry->addMember( "amount", new BLong( numleft ) );

    items_bought->addElement( entry.release() );
  }

  std::unique_ptr<SourcedEvent> sale_event( new SourcedEvent( EVID_MERCHANT_SOLD, client->chr ) );
  sale_event->addMember( "shoppinglist", items_bought.release() );
  vendor->send_event( sale_event.release() );

  send_clear_vendorwindow( client, vendor );
}

bool send_vendorsell( Client* client, NPC* merchant, UContainer* sellfrom, UContainer* buyable,
                      bool send_aos_tooltip )
{
  unsigned short num_items = 0;
  PktHelper::PacketOut<PktOut_9E> msg;
  msg->offset += 2;
  msg->Write<u32>( merchant->serial_ext );
  msg->offset += 2;  // numitems

  UContainer::iterator buyable_itr, buyable_end;
  if ( buyable != nullptr )
    buyable_end = buyable->end();

  UContainer* cont = sellfrom;
  while ( cont != nullptr )
  {
    for ( UContainer::iterator itr = cont->begin(), end = cont->end(); itr != end; ++itr )
    {
      Item* item = GET_ITEM_PTR( itr );
      if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont2 = static_cast<UContainer*>( item );
        if ( cont2->count() )
          continue;
      }
      if ( item->newbie() )
        continue;
      unsigned int buyprice;
      if ( !item->getbuyprice( buyprice ) )
        continue;
      std::string desc = item->merchant_description();
      if ( msg->offset + desc.size() + 14 > sizeof msg->buffer )
      {
        return false;
      }
      if ( buyable != nullptr )
      {
        for ( buyable_itr = buyable->begin(); buyable_itr != buyable_end; ++buyable_itr )
        {
          Item* buyable_item = GET_ITEM_PTR( buyable_itr );
          if ( buyable_item->objtype_ == item->objtype_ )
            break;
        }
        if ( buyable_itr == buyable_end )
          continue;
      }

      msg->Write<u32>( item->serial_ext );
      msg->WriteFlipped<u16>( item->graphic );
      msg->WriteFlipped<u16>( item->color );
      msg->WriteFlipped<u16>( item->getamount() );
      msg->WriteFlipped<u16>( buyprice );
      msg->WriteFlipped<u16>( desc.size() );
      msg->Write( desc.c_str(), static_cast<u16>( desc.size() ), false );  // No null term
      ++num_items;

      if ( send_aos_tooltip )
        SendAOSTooltip( client, item, true );
    }

    cont = nullptr;
  }
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg->offset += 4;
  msg->WriteFlipped<u16>( num_items );
  msg.Send( client, len );
  return true;
}

BObjectImp* UOExecutorModule::mf_SendSellWindow( /* character, vendor, i1, i2, i3, flags */ )
{
  Character *chr, *mrchnt;
  NPC* merchant;
  Item* wi1a;
  Item* wi1b;
  Item* wi1c;
  int flags;
  UContainer* merchant_bought;
  UContainer* merchant_buyable = nullptr;

  if ( !( getCharacterParam( exec, 0, chr ) && getCharacterParam( exec, 1, mrchnt ) &&
          getItemParam( exec, 2, wi1a ) && getItemParam( exec, 3, wi1b ) &&
          getItemParam( exec, 4, wi1c ) && getParam( 5, flags ) ) )
  {
    return new BError( "A parameter was invalid" );
  }

  if ( !chr->has_active_client() )
  {
    return new BError( "No client connected to character" );
  }

  if ( mrchnt->isa( UOBJ_CLASS::CLASS_NPC ) )
  {
    merchant = static_cast<NPC*>( mrchnt );
  }
  else
  {
    return new BError( "Parameter 1 invalid" );
  }
  if ( wi1b->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    merchant_bought = static_cast<UContainer*>( wi1b );
  }
  else
  {
    return new BError( "Parameter 2 must be a container" );
  }

  if ( flags & VENDOR_BUYABLE_CONTAINER_FILTER )
  {
    if ( wi1c->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      merchant_buyable = static_cast<UContainer*>( wi1c );
    }
    else
    {
      return new BError( "Parameter 3 must be a container" );
    }
  }

  if ( chr->backpack() == nullptr )
  {
    return new BError( "Character has no backpack" );
  }

  unsigned char save_layer = wi1a->layer;
  wi1a->layer = LAYER_VENDOR_FOR_SALE;
  send_wornitem( chr->client, merchant, wi1a );
  wi1a->layer = save_layer;

  save_layer = wi1b->layer;
  wi1b->layer = LAYER_VENDOR_PLAYER_ITEMS;
  send_wornitem( chr->client, merchant, wi1b );
  wi1b->layer = save_layer;

  save_layer = wi1c->layer;
  wi1c->layer = LAYER_VENDOR_BUYABLE_ITEMS;
  send_wornitem( chr->client, merchant, wi1c );
  wi1c->layer = save_layer;

  bool send_aos_tooltip = flags & VENDOR_SEND_AOS_TOOLTIP ? true : false;

  send_vendorsell( chr->client, merchant, chr->backpack(), merchant_buyable, send_aos_tooltip );

  chr->client->gd->vendor.set( merchant );
  chr->client->gd->vendor_bought.set( merchant_bought );

  return new BLong( 1 );
}

extern BObjectImp* _create_item_in_container( UContainer* cont, const ItemDesc* descriptor,
                                              unsigned short amount, bool force_stacking,
                                              UOExecutorModule* uoemod );
// player selling to vendor
void oldSellHandler( Client* client, PKTIN_9F* msg )
{
  UContainer* backpack = client->chr->backpack();
  if ( backpack == nullptr )
    return;

  NPC* vendor = client->gd->vendor.get();

  if ( vendor == nullptr || vendor->orphan() || vendor->serial_ext != msg->vendor_serial )
  {
    client->gd->vendor.clear();
    client->gd->vendor_bought.clear();
    return;
  }

  UContainer* vendor_bought = client->gd->vendor_bought.get();
  if ( vendor_bought == nullptr || vendor_bought->orphan() )
  {
    client->gd->vendor.clear();
    client->gd->vendor_bought.clear();
    return;
  }

  unsigned int cost = 0;
  int num_items = cfBEu16( msg->num_items );
  Clib::sanitize_upperlimit( &num_items, ( 0xFFFF - 9 ) / 6 );

  for ( int i = 0; i < num_items; ++i )
  {
    u32 serial = cfBEu32( msg->items[i].serial );
    u16 amount = cfBEu16( msg->items[i].amount );

    unsigned int buyprice;

    Item* item = backpack->find_toplevel( serial );
    if ( item == nullptr )
      return;
    if ( item->newbie() )
      continue;
    if ( item->inuse() )
      continue;
    if ( !item->getbuyprice( buyprice ) )
      continue;
    if ( amount > item->getamount() )
      amount = item->getamount();
    Item* remainder_not_sold = nullptr;
    if ( item->amount_to_remove_is_partial( amount ) )
      remainder_not_sold = item->slice_stacked_item( amount );

    if ( vendor_bought->can_add( *item ) )
    {
      u16 tx, ty;
      vendor_bought->get_random_location( &tx, &ty );
      backpack->remove( item );
      if ( remainder_not_sold != nullptr )
      {
        // FIXME : Add Grid Index Default Location Checks here.
        // Remember, if index fails, move to the ground.
        backpack->add( remainder_not_sold );
        update_item_to_inrange( remainder_not_sold );
        remainder_not_sold = nullptr;
      }
      item->x = tx;
      item->y = ty;
      item->z = 0;
      // FIXME : Add Grid Index Default Location Checks here.
      // Remember, if index fails, move to the ground.
      vendor_bought->add( item );
      update_item_to_inrange( item );
      cost += buyprice * amount;
    }

    if ( remainder_not_sold != nullptr )
    {
      item->add_to_self( remainder_not_sold );
      update_item_to_inrange( item );
    }
  }

  // dave added 12-19. If no items are sold don't create any gold in the player's pack!
  if ( cost > 0 )
  {
    // dave added 12-21, create stacks of 60k gold instead of one huge, invalid stack.
    unsigned int temp_cost = cost;
    while ( temp_cost > 60000 )
    {
      BObject o( _create_item_in_container( backpack, &find_itemdesc( UOBJ_GOLD_COIN ),
                                            static_cast<unsigned short>( 60000 ), false,
                                            nullptr ) );
      temp_cost -= 60000;
    }
    if ( temp_cost > 0 )
    {
      BObject o( _create_item_in_container( backpack, &find_itemdesc( UOBJ_GOLD_COIN ),
                                            static_cast<unsigned short>( temp_cost ), false,
                                            nullptr ) );
    }
  }
  std::unique_ptr<SourcedEvent> sale_event( new SourcedEvent( EVID_MERCHANT_BOUGHT, client->chr ) );
  sale_event->addMember( "amount", new BLong( cost ) );
  vendor->send_event( sale_event.release() );

  client->pause();

  send_full_statmsg( client, client->chr );

  send_clear_vendorwindow( client, vendor );

  client->gd->vendor_bought.clear();
  client->gd->vendor.clear();
  client->restart();
}

void sellhandler( Client* client, PKTIN_9F* msg )
{
  if ( !settingsManager.ssopt.scripted_merchant_handlers )
  {
    oldSellHandler( client, msg );
    return;
  }
  UContainer* backpack = client->chr->backpack();
  if ( backpack == nullptr )
    return;

  NPC* vendor = client->gd->vendor.get();
  if ( vendor == nullptr || vendor->orphan() || vendor->serial_ext != msg->vendor_serial )
  {
    client->gd->vendor.clear();
    client->gd->vendor_bought.clear();
    return;
  }

  UContainer* vendor_bought = client->gd->vendor_bought.get();
  if ( vendor_bought == nullptr || vendor_bought->orphan() )
  {
    client->gd->vendor.clear();
    client->gd->vendor_bought.clear();
    return;
  }

  int num_items = cfBEu16( msg->num_items );
  std::unique_ptr<ObjArray> items_sold( new ObjArray );
  Clib::sanitize_upperlimit( &num_items, ( 0xFFFF - 9 ) / 6 );
  for ( int i = 0; i < num_items; ++i )
  {
    u32 serial = cfBEu32( msg->items[i].serial );
    u32 amount = cfBEu16( msg->items[i].amount );

    Item* item = backpack->find_toplevel( serial );

    if ( item == nullptr )
      return;
    if ( item->newbie() )
      continue;
    if ( item->inuse() )
      continue;
    if ( amount > item->getamount() )
      amount = item->getamount();

    std::unique_ptr<BStruct> entry( new BStruct );
    entry->addMember( "item", item->make_ref() );
    entry->addMember( "amount", new BLong( amount ) );

    items_sold->addElement( entry.release() );
  }
  std::unique_ptr<SourcedEvent> sale_event( new SourcedEvent( EVID_MERCHANT_BOUGHT, client->chr ) );
  sale_event->addMember( "shoppinglist", items_sold.release() );
  vendor->send_event( sale_event.release() );

  send_clear_vendorwindow( client, vendor );
  client->gd->vendor.clear();
  client->gd->vendor_bought.clear();
}

//
//  "GUMP" Functions
//
BObjectImp* UOExecutorModule::mf_SendDialogGump()
{
  /*
   Client* client,
   const char* layout,
   const char* strings[]
   */
  int x, y, flags, gump_id;
  Character* chr;
  ObjArray* layout_arr;
  ObjArray* data_arr;
  if ( !( getCharacterParam( exec, 0, chr ) && exec.getObjArrayParam( 1, layout_arr ) &&
          exec.getObjArrayParam( 2, data_arr ) && exec.getParam( 3, x ) && exec.getParam( 4, y ) &&
          exec.getParam( 5, flags ) && exec.getParam( 6, gump_id ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached" );
  }

  // Now determine the id to be assigned to the gump
  u32 gumpid;
  if ( gump_id )
  {
    // Validate and use the id requested by the script
    if ( gump_id < 1 )
      return new BError( "GumpID out of range" );
    gumpid = (u32)gump_id;
    if ( gumpid >= ScriptScheduler::PID_MIN )
      return new BError( "GumpID out of range" );
  }
  else
  {
    // For keeping backward compatibility: we do not know how many scripts do rely
    // on the fact that pid == gumpid when autogenerated
    gumpid = this->uoexec.pid();
  }

  /*
  if (chr->client->gd->gump_uoemod != nullptr)
  {
  return new BError( "Client already has an active gump" );
  }
  */
  if ( ( chr->client->ClientType & CLIENTTYPE_UOSA ) || ( chr->client->IsUOKRClient() ) ||
       ( ( !( flags & SENDDIALOGMENU_FORCE_OLD ) ) &&
         ( chr->client->compareVersion( CLIENT_VER_5000 ) ) ) )
    return internal_SendCompressedGumpMenu( chr, layout_arr, data_arr, x, y, gumpid );
  else
    return internal_SendUnCompressedGumpMenu( chr, layout_arr, data_arr, x, y, gumpid );
}


BObjectImp* UOExecutorModule::internal_SendUnCompressedGumpMenu( Character* chr,
                                                                 ObjArray* layout_arr,
                                                                 ObjArray* data_arr, int x, int y,
                                                                 u32 gumpid )
{
  PktHelper::PacketOut<PktOut_B0> msg;
  msg->offset += 2;
  msg->Write<u32>( chr->serial_ext );
  msg->WriteFlipped<u32>( gumpid );
  msg->WriteFlipped<u32>( static_cast<u32>( x ) );
  msg->WriteFlipped<u32>( static_cast<u32>( y ) );
  u16 pos = msg->offset;
  msg->offset += 2;  // layoutlen
  size_t layoutlen = 0;
  for ( unsigned i = 0; i < layout_arr->ref_arr.size(); ++i )
  {
    BObject* bo = layout_arr->ref_arr[i].get();
    if ( bo == nullptr )
      continue;
    BObjectImp* imp = bo->impptr();
    std::string s = imp->getStringRep();

    size_t addlen = 4 + s.length();
    layoutlen += addlen;
    if ( msg->offset + addlen > sizeof msg->buffer )
    {
      return new BError( "Buffer length exceeded" );
    }
    msg->Write( "{ ", 2, false );
    msg->Write( s.c_str(), static_cast<u16>( s.length() ), false );
    msg->Write( " }", 2, false );
  }

  if ( msg->offset + 1 > static_cast<int>( sizeof msg->buffer ) )
  {
    return new BError( "Buffer length exceeded" );
  }
  msg->offset++;  // nullterm
  layoutlen++;

  u16 len = msg->offset;
  msg->offset = pos;
  msg->WriteFlipped<u16>( layoutlen );
  msg->offset = len;

  pos = msg->offset;

  if ( msg->offset + 2 > static_cast<int>( sizeof msg->buffer ) )
  {
    return new BError( "Buffer length exceeded" );
  }
  msg->offset += 2;  // numlines

  u16 numlines = 0;
  for ( unsigned i = 0; i < data_arr->ref_arr.size(); ++i )
  {
    BObject* bo = data_arr->ref_arr[i].get();
    if ( bo == nullptr )
      continue;
    BObjectImp* imp = bo->impptr();
    std::string s = imp->getStringRep();

    ++numlines;
    auto utf16 = Bscript::String::toUTF16( s );

    if ( msg->offset + 2 + utf16.size() * 2 > sizeof msg->buffer )
    {
      return new BError( "Buffer length exceeded" );
    }

    msg->WriteFlipped<u16>( utf16.size() );
    msg->WriteFlipped( utf16, false );
  }

  if ( msg->offset + 1 > static_cast<int>( sizeof msg->buffer ) )
  {
    return new BError( "Buffer length exceeded" );
  }
  msg->offset++;  // nullterm

  len = msg->offset;
  msg->offset = pos;
  msg->WriteFlipped<u16>( numlines );
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SendDialogGump():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  msg.Send( chr->client, len );
  chr->client->gd->add_gumpmod( this, gumpid );
  // old_gump_uoemod = this;
  gump_chr = chr;

  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::internal_SendCompressedGumpMenu( Character* chr, ObjArray* layout_arr,
                                                               ObjArray* data_arr, int x, int y,
                                                               u32 gumpid )
{
  PktHelper::PacketOut<PktOut_DD> msg;
  PktHelper::PacketOut<PktOut_DD> bfr;  // compress buffer
  bfr->offset = 0;
  msg->offset += 2;
  msg->Write<u32>( chr->serial_ext );
  msg->WriteFlipped<u32>( gumpid );
  msg->WriteFlipped<u32>( static_cast<u16>( x ) );
  msg->WriteFlipped<u32>( static_cast<u16>( y ) );
  msg->offset += 8;  // u32 layout_clen,layout_dlen

  u32 layoutdlen = 0;

  for ( unsigned i = 0; i < layout_arr->ref_arr.size(); ++i )
  {
    BObject* bo = layout_arr->ref_arr[i].get();
    if ( bo == nullptr )
      continue;
    BObjectImp* imp = bo->impptr();
    std::string s = imp->getStringRep();

    size_t addlen = 4 + s.length();
    if ( layoutdlen + addlen > sizeof bfr->buffer )
    {
      return new BError( "Buffer length exceeded" );
    }
    layoutdlen += static_cast<u32>( addlen );
    bfr->Write( "{ ", 2, false );
    bfr->Write( s.c_str(), static_cast<u16>( s.length() ), false );
    bfr->Write( " }", 2, false );
  }
  if ( layoutdlen + 1 > static_cast<u32>( sizeof bfr->buffer ) )
  {
    return new BError( "Buffer length exceeded" );
  }
  layoutdlen++;
  bfr->offset++;  // nullterm

  unsigned long cbuflen =
      ( ( (unsigned long)( ( (float)( layoutdlen ) ) * 1.001f ) ) + 12 );  // as per zlib spec
  if ( cbuflen > ( (unsigned long)( 0xFFFF - msg->offset ) ) )
  {
    return new BError( "Compression error" );
  }

  if ( compress2( reinterpret_cast<unsigned char*>( msg->getBuffer() ), &cbuflen,
                  reinterpret_cast<unsigned char*>( &bfr->buffer ), layoutdlen,
                  Z_DEFAULT_COMPRESSION ) != Z_OK )
  {
    return new BError( "Compression error" );
  }
  msg->offset -= 8;
  msg->WriteFlipped<u32>( cbuflen + 4 );
  msg->WriteFlipped<u32>( layoutdlen );
  msg->offset += static_cast<u16>( cbuflen );

  bfr->offset = 0;

  u32 numlines = 0;
  u32 datadlen = 0;

  for ( unsigned i = 0; i < data_arr->ref_arr.size(); ++i )
  {
    BObject* bo = data_arr->ref_arr[i].get();
    if ( bo == nullptr )
      continue;
    BObjectImp* imp = bo->impptr();
    std::string s = imp->getStringRep();

    auto utf16 = Bscript::String::toUTF16( s );
    ++numlines;
    size_t addlen = ( utf16.size() + 1 ) * 2;
    if ( datadlen + addlen > sizeof bfr->buffer )
    {
      return new BError( "Buffer length exceeded" );
    }
    datadlen += static_cast<u32>( addlen );
    bfr->WriteFlipped<u16>( utf16.size() );
    bfr->WriteFlipped( utf16, false );
  }
  msg->WriteFlipped<u32>( numlines );
  if ( numlines != 0 )
  {
    msg->offset += 8;  // u32 text_clen, text_dlen

    cbuflen = ( ( (unsigned long)( ( (float)( datadlen ) ) * 1.001f ) ) + 12 );  // as per zlib spec
    if ( cbuflen > ( (unsigned long)( 0xFFFF - msg->offset ) ) )
    {
      return new BError( "Compression error" );
    }
    if ( compress2( reinterpret_cast<unsigned char*>( msg->getBuffer() ), &cbuflen,
                    reinterpret_cast<unsigned char*>( &bfr->buffer ), datadlen,
                    Z_DEFAULT_COMPRESSION ) != Z_OK )
    {
      return new BError( "Compression error" );
    }

    msg->offset -= 8;
    msg->WriteFlipped<u32>( cbuflen + 4 );
    msg->WriteFlipped<u32>( datadlen );
    msg->offset += static_cast<u16>( cbuflen );
  }
  else
    msg->Write<u32>( 0u );
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SendDialogGump():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  msg.Send( chr->client, len );
  chr->client->gd->add_gumpmod( this, gumpid );
  // old_gump_uoemod = this;
  gump_chr = chr;

  return new BLong( 0 );
}

class BIntHash final : public BObjectImp
{
public:
  BIntHash();
  BIntHash( const BIntHash& );
  void add( int key, BObjectImp* value );
  virtual BObjectRef get_member( const char* membername ) override;
  virtual BObjectRef OperSubscript( const BObject& obj ) override;
  virtual BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

private:
  typedef std::map<int, BObjectRef> Contents;
  Contents contents_;

  // not implemented:
  BIntHash& operator=( const BIntHash& );
};

BIntHash::BIntHash() : BObjectImp( OTUnknown ), contents_() {}

BIntHash::BIntHash( const BIntHash& ih ) : BObjectImp( OTUnknown ), contents_( ih.contents_ ) {}

BObjectImp* BIntHash::copy() const
{
  return new BIntHash( *this );
}

size_t BIntHash::sizeEstimate() const
{
  size_t size =
      sizeof( BIntHash ) +
      ( sizeof( int ) + sizeof( BObjectRef ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) * contents_.size();
  for ( const auto& p : contents_ )
  {
    size += p.second.sizeEstimate();
  }
  return size;
}

std::string BIntHash::getStringRep() const
{
  return "<inthash>";
}

void BIntHash::add( int key, BObjectImp* value )
{
  // contents_.insert( Contents::value_type( key, value ) );
  contents_[key].set( new BObject( value ) );
}

BObjectRef BIntHash::get_member( const char* membername )
{
  if ( stricmp( membername, "keys" ) == 0 )
  {
    ObjArray* arr = new ObjArray;
    BObject obj( arr );

    Contents::const_iterator itr, end;
    for ( itr = contents_.begin(), end = contents_.end(); itr != end; ++itr )
    {
      int key = ( *itr ).first;
      arr->addElement( new BLong( key ) );
    }

    return BObjectRef( obj.impptr() );
  }
  else
  {
    return BObjectRef( new BError( "member not found" ) );
  }
}

BObjectRef BIntHash::OperSubscript( const BObject& obj )
{
  const BObjectImp& objimp = obj.impref();
  if ( objimp.isa( OTLong ) )
  {
    const BLong& lng = static_cast<const BLong&>( objimp );
    Contents::iterator itr = contents_.find( lng.value() );
    if ( itr != contents_.end() )
    {
      BObjectRef& oref = ( *itr ).second;
      return BObjectRef( oref.get()->impptr() );
    }
    else
    {
      return BObjectRef( new BError( "Key not found in inthash" ) );
    }
  }
  else
  {
    return BObjectRef( new BError( "Incorrect type used as subscript to inthash" ) );
  }
}

void clear_gumphandler( Client* client, UOExecutorModule* uoemod )
{
  uoemod->uoexec.revive();
  uoemod->gump_chr = nullptr;
  client->gd->remove_gumpmods( uoemod );
}

BObjectImp* UOExecutorModule::mf_CloseGump( /* who, pid, response := 0 */ )
{
  Character* chr;
  unsigned int pid;
  BObjectImp* resp;

  if ( !( getCharacterParam( exec, 0, chr ) && exec.getParam( 1, pid ) &&
          ( getParamImp( 2, resp ) ) ) )
  {
    return new BError( "Invalid parameter" );
  }

  if ( !chr->has_active_client() )
    return new BError( "No client attached" );

  Client* client = chr->client;

  UOExecutorModule* uoemod = client->gd->find_gumpmod( pid );
  if ( uoemod == nullptr )
  {
    return new BError( "Couldnt find script" );
  }

  PktHelper::PacketOut<PktOut_BF_Sub4> msg;
  msg->WriteFlipped<u16>( 13u );
  msg->offset += 2;
  msg->WriteFlipped<u32>( pid );
  msg->offset += 4;  // buttonid

  msg.Send( client );

  uoemod->uoexec.ValueStack.back().set( new BObject( resp ) );
  clear_gumphandler( client, uoemod );

  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_CloseWindow( /* chr, type, obj */ )
{
  Character* chr;
  unsigned int type;
  UObject* obj;

  if ( !getCharacterParam( exec, 0, chr ) || !getParam( 1, type ) ||
       !getUObjectParam( exec, 2, obj ) )
    return new BError( "Invalid parameter" );

  if ( !chr->has_active_client() )
    return new BError( "No client attached" );

  if ( type == PKTBI_BF_16::CONTAINER )
  {
    if ( !obj->script_isa( POLCLASS_CONTAINER ) )
      return new BError( "Invalid object, has to be a containerRef" );
  }
  else if ( type == PKTBI_BF_16::PAPERDOLL || type == PKTBI_BF_16::STATUS ||
            type == PKTBI_BF_16::CHARPROFILE )
  {
    if ( !obj->script_isa( POLCLASS_MOBILE ) )
      return new BError( "Invalid object, has to be a mobRef" );
  }
  else
    return new BError( "Invalid type" );

  PktHelper::PacketOut<PktOut_BF_Sub16> msg;
  msg->WriteFlipped<u16>( 13u );
  msg->offset += 2;  // sub
  msg->WriteFlipped<u32>( type );
  msg->Write<u32>( obj->serial_ext );

  msg.Send( chr->client );

  return new BLong( 1 );
}

void gumpbutton_handler( Client* client, PKTIN_B1* msg )
{
  const u32 VIRTUE_GUMP_ID = 0x1CD;

  char* msgbuf = reinterpret_cast<char*>( msg );
  PKTIN_B1::HEADER* hdr = reinterpret_cast<PKTIN_B1::HEADER*>( msg );
  unsigned short msglen = cfBEu16( hdr->msglen );

  u32 gumpid = cfBEu32( hdr->dialogid );
  u32 buttonid = cfBEu32( hdr->buttonid );

  // When the player double-clicks the virtue pentagram icon in its paperdoll, an unexpected
  // PKTIN_B1 gump reply packet with dialogid == 0x01CD and buttonid == 1 will be sent to us
  if ( gumpid == VIRTUE_GUMP_ID )
  {
    if ( hdr->serial == client->chr->serial_ext )
    {
      if ( buttonid == 1 )
      {
        PKTIN_B1::INTS_HEADER* intshdr_ = reinterpret_cast<PKTIN_B1::INTS_HEADER*>( hdr + 1 );
        if ( cfBEu32( intshdr_->count ) == 1 )
        {
          PKTIN_B1::INT_ENTRY* intentries_ = reinterpret_cast<PKTIN_B1::INT_ENTRY*>( intshdr_ + 1 );
          if ( intentries_->value == client->chr->serial_ext )
          {
            ref_ptr<EScriptProgram> prog = find_script(
                "misc/virtuebutton", true, Plib::systemstate.config.cache_interactive_scripts );
            if ( prog.get() != nullptr )
              client->chr->start_script( prog.get(), false );
            return;
          }
        }
      }
    }
  }


  UOExecutorModule* uoemod = client->gd->find_gumpmod( gumpid );
  if ( uoemod == nullptr )
  {
    if ( Plib::systemstate.config.show_warning_gump )
    {
      POLLOG_INFO.Format(
          "\nWarning: Character 0x{:X} sent an unexpected gump menu selection. Gump ID 0x{:X}, "
          "button ID 0x{:X}\n" )
          << client->chr->serial << gumpid << buttonid;
    }
    return;
  }


  if ( msglen <=
       0x0f )  // Using == instead of <= should do the trick, but i think <= is more robust
  {
    // The virtue button packet is 15 bytes long: it will not carry a switchcount/INTS_HEADER,
    // so prevent full processing code to overflow and save some CPU cycles meanwhile.
    // Maybe other packets could be that short too?
    if ( buttonid == 0 )
    {
      uoemod->uoexec.ValueStack.back().set( new BObject( new BLong( 0 ) ) );
    }
    else
    {
      std::unique_ptr<BIntHash> hash( new BIntHash );
      hash->add( 0, new BLong( buttonid ) );
      hash->add( buttonid, new BLong( 1 ) );
      uoemod->uoexec.ValueStack.back().set( new BObject( hash.release() ) );
    }
  }
  else
  {
    // Process rest of the packet
    PKTIN_B1::INTS_HEADER* intshdr = reinterpret_cast<PKTIN_B1::INTS_HEADER*>( hdr + 1 );
    u32 ints_count = cfBEu32( intshdr->count );
    unsigned stridx = sizeof( PKTIN_B1::HEADER ) + sizeof( PKTIN_B1::INTS_HEADER ) +
                      sizeof( PKTIN_B1::INT_ENTRY ) * ints_count +
                      sizeof( PKTIN_B1::STRINGS_HEADER );
    if ( stridx > msglen )
    {
      ERROR_PRINT << "Blech! B1 message specified too many ints!\n";
      clear_gumphandler( client, uoemod );
      return;
    }
    PKTIN_B1::INT_ENTRY* intentries = reinterpret_cast<PKTIN_B1::INT_ENTRY*>( intshdr + 1 );
    PKTIN_B1::STRINGS_HEADER* strhdr =
        reinterpret_cast<PKTIN_B1::STRINGS_HEADER*>( intentries + ints_count );
    u32 strings_count = cfBEu32( strhdr->count );
    // even if this is ok, it could still overflow.  Have to check each string.
    if ( stridx + ( sizeof( PKTIN_B1::STRING_ENTRY ) - 1 ) * strings_count > msglen + 1u )
    {
      ERROR_PRINT << "Client (Account " << client->acct->name() << ", Character "
                  << client->chr->name()
                  << ") Blech! B1 message specified too many ints and/or strings!\n";
      uoemod->uoexec.ValueStack.back().set(
          new BObject( new BError( "B1 message specified too many ints and/or strings." ) ) );
      clear_gumphandler( client, uoemod );
      return;
    }

    if ( ints_count == 0 && strings_count == 0 && buttonid == 0 )
    {
      uoemod->uoexec.ValueStack.back().set( new BObject( new BLong( 0 ) ) );
    }
    else
    {
      std::unique_ptr<BIntHash> hash( new BIntHash );
      hash->add( 0, new BLong( buttonid ) );
      hash->add( buttonid, new BLong( 1 ) );
      for ( unsigned i = 0; i < ints_count; ++i )
      {
        hash->add( cfBEu32( intentries[i].value ), new BLong( 1 ) );
      }
      for ( unsigned i = 0; i < strings_count; ++i )
      {
        PKTIN_B1::STRING_ENTRY* strentry =
            reinterpret_cast<PKTIN_B1::STRING_ENTRY*>( msgbuf + stridx );
        unsigned short length = cfBEu16( strentry->length );
        stridx += offsetof( PKTIN_B1::STRING_ENTRY, data ) + length * 2;
        if ( stridx > msglen )
        {
          ERROR_PRINT << "Client (Account " << client->acct->name() << ", Character "
                      << client->chr->name()
                      << ") Blech! B1 message strings overflow message buffer!\n";
          break;
        }

        std::string str = Clib::tostring( cfBEu16( strentry->tag ) ) + ": " +
                          Bscript::String::fromUTF16( strentry->data, length, true );
        // oops we're throwing away tag!
        hash->add( cfBEu16( strentry->tag ), new String( str ) );
      }
      uoemod->uoexec.ValueStack.back().set( new BObject( hash.release() ) );
    }
  }

  clear_gumphandler( client, uoemod );
}

BObjectImp* UOExecutorModule::mf_SendTextEntryGump()
{
  Character* chr;
  const String* line1;
  int cancel;
  int style;
  int maximum;
  const String* line2;

  if ( !( getCharacterParam( exec, 0, chr ) && exec.getStringParam( 1, line1 ) &&
          exec.getParam( 2, cancel ) && exec.getParam( 3, style ) && exec.getParam( 4, maximum ) &&
          exec.getStringParam( 5, line2 ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  if ( !chr->has_active_client() )
  {
    return new BError( "No client attached" );
  }

  PktHelper::PacketOut<PktOut_AB> msg;
  msg->offset += 2;
  msg->Write<u32>( chr->serial_ext );
  msg->offset += 2;  // u8 type,index

  size_t numbytes = line1->length() + 1;
  if ( numbytes > 256 )
    numbytes = 256;
  msg->WriteFlipped<u16>( numbytes );
  msg->Write( line1->data(), static_cast<u16>( numbytes ) );  // null-terminated

  msg->Write<u8>( static_cast<u8>( cancel ) );
  msg->Write<u8>( static_cast<u8>( style ) );
  msg->WriteFlipped<s32>( maximum );
  numbytes = line2->length() + 1;
  if ( numbytes > 256 )
    numbytes = 256;
  msg->WriteFlipped<u16>( numbytes );
  msg->Write( line2->data(), static_cast<u16>( numbytes ) );  // null-terminated
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SendTextEntryGump():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  msg.Send( chr->client, len );
  chr->client->gd->textentry_uoemod = this;
  textentry_chr = chr;

  return new BLong( 0 );
}

void handle_textentry( Client* client, PKTIN_AC* msg )
{
  if ( client->gd->textentry_uoemod == nullptr )
  {
    ERROR_PRINT << "Client (Account " << client->chr->acct->name() << ", Character "
                << client->chr->name() << ")used out-of-sequence textentry command?\n";
    return;
  }
  BObjectImp* resimp = new BLong( 0 );
  if ( msg->retcode == PKTIN_AC::RETCODE_OKAY )
  {
    unsigned short datalen = cfBEu16( msg->datalen );
    if ( datalen >= 1 && datalen <= 256 && msg->data[datalen - 1] == '\0' )
    {
      // dave added isprint checking 4/13/3
      bool ok = true;
      --datalen;  // don't include null terminator (already checked)
      for ( int i = 0; i < datalen; ++i )
      {
        if ( !isprint( msg->data[i] ) )
        {
          ok = false;
          break;
        }
      }
      if ( ok )
      {
        resimp = new String( msg->data, datalen );
      }
    }
  }

  client->gd->textentry_uoemod->uoexec.ValueStack.back().set( new BObject( resimp ) );
  client->gd->textentry_uoemod->uoexec.revive();
  client->gd->textentry_uoemod->textentry_chr = nullptr;
  client->gd->textentry_uoemod = nullptr;
}

class PolCore final : public BObjectImp
{
public:
  PolCore();
  virtual BObjectRef get_member( const char* membername ) override;
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  virtual BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override { return sizeof( PolCore ); }
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;

private:
  // not implemented:
  PolCore& operator=( const PolCore& );
};

PolCore::PolCore() : BObjectImp( OTPolCoreRef ) {}

BObjectImp* PolCore::copy() const
{
  return new PolCore;
}

std::string PolCore::getStringRep() const
{
  return "<polcore>";
}

const char* PolCore::typeOf() const
{
  return "PolCoreRef";
}
u8 PolCore::typeOfInt() const
{
  return OTPolCoreRef;
}

BObjectImp* GetPackageList()
{
  std::unique_ptr<ObjArray> arr( new ObjArray );
  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    arr->addElement( new String( pkg->name() ) );
  }
  return arr.release();
}

void add_script( ObjArray* arr, UOExecutor* uoexec, const char* /*state*/ )
{
  arr->addElement( new ScriptExObjImp( uoexec ) );
}

BObjectImp* GetRunningScriptList()
{
  ObjArray* arr = new ObjArray;

  const ExecList& runlist = scriptScheduler.getRunlist();
  const ExecList& ranlist = scriptScheduler.getRanlist();

  for ( const auto& script : ranlist )
  {
    add_script( arr, script, "Running" );
  }
  for ( const auto& script : runlist )
  {
    add_script( arr, script, "Running" );
  }
  return arr;
}

BObjectImp* GetAllScriptList()
{
  ObjArray* arr = new ObjArray;

  const ExecList& runlist = scriptScheduler.getRunlist();
  const ExecList& ranlist = scriptScheduler.getRanlist();
  const HoldList& holdlist = scriptScheduler.getHoldlist();
  const NoTimeoutHoldList& notimeoutholdlist = scriptScheduler.getNoTimeoutHoldlist();

  for ( const auto& script : ranlist )
  {
    add_script( arr, script, "Running" );
  }
  for ( const auto& script : runlist )
  {
    add_script( arr, script, "Running" );
  }
  for ( const auto& script : holdlist )
  {
    add_script( arr, ( script ).second, "Sleeping" );
  }
  for ( const auto& script : notimeoutholdlist )
  {
    add_script( arr, script, "Sleeping" );
  }
  return arr;
}

BObjectImp* GetScriptProfiles()
{
  std::unique_ptr<ObjArray> arr = Clib::make_unique<ObjArray>();

  u64 total_instr = 0;
  for ( const auto& source : scriptScheduler.scrstore )
  {
    EScriptProgram* eprog = ( ( source ).second ).get();
    total_instr += eprog->instr_cycles;
  }

  for ( const auto& src : scriptScheduler.scrstore )
  {
    EScriptProgram* eprog = ( ( src ).second ).get();


    std::unique_ptr<BStruct> elem = Clib::make_unique<BStruct>();
    elem->addMember( "name", new String( eprog->name ) );
    elem->addMember( "instr", new Double( static_cast<double>( eprog->instr_cycles ) ) );
    elem->addMember( "invocations", new BLong( eprog->invocations ) );
    u64 cycles_per_invoc = eprog->instr_cycles / ( eprog->invocations ? eprog->invocations : 1 );
    elem->addMember( "instr_per_invoc", new Double( static_cast<double>( cycles_per_invoc ) ) );
    double cycle_percent =
        total_instr != 0 ? ( static_cast<double>( eprog->instr_cycles ) / total_instr * 100.0 ) : 0;
    elem->addMember( "instr_percent", new Double( cycle_percent ) );

    arr->addElement( elem.release() );
  }
  return arr.release();
}

BObjectImp* GetIoStatsObj( const IOStats& stats )
{
  std::unique_ptr<BStruct> arr( new BStruct );

  ObjArray* sent = new ObjArray;
  arr->addMember( "sent", sent );

  ObjArray* received = new ObjArray;
  arr->addMember( "received", received );

  for ( unsigned i = 0; i < 256; ++i )
  {
    std::unique_ptr<BStruct> elem = Clib::make_unique<BStruct>();
    elem->addMember( "count", new BLong( stats.sent[i].count ) );
    elem->addMember( "bytes", new BLong( stats.sent[i].bytes ) );
    sent->addElement( elem.release() );
  }

  for ( unsigned i = 0; i < 256; ++i )
  {
    std::unique_ptr<BStruct> elem( new BStruct );
    elem->addMember( "count", new BLong( stats.received[i].count ) );
    elem->addMember( "bytes", new BLong( stats.received[i].bytes ) );
    received->addElement( elem.release() );
  }

  return arr.release();
}

BObjectImp* GetIoStats()
{
  return GetIoStatsObj( Core::networkManager.iostats );
}

BObjectImp* GetQueuedIoStats()
{
  return GetIoStatsObj( Core::networkManager.queuedmode_iostats );
}

BObjectImp* GetPktStatusObj()
{
  using namespace PacketWriterDefs;
  std::unique_ptr<ObjArray> pkts = Clib::make_unique<ObjArray>();
  PacketQueueMap* map = networkManager.packetsSingleton->getPackets();
  for ( PacketQueueMap::iterator it = map->begin(); it != map->end(); ++it )
  {
    std::unique_ptr<BStruct> elem( new BStruct );
    elem->addMember( "pkt", new BLong( it->first ) );
    elem->addMember( "count", new BLong( static_cast<int>( it->second->Count() ) ) );
    pkts->addElement( elem.release() );
    if ( it->second->HasSubs() )
    {
      PacketInterfaceQueueMap* submap = it->second->GetSubs();
      for ( PacketInterfaceQueueMap::iterator s_it = submap->begin(); s_it != submap->end();
            ++s_it )
      {
        std::unique_ptr<BStruct> elemsub( new BStruct );
        elemsub->addMember( "pkt", new BLong( it->first ) );
        elemsub->addMember( "sub", new BLong( s_it->first ) );
        elemsub->addMember( "count", new BLong( static_cast<int>( s_it->second.size() ) ) );
        pkts->addElement( elemsub.release() );
      }
    }
  }
  return pkts.release();
}

BObjectImp* GetCoreVariable( const char* corevar )
{
#define LONG_COREVAR( name, expr )      \
  if ( stricmp( corevar, #name ) == 0 ) \
    return new BLong( static_cast<int>( expr ) );

  if ( stricmp( corevar, "itemcount" ) == 0 )
    return new BLong( get_toplevel_item_count() );
  if ( stricmp( corevar, "mobilecount" ) == 0 )
    return new BLong( get_mobile_count() );

  if ( stricmp( corevar, "bytes_sent" ) == 0 )
    return new Double( static_cast<double>( networkManager.polstats.bytes_sent ) );
  if ( stricmp( corevar, "bytes_received" ) == 0 )
    return new Double( static_cast<double>( networkManager.polstats.bytes_received ) );

  LONG_COREVAR( uptime, polclock() / POLCLOCKS_PER_SEC );
  LONG_COREVAR( sysload, stateManager.profilevars.last_sysload );
  LONG_COREVAR( sysload_severity, stateManager.profilevars.last_sysload_nprocs );
  //  LONG_COREVAR( bytes_sent, polstats.bytes_sent );
  //  LONG_COREVAR( bytes_received, polstats.bytes_received );
  LONG_COREVAR( version, POL_VERSION );
  LONG_COREVAR( systime, time( nullptr ) );
  LONG_COREVAR( events_per_min, GET_PROFILEVAR_PER_MIN( events ) );
  LONG_COREVAR( skill_checks_per_min, GET_PROFILEVAR_PER_MIN( skill_checks ) );
  LONG_COREVAR( combat_operations_per_min, GET_PROFILEVAR_PER_MIN( combat_operations ) );
  LONG_COREVAR( error_creations_per_min, GET_PROFILEVAR_PER_MIN( error_creations ) );

  LONG_COREVAR( tasks_ontime_per_min, GET_PROFILEVAR_PER_MIN( tasks_ontime ) );
  LONG_COREVAR( tasks_late_per_min, GET_PROFILEVAR_PER_MIN( tasks_late ) );
  LONG_COREVAR( tasks_late_ticks_per_min, GET_PROFILEVAR_PER_MIN( tasks_late_ticks ) );

  LONG_COREVAR( scripts_late_per_min, GET_PROFILEVAR_PER_MIN( scripts_late ) );
  LONG_COREVAR( scripts_ontime_per_min, GET_PROFILEVAR_PER_MIN( scripts_ontime ) );

  LONG_COREVAR( instr_per_min, stateManager.profilevars.last_sipm );
  LONG_COREVAR( priority_divide, scriptScheduler.priority_divide );
  if ( stricmp( corevar, "verstr" ) == 0 )
    return new String( POL_VERSION_ID );
  if ( stricmp( corevar, "compiledatetime" ) == 0 )
    return new String( Clib::ProgramConfig::build_datetime() );
  if ( stricmp( corevar, "packages" ) == 0 )
    return GetPackageList();
  if ( stricmp( corevar, "running_scripts" ) == 0 )
    return GetRunningScriptList();
  if ( stricmp( corevar, "all_scripts" ) == 0 )
    return GetAllScriptList();
  if ( stricmp( corevar, "script_profiles" ) == 0 )
    return GetScriptProfiles();
  if ( stricmp( corevar, "iostats" ) == 0 )
    return GetIoStats();
  if ( stricmp( corevar, "queued_iostats" ) == 0 )
    return GetQueuedIoStats();
  if ( stricmp( corevar, "pkt_status" ) == 0 )
    return GetPktStatusObj();
  if ( stricmp( corevar, "memory_usage" ) == 0 )
    return new BLong( static_cast<int>( Clib::getCurrentMemoryUsage() / 1024 ) );

  return new BError( std::string( "Unknown core variable " ) + corevar );
}

BObjectRef PolCore::get_member( const char* membername )
{
  return BObjectRef( GetCoreVariable( membername ) );
}

BObjectImp* PolCore::call_method( const char* methodname, Executor& ex )
{
  if ( stricmp( methodname, "log_profile" ) == 0 )
  {
    if ( ex.numParams() != 1 )
      return new BError( "polcore.log_profile(clear) requires 1 parameter." );
    int clear;
    if ( ex.getParam( 0, clear ) )
    {
      log_all_script_cycle_counts( clear ? true : false );
      return new BLong( 1 );
    }
  }
  else if ( stricmp( methodname, "set_priority_divide" ) == 0 )
  {
    if ( ex.numParams() != 1 )
      return new BError( "polcore.set_priority_divide(value) requires 1 parameter." );
    int div;
    if ( ex.getParam( 0, div, 1, 1000 ) )
    {
      scriptScheduler.priority_divide = div;
      return new BLong( 1 );
    }
    else
    {
      return nullptr;
    }
  }
  else if ( stricmp( methodname, "clear_script_profile_counters" ) == 0 )
  {
    if ( ex.numParams() > 0 )
      return new BError( "polcore.clear_script_profile_counters() doesn't take parameters." );
    clear_script_profile_counters();
    return new BLong( 1 );
  }
  else if ( stricmp( methodname, "internal" ) == 0 )  // Just for internal Development...
  {
    int type;
    if ( ex.getParam( 0, type ) )
    {
#ifdef MEMORYLEAK
      if ( type == 1 )
      {
        char buffer[30];
        auto time_tm = Clib::localtime( time( nullptr ) );

        strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", &time_tm );
        DEBUGLOG << "[" << buffer << "] polcore().internal\n";
        LEAKLOG << buffer << ";";

        bobject_alloc.log_stuff( "bobject" );
        uninit_alloc.log_stuff( "uninit" );
        blong_alloc.log_stuff( "blong" );
        double_alloc.log_stuff( "double" );
        ConfigFiles_log_stuff();
        Clib::PrintHeapData();  // Will print endl in llog
      }
#endif
#ifdef ESCRIPT_PROFILE
      DEBUGLOG << "FuncName,Count,Min,Max,Sum,Avarage\n";
      for ( escript_profile_map::iterator itr = EscriptProfileMap.begin();
            itr != EscriptProfileMap.end(); ++itr )
      {
        DEBUGLOG << itr->first << "," << itr->second.count << "," << itr->second.min << ","
                 << itr->second.max << "," << itr->second.sum << ","
                 << ( itr->second.sum / ( 1.0 * itr->second.count ) ) << "\n";
      }
#endif
      if ( type == 2 )
      {
        Core::MemoryUsage::log();
      }
      else if ( type == 3 )
      {
        POLLOG_ERROR << "Forcing crash\n";
        int* i = nullptr;
        *i = 1;
      }
      else if ( type == 4 )
      {
        POLLOG_ERROR << "Forcing assert crash\n";
        passert_always( false );
      }
      else if ( type == 5 )
      {
        Core::scriptScheduler.estimateSize( true );
      }
      else if ( type == 6 )
      {
        const String* script;
        if ( !ex.getStringParam( 1, script ) )
          return new BLong( 0 );
        Core::scriptScheduler.logScriptVariables( script->data() );
        return new BLong( 1 );
      }
      return new BLong( 1 );
    }
    else
      return new BError( "polcore.internal(value) requires 1 parameter." );
  }
  return nullptr;
}

BObjectImp* UOExecutorModule::mf_POLCore()
{
  return new PolCore;
}

BObjectImp* MakeMapCacheImp()
{
  ObjArray* arr = new ObjArray;
  return arr;
}


BObjectImp* UOExecutorModule::mf_CreateAccount()
{
  const String* acctname;
  const String* password;
  int enabled;
  if ( getStringParam( 0, acctname ) && getStringParam( 1, password ) && getParam( 2, enabled ) )
  {
    if ( acctname->SafeCharAmt() < acctname->length() )
    {
      return new BError(
          "Attempted to use username in account creation with non-allowed characters." );
    }
    if ( password->SafeCharAmt() < password->length() )
    {
      return new BError(
          "Attempted to use password in account creation with non-allowed characters." );
    }

    if ( Accounts::find_account( acctname->data() ) )
    {
      return new BError( "Account already exists" );
    }

    // Dave 6/5/3 let this function handle the hashing (Account ctor does it)
    Accounts::Account* acct = Accounts::create_new_account( acctname->value(), password->value(),
                                                            enabled ? true : false );  // MD5

    return new Accounts::AccountObjImp( Accounts::AccountPtrHolder( AccountRef( acct ) ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_FindAccount()
{
  const String* acctname;
  if ( getStringParam( 0, acctname ) )
  {
    Accounts::Account* acct = Accounts::find_account( acctname->data() );
    if ( acct != nullptr )
    {
      return new Accounts::AccountObjImp( Accounts::AccountPtrHolder( AccountRef( acct ) ) );
    }
    else
    {
      return new BError( "Account not found." );
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UOExecutorModule::mf_ListAccounts()
{
  std::unique_ptr<ObjArray> arr( new ObjArray );
  for ( unsigned idx = 0; idx < gamestate.accounts.size(); idx++ )
  {
    arr->addElement( new String( gamestate.accounts[idx]->name() ) );
  }
  return arr.release();
}

void handle_resurrect_menu( Client* client, PKTBI_2C* msg )
{
  if ( msg->choice )
  {
    INFO_PRINT << "Resurrect Menu Choice: " << int( msg->choice ) << "\n";
    // transmit( client, msg, sizeof *msg );
  }

  if ( client->chr != nullptr && client->gd != nullptr && client->gd->resurrect_uoemod != nullptr )
  {
    client->gd->resurrect_uoemod->uoexec.ValueStack.back().set(
        new BObject( new BLong( msg->choice ) ) );
    client->gd->resurrect_uoemod->uoexec.revive();
    client->gd->resurrect_uoemod->resurrect_chr = nullptr;
    client->gd->resurrect_uoemod = nullptr;
  }
}

BObjectImp* UOExecutorModule::mf_SendInstaResDialog()
{
  Character* chr;
  if ( !getCharacterParam( exec, 0, chr ) )
    return new BError( "Invalid parameter type" );
  if ( !chr->has_active_client() )
    return new BError( "No client attached" );
  if ( chr->client->gd->resurrect_uoemod != nullptr )
    return new BError( "Client busy with another instares dialog" );

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SendInstaResDialog():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  PktHelper::PacketOut<PktOut_2C> msg;
  msg->Write<u8>( RESURRECT_CHOICE_SELECT );
  msg.Send( chr->client );
  chr->client->gd->resurrect_uoemod = this;
  resurrect_chr = chr;

  return new BLong( 0 );
}

void handle_selcolor( Client* client, PKTBI_95* msg )
{
  if ( client->chr != nullptr && client->gd != nullptr && client->gd->selcolor_uoemod != nullptr )
  {
    unsigned short color = cfBEu16( msg->graphic_or_color ) & Plib::VALID_ITEM_COLOR_MASK;
    BObject* valstack;
    if ( color >= 2 && color <= 1001 )
    {
      valstack = new BObject( new BLong( color ) );
    }
    else
    {
      valstack = new BObject( new BError( "Client selected an out-of-range color" ) );

      // unsigned short newcolor = ((color - 2) % 1000) + 2;
      POLLOG_ERROR.Format( "Client #{:d} (account {}) selected an out-of-range color 0x{:X}\n" )
          << static_cast<unsigned long>( client->instance_ )
          << ( ( client->acct != nullptr ) ? client->acct->name() : "unknown" ) << color;
    }

    // client->gd->selcolor_uoemod->uoexec.ValueStack.back().set( new BObject( new BLong( color )
    // )
    // );
    client->gd->selcolor_uoemod->uoexec.ValueStack.back().set( valstack );
    client->gd->selcolor_uoemod->uoexec.revive();
    client->gd->selcolor_uoemod->selcolor_chr = nullptr;
    client->gd->selcolor_uoemod = nullptr;
  }
}


BObjectImp* UOExecutorModule::mf_SelectColor()
{
  Character* chr;
  Item* item;
  if ( !getCharacterParam( exec, 0, chr ) || !getItemParam( exec, 1, item ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( !chr->has_active_client() )
    return new BError( "No client attached" );
  if ( chr->client->gd->resurrect_uoemod != nullptr )
    return new BError( "Client is already selecting a color" );

  PktHelper::PacketOut<PktOut_95> msg;
  msg->Write<u32>( item->serial_ext );
  msg->offset += 2;  // u16 unk
  msg->WriteFlipped<u16>( item->graphic );

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SelectColor():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  msg.Send( chr->client );

  chr->client->gd->selcolor_uoemod = this;
  selcolor_chr = chr;
  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_SendOpenBook()
{
  Character* chr;
  Item* book;

  if ( !( getCharacterParam( exec, 0, chr ) && getItemParam( exec, 1, book ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( !chr->has_active_client() )
  {
    return new BError( "No active client" );
  }

  bool writable = book->call_custom_method( "iswritable" )->isTrue();
  BObject nlines_ob( book->call_custom_method( "getnumlines" ) );
  int nlines;
  if ( nlines_ob.isa( BObjectImp::OTLong ) )
  {
    BLong* blong = static_cast<BLong*>( nlines_ob.impptr() );
    nlines = blong->value();
  }
  else
  {
    return new BError( "book.GetNumLines() did not return an Integer" );
  }
  std::string title = book->call_custom_method( "gettitle" )->getStringRep();
  std::string author = book->call_custom_method( "getauthor" )->getStringRep();

  int npages = ( nlines + 7 ) / 8;

  BObject contents_ob( UninitObject::create() );
  if ( writable )
  {
    contents_ob.setimp( book->call_custom_method( "getcontents" ).impptr() );
    if ( !contents_ob.isa( BObjectImp::OTArray ) )
    {
      if ( contents_ob.isa( BObjectImp::OTError ) )
        return contents_ob->copy();
      else
        return new BError( "book.GetContents() must return an array" );
    }
  }

  PktHelper::PacketOut<PktOut_93> msg93;
  msg93->Write<u32>( book->serial_ext );
  msg93->Write<u8>( writable ? 1u : 0u );
  msg93->Write<u8>( 1u );
  msg93->WriteFlipped<u16>( static_cast<u16>( npages ) );
  msg93->Write( title.c_str(), 60, false );
  msg93->Write( author.c_str(), 30, false );
  msg93.Send( chr->client );

  if ( writable )
  {
    PktHelper::PacketOut<PktOut_66> msg;
    msg->offset += 2;
    msg->Write<u32>( book->serial_ext );
    msg->WriteFlipped<u16>( static_cast<u16>( npages ) );

    ObjArray* arr = static_cast<ObjArray*>( contents_ob.impptr() );

    int linenum = 1;
    for ( int page = 1; page <= npages; ++page )
    {
      if ( msg->offset + 4 > static_cast<int>( sizeof msg->buffer ) )
      {
        return new BError( "Buffer overflow" );
      }
      msg->WriteFlipped<u16>( static_cast<u16>( page ) );
      u16 offset = msg->offset;
      msg->offset += 2;

      int pagelines;
      for ( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
      {
        const BObjectImp* line_imp = arr->imp_at( linenum );
        std::string linetext;
        if ( line_imp )
          linetext = line_imp->getStringRep();
        if ( msg->offset + linetext.size() + 1 > sizeof msg->buffer )
        {
          return new BError( "Buffer overflow" );
        }
        msg->Write( linetext.c_str(), static_cast<u16>( linetext.size() + 1 ) );
      }
      u16 len = msg->offset;
      msg->offset = offset;
      msg->WriteFlipped<u16>( static_cast<u16>( pagelines ) );
      msg->offset = len;
    }

    /*
            int linenum = 1;
            for( int page = 1; page <= npages; ++page )
            {
            PKTBI_66_CONTENTS* ppage = reinterpret_cast<PKTBI_66_CONTENTS*>(&buffer[msglen]);
            msglen += sizeof(*ppage);
            if (msglen > sizeof buffer)
            return new BError( "Buffer overflow" );
            ppage->page = ctBEu16( page );

            int pagelines;
            for( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
            {
            string linetext;

            BObjectVec params;
            params.push_back( BObject(new BLong(linenum)) );
            BObject line_ob = book->call_custom_method( "getline", params );
            linetext = line_ob->getStringRep();

            char* linebuf = reinterpret_cast<char*>(&buffer[msglen]);
            msglen += linetext.size()+1;
            if (msglen > sizeof buffer)
            return new BError( "Buffer overflow" );
            memcpy( linebuf, linetext.c_str(), linetext.size()+1 );
            }
            ppage->lines = ctBEu16( pagelines );
            }
            */
    u16 len = msg->offset;
    msg->offset = 1;
    msg->WriteFlipped<u16>( len );
    msg.Send( chr->client, len );
  }

  return new BLong( 1 );
}

void read_book_page_handler( Client* client, PKTBI_66* msg )
{
  unsigned int book_serial = cfBEu32( msg->book_serial );
  u16 page = cfBEu16( msg->page );
  Item* book = find_legal_item( client->chr, book_serial );
  if ( book == nullptr )
  {
    POLLOG.Format( "Unable to find book 0x{:X} for character 0x{:X}\n" )
        << book_serial << client->chr->serial;
    return;
  }

  if ( msg->lines == 0xFFFF )
  {
    BObject nlines_ob( book->call_custom_method( "getnumlines" ) );
    int nlines;
    if ( nlines_ob.isa( BObjectImp::OTLong ) )
    {
      BLong* blong = static_cast<BLong*>( nlines_ob.impptr() );
      nlines = blong->value();
    }
    else
    {
      return;
    }

    PktHelper::PacketOut<PktOut_66> msgOut;
    msgOut->offset += 2;
    msgOut->Write<u32>( book->serial_ext );
    msgOut->WriteFlipped<u16>( 1u );

    int linenum = ( page - 1 ) * 8 + 1;

    msgOut->WriteFlipped<u16>( page );
    u16 offset = msgOut->offset;
    msgOut->offset += 2;

    int pagelines;
    for ( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
    {
      std::string linetext;

      BObjectImpRefVec params;
      params.push_back( ref_ptr<BObjectImp>( new BLong( linenum ) ) );
      BObject line_ob = book->call_custom_method( "getline", params );
      linetext = line_ob->getStringRep();

      if ( msgOut->offset + linetext.size() + 1 > sizeof msgOut->buffer )
      {
        return;
      }
      msgOut->Write( linetext.c_str(), static_cast<u16>( linetext.size() + 1 ) );
    }

    u16 len = msgOut->offset;
    msgOut->offset = offset;
    msgOut->WriteFlipped<u16>( static_cast<u16>( pagelines ) );
    msgOut->offset = 1;
    msgOut->WriteFlipped<u16>( len );
    msgOut.Send( client, len );
  }
  else
  {
    size_t msglen = cfBEu16( msg->msglen );
    const char* ptext = msg->text;
    size_t bytesleft = msglen - offsetof( PKTBI_66, text );
    for ( int i = 1; i <= 8; ++i )
    {
      std::string line;
      while ( bytesleft )
      {
        --bytesleft;
        if ( *ptext )
        {
          line.append( 1, *ptext );
          ++ptext;
        }
        else
        {
          ++ptext;  // skip null terminator
          break;
        }
      }

      BObjectImpRefVec params;
      params.push_back( ref_ptr<BObjectImp>( new BLong( ( page - 1 ) * 8 + i ) ) );
      params.push_back( ref_ptr<BObjectImp>( new String( line ) ) );

      BObject page_on = book->call_custom_method( "setline", params );
    }
  }
}

char strip_ctrl_chars( char c )
{
  if ( c < 0x20 )
    return 0x20;
  else
    return c;
}

void open_book_handler( Client* client, PKTBI_93* msg )
{
  // fdump( stdout, msg, sizeof *msg );

  // string title( msg->title, sizeof msg->title );
  // string author( msg->author, sizeof msg->author );

  // Dave changed this 12/19 from sizeof msg->title. The protocol defines garbage after the
  // terminator for
  // the title and author strings, so we were writing this garbage into save files. This caused
  // some
  //"No SERIAL property" bugs, because the parser barfed on the bad characters.
  std::string title( msg->title, strlen( msg->title ) );
  std::string author( msg->author, strlen( msg->author ) );

  // dave 1/20/3 cheaters insert cntrl chars into books causing severe problems
  std::transform( title.begin(), title.end(), title.begin(), strip_ctrl_chars );
  std::transform( author.begin(), author.end(), author.begin(), strip_ctrl_chars );


  unsigned int book_serial = cfBEu32( msg->serial );
  Item* book = find_legal_item( client->chr, book_serial );
  if ( book == nullptr )
  {
    POLLOG.Format( "Unable to find book 0x{:X} for character 0x{:X}\n" )
        << book_serial << client->chr->serial;
    return;
  }
  BObjectImpRefVec params;
  params.push_back( ref_ptr<BObjectImp>( new String( title ) ) );
  book->call_custom_method( "settitle", params );

  params[0].set( new String( author ) );
  book->call_custom_method( "setauthor", params );
}

BObjectImp* UOExecutorModule::mf_SendHousingTool()
{
  Character* chr;
  Multi::UMulti* multi;

  if ( !( getCharacterParam( exec, 0, chr ) && getMultiParam( exec, 1, multi ) ) )
  {
    return new BError( "Invalid parameter type" );
  }
  if ( ( chr->client->UOExpansionFlag & AOS ) == 0 )
    return new BError( "Charater does not have AOS enabled." );

  if ( multi == nullptr )
    return new BError( "House not found." );

  Multi::UHouse* house = multi->as_house();
  if ( house == nullptr )
    return new BError( "Not a House multi." );

  if ( !house->IsCustom() )
    return new BError( "Not a Custom House." );

  if ( house->editing )
    return new BError( "House currently being customized." );

  if ( house->IsWaitingForAccept() )
    return new BError( "House currently being waiting for a commit" );

  if ( chr->realm->find_supporting_multi( chr->x, chr->y, chr->z ) != house )
    return new BError( "You must be inside the house to customize it." );

  chr->client->gd->custom_house_serial = house->serial;

  {
    PktHelper::PacketOut<PktOut_BF_Sub20> msg;
    msg->WriteFlipped<u16>( 17u );
    msg->offset += 2;  // sub
    msg->Write<u32>( house->serial_ext );
    msg->Write<u8>( 0x4u );          // begin
    msg->offset += 2;                // u16 unk2 FIXME what's the meaning
    msg->Write<u32>( 0xFFFFFFFFu );  // fixme
    msg->Write<u8>( 0xFFu );         // fixme
    msg.Send( chr->client );
  }
  move_character_to( chr, house->x, house->y, house->z + 7, MOVEITEM_FORCELOCATION, nullptr );
  // chr->set_script_member("hidden",1);
  // chr->set_script_member("frozen",1);

  house->WorkingDesign.AddComponents( house );
  house->CurrentDesign.AddComponents( house );
  Multi::CustomHouseDesign::ClearComponents( house );
  Multi::ItemList itemlist;
  Multi::MobileList moblist;
  Multi::UHouse::list_contents( house, itemlist, moblist );
  const Multi::MultiDef& def = house->multidef();
  while ( !itemlist.empty() )
  {
    Item* item = itemlist.front();
    send_remove_object_if_inrange( chr->client, item );
    itemlist.pop_front();
  }

  while ( !moblist.empty() )
  {
    Character* multichr = moblist.back();
    if ( multichr != chr )
      move_character_to( multichr, house->x + def.minrx, house->y + def.maxry + 1, house->z,
                         MOVEITEM_FORCELOCATION, nullptr );
    moblist.pop_back();
  }

  house->editing = true;
  house->editing_floor_num = 1;
  CustomHousesSendFull( house, chr->client, Multi::HOUSE_DESIGN_WORKING );

  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_SendCharacterRaceChanger( /* Character */ )
{
  Character* chr;
  if ( getCharacterParam( exec, 0, chr ) )
  {
    PktHelper::PacketOut<PktOut_BF_Sub2A> msg;
    msg->WriteFlipped<u16>( 7u );
    msg->offset += 2;  // sub
    msg->Write<u8>( chr->gender );
    msg->Write<u8>( chr->race + 1u );
    msg.Send( chr->client );
    return new BLong( 1 );
  }
  else
    return new BError( "Invalid parameter" );
}


void character_race_changer_handler( Client* client, PKTBI_BF* msg )
{
  Item* tmpitem;

  if ( msg->msglen == ctBEu16( 5 ) )
    return;

  if ( ( msg->characterracechanger.result.BodyHue == 0 ) &&
       ( msg->characterracechanger.result.HairId == 0 ) &&
       ( msg->characterracechanger.result.HairHue == 0 ) &&
       ( msg->characterracechanger.result.BeardId == 0 ) &&
       ( msg->characterracechanger.result.BeardHue == 0 ) )
    return;

  client->chr->setcolor( cfBEu16( msg->characterracechanger.result.BodyHue ) | 0x8000 );
  client->chr->truecolor = client->chr->color;

  client->chr->on_color_changed();

  // Create Hair
  if ( client->chr->layer_is_equipped( LAYER_HAIR ) )
    destroy_item( client->chr->wornitem( LAYER_HAIR ) );

  if ( validhair( cfBEu16( msg->characterracechanger.result.HairId ) ) )
  {
    tmpitem = Item::create( cfBEu16( msg->characterracechanger.result.HairId ) );
    tmpitem->layer = LAYER_HAIR;
    tmpitem->color = cfBEu16( msg->characterracechanger.result.HairHue );
    tmpitem->realm = client->chr->realm;
    client->chr->equip( tmpitem );
    send_wornitem_to_inrange( client->chr, tmpitem );
  }

  // Create Beard
  if ( client->chr->layer_is_equipped( LAYER_BEARD ) )
    destroy_item( client->chr->wornitem( LAYER_BEARD ) );

  if ( validbeard( cfBEu16( msg->characterracechanger.result.BeardId ) ) )
  {
    tmpitem = Item::create( cfBEu16( msg->characterracechanger.result.BeardId ) );
    tmpitem->layer = LAYER_BEARD;
    tmpitem->color = cfBEu16( msg->characterracechanger.result.BeardHue );
    tmpitem->realm = client->chr->realm;
    client->chr->equip( tmpitem );
    send_wornitem_to_inrange( client->chr, tmpitem );
  }
}

// Called when selection made or when selection canceled with nullptr parameters
void popup_menu_selection_made( Network::Client* client, u32 serial, u16 id )
{
  if ( client == nullptr )
    return;

  Character* chr = client->chr;
  if ( chr == nullptr || chr->client->gd->popup_menu_selection_uoemod == nullptr )
    return;

  // The function sending the PopUp menu is responsible to set this
  passert_always_r(
      chr->client->gd->popup_menu_selection_uoemod->popup_menu_selection_above != nullptr,
      "Bug in handling PopUp menu selection. Please report this on the forums." );

  if ( id && serial )
  {
    if ( chr->client->gd->popup_menu_selection_uoemod->popup_menu_selection_above->serial ==
         serial )
      chr->client->gd->popup_menu_selection_uoemod->uoexec.ValueStack.back().set(
          new BObject( new BLong( id ) ) );
    else
      POLLOG_INFO.Format( "{}/{} send an unexpected popup reply for {}.\n" )
          << client->acct->name() << client->chr->name() << serial;
  }

  chr->client->gd->popup_menu_selection_uoemod->uoexec.revive();
  chr->client->gd->popup_menu_selection_uoemod->popup_menu_selection_chr = nullptr;
  chr->client->gd->popup_menu_selection_uoemod->popup_menu_selection_above = nullptr;
  chr->client->gd->popup_menu_selection_uoemod = nullptr;
}

/// Sends a PopUp/Context menu
BObjectImp* UOExecutorModule::mf_SendPopUpMenu()
{
  Character* chr;
  UObject* above;
  ObjArray* menu_arr;
  if ( !( getCharacterParam( exec, 0, chr ) && getUObjectParam( exec, 1, above ) &&
          exec.getObjArrayParam( 2, menu_arr ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  if ( !chr->has_active_client() )
    return new BError( "No client attached" );
  if ( !menu_arr->ref_arr.size() )
    return new BError( "Can't send empty menu" );
  if ( menu_arr->ref_arr.size() > 0xfffe )
    return new BError( "Too many entries in menu" );

  // Prepare packet
  // TODO: add KR support?
  PktHelper::PacketOut<PktOut_BF_Sub14> msg;
  msg->offset += 4;
  msg->Write<u8>( 0u );                  // unknown
  msg->Write<u8>( 1u );                  // 1=2D, 2=KR
  msg->Write<u32>( above->serial_ext );  // Above serial
  u16 offset_num_entries = msg->offset;
  msg->offset += 1;  // Skip num entries now, write it later

  u8 num_entries = 0;
  for ( u16 i = 0; i < menu_arr->ref_arr.size(); ++i )
  {
    BObject* bo = menu_arr->ref_arr[i].get();
    if ( bo == nullptr )
      continue;
    BObjectImp* imp = bo->impptr();

    if ( !++num_entries )  // overflow
      return new BError( "Too many entries in menu" );

    int cliloc;
    bool disabled = false;
    bool arrow = false;
    u16 color = 0;
    bool use_color = false;
    if ( imp->isa( BObjectImp::OTLong ) )
    {
      // Short form: meu is just an int
      const BLong* lng = static_cast<BLong*>( imp );
      cliloc = lng->value();
    }
    else if ( imp->isa( BObjectImp::OTStruct ) )
    {
      // Full form: menu is a struct
      BStruct* elem = static_cast<BStruct*>( imp );

      BObjectImp* cl = const_cast<BObjectImp*>( elem->FindMember( "cliloc" ) );
      if ( cl == nullptr )
        return new BError( "Missing cliloc for menu element" );
      if ( !cl->isa( BObjectImp::OTLong ) )
        return new BError( "Invalid cliloc for menu element" );
      const BLong* lng = static_cast<BLong*>( cl );
      cliloc = lng->value();

      const BObjectImp* ds = elem->FindMember( "disabled" );
      if ( ds != nullptr )
        disabled = ds->isTrue();

      const BObjectImp* ar = elem->FindMember( "arrow" );
      if ( ar != nullptr )
        arrow = ar->isTrue();

      BObjectImp* co = const_cast<BObjectImp*>( elem->FindMember( "color" ) );
      if ( co != nullptr && co->isa( BObjectImp::OTLong ) )
      {
        const BLong* colng = static_cast<BLong*>( co );
        color = static_cast<u16>( colng->value() );
        use_color = true;
      }
    }
    else
      return new BError( "Menu elements must be int or struct" );

    if ( cliloc < 3000000 || cliloc > 3065535 )
      return new BError( "Cliloc out of range in menu" );

    u16 flags = 0x00;
    if ( disabled )
      flags |= 0x01;
    if ( arrow )
      flags |= 0x02;
    if ( use_color )
      flags |= 0x20;
    msg->WriteFlipped<u16>( static_cast<u16>( i + 1 ) );             // Menu element ID
    msg->WriteFlipped<u16>( static_cast<u16>( cliloc - 3000000 ) );  // Cliloc ID, adjusted
    msg->WriteFlipped<u16>( flags );                                 // Flags
    if ( use_color )
      msg->WriteFlipped<u16>( static_cast<u16>( color ) );
  }

  // Add lengths and send
  u16 len = msg->offset;
  msg->offset = offset_num_entries;
  msg->Write<u8>( num_entries );
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( chr->client, len );

  // Cancel any previously waiting popup response
  if ( chr->client->gd->popup_menu_selection_uoemod != nullptr )
  {
    chr->client->gd->popup_menu_selection_uoemod->uoexec.revive();

    chr->client->gd->popup_menu_selection_uoemod = nullptr;
    chr->on_popup_menu_selection = nullptr;
  }

  // Suspend the script first
  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::SendPopupMenu():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }

  // Prepare to restart the script once the response arrives
  chr->on_popup_menu_selection = popup_menu_selection_made;
  chr->client->gd->popup_menu_selection_uoemod = this;
  popup_menu_selection_chr = chr;
  popup_menu_selection_above = above;

  return new BLong( 0 );
}

BObjectImp* UOExecutorModule::mf_SingleClick()
{
  Character* chr = nullptr;
  UObject* what = nullptr;

  if ( !getCharacterParam( exec, 0, chr ) || !getUObjectParam( exec, 1, what ) )
    return new BError( "Invalid parameter" );

  if ( !chr->has_active_client() )
    return new BError( "Mobile has no active client" );

  // If it got here, clear any errors from getUObjectParam/getCharacterParam
  exec.setFunctionResult( nullptr );

  singleclick( chr->client, what->serial );
  return new BLong( 1 );
}

BObjectImp* UOExecutorModule::mf_ListStaticsNearLocationOfType(
    /* x, y, z, range, objtype, flags, realm */ )
{
  unsigned short x, y;
  int z, flags;
  short range;
  unsigned int objtype;
  const String* strrealm;
  Realms::Realm* realm;

  if ( getParam( 0, x ) && getParam( 1, y ) && getParam( 2, z ) && getParam( 3, range ) &&
       getObjtypeParam( exec, 4, objtype ) && getParam( 5, flags ) &&
       getStringParam( 6, strrealm ) )
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
            if ( slist[i].objtype != objtype )
              continue;
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
            if ( mlist[i].graphic != objtype )
              continue;
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


BObjectImp* UOExecutorModule::mf_ListStaticsNearLocationWithFlag(
    /* x, y, z, range, flags, realm */ )
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
            if ( ( Plib::tile_uoflags( slist[i].objtype ) & flags ) )
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
        }

        if ( !( flags & ITEMS_IGNORE_MULTIS ) )
        {
          Plib::StaticList mlist;
          realm->readmultis( mlist, wx, wy );
          for ( unsigned i = 0; i < mlist.size(); ++i )
          {
            if ( ( Plib::tile_uoflags( mlist[i].graphic ) & flags ) )
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
    }

    return newarr.release();
  }
  else
    return new BError( "Invalid parameter" );
}
}  // namespace Module
}  // namespace Pol

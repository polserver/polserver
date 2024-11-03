/** @file
 *
 * @par History
 * - 2006/05/16 Shinigami: updated doubleclick() to support Elf
 * - 2006/05/26 Shinigami: fixed start_script call of dblclickother
 * - 2007/01/04 MuadDib:   changed a large if || || || to a case switch in dbl click check.
 * - 2007/01/04 MuadDib:   changed flag status in sendpaperdoll to send status, instead of
 *                         only warmode status.
 * - 2008/09/24 Luth:      If this is my paperdoll, tell me all about me otherwise tell less
 * - 2008/09/xx Turley:    Paperdoll info decided with ssopt.cfg option (for luth's update)
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added gargoyle support
 */


#include <cstring>
#include <string>

#include "../bscript/eprog.h"
#include "../clib/clib_endian.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../plib/systemstate.h"
#include "containr.h"
#include "eventid.h"
#include "gameclck.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "mobile/npc.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "plib/objtype.h"
#include "realms/realm.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Core
{
void send_paperdoll( Network::Client* client, Mobile::Character* chr )
{
  Network::PktHelper::PacketOut<Network::PktOut_88> msg;
  msg->Write<u32>( chr->serial_ext );

  if ( ( !settingsManager.ssopt.privacy_paperdoll ) || ( client->chr == chr ) )
  {
    std::string name = ( !chr->has_title_prefix() ? "" : chr->title_prefix() + " " ) + chr->name() +
                       ( !chr->has_title_suffix() ? "" : " " + chr->title_suffix() );
    if ( chr->has_title_race() )
      name += " (" + chr->title_race() + ")";
    msg->Write( Clib::strUtf8ToCp1252( name ).c_str(), 60 );
  }
  else
    msg->Write( Clib::strUtf8ToCp1252( chr->name() ).c_str(), 60 );


  // MuadDib changed to reflect true status for 0x20 packet. 1/4/2007
  // Paperdoll Appears different type Status byte than other walk/update
  // packets. Using poison/hidden here will break peace/war button.
  u8 flag1 = chr->warmode() ? 1 : 0;
  if ( client->UOExpansionFlag & Network::AOS && client->chr->serial_ext == chr->serial_ext )
    flag1 |= CHAR_FLAG1_CANALTER;
  msg->Write<u8>( flag1 );

  msg.Send( client );
}

void doubleclick( Network::Client* client, PKTIN_06* msg )
{
  u32 serial = cfBEu32( msg->serial );
  u32 paperdoll_macro_flag = serial & 0x80000000Lu;
  serial &= ~0x80000000Lu;  // keypress versus doubleclick switch?

  // the find_character would find this, but most of the time it's your own paperdoll.
  // this is special-cased for two reasons:
  //    1) it's commonly done
  //    2) ghosts can doubleclick ONLY their paperdoll.
  if ( serial == client->chr->serial )
  {
    if ( !paperdoll_macro_flag )
    {
      ScriptDef sd;
      sd.quickconfig( "scripts/misc/dblclickself.ecl" );
      if ( sd.exists() )
      {
        ref_ptr<Bscript::EScriptProgram> prog;
        prog = find_script2( sd, false, Plib::systemstate.config.cache_interactive_scripts );
        if ( prog.get() != nullptr && client->chr->start_script( prog.get(), false ) )
        {
          return;
        }
      }
    }
    send_paperdoll( client, client->chr );
    return;
  }

  if ( client->chr->dblclick_wait() > read_gameclock() )
  {
    private_say_above( client->chr, client->chr, "You must wait to use something again." );
    return;
  }
  else
    client->chr->dblclick_wait( read_gameclock() + settingsManager.ssopt.dblclick_wait );

  if ( IsCharacter( serial ) )
  {
    if ( client->chr->dead() )
      return;
    Mobile::Character* chr = find_character( serial );
    if ( !chr )
      return;

    if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) )
    {
      Mobile::NPC* npc = static_cast<Mobile::NPC*>( chr );
      if ( npc->can_accept_event( EVID_DOUBLECLICKED ) )
      {
        npc->send_event( new Module::SourcedEvent( EVID_DOUBLECLICKED, client->chr ) );
        return;
      }
    }

    bool script_ran = false;
    ScriptDef sd;
    sd.quickconfig( "scripts/misc/dblclickother.ecl" );
    if ( sd.exists() )
    {
      ref_ptr<Bscript::EScriptProgram> prog;
      prog = find_script2( sd, false, Plib::systemstate.config.cache_interactive_scripts );
      if ( prog.get() != nullptr )
        script_ran =
            client->chr->start_script( prog.get(), false, new Module::ECharacterRefObjImp( chr ) );
    }
    if ( !script_ran && client->chr->in_visual_range( chr ) )
    {
      // MuadDib Changed from a large if || || || to switch case. 1/4/2007
      switch ( chr->graphic )
      {
      case UOBJ_HUMAN_MALE:
      case UOBJ_HUMAN_FEMALE:
      case UOBJ_HUMAN_MALE_GHOST:
      case UOBJ_HUMAN_FEMALE_GHOST:
      case UOBJ_ELF_MALE:
      case UOBJ_ELF_FEMALE:
      case UOBJ_ELF_MALE_GHOST:
      case UOBJ_ELF_FEMALE_GHOST:
      case UOBJ_GARGOYLE_MALE:
      case UOBJ_GARGOYLE_FEMALE:
      case UOBJ_GARGOYLE_MALE_GHOST:
      case UOBJ_GARGOYLE_FEMALE_GHOST:
      case UOBJ_GAMEMASTER:
      case 0x3de:
      case 0x3df:
      case 0x3e2:
        send_paperdoll( client, chr );
        break;
      }
    }
    return;
  }
  else  // doubleclicked an item
  {
    // next, search worn items, items in the backpack, and items in the world.
    Items::Item* item = find_legal_item( client->chr, serial );

    Mobile::Character* owner = nullptr;
    if ( item == nullptr )
    {
      item = find_snoopable_item( serial, &owner );
    }

    if ( item != nullptr )
    {
      const Items::ItemDesc& id = item->itemdesc();

      if ( !id.ghosts_can_use && client->chr->dead() )
      {
        private_say_above( client->chr, client->chr, "I am dead and cannot do that." );
        return;
      }
      if ( !id.can_use_while_frozen && client->chr->frozen() )
      {
        private_say_above( client->chr, client->chr, "I am frozen and cannot do that." );
        return;
      }
      if ( !id.can_use_while_paralyzed && client->chr->paralyzed() )
      {
        private_say_above( client->chr, client->chr, "I am paralyzed and cannot do that." );
        return;
      }

      if ( !client->chr->can_dblclickany() && !item->in_range( client->chr, id.doubleclick_range ) )
      {
        private_say_above( client->chr, item, "That is too far away." );
        return;
      }
      UObject* obj = item->toplevel_owner();
      obj = obj->self_as_owner();
      if ( id.use_requires_los && !client->chr->realm()->has_los( *client->chr, *obj ) )  // DAVE
                                                                                          // 11/24
      {
        private_say_above( client->chr, item, "I can't see that." );
        return;
      }

      ScriptDef sd;
      sd.quickconfig( "scripts/misc/dblclickitem.ecl" );
      if ( sd.exists() )
      {
        ref_ptr<Bscript::EScriptProgram> prog;
        prog = find_script2( sd, false, Plib::systemstate.config.cache_interactive_scripts );
        if ( prog.get() != nullptr )
          client->chr->start_script( prog.get(), false, new Module::EItemRefObjImp( item ) );
      }

      if ( owner == nullptr )
      {
        item->double_click( client );
      }
      else
      {
        item->snoop( client, owner );
      }
      return;
    }

    // allow looking into containers being traded
    if ( client->chr->is_trading() )
    {
      UContainer* cont = client->chr->trade_container()->find_container( serial );
      if ( cont != nullptr )
      {
        cont->builtin_on_use( client );
        if ( !cont->locked() )
        {
          if ( client->chr->trading_with->client != nullptr )
            cont->builtin_on_use( client->chr->trading_with->client );
        }
        return;
      }
    }
  }
}
}  // namespace Core
}  // namespace Pol

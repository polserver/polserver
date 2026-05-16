/** @file
 *
 * @par History
 * - 2003/03/15 Dave:      Support configurable max skillid
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 */


#include <stddef.h>

#include "../bscript/eprog.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/settings.h"
#include "item/item.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "party.h"
#include "scrdef.h"
#include "scrstore.h"
#include "statmsg.h"
#include "ufunc.h"
#include "uoclient.h"
#include "uoskills.h"


namespace Pol::Core
{
void send_short_statmsg( Network::Client* client, Items::Item* item )
{
  Network::PktHelper::PacketOut<Network::PktOut_11> msg;
  msg->offset += 2;  // msglen
  msg->Write<u32>( item->serial_ext );
  msg->Write( Clib::strUtf8ToCp1252( item->name() ).c_str(), 30, false );

  msg->WriteFlipped<u16>( Clib::clamp_convert<u16>( item->hp_ * 1000 / item->maxhp() ) );
  msg->WriteFlipped<u16>( 1000_u16 );
  msg->Write<u8>( 0_u8 );
  msg->Write<u8>( 0_u8 );  // moreinfo

  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

void statrequest( Network::Client* client, u32 serial )
{
  auto chr = client->chr;
  if ( serial == chr->serial )
  {
    send_full_statmsg( client, chr );
    return;
  }
  Mobile::Character* bob = find_character( serial );
  if ( !bob )
  {
    auto* item = system_find_item( serial );
    if ( !item || !item->is_attackable() )
      return;
    if ( item->invisible() && !client->chr->can_seeinvisitems() )
      return;
    if ( !client->chr->in_visual_range( item ) )
      return;
    send_short_statmsg( client, item );
    return;
  }
  if ( chr->is_visible_to_me( bob ) )
    send_short_statmsg( client, bob );
  if ( chr->has_party() )
    chr->party()->send_stat_to( chr, bob );
}

void send_skillmsg( Network::Client* client, const Mobile::Character* chr )
{
  Network::PktHelper::PacketOut<Network::PktOut_3A> msg;
  msg->offset += 2;
  if ( settingsManager.ssopt.core_sends_caps )
    msg->Write<u8>( PKTBI_3A_VALUES::FULL_LIST_CAP );
  else
    msg->Write<u8>( PKTBI_3A_VALUES::FULL_LIST );

  for ( unsigned short i = 0; i <= networkManager.uoclient_general.maxskills; ++i )
  {
    const UOSkill& uoskill = GetUOSkill( i );
    msg->WriteFlipped<u16>( static_cast<u16>( i + 1 ) );  // for some reason, we send this 1-based
    if ( uoskill.pAttr )
    {
      const Mobile::AttributeValue& av = chr->attribute( uoskill.pAttr->attrid );
      int value;
      value = av.effective_tenths();
      if ( value > 0xFFFF )
        value = 0xFFFF;
      msg->WriteFlipped<u16>( static_cast<u16>( value ) );

      value = av.base();
      if ( value > 0xFFFF )
        value = 0xFFFF;
      msg->WriteFlipped<u16>( static_cast<u16>( value ) );
      msg->Write<u8>( av.lock() );
      if ( settingsManager.ssopt.core_sends_caps )
        msg->WriteFlipped<u16>( av.cap() );
    }
    else
    {
      msg->offset += 4;  // u16 value/value_unmod
      msg->Write<u8>( PKTBI_3A_VALUES::LOCK_DOWN );
      if ( settingsManager.ssopt.core_sends_caps )
        msg->WriteFlipped<u16>( settingsManager.ssopt.default_attribute_cap );
    }
  }
  if ( !settingsManager.ssopt.core_sends_caps )
    msg->offset += 2;  // u16 nullterm
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}

void handle_skill_lock( Network::Client* client, PKTBI_3A_LOCKS* msg )
{
  if ( settingsManager.ssopt.core_handled_locks )
  {
    unsigned int skillid = cfBEu16( msg->skillid );
    if ( skillid > networkManager.uoclient_general.maxskills )
      return;

    const UOSkill& uoskill = GetUOSkill( skillid );
    if ( !uoskill.pAttr )  // tried to set lockstate for a skill that isn't defined
      return;

    if ( msg->lock_mode < 3 )
      client->chr->attribute( uoskill.pAttr->attrid ).lock( msg->lock_mode );
    else
      INFO_PRINTLN( "Client {} tried to set an illegal lock state.", client->chr->name() );
  }
}

void skillrequest( Network::Client* client, u32 serial )
{
  if ( serial == client->chr->serial )
  {
    ScriptDef sd;
    sd.quickconfig( "scripts/misc/skillwin.ecl" );
    if ( sd.exists() )
    {
      ref_ptr<Bscript::EScriptProgram> prog;
      prog = find_script2( sd,
                           false,  // complain if not found
                           Plib::systemstate.config.cache_interactive_scripts );
      if ( prog.get() != nullptr && client->chr->start_script( prog.get(), false ) )
      {
        return;
      }
    }
    send_skillmsg( client, client->chr );
  }
}

void srequest( Network::Client* client, PKTIN_34* msg )
{
  u32 serial = cfBEu32( msg->serial2 );

  if ( msg->stattype == STATTYPE_STATWINDOW )
    statrequest( client, serial );
  else if ( msg->stattype == STATTYPE_SKILLWINDOW )
    skillrequest( client, serial );
}
}  // namespace Pol::Core

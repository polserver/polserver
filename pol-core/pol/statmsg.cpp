/** @file
 *
 * @par History
 * - 2006/05/23 Shinigami: send_full_statmsg() updated to support Elfs
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/09 MuadDib:   Re factor of Packet 0x25, 0x11 for naming convention
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/12/02 Turley:    fixed 0x11 packet (race flag)
 */


#include "statmsg.h"

#include "../clib/clib.h"
#include "../clib/rawtypes.h"
#include "globals/network.h"
#include "globals/settings.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "ufunc.h"
#include "uoclient.h"


namespace Pol
{
namespace Core
{
using namespace Network::PktHelper;

void send_full_statmsg( Network::Client* client, Mobile::Character* chr )
{
  PacketOut<Network::PktOut_11> msg;
  msg->offset += 2;  // msglen
  msg->Write<u32>( chr->serial_ext );
  msg->Write( chr->name().c_str(), 30, false );
  if ( networkManager.uoclient_general.hits.any )
  {
    int v = chr->vital( networkManager.uoclient_general.hits.id ).current_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );  // hits
    v = chr->vital( networkManager.uoclient_general.hits.id ).maximum_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );  // max_hits
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );  // hits
    msg->WriteFlipped<u16>( 0u );  // max_hits
  }
  msg->Write<u8>( 0u );  // (client->chr->can_rename( chr ) ? 0xFF : 0);
  if ( client->ClientType & Network::CLIENTTYPE_70300 )
    msg->Write<u8>( 6u );  // New entries for classic client
  else if ( ( client->UOExpansionFlag & Network::ML ) &&
            ( client->ClientType & Network::CLIENTTYPE_5000 ) )
    msg->Write<u8>( 5u );  // Set to ML level
  else if ( ( client->UOExpansionFlag & Network::AOS ) )
    msg->Write<u8>( 4u );  // Set to AOS level statbar for full info
  else
    msg->Write<u8>( 1u );  // Set to oldschool statbar info.

  // if (chr->race == Plib::RACE_ELF)
  //  msg->Write(static_cast<u8>(chr->gender | FLAG_RACE));
  // else
  msg->Write<u8>( chr->gender );  // GENDER_MALE or GENDER_FEMALE (see ../plib/uconst.h)

  if ( networkManager.uoclient_general.strength.any )
  {
    int v = chr->attribute( networkManager.uoclient_general.strength.id ).effective();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.dexterity.any )
  {
    int v = chr->attribute( networkManager.uoclient_general.dexterity.id ).effective();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.intelligence.any )
  {
    int v = chr->attribute( networkManager.uoclient_general.intelligence.id ).effective();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.stamina.any )
  {
    int v = chr->vital( networkManager.uoclient_general.stamina.id ).current_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );

    v = chr->vital( networkManager.uoclient_general.stamina.id ).maximum_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );
    msg->WriteFlipped<u16>( 0u );
  }

  if ( networkManager.uoclient_general.mana.any )
  {
    int v = chr->vital( networkManager.uoclient_general.mana.id ).current_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );

    v = chr->vital( networkManager.uoclient_general.mana.id ).maximum_ones();
    if ( v > 0xFFFF )
      v = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );
    msg->WriteFlipped<u16>( 0u );
  }

  msg->WriteFlipped<u32>( chr->gold_carried() );
  // Adjusted to work with Physical Resist if AOS client, and AOS Resistances enabled.
  if ( ( client->UOExpansionFlag & Network::AOS ) && client->aosresist )
  {
    s16 value = chr->physical_resist().value;
    msg->WriteFlipped<u16>( static_cast<u16>( ( value < 0 ) ? ( 0x10000 + value ) : value ) );
  }
  else
    msg->WriteFlipped<u16>( chr->ar() );

  unsigned int weight = chr->weight();
  Clib::sanitize_upperlimit( &weight, 0xFFFFu );
  msg->WriteFlipped<u16>( weight );

  // moreinfo 5
  if ( ( client->UOExpansionFlag & Network::ML ) &&
       ( client->ClientType & Network::CLIENTTYPE_5000 ) )
  {
    msg->WriteFlipped<u16>( chr->carrying_capacity() );
    msg->Write<u8>( chr->race + 1u );
  }

  // moreinfo 3 start
  if ( ( client->UOExpansionFlag & Network::AOS ) )
  {
    msg->WriteFlipped<s16>( chr->skillstatcap().statcap );
    auto follow_value = chr->followers();
    msg->Write<s8>( follow_value.followers );
    msg->Write<s8>( follow_value.followers_max );
    // moreinfo 4 start
    s16 value = chr->fire_resist().value;
    msg->WriteFlipped<u16>( static_cast<u16>( ( value < 0 ) ? ( 0x10000 + value ) : value ) );
    value = chr->cold_resist().value;
    msg->WriteFlipped<u16>( static_cast<u16>( ( value < 0 ) ? ( 0x10000 + value ) : value ) );
    value = chr->poison_resist().value;
    msg->WriteFlipped<u16>( static_cast<u16>( ( value < 0 ) ? ( 0x10000 + value ) : value ) );
    value = chr->energy_resist().value;
    msg->WriteFlipped<u16>( static_cast<u16>( ( value < 0 ) ? ( 0x10000 + value ) : value ) );
    msg->WriteFlipped<s16>( chr->luck() );
    msg->WriteFlipped<u16>( chr->min_weapon_damage() );
    msg->WriteFlipped<u16>( chr->max_weapon_damage() );
    msg->WriteFlipped<s32>( chr->tithing() );
  }

  // Add the new entries as 0's for now
  if ( client->ClientType & Network::CLIENTTYPE_70300 )
  {
    msg->offset += 30;
    /*
    msg->WriteFlipped<u16>( 0 ); // Physical resist cap
    msg->WriteFlipped<u16>( 0 ); // Fire resist cap
    msg->WriteFlipped<u16>( 0 ); // Cold resist cap
    msg->WriteFlipped<u16>( 0 ); // Poison resist cap
    msg->WriteFlipped<u16>( 0 ); // Energy resist cap
    msg->WriteFlipped<u16>( 0 ); // Defense chance increase
    msg->WriteFlipped<u16>( 0 ); // Defense chance cap increase
    msg->WriteFlipped<u16>( 0 ); // Hit chance increase
    msg->WriteFlipped<u16>( 0 ); // Swing speed increase
    msg->WriteFlipped<u16>( 0 ); // Weapon damage increase
    msg->WriteFlipped<u16>( 0 ); // Lower reagent cose
    msg->WriteFlipped<u16>( 0 ); // Spell damage increase
    msg->WriteFlipped<u16>( 0 ); // Faster cast recovery
    msg->WriteFlipped<u16>( 0 ); // Faster casting
    msg->WriteFlipped<u16>( 0 ); // Lower mana cost
    */
  }

  u16 len = msg->offset;

  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );

  if ( settingsManager.ssopt.send_stat_locks )
    send_stat_locks( client, chr );
}

void send_stat_locks( Network::Client* client, Mobile::Character* chr )
{
  if ( client->getversiondetail().major < 3 )  // only in AOS, I think
    return;

  u8 lockbit = 0;

  lockbit |= chr->attribute( networkManager.uoclient_general.strength.id ).lock()
             << 4;  // XX SS DD II (2 bits for each lock)
  lockbit |= chr->attribute( networkManager.uoclient_general.dexterity.id ).lock() << 2;
  lockbit |= chr->attribute( networkManager.uoclient_general.intelligence.id ).lock();

  PacketOut<Network::PktOut_BF_Sub19> msg;
  msg->WriteFlipped<u16>( 12u );
  msg->offset += 2;         // sub
  msg->Write<u8>( 0x02u );  // 2D Client = 0x02, KR = 0x05
  msg->Write<u32>( chr->serial_ext );
  msg->offset++;  // unk
  msg->Write<u8>( lockbit );
  msg.Send( client );
}

void send_short_statmsg( Network::Client* client, Mobile::Character* chr )
{
  PacketOut<Network::PktOut_11> msg;
  msg->offset += 2;  // msglen
  msg->Write<u32>( chr->serial_ext );
  msg->Write( chr->name().c_str(), 30, false );

  if ( networkManager.uoclient_general.hits.any )
  {
    msg->WriteFlipped<u16>( static_cast<u16>(
        chr->vital( networkManager.uoclient_general.hits.id ).current_thousands() ) );
    msg->WriteFlipped<u16>( 1000u );  // max_hits
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );  // hits
    msg->WriteFlipped<u16>( 0u );  // max_hits
  }
  msg->Write<u8>( client->chr->can_rename( chr ) ? 0xFFu : 0u );
  msg->Write<u8>( 0u );  // moreinfo

  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );

  msg.Send( client, len );
}

void send_update_hits_to_inrange( Mobile::Character* chr )
{
  PacketOut<Network::PktOut_A1> msg;
  msg->Write<u32>( chr->serial_ext );

  if ( networkManager.uoclient_general.hits.any )
  {
    int h = chr->vital( networkManager.uoclient_general.hits.id ).current_ones();
    if ( h > 0xFFFF )
      h = 0xFFFF;
    int mh = chr->vital( networkManager.uoclient_general.hits.id ).maximum_ones();
    if ( mh > 0xFFFF )
      mh = 0xFFFF;
    msg->WriteFlipped<u16>( static_cast<u16>( mh ) );
    msg->WriteFlipped<u16>( static_cast<u16>( h ) );

    // Send proper data to self (if we exist?)
    if ( chr->client && chr->client->ready )
      msg.Send( chr->client );

    // To stop "HP snooping"...
    msg->offset = 5;
    msg->WriteFlipped<u16>( 1000u );
    msg->WriteFlipped<u16>( static_cast<u16>( h * 1000 / mh ) );
  }
  else
  {
    msg->offset += 4;  // hits,maxhits=0
    if ( chr->client && chr->client->ready )
      msg.Send( chr->client );
  }

  // Exclude self... otherwise their status-window shows 1000 hp!! >_<
  transmit_to_others_inrange( chr, &msg->buffer, msg->offset );
}
}
}

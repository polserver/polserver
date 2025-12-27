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


namespace Pol::Core
{
using namespace Network::PktHelper;

void send_full_statmsg( Network::Client* client, Mobile::Character* chr )
{
  PacketOut<Network::PktOut_11> msg;
  msg->offset += 2;  // msglen
  msg->Write<u32>( chr->serial_ext );
  msg->Write( Clib::strUtf8ToCp1252( chr->name() ).c_str(), 30, false );
  bool ignore_caps = Core::settingsManager.ssopt.core_ignores_defence_caps;
  if ( networkManager.uoclient_general.hits.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.hits.id ).current_ones() );
    msg->WriteFlipped<u16>( v );  // hits
    v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.hits.id ).maximum_ones() );
    msg->WriteFlipped<u16>( v );  // max_hits
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );  // hits
    msg->WriteFlipped<u16>( 0u );  // max_hits
  }
  msg->Write<u8>( 0u );  // (client->chr->can_rename( chr ) ? 0xFF : 0);

  u8 type = 1u;  // Set to oldschool statbar info.
  if ( client->ClientType & Network::CLIENTTYPE_70300 )
    type = 6u;  // New entries for classic client
  else if ( client->acctSupports( Plib::ExpansionVersion::ML ) &&
            ( client->ClientType & Network::CLIENTTYPE_5000 ) )
    type = 5u;  // Set to ML level
  else if ( client->acctSupports( Plib::ExpansionVersion::AOS ) )
    type = 4u;  // Set to AOS level statbar for full info
  msg->Write<u8>( type );

  // if (chr->race == Plib::RACE_ELF)
  //  msg->Write(static_cast<u8>(chr->gender | FLAG_RACE));
  // else
  msg->Write<u8>( chr->gender );  // GENDER_MALE or GENDER_FEMALE (see ../plib/uconst.h)

  if ( networkManager.uoclient_general.strength.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->attribute( networkManager.uoclient_general.strength.id ).effective() );
    msg->WriteFlipped<u16>( v );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.dexterity.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->attribute( networkManager.uoclient_general.dexterity.id ).effective() );
    msg->WriteFlipped<u16>( v );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.intelligence.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->attribute( networkManager.uoclient_general.intelligence.id ).effective() );
    msg->WriteFlipped<u16>( v );
  }
  else
    msg->WriteFlipped<u16>( 0u );

  if ( networkManager.uoclient_general.stamina.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.stamina.id ).current_ones() );
    msg->WriteFlipped<u16>( v );

    v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.stamina.id ).maximum_ones() );
    msg->WriteFlipped<u16>( v );
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );
    msg->WriteFlipped<u16>( 0u );
  }

  if ( networkManager.uoclient_general.mana.any )
  {
    auto v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.mana.id ).current_ones() );
    msg->WriteFlipped<u16>( v );

    v = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.mana.id ).maximum_ones() );
    msg->WriteFlipped<u16>( v );
  }
  else
  {
    msg->WriteFlipped<u16>( 0u );
    msg->WriteFlipped<u16>( 0u );
  }

  msg->WriteFlipped<u32>( chr->gold_carried() );
  // Adjusted to work with Physical Resist if AOS client, and AOS Resistances enabled.
  if ( client->acctSupports( Plib::ExpansionVersion::AOS ) && client->aosresist )
  {
    s16 value = chr->physical_resist().sum();
    if ( chr->has_physical_resist_cap() && !ignore_caps )
    {
      auto cap = chr->physical_resist_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );
  }
  else
    msg->WriteFlipped<u16>( chr->ar() );

  auto weight = Clib::clamp_convert<u16>( chr->weight() );
  msg->WriteFlipped<u16>( weight );

  // moreinfo 5
  if ( type >= 5 )
  {
    msg->WriteFlipped<u16>( chr->carrying_capacity() );
    msg->Write<u8>( chr->race + 1u );
  }

  // moreinfo 3 start
  if ( type >= 3 )
  {
    msg->WriteFlipped<s16>( chr->skillstatcap().statcap );
    auto follow_value = chr->followers();
    msg->Write<s8>( follow_value.followers );
    msg->Write<s8>( follow_value.followers_max );
    // moreinfo 4 start
    s16 value = chr->fire_resist().sum();
    if ( chr->has_fire_resist_cap() && !ignore_caps )
    {
      auto cap = chr->fire_resist_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );
    value = chr->cold_resist().sum();
    if ( chr->has_cold_resist_cap() && !ignore_caps )
    {
      auto cap = chr->cold_resist_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );
    value = chr->poison_resist().sum();
    if ( chr->has_poison_resist_cap() && !ignore_caps )
    {
      auto cap = chr->poison_resist_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );
    value = chr->energy_resist().sum();
    if ( chr->has_energy_resist_cap() && !ignore_caps )
    {
      auto cap = chr->energy_resist_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );
    msg->WriteFlipped<u16>( static_cast<u16>( chr->luck().sum() ) );

    msg->WriteFlipped<u16>( chr->min_weapon_damage() );
    msg->WriteFlipped<u16>( chr->max_weapon_damage() );
    msg->WriteFlipped<s32>( chr->tithing() );
  }

  if ( type >= 6 )
  {
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->physical_resist_cap().sum() ) );  // Physical resist cap
    msg->WriteFlipped<u16>( static_cast<u16>( chr->fire_resist_cap().sum() ) );  // Fire resist cap
    msg->WriteFlipped<u16>( static_cast<u16>( chr->cold_resist_cap().sum() ) );  // Cold resist cap
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->poison_resist_cap().sum() ) );  // Poison resist cap
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->energy_resist_cap().sum() ) );  // Energy resist cap
    s16 value = chr->defence_increase().sum();
    if ( chr->has_defence_increase_cap() && !ignore_caps )
    {
      auto cap = chr->defence_increase_cap().sum();
      value = std::min( cap, value );
    }
    msg->WriteFlipped<s16>( value );  // Defense chance increase
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->defence_increase_cap().sum() ) );  // Defense chance cap increase
    msg->WriteFlipped<u16>( static_cast<u16>( chr->hit_chance().sum() ) );  // Hit chance increase
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->swing_speed_increase().sum() ) );  // swing_speed_increase
    msg->offset += 2;                                             // damage_increase
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->lower_reagent_cost().sum() ) );  // Lower reagent cost
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->spell_damage_increase().sum() ) );  // Spell damage increase
    msg->WriteFlipped<u16>(
        static_cast<u16>( chr->faster_cast_recovery().sum() ) );  // Faster cast recovery
    msg->WriteFlipped<u16>( static_cast<u16>( chr->faster_casting().sum() ) );   // Faster casting
    msg->WriteFlipped<u16>( static_cast<u16>( chr->lower_mana_cost().sum() ) );  // Lower mana cost
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
  msg->Write( Clib::strUtf8ToCp1252( chr->name() ).c_str(), 30, false );

  if ( networkManager.uoclient_general.hits.any )
  {
    msg->WriteFlipped<u16>( Clib::clamp_convert<u16>(
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
    auto h = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.hits.id ).current_ones() );
    auto mh = Clib::clamp_convert<u16>(
        chr->vital( networkManager.uoclient_general.hits.id ).maximum_ones() );
    msg->WriteFlipped<u16>( mh );
    msg->WriteFlipped<u16>( h );

    // Send proper data to self (if we exist?)
    if ( chr->client && chr->client->ready )
      msg.Send( chr->client );

    // To stop "HP snooping"...
    msg->offset = 5;
    msg->WriteFlipped<u16>( 1000u );
    msg->WriteFlipped<u16>( Clib::clamp_convert<u16>( h * 1000 / mh ) );
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
}  // namespace Pol::Core

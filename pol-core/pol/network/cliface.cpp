/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 */


#include "cliface.h"

#include <memory>
#include <stddef.h>

#include "../../clib/rawtypes.h"
#include "../globals/network.h"
#include "../globals/settings.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../network/packethelper.h"
#include "../party.h"
#include "../statmsg.h"
#include "../ufunc.h"
#include "../uoclient.h"
#include "../uoskills.h"
#include "../vital.h"
#include "client.h"
#include "packets.h"


namespace Pol::Network
{
void initialize_client_interfaces()
{
  Core::networkManager.uo_client_interface->Initialize();
}

ClientVitalUpdaters::ClientVitalUpdaters()
    : my_vital_changed( nullptr ), others_vital_changed( nullptr )
{
}

ClientAttributeUpdaters::ClientAttributeUpdaters() : my_attr_changed( nullptr ), pUOSkill( nullptr )
{
}

void ClientInterface::register_client( Client* client )
{
  clients.push_back( client );
}
void ClientInterface::deregister_client( Client* client )
{
  for ( unsigned i = 0; i < clients.size(); ++i )
  {
    if ( clients[i] == client )
    {
      clients[i] = clients.back();
      clients.pop_back();
      break;
    }
  }
}

// static
void ClientInterface::tell_vital_changed( Mobile::Character* who, const Core::Vital* vital )
{
  Client* client = who->client;
  if ( client && client->ready )
  {
    const ClientVitalUpdaters& cvu = client->Interface.vital_updaters[vital->vitalid];
    if ( cvu.my_vital_changed )
      cvu.my_vital_changed( client, who, vital );
  }

  // on all the client interfaces, for their connected clients, possibly tell them
  Core::networkManager.uo_client_interface->bcast_vital_changed( who, vital );
}
void ClientInterface::tell_attribute_changed( Mobile::Character* who,
                                              const Mobile::Attribute* attr )
{
  Client* client = who->client;
  if ( client && client->ready )
  {
    const ClientAttributeUpdaters& cau = client->Interface.attribute_updaters[attr->attrid];
    if ( cau.my_attr_changed )
      cau.my_attr_changed( client, who, attr );
  }
}

/// Just like send_uo_hits, but sends hits on a 1000-basis instead
void send_fake_hits( Client* client, Mobile::Character* me, const Core::Vital* vital )
{
  PktHelper::PacketOut<PktOut_A1> msg;
  msg->Write<u32>( me->serial_ext );
  msg->WriteFlipped<u16>( 1000u );
  msg->WriteFlipped<u16>( static_cast<u16>( me->vital( vital->vitalid ).current_thousands() ) );
  msg.Send( client );
}

void send_uo_hits( Client* client, Mobile::Character* me, const Core::Vital* vital )
{
  PktHelper::PacketOut<PktOut_A1> msg;
  msg->Write<u32>( me->serial_ext );
  int v = me->vital( vital->vitalid ).maximum_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );

  v = me->vital( vital->vitalid ).current_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  msg.Send( client );
}

void send_uo_mana( Client* client, Mobile::Character* me, const Core::Vital* vital )
{
  PktHelper::PacketOut<PktOut_A2> msg;
  msg->Write<u32>( me->serial_ext );
  int v = me->vital( vital->vitalid ).maximum_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );

  v = me->vital( vital->vitalid ).current_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  msg.Send( client );

  if ( me->has_party() )
    me->party()->on_mana_changed( me );
}

void send_uo_stamina( Client* client, Mobile::Character* me, const Core::Vital* vital )
{
  PktHelper::PacketOut<PktOut_A3> msg;
  msg->Write<u32>( me->serial_ext );
  int v = me->vital( vital->vitalid ).maximum_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );

  v = me->vital( vital->vitalid ).current_ones();
  if ( v > 0xFFFF )
    v = 0xFFFF;
  msg->WriteFlipped<u16>( static_cast<u16>( v ) );
  msg.Send( client );

  if ( me->has_party() )
    me->party()->on_stam_changed( me );
}
void send_uo_strength( Client* client, Mobile::Character* me, const Mobile::Attribute* /*attr*/ )
{
  Core::send_full_statmsg( client, me );
}
void send_uo_dexterity( Client* client, Mobile::Character* me, const Mobile::Attribute* /*attr*/ )
{
  Core::send_full_statmsg( client, me );
}
void send_uo_intelligence( Client* client, Mobile::Character* me,
                           const Mobile::Attribute* /*attr*/ )
{
  Core::send_full_statmsg( client, me );
}
void send_uo_skill( Client* client, Mobile::Character* me, const Mobile::Attribute* attr )
{
  ClientAttributeUpdaters& cau = client->Interface.attribute_updaters[attr->attrid];

  PktHelper::PacketOut<PktOut_3A> msg;
  msg->offset += 2;
  if ( !Core::settingsManager.ssopt.core_sends_caps )
    msg->Write<u8>( Core::PKTBI_3A_VALUES::SINGLE_SKILL );
  else
    msg->Write<u8>( Core::PKTBI_3A_VALUES::SINGLE_SKILL_CAP );
  msg->WriteFlipped<u16>( cau.pUOSkill->skillid );
  const Mobile::AttributeValue& av = me->attribute( attr->attrid );
  msg->WriteFlipped<u16>( static_cast<u16>( av.effective_tenths() ) );  // value
  msg->WriteFlipped<u16>(
      static_cast<u16>( av.base() ) );  // value_unmod base is always in tenths...
  msg->Write<u8>( av.lock() );
  if ( Core::settingsManager.ssopt.core_sends_caps )
    msg->WriteFlipped<u16>( av.cap() );
  u16 len = msg->offset;
  msg->offset = 1;
  msg->WriteFlipped<u16>( len );
  msg.Send( client, len );
}
void ClientInterface::Initialize()
{
  vital_updaters.resize( Core::gamestate.vitals.size() );
  attribute_updaters.resize( Core::gamestate.attributes.size() );
}

void UOClientInterface::Initialize()
{
  ClientInterface::Initialize();

  // tell a player's own client when his life changes
  if ( Core::networkManager.uoclient_general.hits.any )
  {
    vital_updaters[Core::networkManager.uoclient_general.hits.id].my_vital_changed = send_uo_hits;
    vital_updaters[Core::networkManager.uoclient_general.hits.id].others_vital_changed =
        send_fake_hits;
  }
  if ( Core::networkManager.uoclient_general.mana.any )
  {
    vital_updaters[Core::networkManager.uoclient_general.mana.id].my_vital_changed = send_uo_mana;
  }
  if ( Core::networkManager.uoclient_general.stamina.any )
  {
    vital_updaters[Core::networkManager.uoclient_general.stamina.id].my_vital_changed =
        send_uo_stamina;
  }

  if ( Core::networkManager.uoclient_general.strength.any )
  {
    attribute_updaters[Core::networkManager.uoclient_general.strength.id].my_attr_changed =
        send_uo_strength;
  }
  if ( Core::networkManager.uoclient_general.intelligence.any )
  {
    attribute_updaters[Core::networkManager.uoclient_general.intelligence.id].my_attr_changed =
        send_uo_intelligence;
  }
  if ( Core::networkManager.uoclient_general.dexterity.any )
  {
    attribute_updaters[Core::networkManager.uoclient_general.dexterity.id].my_attr_changed =
        send_uo_dexterity;
  }

  for ( unsigned short i = 0; i < Core::networkManager.uoclient_general.maxskills + 1; ++i )
  {
    const Core::UOSkill& uoskill = Core::GetUOSkill( i );
    if ( uoskill.inited && uoskill.pAttr )
    {
      attribute_updaters[uoskill.pAttr->attrid].my_attr_changed = send_uo_skill;
      attribute_updaters[uoskill.pAttr->attrid].pUOSkill = &uoskill;
    }
  }
}

void UOClientInterface::bcast_vital_changed( Mobile::Character* who,
                                             const Core::Vital* vital ) const
{
  const ClientVitalUpdaters& cvu = vital_updaters[vital->vitalid];
  if ( cvu.others_vital_changed != nullptr )
  {
    for ( auto& client : clients )
    {
      if ( who->client != client && client->ready && client->chr->in_visual_range( who ) )
      {
        cvu.others_vital_changed( client, who, vital );
      }
    }
  }
}
}  // namespace Pol::Network

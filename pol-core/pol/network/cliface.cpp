/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID

Notes
=======

*/

#include "../../clib/stl_inc.h"
#include "../../clib/endian.h"

#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../network/packets.h"
#include "client.h"
#include "cliface.h"
#include "../party.h"
#include "../pktboth.h"
#include "../ssopt.h"
#include "../statmsg.h"
#include "../ufunc.h"
#include "../uoclient.h"
#include "../uoskills.h"
#include "../vital.h"

UOClientInterface uo_client_interface;

void initialize_client_interfaces()
{
    uo_client_interface.Initialize();
}

ClientVitalUpdaters::ClientVitalUpdaters() :
    my_vital_changed(NULL),
    others_vital_changed(NULL)
{
}

ClientAttributeUpdaters::ClientAttributeUpdaters() :
    my_attr_changed(NULL)
{
}

void ClientInterface::register_client( Client* client )
{
    clients.push_back( client );
}
void ClientInterface::deregister_client( Client* client )
{
    for( unsigned i = 0; i < clients.size(); ++i )
    {
        if (clients[i] == client)
        {
            clients[i] = clients.back();
            clients.pop_back();
            break;
        }
    }
}

// static
void ClientInterface::tell_vital_changed( Character* who, const Vital* vital )
{
    Client* client = who->client;
    if (client && client->ready)
    {
        const ClientVitalUpdaters& cvu = client->Interface.vital_updaters[ vital->vitalid ];
        if (cvu.my_vital_changed)
            cvu.my_vital_changed(client,who,vital);
    }

    // on all the client interfaces, for their connected clients, possibly tell them 
    uo_client_interface.bcast_vital_changed( who, vital );
}
void ClientInterface::tell_attribute_changed( Character* who, const Attribute* attr )
{
    Client* client = who->client;
    if (client && client->ready)
    {
        const ClientAttributeUpdaters& cau = client->Interface.attribute_updaters[ attr->attrid ];
        if (cau.my_attr_changed)
            cau.my_attr_changed(client,who,attr);
    }
}


void send_uo_hits( Client* client, Character* me, const Vital* vital )
{
	PktOut_A1* msg = REQUESTPACKET(PktOut_A1,PKTOUT_A1_ID);
	msg->Write(me->serial_ext);
	int v = me->vital( vital->vitalid ).maximum_ones();
	if (v > 0xFFFF)
		v = 0xFFFF;
	msg->WriteFlipped(static_cast<u16>(v));

    v = me->vital( vital->vitalid ).current_ones();
    if (v > 0xFFFF)
        v = 0xFFFF;
    msg->WriteFlipped(static_cast<u16>(v));
    client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_uo_mana( Client* client, Character* me, const Vital* vital )
{
	PktOut_A2* msg = REQUESTPACKET(PktOut_A2,PKTOUT_A2_ID);
	msg->Write(me->serial_ext);
	int v = me->vital( vital->vitalid ).maximum_ones();
	if (v > 0xFFFF)
		v = 0xFFFF;
	msg->WriteFlipped(static_cast<u16>(v));

	v = me->vital( vital->vitalid ).current_ones();
	if (v > 0xFFFF)
		v = 0xFFFF;
	msg->WriteFlipped(static_cast<u16>(v));
	client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);

	if (me->party() != NULL)
		me->party()->on_mana_changed(me);
}

void send_uo_stamina( Client* client, Character* me, const Vital* vital )
{
	PktOut_A3* msg = REQUESTPACKET(PktOut_A3,PKTOUT_A3_ID);
	msg->Write(me->serial_ext);
	int v = me->vital( vital->vitalid ).maximum_ones();
	if (v > 0xFFFF)
		v = 0xFFFF;
	msg->WriteFlipped(static_cast<u16>(v));

	v = me->vital( vital->vitalid ).current_ones();
	if (v > 0xFFFF)
		v = 0xFFFF;
	msg->WriteFlipped(static_cast<u16>(v));
	client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);

	if (me->party()!=NULL)
		me->party()->on_stam_changed(me);
}
void send_uo_strength( Client* client, Character* me, const Attribute* attr )
{
    send_full_statmsg( client, me );
}
void send_uo_dexterity( Client* client, Character* me, const Attribute* attr )
{
    send_full_statmsg( client, me );
}
void send_uo_intelligence( Client* client, Character* me, const Attribute* attr )
{
    send_full_statmsg( client, me );
}
void send_uo_skill( Client* client, Character* me, const Attribute* attr )
{
    ClientAttributeUpdaters& cau = client->Interface.attribute_updaters[ attr->attrid ];

	if (!ssopt.core_sends_caps) {
		PKTBI_3A_VALUES msg;
		unsigned short msglen = offsetof(PKTBI_3A_VALUES,skills) +
								sizeof(msg.skills[0]);

		msg.msgtype = PKTBI_3A_VALUES_ID;
		msg.msglen = ctBEu16( msglen );
		msg.unk3 = PKTBI_3A_VALUES::SINGLE_SKILL;
		msg.skills[0].skillid = ctBEu16(cau.pUOSkill->skillid); // er, no conversion here. !!
		const AttributeValue& av = me->attribute(attr->attrid);
		msg.skills[0].value = ctBEu16( static_cast<u16>(av.effective_tenths()) );
		msg.skills[0].value_unmod = ctBEu16( static_cast<u16>(av.base()) ); // base is always in tenths...
		msg.skills[0].lock_mode = (u8)av.lock();//PKTBI_3A_VALUES::LOCK_NONE;
		client->transmit( &msg, msglen );
	} else {
		PKTBI_3A_CAPS msg;
		unsigned short msglen = offsetof(PKTBI_3A_VALUES,skills) +
								sizeof(msg.skills[0]);

		msg.msgtype = PKTBI_3A_VALUES_ID;
		msg.msglen = ctBEu16( msglen );
		msg.unk3 = PKTBI_3A_VALUES::SINGLE_SKILL_CAP;
		msg.skills[0].skillid = ctBEu16(cau.pUOSkill->skillid); // er, no conversion here. !!
		const AttributeValue& av = me->attribute(attr->attrid);
		msg.skills[0].value = ctBEu16( static_cast<u16>(av.effective_tenths()) );
		msg.skills[0].value_unmod = ctBEu16( static_cast<u16>(av.base()) ); // base is always in tenths...
		msg.skills[0].lock_mode = (u8)av.lock();//PKTBI_3A_VALUES::LOCK_NONE;
		msg.skills[0].cap = ctBEu16( static_cast<u16>(av.cap()) );
		client->transmit( &msg, msglen );
	}
}
void ClientInterface::Initialize()
{
    vital_updaters.resize( vitals.size() );
    attribute_updaters.resize( attributes.size() );
}

void UOClientInterface::Initialize()
{
    ClientInterface::Initialize();

    // tell a player's own client when his life changes
    if (uoclient_general.hits.any)
    {
        vital_updaters[ uoclient_general.hits.id ].my_vital_changed = send_uo_hits;
        vital_updaters[ uoclient_general.hits.id ].others_vital_changed = send_uo_hits;
    }
    if (uoclient_general.mana.any)
    {
        vital_updaters[ uoclient_general.mana.id ].my_vital_changed = send_uo_mana;
    }
    if (uoclient_general.stamina.any)
    {
        vital_updaters[ uoclient_general.stamina.id ].my_vital_changed = send_uo_stamina;
    }

    if (uoclient_general.strength.any)
    {
        attribute_updaters[ uoclient_general.strength.id ].my_attr_changed = send_uo_strength;
    }
    if (uoclient_general.intelligence.any)
    {
        attribute_updaters[ uoclient_general.intelligence.id ].my_attr_changed = send_uo_intelligence;
    }
    if (uoclient_general.dexterity.any)
    {
        attribute_updaters[ uoclient_general.dexterity.id ].my_attr_changed = send_uo_dexterity;
    }

    for( unsigned i = 0; i < uoclient_general.maxskills+1; ++i )
    {
        const UOSkill& uoskill = GetUOSkill( i );
        if (uoskill.inited && uoskill.pAttr)
        {
            attribute_updaters[ uoskill.pAttr->attrid ].my_attr_changed = send_uo_skill;
            attribute_updaters[ uoskill.pAttr->attrid ].pUOSkill = &uoskill;
        }
    }
}

void UOClientInterface::bcast_vital_changed( Character* who, const Vital* vital ) const
{
    const ClientVitalUpdaters& cvu = vital_updaters[ vital->vitalid ];
    if (cvu.others_vital_changed != NULL)
    {
        for( unsigned i = 0; i < clients.size(); ++i )
        {
            Client* client = clients[i];
            if (client->ready && inrange(who, client->chr))
            {
                cvu.others_vital_changed(client,who,vital);
            }
        }
    }
}

/*
History
=======
2006/05/23 Shinigami: send_full_statmsg() updated to support Elfs
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/09 MuadDib:   Re factor of Packet 0x25, 0x11 for naming convention
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
2009/12/02 Turley:    fixed 0x11 packet (race flag)

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/clib.h"
#include "../clib/endian.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "sockio.h"
#include "pktboth.h"
#include "ufunc.h"
#include "uoclient.h"
#include "uvars.h"

#include "statmsg.h"

#include "ssopt.h"

using namespace PktHelper;

void send_full_statmsg( Client *client, Character *chr )
{
	PacketOut<PktOut_11> msg;
	msg->offset+=2; // msglen
	msg->Write(chr->serial_ext);
	msg->Write(chr->name().c_str(),30,false);
	if (uoclient_general.hits.any)
	{
		int v = chr->vital(uoclient_general.hits.id).current_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) ); // hits

		v = chr->vital(uoclient_general.hits.id).maximum_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) ); // max_hits
	}
	else
	{
		msg->WriteFlipped(static_cast<u16>(0));  // hits
		msg->WriteFlipped(static_cast<u16>(0));  // max_hits
	}
	msg->Write(static_cast<u8>(0)); // (client->chr->can_rename( chr ) ? 0xFF : 0);
	if ( (client->UOExpansionFlag & ML) && (client->ClientType & CLIENTTYPE_5000) )
		msg->Write(static_cast<u8>(5)); // Set to ML level
	else if ( (client->UOExpansionFlag & AOS) )
		msg->Write(static_cast<u8>(4)); // Set to AOS level statbar for full info
	else
		msg->Write(static_cast<u8>(1)); // Set to oldschool statbar info.

	//if (chr->race == RACE_ELF)
	//  msg->Write(static_cast<u8>(chr->gender | FLAG_RACE));
	//else
	msg->Write(static_cast<u8>(chr->gender)); // GENDER_MALE or GENDER_FEMALE (see uconst.h)

	if (uoclient_general.strength.any)
	{
		int v = chr->attribute(uoclient_general.strength.id).effective();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );
	}
	else
		msg->WriteFlipped( static_cast<u16>(0) );

	if (uoclient_general.dexterity.any)
	{
		int v = chr->attribute(uoclient_general.dexterity.id).effective();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );
	}
	else
		msg->WriteFlipped(static_cast<u16>(0));

	if (uoclient_general.intelligence.any)
	{
		int v = chr->attribute(uoclient_general.intelligence.id).effective();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );
	}
	else
		msg->WriteFlipped( static_cast<u16>(0) );

	if (uoclient_general.stamina.any)
	{
		int v = chr->vital(uoclient_general.stamina.id).current_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );

		v = chr->vital( uoclient_general.stamina.id ).maximum_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );
	}
	else
	{
		msg->WriteFlipped( static_cast<u16>(0) );
		msg->WriteFlipped( static_cast<u16>(0) );
	}

	if (uoclient_general.mana.any)
	{
		int v = chr->vital(uoclient_general.mana.id).current_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );

		v = chr->vital(uoclient_general.mana.id).maximum_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped( static_cast<u16>(v) );
	}
	else
	{
		msg->WriteFlipped( static_cast<u16>(0) );
		msg->WriteFlipped( static_cast<u16>(0) );
	}

	msg->WriteFlipped( chr->gold_carried() );
	// Adjusted to work with Physical Resist if AOS client, and AOS Resistances enabled.
	if( (client->UOExpansionFlag & AOS) && client->aosresist )
		msg->WriteFlipped( (chr->element_resist.physical < 0) ?
		     static_cast<u16>(0x10000+chr->element_resist.physical) :
			static_cast<u16>(chr->element_resist.physical) );
	else
		msg->WriteFlipped( chr->ar() );

	msg->WriteFlipped( static_cast<u16>(chr->weight()) );


	// moreinfo 5
	if ( (client->UOExpansionFlag & ML) && (client->ClientType & CLIENTTYPE_5000) )
	{
		msg->WriteFlipped( chr->carrying_capacity() );
		msg->Write( static_cast<u8>(chr->race+1) );
	}

	// moreinfo 3 start
	if ( (client->UOExpansionFlag & AOS) )
	{
		msg->WriteFlipped( chr->expanded_statbar.statcap );
		msg->Write( chr->expanded_statbar.followers );
		msg->Write( chr->expanded_statbar.followers_max );
		// moreinfo 4 start
		msg->WriteFlipped( (chr->element_resist.fire < 0) ?
			static_cast<u16>(0x10000+chr->element_resist.fire) :
			static_cast<u16>(chr->element_resist.fire) );
		msg->WriteFlipped( (chr->element_resist.cold < 0) ?
			static_cast<u16>(0x10000+chr->element_resist.cold) :
			static_cast<u16>(chr->element_resist.cold) );
		msg->WriteFlipped( (chr->element_resist.poison < 0) ?
			static_cast<u16>(0x10000+chr->element_resist.poison) :
			static_cast<u16>(chr->element_resist.poison) );
		msg->WriteFlipped( (chr->element_resist.energy < 0) ?
			static_cast<u16>(0x10000+chr->element_resist.energy) :
			static_cast<u16>(chr->element_resist.energy) );
		msg->WriteFlipped( chr->expanded_statbar.luck );
		msg->WriteFlipped( chr->min_weapon_damage() );
		msg->WriteFlipped( chr->max_weapon_damage() );
		msg->WriteFlipped( chr->expanded_statbar.tithing );
	}
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	msg.Send(client,len);

	if (ssopt.send_stat_locks)
		send_stat_locks(client, chr);
}

void send_stat_locks (Client *client, Character *chr) {
	if (client->getversiondetail().major < 3) // only in AOS, I think
		return;

	u8 lockbit = 0;
	
	lockbit |= chr->attribute(uoclient_general.strength.id).lock()  << 4; //XX SS DD II (2 bits for each lock)
	lockbit |= chr->attribute(uoclient_general.dexterity.id).lock() << 2;
	lockbit |= chr->attribute(uoclient_general.intelligence.id).lock();

	PacketOut<PktOut_BF_Sub19> msg;
	msg->WriteFlipped(static_cast<u16>(12));
	msg->offset+=2; //sub
	msg->Write(static_cast<u8>(0x02)); // 2D Client = 0x02, KR = 0x05
	msg->Write(chr->serial_ext);
	msg->offset++; //unk
	msg->Write(lockbit);
	msg.Send(client);
}

void send_short_statmsg( Client *client, Character *chr )
{
	PacketOut<PktOut_11> msg;
	msg->offset+=2; // msglen
	msg->Write(chr->serial_ext);
	msg->Write(chr->name().c_str(),30,false);

	if (uoclient_general.hits.any)
	{
		int h, mh;
		h = chr->vital(uoclient_general.hits.id).current_ones();
		if (h > 0xFFFF)
			h = 0xFFFF;
		mh = chr->vital(uoclient_general.hits.id).maximum_ones();
		if (mh > 0xFFFF)
			mh = 0xFFFF;

		msg->WriteFlipped( static_cast<u16>(h * 1000 / mh) );
		msg->WriteFlipped( static_cast<u16>(1000) ); // max_hits
	}
	else
	{
		msg->WriteFlipped(static_cast<u16>(0));  // hits
		msg->WriteFlipped(static_cast<u16>(0));  // max_hits
	}
	msg->Write(static_cast<u8>((client->chr->can_rename( chr ) ? 0xFF : 0)));
	msg->Write(static_cast<u8>(0)); //moreinfo

	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

	msg.Send(client, len );
}

void send_update_hits_to_inrange( Character *chr )
{
	PacketOut<PktOut_A1> msg;
	msg->Write(chr->serial_ext);
	
    if (uoclient_general.hits.any)
    {
        int h = chr->vital(uoclient_general.hits.id).current_ones();
        if (h > 0xFFFF)
            h = 0xFFFF;
        int mh = chr->vital(uoclient_general.hits.id).maximum_ones();
        if (mh > 0xFFFF)
            mh = 0xFFFF;
		msg->WriteFlipped(static_cast<u16>(mh));
		msg->WriteFlipped(static_cast<u16>(h));

		// Send proper data to self (if we exist?)
		if (chr->client && chr->client->ready)
			msg.Send(chr->client);

		// To stop "HP snooping"...
		msg->offset=5;
		msg->WriteFlipped(static_cast<u16>(1000));
		msg->WriteFlipped(static_cast<u16>(h * 1000 / mh));
    }
    else
    {
		msg->offset+=4; // hits,maxhits=0
		if (chr->client && chr->client->ready)
			msg.Send(chr->client);
    }

	// Exclude self... otherwise their status-window shows 1000 hp!! >_<
	transmit_to_others_inrange( chr, &msg->buffer, msg->offset, false, false );
}

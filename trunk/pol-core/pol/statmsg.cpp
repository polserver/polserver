/*
History
=======
2006/05/23 Shinigami: send_full_statmsg() updated to support Elfs
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/09 MuadDib:   Re factor of Packet 0x25, 0x11 for naming convention

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/clib.h"
#include "clib/endian.h"

#include "mobile/charactr.h"
#include "client.h"
#include "sockio.h"
#include "pktboth.h"
#include "ufunc.h"
#include "uoclient.h"
#include "uvars.h"

#include "statmsg.h"

#include "ssopt.h"

void send_full_statmsg( Client *client, Character *chr )
{
	// With ML and newer clients adding stuff mid-packet to this one, and it
	// NOT being a "6017" issue, we are splitting the send full now to easier
	// maintain this control with it. Unless they stick even more stuff mid-packet
	// after KR (like in SA - Stygian Abyss), all newer supported stuff will go
	// to the send_full_statmsg_new() function. Boooo to OSI!
	if ( (client->UOExpansionFlag & ML) && (client->getversiondetail().major>=5 || client->isUOKR) )
		send_full_statmsg_new(client, chr);
	else
		send_full_statmsg_std(client, chr);

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


	PKTBI_BF msg;
	msg.msgtype = PKTBI_BF_ID;
	msg.msglen = ctBEu16(12);
	msg.subcmd = ctBEu16(PKTBI_BF::TYPE_EXTENDED_STATS_OUT);
	msg.extstatsout.type = 0x02; // 2D Client = 0x02, KR = 0x05
	msg.extstatsout.serial = chr->serial_ext;
	msg.extstatsout.unk = 0;
	msg.extstatsout.lockbits = lockbit;

	client->transmit(&msg, 12);
}

// This is for legacy up to 5.0(?). Cuz ML added new bytes in the middle of the packet.
void send_full_statmsg_std( Client *client, Character *chr )
{
	PKTOUT_11_V1 msg;
	msg.msgtype = PKTOUT_11_V1_ID;

	if( (client->UOExpansionFlag & AOS) )
	{
		msg.msglen = ctBEu16(sizeof msg);
		msg.moreinfo = 04;  // Set to AOS level statbar for full info
	}
	else
	{
		unsigned short msglen = offsetof( PKTOUT_11_V1, statcap );
		msg.msglen = ctBEu16(msglen);
		msg.moreinfo = 01;  // Set to oldschool statbar info.
	}
	msg.serial = chr->serial_ext;
	strzcpy( msg.name, chr->name().c_str(), sizeof msg.name );
	
    if (uoclient_general.hits.any)
    {
        long v = chr->vital(uoclient_general.hits.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.hits = ctBEu16( static_cast<u16>(v) );

        v = chr->vital(uoclient_general.hits.id).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.max_hits = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.hits = 0;
        msg.max_hits = 0;
    }
	
	msg.renameable = 0; // (client->chr->can_rename( chr ) ? 0xFF : 0);

	if (chr->race == RACE_ELF)
		msg.gender = static_cast<u8>(chr->gender | FLAG_RACE);
	else
		msg.gender = static_cast<u8>(chr->gender);
	
    if (uoclient_general.strength.any)
    {
        long v = chr->attribute(uoclient_general.strength.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.str = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.str = 0;
    }

    if (uoclient_general.dexterity.any)
    {
        long v = chr->attribute(uoclient_general.dexterity.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.dex = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.dex = 0;
    }
    
    if (uoclient_general.intelligence.any)
    {
        long v = chr->attribute(uoclient_general.intelligence.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.intel = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.intel = 0;
    }
	
    if (uoclient_general.stamina.any)
    {
        long v = chr->vital(uoclient_general.stamina.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.stamina = ctBEu16( static_cast<u16>(v) );

        v = chr->vital( uoclient_general.stamina.id ).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;

	    msg.max_stamina = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.stamina = 0;
	    msg.max_stamina = 0;
    }
	
    if (uoclient_general.mana.any)
    {
        long v = chr->vital(uoclient_general.mana.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.mana = ctBEu16( static_cast<u16>(v) );

        v = chr->vital(uoclient_general.mana.id).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
	    msg.max_mana = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.mana = 0;
	    msg.max_mana = 0;
    }

	
    msg.gold = ctBEu32( chr->gold_carried() );
	// Adjusted to work with Physical Resist if AOS client, and AOS Resistances enabled.
	if( (client->UOExpansionFlag & AOS) && client->aosresist )
		msg.AR = (chr->element_resist.physical < 0)?ctBEu16(0x10000+chr->element_resist.physical):ctBEu16(chr->element_resist.physical);
	else
		msg.AR = ctBEu16( chr->ar() );

	msg.weight = ctBEu16( static_cast<u16>(chr->weight()) );

	if ( msg.moreinfo >= 4 )
	{
		msg.statcap = ctBEu16( chr->expanded_statbar.statcap );
		msg.followers = chr->expanded_statbar.followers;
		msg.followers_max = chr->expanded_statbar.followers_max;
		msg.fireresist = (chr->element_resist.fire < 0)?ctBEu16(0x10000+chr->element_resist.fire):ctBEu16(chr->element_resist.fire);
		msg.coldresist = (chr->element_resist.cold < 0)?ctBEu16(0x10000+chr->element_resist.cold):ctBEu16(chr->element_resist.cold);
		msg.poisonresist = (chr->element_resist.poison < 0)?ctBEu16(0x10000+chr->element_resist.poison):ctBEu16(chr->element_resist.poison);
		msg.energyresist = (chr->element_resist.energy < 0)?ctBEu16(0x10000+chr->element_resist.energy):ctBEu16(chr->element_resist.energy);
		msg.luck = ctBEu16( chr->expanded_statbar.luck );
		msg.damage_min = ctBEu16( chr->min_weapon_damage() );
		msg.damage_max = ctBEu16( chr->max_weapon_damage() );
		msg.titching = ctBEu32( chr->expanded_statbar.tithing );
	}

	transmit(client, &msg, cfBEu16(msg.msglen) );
}

// This is for ML clients and newer. Booyah. :(
void send_full_statmsg_new( Client *client, Character *chr )
{
	PKTOUT_11_V2 msg;
	msg.msgtype = PKTOUT_11_V2_ID;

	if( (client->UOExpansionFlag & ML) )
	{
		msg.msglen = ctBEu16(sizeof msg);
		msg.moreinfo = 05;  // Set to AOS level statbar for full info
	}
	else
	{
		unsigned short msglen = offsetof( PKTOUT_11_V2, max_weight );
		msg.msglen = ctBEu16(msglen);
		msg.moreinfo = 01;  // Set to oldschool statbar info.
	}
	msg.serial = chr->serial_ext;
	strzcpy( msg.name, chr->name().c_str(), sizeof msg.name );
	
    if (uoclient_general.hits.any)
    {
        long v = chr->vital(uoclient_general.hits.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.hits = ctBEu16( static_cast<u16>(v) );

        v = chr->vital(uoclient_general.hits.id).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.max_hits = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.hits = 0;
        msg.max_hits = 0;
    }
	
	msg.renameable = 0; // (client->chr->can_rename( chr ) ? 0xFF : 0);

	if (chr->race == RACE_ELF)
		msg.gender = static_cast<u8>(chr->gender | FLAG_RACE);
	else
		msg.gender = static_cast<u8>(chr->gender);
	
    if (uoclient_general.strength.any)
    {
        long v = chr->attribute(uoclient_general.strength.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.str = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.str = 0;
    }

    if (uoclient_general.dexterity.any)
    {
        long v = chr->attribute(uoclient_general.dexterity.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.dex = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.dex = 0;
    }
    
    if (uoclient_general.intelligence.any)
    {
        long v = chr->attribute(uoclient_general.intelligence.id).effective();
        if (v > 0xFFFF)
            v = 0xFFFF;
       
        msg.intel = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.intel = 0;
    }
	
    if (uoclient_general.stamina.any)
    {
        long v = chr->vital(uoclient_general.stamina.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.stamina = ctBEu16( static_cast<u16>(v) );

        v = chr->vital( uoclient_general.stamina.id ).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;

	    msg.max_stamina = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.stamina = 0;
	    msg.max_stamina = 0;
    }
	
    if (uoclient_general.mana.any)
    {
        long v = chr->vital(uoclient_general.mana.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.mana = ctBEu16( static_cast<u16>(v) );

        v = chr->vital(uoclient_general.mana.id).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
	    msg.max_mana = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.mana = 0;
	    msg.max_mana = 0;
    }

	
    msg.gold = ctBEu32( chr->gold_carried() );

	// Adjusted to work with Physical Resist if AOS client, and AOS Resistances enabled.
	if( (client->UOExpansionFlag & AOS) && client->aosresist )
		msg.AR = (chr->element_resist.physical < 0)?ctBEu16(0x10000+chr->element_resist.physical):ctBEu16(chr->element_resist.physical);
	else
		msg.AR = ctBEu16( chr->ar() );

	msg.weight = ctBEu16( static_cast<u16>(chr->weight()) );

	if (msg.moreinfo >= 4)
	{
		msg.statcap = ctBEu16( chr->expanded_statbar.statcap );
		msg.followers = chr->expanded_statbar.followers;
		msg.followers_max = chr->expanded_statbar.followers_max;
		msg.fireresist = (chr->element_resist.fire < 0)?ctBEu16(0x10000+chr->element_resist.fire):ctBEu16(chr->element_resist.fire);
		msg.coldresist = (chr->element_resist.cold < 0)?ctBEu16(0x10000+chr->element_resist.cold):ctBEu16(chr->element_resist.cold);
		msg.poisonresist = (chr->element_resist.poison < 0)?ctBEu16(0x10000+chr->element_resist.poison):ctBEu16(chr->element_resist.poison);
		msg.energyresist = (chr->element_resist.energy < 0)?ctBEu16(0x10000+chr->element_resist.energy):ctBEu16(chr->element_resist.energy);
		msg.luck = ctBEu16( chr->expanded_statbar.luck );
		msg.damage_min = ctBEu16( chr->min_weapon_damage() );
		msg.damage_max = ctBEu16( chr->max_weapon_damage() );
		msg.titching = ctBEu32( chr->expanded_statbar.tithing );
	}

	if (msg.moreinfo >= 5)
	{
		msg.max_weight = ctBEu16(chr->carrying_capacity());
		msg.race = static_cast<u8>(chr->race);
	}

	transmit(client, &msg, cfBEu16(msg.msglen) );
}

void send_short_statmsg( Client *client, Character *chr )
{
	unsigned short msglen = offsetof( PKTOUT_11_V1, gender );

	PKTOUT_11_V1 msg;
	msg.msgtype = PKTOUT_11_V1_ID;
	msg.msglen = ctBEu16(msglen);
	msg.serial = chr->serial_ext;
	strzcpy( msg.name, chr->name().c_str(), sizeof msg.name );

    if (uoclient_general.hits.any)
    {
		long h, mh;

        h = chr->vital(uoclient_general.hits.id).current_ones();
        if (h > 0xFFFF)
            h = 0xFFFF;
        //msg.hits = ctBEu16( static_cast<u16>(h) );

        mh = chr->vital(uoclient_general.hits.id).maximum_ones();
        if (mh > 0xFFFF)
            mh = 0xFFFF;
        //msg.max_hits = ctBEu16( static_cast<u16>(mh) );

		msg.hits = ctBEu16( static_cast<u16>(h * 1000 / mh) );
		msg.max_hits = ctBEu16( 1000 );
    }
    else
    {
        msg.hits = 0;
        msg.max_hits = 0;
    }

    msg.renameable = (client->chr->can_rename( chr ) ? 0xFF : 0);
	msg.moreinfo = 0;

	transmit(client, &msg, msglen );
}

void send_update_hits_to_inrange( Character *chr )
{
	PKTOUT_A1 msg;
	msg.msgtype = PKTOUT_A1_ID;
	msg.serial = chr->serial_ext;
	long h, mh;
    if (uoclient_general.hits.any)
    {
        h = chr->vital(uoclient_general.hits.id).current_ones();
        if (h > 0xFFFF)
            h = 0xFFFF;
        msg.hits = ctBEu16( static_cast<u16>(h) );

        mh = chr->vital(uoclient_general.hits.id).maximum_ones();
        if (mh > 0xFFFF)
            mh = 0xFFFF;
        msg.max_hits = ctBEu16( static_cast<u16>(mh) );

		// Send proper data to self (if we exist?)
		if (chr->client && chr->client->ready)
			transmit( chr->client, &msg, sizeof msg );

		// To stop "HP snooping"...
		msg.hits = ctBEu16( static_cast<u16>(h * 1000 / mh) );
		msg.max_hits = ctBEu16( 1000 );
    }
    else
    {
        msg.hits = 0;
        msg.max_hits = 0;
		if (chr->client && chr->client->ready)
			transmit( chr->client, &msg, sizeof msg );
    }

	// Exclude self... otherwise their status-window shows 1000 hp!! >_<
	transmit_to_others_inrange( chr, &msg, sizeof msg, false, false );

}

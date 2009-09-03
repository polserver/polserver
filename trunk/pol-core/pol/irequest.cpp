/*
History
=======
2003/03/15 Dave:      Support configurable max skillid
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/endian.h"
#include "clib/fdump.h"
#include "clib/clib.h"

#include "attribute.h"
#include "charactr.h"
#include "client.h"
#include "msghandl.h"
#include "pktboth.h"
#include "pktin.h"
#include "polcfg.h"
#include "scrdef.h"
#include "scrstore.h"
#include "statmsg.h"
#include "uoskills.h"
#include "uvars.h"
#include "ufunc.h"
#include "ssopt.h"
#include "party.h"


void statrequest( Client *client, u32 serial)
{
    if (serial == client->chr->serial)
    {
        send_full_statmsg( client, client->chr );
    }
    else
    {
        Character *chr = find_character( serial );
        if (chr != NULL)
        {
			if (client->chr->is_visible_to_me(chr))
			{
				if (inrange( client->chr, chr ))
					send_short_statmsg( client, chr );
			}
        }
    }
}

// FIXME: KLUUUUDGE!!!
void send_skillmsg_caps( Client *client, const Character *chr ) {
	unsigned short msglen;

    static PKTBI_3A_CAPS msg;
	//Nando changed 3/17/09 (almost six years after!) to support caps in a kludge way (NOTICE: !! 9 !! bytes per skill!)
	// and the rest of the message is just 4 (instead of 6) bytes
    msglen = static_cast<unsigned short>(4 + (9*(uoclient_general.maxskills+1))); //0-based, only send skill structs up to configured maxskillid, reserve space for header, term, etc.

    msg.msgtype = PKTBI_3A_VALUES_ID;
    msg.msglen = ctBEu16( msglen );
	msg.unk3 = PKTBI_3A_VALUES::FULL_LIST_CAP; // Type = full_list + caps
    unsigned int i;
    for( i = 0; i <= uoclient_general.maxskills; i++ )
    {
        const UOSkill& uoskill = GetUOSkill(i);

        msg.skills[i].skillid = ctBEu16(i+1); // for some reason, we send this 1-based
        if (uoskill.pAttr)
        {
			const AttributeValue& av = chr->attribute(uoskill.pAttr->attrid);
            long value;
            
            value = av.effective_tenths();
            if (value > 0xFFFF) 
                value = 0xFFFF;
            msg.skills[i].value = ctBEu16( static_cast<u16>(value) );

            value = av.base();
            if (value > 0xFFFF)
                value = 0xFFFF;
            msg.skills[i].value_unmod = ctBEu16( static_cast<u16>(value) );
			msg.skills[i].lock_mode = (u8)(av.lock());// PKTBI_3A_VALUES::LOCK_NONE;
			msg.skills[i].cap = ctBEu16(static_cast<u16>(av.cap()));
        }
        else
        {
            msg.skills[i].value = 0;
            msg.skills[i].value_unmod = 0;
            msg.skills[i].lock_mode = PKTBI_3A_VALUES::LOCK_DOWN;
			msg.skills[i].cap = ctBEu16(ssopt.default_attribute_cap);
        }
    }
    client->transmit( &msg, msglen );
}
void send_skillmsg_no_caps( Client *client, const Character *chr ) {
	unsigned short msglen;

    static PKTBI_3A_VALUES msg;
    msglen = static_cast<unsigned short>(6 + (7*(uoclient_general.maxskills+1))); //0-based, only send skill structs up to configured maxskillid, reserve space for header, term, etc.

    msg.msgtype = PKTBI_3A_VALUES_ID;
    msg.msglen = ctBEu16( msglen );
    msg.unk3 = 0;
    unsigned int i;
    for( i = 0; i <= uoclient_general.maxskills; i++ )
    {
        const UOSkill& uoskill = GetUOSkill(i);

        msg.skills[i].skillid = ctBEu16(i+1); // for some reason, we send this 1-based
        if (uoskill.pAttr)
        {
            long value;
            
            value = chr->attribute(uoskill.pAttr->attrid).effective_tenths();
            if (value > 0xFFFF) 
                value = 0xFFFF;
            msg.skills[i].value = ctBEu16( static_cast<u16>(value) );

            value = chr->attribute(uoskill.pAttr->attrid).base();
            if (value > 0xFFFF)
                value = 0xFFFF;
            msg.skills[i].value_unmod = ctBEu16( static_cast<u16>(value) );
			msg.skills[i].lock_mode = (u8)(chr->attribute(uoskill.pAttr->attrid).lock());// PKTBI_3A_VALUES::LOCK_NONE;
        }
        else
        {
            msg.skills[i].value = 0;
            msg.skills[i].value_unmod = 0;
            msg.skills[i].lock_mode = PKTBI_3A_VALUES::LOCK_DOWN;
        }
    }
    msg.terminator = 0;
	u16* term1 = (u16*)((u8*)(&msg)+(msglen-1)); //this points to the last 2 bytes in the msg no matter the length, we want to set those to 0.
	*term1 = 0;
    client->transmit( &msg, msglen );
}
void send_skillmsg( Client *client, const Character *chr )
{
	// This should be a temporary kludge before 099...
	if (!ssopt.core_sends_caps)
		send_skillmsg_no_caps(client, chr);
	else
		send_skillmsg_caps(client, chr);
}

void handle_skill_lock( Client *client, PKTBI_3A_LOCKS *msg )
{
	if (ssopt.core_handled_locks) {
		unsigned int skillid = cfBEu16(msg->skillid);
		if (skillid > uoclient_general.maxskills)
			return;

		const UOSkill& uoskill = GetUOSkill(skillid);
		if (!uoskill.pAttr) // tried to set lockstate for a skill that isn't defined
			return;

		if (msg->lock_mode < 3)
			client->chr->attribute(uoskill.pAttr->attrid).lock(msg->lock_mode);
		else
			cout << "Client " + client->chr->name() + " tried to set an illegal lock state." << endl;
	}
}
MESSAGE_HANDLER( PKTBI_3A_LOCKS, handle_skill_lock );

void skillrequest( Client *client, u32 serial )
{
    if (serial == client->chr->serial)
    {
        ScriptDef sd;
        sd.quickconfig( "scripts/misc/skillwin.ecl" );
        if (sd.exists())
        {
            ref_ptr<EScriptProgram> prog;
            prog = find_script2( sd, 
                                 false, // complain if not found
                                 config.cache_interactive_scripts );
            if (prog.get() != NULL &&
                client->chr->start_script( prog.get(), false ))
            {
                return;
            }
        }
        send_skillmsg( client, client->chr );
    }
}

void srequest( Client *client, PKTIN_34 *msg )
{
    u32 serial = cfBEu32( msg->serial2 );
    
    if (msg->stattype == STATTYPE_STATWINDOW)
	{
		if ( client->chr->serial == serial )
			statrequest( client, serial );
		else
		{
			Character *bob = find_character( serial );
			if (bob == NULL)
				return;
			if ( !client->chr->is_concealed_from_me(bob) && client->chr->is_visible_to_me(bob) )
			{
				if ( pol_distance(client->chr->x, client->chr->y, bob->x, bob->y) < 20 )
					statrequest( client, serial );
			}
			if (client->chr->party()!=NULL)
				client->chr->party()->send_stat_to(client->chr,bob);
		}
	}
    else if (msg->stattype == STATTYPE_SKILLWINDOW)
        skillrequest( client, serial );
}
MESSAGE_HANDLER( PKTIN_34, srequest );
